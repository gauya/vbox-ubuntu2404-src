import psycopg2
from psycopg2 import sql
import mutagen  # MP3 메타데이터 추출을 위한 라이브러리
import os
import shutil
from typing import Optional

"""
MP3 data manage
1. file -> DB
2. DB -> file

DB format
    1. large_object
    2. bytea
    3. hybrid( meta + file(inside DB system)

    if lo_oid 
    if file_path( inside db space - only postgres user )
    if audio_data (bytea)

gau=> \d mp3_library
                              Table "public.mp3_library"
    Column    |            Type             | Collation | Nullable |      Default      
--------------+-----------------------------+-----------+----------+-------------------
 id           | uuid                        |           | not null | gen_random_uuid()
 title        | character varying(255)      |           | not null | 
 artist       | character varying(255)      |           | not null | 
 composer     | character varying(255)      |           |          | 
 sample_rate  | integer                     |           |          | 
 file_size    | bigint                      |           |          | 
 duration     | interval                    |           | not null | 
 album        | character varying(255)      |           |          | 
 release_date | date                        |           |          | 
 genre1       | character varying(50)       |           |          | 
 genre2       | character varying(50)       |           |          | 
 file_path    | character varying(512)      |           | not null | 
 audio_data   | bytea                       |           |          | 
 created_at   | timestamp without time zone |           |          | CURRENT_TIMESTAMP
 updated_at   | timestamp without time zone |           |          | CURRENT_TIMESTAMP
Indexes:
    "mp3_library_pkey" PRIMARY KEY, btree (id)
    "idx_mp3_album" btree (album)
    "idx_mp3_artist" btree (artist)
    "idx_mp3_composer" btree (composer)
    "idx_mp3_genres" btree (genre1, genre2)
    "idx_mp3_search" gin (title gin_trgm_ops, artist gin_trgm_ops)
    "idx_mp3_title" btree (title)
    "mp3_library_file_path_key" UNIQUE CONSTRAINT, btree (file_path)
Check constraints:
    "mp3_library_file_size_check" CHECK (file_size > 0)
    "mp3_library_sample_rate_check" CHECK (sample_rate > 0)
Triggers:
    trg_mp3_update BEFORE UPDATE ON mp3_library FOR EACH ROW EXECUTE FUNCTION update_timestamp()

"""

CREATE_SCHEMA="""
CREATE TABLE if not exists mp3_library (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    title VARCHAR(255) NOT NULL,
    artist VARCHAR(255) NOT NULL,
    composer VARCHAR(255),
    sample_rate INT CHECK (sample_rate > 0),
    file_size BIGINT CHECK (file_size > 0),
    duration INTERVAL NOT NULL,
    album VARCHAR(255),
    release_date DATE,
    genre1 VARCHAR(50),
    genre2 VARCHAR(50),
    file_path VARCHAR(512) UNIQUE NOT NULL,
    audio_data BYTEA,  -- 선택적: 실제 오디오 데이터 저장
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 인덱스 생성
CREATE INDEX idx_mp3_title ON mp3_library(title);
CREATE INDEX idx_mp3_artist ON mp3_library(artist);
CREATE INDEX idx_mp3_composer ON mp3_library(composer);
CREATE INDEX idx_mp3_album ON mp3_library(album);
CREATE INDEX idx_mp3_genres ON mp3_library(genre1, genre2);
create unique index idx_mp3_3match on mp3_library (title,artist,sample_rate) where title is not null or artist is not null or sample_rate is not null;
"""

class MP3DatabaseManager:
    InnerPath = None
    TableName = None

    def __init__(self, dbname, user, password, host='localhost'):
        self.conn = psycopg2.connect(
            dbname=dbname, user=user, password=password, host=host
        )
        self.tbl = InnerPath
        self.inner_path = InnerPath

    def __del__(self):
        try:
            self.conn.close()
        except:
            pass

    def tablename(self, tablename=None):
        if tablename:
            self.tbl = tablename
        return self.tablename

    def inner_path(self, ipath=None):
        if ipath:
            self.inner_path = ipath
        return self.inner_path

