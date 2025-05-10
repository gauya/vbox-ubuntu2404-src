-- create.pgsql


-- bytea inner mp3 data

-- CREATE EXTENSION IF NOT EXISTS "uuid-ossp";  -- uuid_generate_v4(), uuid_generate_v1()
-- CREATE EXTENSION IF NOT EXISTS pgcrypto; -- gen_random_uuid()

CREATE TABLE if not exists mp3_schema.songs (
    --id uuid PRIMARY KEY default gen_random_uuid(), --uuid_generate_v3(), uuid_generate_v1()
    id serial PRIMARY KEY,
    
    title text,
    artist text,
    album text,
    track int,
    album_artist text,
    composer text,

    genre text[],

    sample_rate int,
    duration interval,
    release_date date,

    description text,
    lyrics text,
    cover_image bytea, -- jpeg

    -- numberic == decimal, 
    -- real(4), double(8) 작은공간에 빠르지만 근사값

    --tempo numeric(3,2) check (tempo between 0.00 and 1.00), 
    --genre_2 numeric(3,2) check (genre_2 between 0.00 and 1.00), 
    --genre_3 numeric(3,2) check (genre_3 between 0.00 and 1.00), 
    
    song bytea,  -- 
    dtype varchar(5) default 'mp3' check ( dtype in ('mp3','wav','m4a','aac','ogg','flac','alac','m4a','aif','aiff')),
    dsize int check( dsize > 0 ),
    file_name text,

    likeno bigint,
    played bigint,
    evaluation float default 0.0, -- 0.0 - 1.0

    updated_at timestamp default CURRENT_TIMESTAMP,

    UNIQUE (title, artist, album)
) tablespace mp3_space;

-- Melody (멜로디), Harmony (화성), Rhythm (리듬), Instrumentation (악기), Dynamics (강약), Timbre (음색), Genre (장르), Mood (분위기), Lyrics (가사), Structure (구성), Texture (질감)

create index i_songs_title on mp3_schema.songs (title) tablespace mp3_space;
create index i_songs_artist on mp3_schema.songs (artist) tablespace mp3_space;
create index i_songs_album on mp3_schema.songs (album) tablespace mp3_space;
create index i_songs_release_date on mp3_schema.songs (release_date desc) tablespace mp3_space;
create index i_songs_file_name on mp3_schema.songs (file_name) tablespace mp3_space;
create index i_songs_composer on mp3_schema.songs (composer) tablespace mp3_space;

create view v_songs_artist_s (artist,cnt) as select artist,count(*) from songs group by artist order by count(*) desc;

-- date_part(), year,month,day,week,doy,dow
create view v_songs_release_date_y as select date_part('year',release_date) as y,count(*) as cnt from songs group by y order by y desc ;
--create view v_songs_release_date_y as select substring(release_date::date::varchar,0,5) as y,count(*) as cnt from songs group by y order by y desc ;

create view v_songs_artist_num as select count(*) from (select artist from songs group by artist);


insert into mp3_schema.songs (
    title,artist,album,track,album_artist,composer,genre[0],
    sample_rate,duration,release_date,description,lyrics,cover_image,
    dsize,file_name 
    ) select 
    title,artist,album,track,album_artist,composer,genre,
    sample_rate,duration,release_date,description,lyrics,cover_image,
    file_size,file_name
    from mp3_schema.mp3_library;
--    returning id
--    on conflict do nothing;


