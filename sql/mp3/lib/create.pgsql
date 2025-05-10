-- database 'gau'
-- MP3 managment
create tablespace mp3_space owner gau
    location '/db/mp3/db' ;
create schema if not exists mp3_schema ;
alter schema mp3_schema owner to gau;
grant usage on schema mp3_schema to gau;
alter database gau set search_path to public,mp3_schema;
alter role gau set search_path to mp3_schema,public;

-- SELECT schemaname, tablename FROM pg_tables WHERE tablename = 'mp3_library';

create table mp3_schema.artist_group (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name varchar(120) not null,
    members varchar(60)[],
    descript text
    ) tablespace mp3_space;
create index i_artist_group on mp3_schema.artist_group (name) tablespace mp3_space;

create table mp3_schema.artist (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name varchar(120),
    groupid UUID references mp3_schema.artist_group(id) 
        on delete set null 
        -- on update set null 
        default null, 
    descript text,
    updated_at timestamp default CURRENT_TIMESTAMP
    ) tablespace mp3_space;
create index i_artist on mp3_schema.artist (name) tablespace mp3_space;

-- 아티스트 이름을 한글로할지 영문으로 할지(Apink/A-PINK/에이핑크), 띄어쓰기는? 
-- 이렇게 다름에도 노래사이트에서는 가수를 잘 찾아준다. 어떻게?

CREATE TABLE if not exists mp3_schema.mp3_library (
    id SERIAL PRIMARY KEY, --id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    
    title varchar(120),
    artist varchar(120),
    composer varchar(120),

    genre varchar(60),
    -- numberic == decimal, 
    -- real(4), double(8) 작은공간에 빠르지만 근사값
    genre_1 numeric(3,2) check (genre_1 between 0.00 and 1.00), 
    genre_2 numeric(3,2) check (genre_2 between 0.00 and 1.00), 
    genre_3 numeric(3,2) check (genre_3 between 0.00 and 1.00), 

-- album
    album varchar(120),
    track INTEGER,
    release_date DATE,
    album_artist varchar(120),
    
    sample_rate INTEGER default 128,
    duration INTERVAL,
    description TEXT,
    lyrics TEXT,
    cover_image BYTEA, -- jpeg
    audio_data bytea, 

    file_name varchar(120) NOT NULL, 
    file_path varchar(120) NOT NULL, -- '/db/mp3/file'
    file_size BIGINT,

    updated_at timestamp default CURRENT_TIMESTAMP,

    UNIQUE (title, artist, album),
    CHECK (file_size > 0)
) tablespace mp3_space;

create index i_mp3_library_title on mp3_schema.mp3_library (title) tablespace mp3_space;
create index i_mp3_library_artist on mp3_schema.mp3_library (artist) tablespace mp3_space;
create index i_mp3_library_album on mp3_schema.mp3_library (album) tablespace mp3_space;
create index i_mp3_library_release_date on mp3_schema.mp3_library (release_date desc) tablespace mp3_space;
create index i_mp3_library_file_name on mp3_schema.mp3_library (file_name) tablespace mp3_space;
create index i_mp3_library_composer on mp3_schema.mp3_library (composer) tablespace mp3_space;

-- drop view v_artist_s, v_release_date_s cascade;
create view v_artist_s (artist,cnt) as select artist,count(*) from mp3_library group by artist order by count(*) desc;

create view v_release_date_s (day,cnt) as select release_date, count(*) from mp3_library group by release_date order by release_date desc;

create view v_artist_num as select count(*) from (select artist from mp3_library group by artist);

