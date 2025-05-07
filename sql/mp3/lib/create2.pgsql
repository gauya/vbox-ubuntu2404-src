-- create.pgsql


-- bytea inner mp3 data

-- CREATE EXTENSION IF NOT EXISTS "uuid-ossp";  -- uuid_generate_v4(), uuid_generate_v1()
-- CREATE EXTENSION IF NOT EXISTS pgcrypto; -- gen_random_uuid()

CREATE TABLE if not exists mp3_schema.songs (
    --id uuid PRIMARY KEY default gen_random_uuid(), --uuid_generate_v3(), uuid_generate_v1()
    id serial PRIMARY KEY,
    
    title varchar(120),
    artist varchar(120),
    composer varchar(120),

    song bytea,  -- 
    dtype varchar(5) default 'mp3' check ( dtype in ('mp3','wav','m4a','aac','ogg','flac','alac','m4a','aif','aiff')),
    dsize int check( dsize > 0 ),

    sample_rate int,
    duration interval,

-- album
    album varchar(120),
    track int,
    release_date date,
    album_artist varchar(120),
    description text,
    lyrics text,
    cover_image bytea, -- jpeg

    genre varchar(60)[],
    -- numberic == decimal, 
    -- real(4), double(8) 작은공간에 빠르지만 근사값

    --tempo numeric(3,2) check (tempo between 0.00 and 1.00), 
    --genre_2 numeric(3,2) check (genre_2 between 0.00 and 1.00), 
    --genre_3 numeric(3,2) check (genre_3 between 0.00 and 1.00), 
    
    likeno bigint,
    played bigint,
    evaluation float default 0.0, -- 0.0 - 1.0

    updated_at timestamp default CURRENT_TIMESTAMP,

    UNIQUE (title, artist, album)
) tablespace mp3_space;

-- Melody (멜로디), Harmony (화성), Rhythm (리듬), Instrumentation (악기), Dynamics (강약), Timbre (음색), Genre (장르), Mood (분위기), Lyrics (가사), Structure (구성), Texture (질감)


