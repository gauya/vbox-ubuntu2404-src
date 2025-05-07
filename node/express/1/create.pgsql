create table public.songs (
    id  serial,
    dtype   varchar(20) default 'mp3',
    name varchar(120),
    audio_data  bytea
);



create view v_songs_sel as select id,name,dtype,length(audio_data) from songs;


