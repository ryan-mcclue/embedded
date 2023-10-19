drop domain if exists u32 cascade; 
create domain u32 as integer default 0 check (value >= 0);

drop domain if exists sha1 cascade; 
create domain sha1 as char(40) check (length(value) = 40);

drop type if exists build_enum cascade; 
create type build_enum as enum ('debug', 'release', 'test');

drop table if exists build_machines cascade;
create table build_machines (
  id serial primary key,
  os text not null,
  compiler text not null,
  linker text not null,
  target text not null,
  hal text not null,
);

-- TODO(Ryan): How to incorporate boot loader?
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
  build_time numeric(2, 4) not null,
  flash_time numeric(2, 4) not null,
  max_heap u32 not null,
  loc u32 not null,
  build_machine integer references build_machines(id),
  primary key (hash, build_type)
);

with build_id as (
  insert into build_machines values
    ('Ubuntu 20.04.6 LTS x86_64',
     'arm-none-eabi-gcc 9.2.1',
     'arm-none-eabi-ld 2.34',
     'armv7-m cortex-m4 stm32f429zitx',
     'STMicroelectronics 1.8.1')
   returning id
)
insert into build_metrics values 
  (now(), 
   'example',
   '5c794cd5e3e4edb9a2029424a7b35b508c7170a5',
   '4f123cd5e3e4edb9a2029424a7b35b508c7170a5',
   'debug',
   4096,
   1024,
   256,
   1.4,
   1.2,
   2 << 20,
   1200,
   (select id from build_id));
