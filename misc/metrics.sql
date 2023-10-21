-- TYPES/DOMAINS --
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

-- TABLES --
drop table if exists build_machines cascade;
create table build_machines (
  id serial primary key,
  os text not null,
  compiler text not null,
  linker text not null,
  target text not null,
  hal text not null
);

-- probably want to put flash size in here to allow for pie-chart visualisation
drop table if exists top10_symbols cascade;
create table top10_symbols (
  id serial primary key,
  symbols symbol[]
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
  remaining_flash real not null,
  build_time real not null,
  flash_time real not null,
  max_heap u32 not null,
  remaining_ram real not null,
  loc u32 not null,
  build_machine integer references build_machines(id),
  top10_symbol integer references top10_symbols(id),
  primary key (hash, build_type)
);


-- DEMO INSERTIONS --
with build_id as (
  insert into build_machines values
    (default,
     'Ubuntu 20.04.6 LTS x86_64',
     'arm-none-eabi-gcc 9.2.1',
     'arm-none-eabi-ld 2.34',
     'armv7-m cortex-m4 stm32f429zitx',
     'STMicroelectronics 1.8.1')
   returning id
),
top10_id as (
  insert into top10_symbols values
    (default,
      array[
        row('symbol0_name', 0)::symbol,
        row('symbol1_name', 1)::symbol,
        row('symbol2_name', 2)::symbol,
        row('symbol3_name', 3)::symbol,
        row('symbol4_name', 4)::symbol,
        row('symbol5_name', 5)::symbol,
        row('symbol6_name', 6)::symbol,
        row('symbol7_name', 7)::symbol,
        row('symbol8_name', 8)::symbol,
        row('symbol9_name', 9)::symbol
    ]::symbol[])
   returning id
)
insert into build_metrics values 
  (default, 
   'example',
   '5c794cd5e3e4edb9a2029424a7b35b508c7170a5',
   '4f123cd5e3e4edb9a2029424a7b35b508c7170a5',
   'debug',
   4096,
   1024,
   256,
   30.5,
   1.4,
   1.2,
   2 << 20,
   20.8,
   1200,
   (select id from build_id),
   (select id from top10_id)),
  (default, 
   'example',
   '18273825e3e4edb9a2029424a7b35b508c7170a5',
   '5c794cd5e3e4edb9a2029424a7b35b508c7170a5',
   'debug',
   2*4096,
   4*1024,
   256/2,
   20.5,
   1.4,
   1.2,
   2 << 20,
   20.8,
   1200,
   (select id from build_id),
   (select id from top10_id)),
  (default, 
   'example',
   '12938213e3e4edb9a2029424a7b35b508c7170a5',
   '18273825e3e4edb9a2029424a7b35b508c7170a5',
   'debug',
   8*4096,
   2*1024,
   256,
   10.5,
   1.4,
   1.2,
   2 << 20,
   20.8,
   1200,
   (select id from build_id),
   (select id from top10_id));
;

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