# output
    def extract_metadata(self, filepath):
        """MP3 파일에서 메타데이터 추출"""
        audio = mutagen.File(filepath)
        return {
            'title': audio.get('TIT2', ['Unknown'])[0],
            'artist': audio.get('TPE1', ['Unknown'])[0],
            'composer': audio.get('TCOM', [None])[0],
            'sample_rate': audio.info.sample_rate,
            'file_size': os.path.getsize(filepath),
            'duration': audio.info.length,
            'album': audio.get('TALB', [None])[0],
            'release_date': audio.get('TDRC', [None])[0],
            'genre1': (audio.get('TCON', [None, None])[0],
            'genre2': (audio.get('TCON', [None, None])[1],
            'file_path': filepath
        }

# input
    def insert_mp3(self, filepath, store_audio=False):
        """MP3 파일을 데이터베이스에 등록"""
        metadata = self.extract_metadata(filepath)

        query = sql.SQL("""
            INSERT INTO mp3_library (
                title, artist, composer, sample_rate,
                file_size, duration, album, release_date,
                genre1, genre2, file_path
                {audio_field}
            ) VALUES (
                %(title)s, %(artist)s, %(composer)s, %(sample_rate)s,
                %(file_size)s, %(duration)s, %(album)s, %(release_date)s,
                %(genre1)s, %(genre2)s, %(file_path)s
                {audio_value}
            )
            RETURNING id
        """).format(
            audio_field=sql.SQL(", audio_data") if store_audio else sql.SQL(""),
            audio_value=sql.SQL(", %(audio_data)s") if store_audio else sql.SQL("")
        )

        params = metadata
        if store_audio:
            with open(filepath, 'rb') as f:
                params['audio_data'] = f.read()

        with self.conn.cursor() as cur:
            cur.execute(query, params)
            mp3_id = cur.fetchone()[0]
            self.conn.commit()

        return mp3_id

    def search(self, term, limit=100):
        """음원 검색"""
        with self.conn.cursor() as cur:
            cur.callproc('search_mp3', (term, limit))
            return cur.fetchall()

    # 파일 시스템에 저장 + DB에 메타데이터 (권장 방식)
    def insert_mp3_hybrid(self,filepath):
        metadata = extract_metadata(filepath)
        target_path = f"/media/mp3/{uuid.uuid4()}.mp3"
    
        # 트랜잭션 시작
        with conn:
            with conn.cursor() as cur:
                # 1. 메타데이터 먼저 저장
                cur.execute("""
                    INSERT INTO mp3_library
                    (title, artist, ..., file_path)
                    VALUES (%s, %s, ..., %s)
                    RETURNING id
                """, (metadata['title'], ..., target_path))
                mp3_id = cur.fetchone()[0]
    
                # 2. 파일 복사 (실패 시 트랜잭션 롤백)
                shutil.copy2(filepath, target_path)
    
        return mp3_id


    def export_mp3(self, mp3_id: str, output_dir: str, 
                  new_filename: Optional[str] = None) -> str:
        """
        모든 저장 방식 지원 통합 MP3 내보내기
        """
        with self.conn.cursor() as cur:
            # 저장 방식 확인
            cur.execute("""
                SELECT title, artist, file_path, lo_oid, audio_data IS NOT NULL as has_audio_data
                FROM %s
                WHERE id = %s
            """, (self.tbl, mp3_id,))
            
            result = cur.fetchone()
            if not result:
                raise ValueError(f"MP3 with ID {mp3_id} not found")
            
            title, artist, file_path, lo_oid, has_audio_data = result
            
            # 출력 파일명 결정
            filename = new_filename if new_filename else f"{artist} - {title}.mp3"
            safe_filename = self._sanitize_filename(filename)
            output_path = os.path.join(output_dir, safe_filename)
            
            # 저장 방식에 따라 처리
            if lo_oid:
                self._export_largeobject(lo_oid, output_path)
            elif file_path:
                self._copy_file(file_path, output_path)
            elif has_audio_data:
                self._export_bytea(mp3_id, output_path)
            else:
                raise ValueError("No audio data available for export")
            
            return output_path
    
    def _export_largeobject(self, oid: int, output_path: str):
        """Large Object 내보내기"""
        with self.conn.cursor() as cur:
            cur.execute("SELECT lo_export(%s, %s)", (oid, output_path))
    
    def _copy_file(self, source_path: str, dest_path: str):
        """파일 시스템에서 복사"""
        shutil.copy2(source_path, dest_path)
    
    def _export_bytea(self, mp3_id: str, output_path: str):
        """BYTEA 데이터 내보내기"""
        with self.conn.cursor() as cur:
            cur.execute("SELECT audio_data FROM %s WHERE id = %s", ( self.tbl, mp3_id,))
            audio_data = cur.fetchone()[0]
            with open(output_path, 'wb') as f:
                f.write(audio_data)
    
    def _sanitize_filename(self, filename: str) -> str:
        """안전한 파일명 생성"""
        return "".join(c for c in filename if c.isalnum() or c in " .-_").rstrip()

    def print(self):
        print("MP3exporter")

# 사용 예제

def manage_example():
    manager = MP3DatabaseManager("music_db", "user", "password")

    # MP3 파일 등록 (메타데이터만 저장)
    song_id = manager.insert_mp3("/path/to/song.mp3")
    print(f"등록된 음원 ID: {song_id}")

    # 검색
    results = manager.search("love")
    for song in results:
        print(f"{song[1]} - {song[2]} ({song[7]})")


def export_example():
    try:
        # Large Object로 저장된 경우
        path1 = exporter.export_mp3(
            mp3_id="a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11",
            output_dir="~/Downloads"
        )
        
        # BYTEA로 저장된 경우
        path2 = exporter.export_mp3(
            mp3_id="b1ffc99-9c0b-4ef8-bb6d-6bb9bd380a12",
            output_dir="~/Downloads",
            new_filename="custom_name.mp3"
        )
        
        print(f"Exported files: {path1}, {path2}")
    finally:
        pass

def example(mp3):
    mp3.print()

if __name__ == "__main__":
    conn = psycopg2.connect("dbname=gau user=postgres password=qjemf")
    mp3 = MP3Exporter(conn, 'mp3_library')
    
    example(mp3)
    conn.close()
