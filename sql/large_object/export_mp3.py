import psycopg2
import os

def export_mp3_from_db(mp3_id, output_dir):
    """
    DB에 BYTEA로 저장된 MP3를 파일로 출력
    :param mp3_id: 출력할 MP3의 ID
    :param output_dir: 출력 디렉토리 경로
    :return: 생성된 파일 경로
    """
    conn = psycopg2.connect("dbname=music_db user=postgres")
    
    try:
        with conn.cursor() as cur:
            # 메타데이터와 오디오 데이터 조회
            cur.execute("""
                SELECT title, artist, audio_data 
                FROM mp3_library 
                WHERE id = %s
            """, (mp3_id,))
            
            result = cur.fetchone()
            if not result:
                raise ValueError(f"MP3 with ID {mp3_id} not found")
            
            title, artist, audio_data = result
            
            # 안전한 파일명 생성
            safe_filename = f"{artist} - {title}.mp3".replace("/", "_")
            output_path = os.path.join(output_dir, safe_filename)
            
            # 파일 쓰기
            with open(output_path, 'wb') as f:
                f.write(audio_data)
            
            return output_path
            
    finally:
        conn.close()

# 사용 예제
exported_file = export_mp3_from_db(
    mp3_id="a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11",
    output_dir="/path/to/export"
)
print(f"Exported MP3: {exported_file}")

import shutil
import os

def export_hybrid_mp3(mp3_id, output_dir):
    """
    하이브리드 방식(파일 경로 저장)으로 저장된 MP3를 복사
    :param mp3_id: 출력할 MP3의 ID
    :param output_dir: 출력 디렉토리 경로
    :return: 생성된 파일 경로
    """
    conn = psycopg2.connect("dbname=music_db user=postgres")
    
    try:
        with conn.cursor() as cur:
            # 메타데이터와 원본 경로 조회
            cur.execute("""
                SELECT title, artist, file_path 
                FROM mp3_library 
                WHERE id = %s
            """, (mp3_id,))
            
            result = cur.fetchone()
            if not result:
                raise ValueError(f"MP3 with ID {mp3_id} not found")
            
            title, artist, source_path = result
            
            # 안전한 파일명 생성
            safe_filename = f"{artist} - {title}.mp3".replace("/", "_")
            output_path = os.path.join(output_dir, safe_filename)
            
            # 파일 복사
            shutil.copy2(source_path, output_path)
            
            return output_path
            
    finally:
        conn.close()

# 사용 예제
exported_file = export_hybrid_mp3(
    mp3_id="a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11",
    output_dir="/path/to/export"
)
print(f"Exported MP3: {exported_file}")
import psycopg2
from psycopg2 import sql
import os

def export_largeobject_mp3(oid, output_dir, metadata=None):
    """
    Large Object로 저장된 MP3를 파일로 출력
    :param oid: Large Object OID
    :param output_dir: 출력 디렉토리 경로
    :param metadata: 추가 메타데이터 (선택적)
    :return: 생성된 파일 경로
    """
    conn = psycopg2.connect("dbname=music_db user=postgres")
    
    try:
        with conn.cursor() as cur:
            # 메타데이터가 제공되지 않으면 DB에서 조회
            if not metadata:
                cur.execute("""
                    SELECT title, artist 
                    FROM mp3_library 
                    WHERE lo_oid = %s
                """, (oid,))
                metadata = cur.fetchone()
            
            title = "Unknown"
            artist = "Unknown"
            if metadata:
                title, artist = metadata
            
            # 안전한 파일명 생성
            safe_filename = f"{artist} - {title}.mp3".replace("/", "_")
            output_path = os.path.join(output_dir, safe_filename)
            
            # Large Object 내보내기
            cur.execute("SELECT lo_export(%s, %s)", (oid, output_path))
            
            return output_path
            
    finally:
        conn.close()

# 사용 예제
exported_file = export_largeobject_mp3(
    oid=12345,
    output_dir="/path/to/export",
    metadata=("My Song", "The Artist")  # 선택적: 메타데이터 직접 제공 가능
)
print(f"Exported MP3: {exported_file}")
