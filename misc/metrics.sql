drop domain if exists u32 cascade; 
create domain u32 as integer default 0 check (value >= 0);

drop domain if exists sha1 cascade; 
create domain sha1 as char(40) check (length(value) = 40);

drop type if exists build_enum cascade; 
create type build_enum as enum ('debug', 'release', 'test');

drop table if exists metrics cascade;
create table metrics (
  created_at timestamp not null default now(),
  hash sha1 not null,
  parent_hash sha1 not null,
  build_type build_enum not null,
  text_size u32 not null,
  data_size u32 not null,
  bss_size u32 not null,
  primary key (hash, build_type)
);



insert into metrics values (
  ( ,"5c794cd5e3e4edb9a2029424a7b35b508c7170a5", 
