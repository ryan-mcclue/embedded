# vim: set ft=bash :

shopt -s extglob
CPU_INFO=$(lscpu)
parse_cpu_info() {
  local field_name="$1"
  local variable_name="$2"

  field_value=$(echo "$CPU_INFO" | grep "$field_name" | cut -d ':' -f2)
  trimmed_value=${field_value##+([[:space:]])}

  eval "$variable_name=\"$trimmed_value\""
}

insert_metrics_db() {
  parse_cpu_info "Architecture" "ARCH"
  parse_cpu_info "Model name" "MICROARCH"
  parse_cpu_info "CPU(s)" "CORES"
  parse_cpu_info "CPU max MHz" "FREQUENCY"
  parse_cpu_info "Flags" "FLAGS"
  SIMD=$(echo "$FLAGS" | grep -o '\bavx\w*\b\|\bsse\w*\b')
  OS_RAW=$(lsb_release --description)
  OS=${os_raw##+[[:space:]]}

  local build_machine_str="${ARCH}${MICROARCH}${CORES}${FREQUENCY}${FLAGS}${SIMD}${OS}"
  BUILD_MACHINE_HASH=$(echo -n "$build_machine_str" | sha256sum | awk '{print $1}')

  local build_machine_str=""
  read -r -d '' build_machine_str <<- EOV
    insert into build_machines values 
    (\"$BUILD_MACHINE_HASH\",
     \"$OS\",
     \"$ARCH, $MICROARCH@$FREQUENCY ($CORES cores)\", 
     \"$COMPILER\", 
     \"$LINKER\")
  EOV

  # NOTE(Ryan): Will error on duplicate build machine
  sqlite3 "$NAME.db" < "$build_machine_str"

  local top10_symbols=$(nm -C --print-size --size-sort --reverse-sort --radix=d build/$BINARY_NAME | head -10)
  local top10_symbols_str=""
  while IFS= read -r line; do
    read -r addr size type name <<< $(echo "$line")
    top10_symbols_str+="\"${name}\",${size},"
  done <<< "$top10_symbols"

  local sizes=$(size build/$BINARY_NAME | tail -1)
  local text_size=$(sizes | cut -d ' ' -f1)
  local data_size=$(sizes | cut -d ' ' -f2)
  local bss_size=$(sizes | cut -d ' ' -f3)
  local child_hash=$(git rev-parse HEAD)
  local parent_hash=$(git rev-parse HEAD^@)

  local metrics_str=""
  read -r -d '' metrics_str <<- EOV
    with top10_id as (
      insert into top10_symbols values
      (default,
       $top10_symbols_str)
     returning id)
    insert into build_metrics values 
      (default, 
       \"$NAME\",
       \"$PARENT_HASH\"
       \"$CHILD_HASH\",
       \"$BUILD_TYPE\",
       $TEXT_SIZE,
       $DATA_SIZE,
       $BSS_SIZE,
       $BUILD_TIME,
       $LOC,
       $BUILD_MACHINE_HASH,
       (select id from top10_id)
       );
  EOV

  sqlite3 "$NAME.db" < "$metrics_str"
}


if [[ "$PROJECT_TYPE" == "embedded" ]]; then
  read -r -d '' build_machine_specific <<- EOV
    target text not null,
    hal text not null,
  EOV
  read -r -d '' build_metrics_specific <<- EOV
    flash_time real not null,
    max_heap u32 not null,
    remaining_flash real not null,
    remaining_ram real not null,
  EOV
  printf "Creating embedded database"
else
  printf "Creating desktop database"
fi

read -r -d '' DATABASE_SQL <<- EOF
-- NOTE(Ryan): TYPES/DOMAINS
drop domain if exists u32 cascade; 
create domain u32 as integer default 0 check (value >= 0);

drop domain if exists sha1 cascade; 
create domain sha1 as char(40) check (length(value) = 40);

drop type if exists build_enum cascade; 
create type build_enum as enum ('debug', 'release', 'test');

drop type if exists symbol cascade; 
create type symbol as (
  name text, 
  sz u32
);

-- NOTE(Ryan): TABLES
-- TODO(Ryan): could add more details, e.g. core count, ram, sse etc.
drop table if exists build_machines cascade;
create table build_machines (
  id serial primary key,
  os text not null,
  compiler text not null,
  linker text not null,
  $build_machine_specific
);

-- probably want to put flash size in here to allow for pie-chart visualisation
drop table if exists top10_symbols cascade;
create table top10_symbols (
  id serial primary key,
  symbols symbol[] check (array_length(symbols, 1) = 10)
);

-- TODO(Ryan): Incorporate boot loader, i.e. track more 'sections', e.g. bootloader size, main size, etc.
drop table if exists build_metrics cascade;
create table build_metrics (
  created_at timestamp not null default now(),
  name text not null,
  hash sha1 not null,
  parent_hash sha1 not null,
  build_type build_enum not null,
  text_size u32 not null,
  data_size u32 not null,
  bss_size u32 not null,
  build_time real not null,
  loc u32 not null,
  $build_metrics_specific
  build_machine integer references build_machines(id),
  top10_symbol integer references top10_symbols(id),
  constraint hash_different check (parent_hash != hash),
  primary key (hash, build_type)
);

-- NOTE(Ryan): Use graph visualiser in pgadmin query tool
-- VIEWS/FUNCTIONS --
-- IMPORTANT(Ryan): Requires type at call-time, e.g: `select * from delta(NULL::u32, 'text_size');`
drop function if exists delta cascade;
create or replace function delta(attr_type anyelement, attr text) returns table (child anyelement, parent anyelement)
as $$
declare 
  column_exists integer;
  query_str text;
begin
  query_str := 'select 1 from information_schema.columns where table_name = ''build_metrics'' AND column_name = ' || quote_literal(attr); 
  execute query_str into column_exists;
  if column_exists is not null then
    query_str := 'select child.' || quote_ident(attr) || ', ' ||
                 'parent.' || quote_ident(attr) || ' as parent_' || quote_ident(attr) || 
                 ' from build_metrics child ' ||
                 'join build_metrics parent on child.parent_hash = parent.hash ' ||
                 'order by child.created_at desc';
    return query execute query_str;
  else
    raise exception 'Column % does not exist', attr;
  end if;
end;
$$
language plpgsql;
EOF
