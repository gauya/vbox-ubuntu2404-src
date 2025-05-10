import psycopg2
from psycopg2 import sql
import os,dotenv

# 데이터베이스 연결 설정
def connect_db():
    try:
        conn = psycopg2.connect(dbname=os.getenv("DB_NAME"),user=os.getenv("DB_USER"))
        return conn
    except Exception as e:
        print(f"데이터베이스 연결 오류: {e}")
        return None

# 테이블 생성 함수
def create_table(conn):
    try:
        with conn.cursor() as cur:
            cur.execute("""
                CREATE TABLE IF NOT EXISTS tt (
                    name TEXT,
                    song BYTEA
                )
            """)
            conn.commit()
            print("테이블 생성/확인 완료")
    except Exception as e:
        print(f"테이블 생성 오류: {e}")
        conn.rollback()

# MP3 파일을 데이터베이스에 저장
def insert_mp3(conn, file_path, name):
    try:
        with open(file_path, 'rb') as f:
            mp3_data = f.read()
        
        with conn.cursor() as cur:
            cur.execute(
                "INSERT INTO tt (name, song) VALUES (%s, %s)",
                (name, mp3_data)
            )
            conn.commit()
            print(f"{file_path} 파일이 성공적으로 데이터베이스에 저장되었습니다.")
    except Exception as e:
        print(f"MP3 파일 저장 오류: {e}")
        conn.rollback()

"""
cur.execute(
    "SELECT * FROM table_name WHERE LOWER(column_name) LIKE LOWER(%s)",
    (f"%{search_term}%",)
)
"""

# MP3 파일을 데이터베이스에 저장
def update_mp3(conn, file_path, name):
    try:
        with open(file_path, 'rb') as f:
            mp3_data = f.read()
        
        with conn.cursor() as cur:
            cur.execute(
            "UPDATE tt SET name = name||'+', song = %s 
                WHERE name = %s RETURNING name",
            (mp3_data, name)
            )

            result = cur.fetchone()
            print(f"update name={result[0]}")
            conn.commit()
            print(f"{file_path} 파일이 성공적으로 데이터베이스에 저장되었습니다.")
    except Exception as e:
        print(f"MP3 파일 저장 오류: {e}")
        conn.rollback()

# 데이터베이스에서 MP3 파일 읽어서 저장
def retrieve_mp3(conn, name, output_path):
    try:
        with conn.cursor() as cur:
            cur.execute(
                "SELECT song FROM tt WHERE name = %s",
                (name,)
            )
            data = cur.fetchone()
            
            if data:
                with open(output_path, 'wb') as f:
                    f.write(data[0])
                print(f"{output_path} 파일로 성공적으로 저장되었습니다.")
            else:
                print("해당 이름의 MP3 파일을 찾을 수 없습니다.")
    except Exception as e:
        print(f"MP3 파일 읽기 오류: {e}")

# 메인 실행 함수
def main():
    # 데이터베이스 연결
    conn = connect_db()
    if not conn:
        return
    
    try:
        # 테이블 생성
        create_table(conn)
        
        # MP3 파일 경로
        input_file = '2.mp3'
        output_file = '2-1.mp3'
        song_name = 'sample_song'
        
        # MP3 파일이 존재하는지 확인
        if not os.path.exists(input_file):
            print(f"{input_file} 파일을 찾을 수 없습니다.")
            return
        
        # MP3 파일을 데이터베이스에 저장
        insert_mp3(conn, input_file, song_name)
        
        # 데이터베이스에서 MP3 파일 읽어서 저장
        retrieve_mp3(conn, song_name, output_file)

        update_mp3(conn,input_file, 'sample_song')
        
        # 결과 확인
        if os.path.exists(output_file):
            print("처리 완료. 원본 파일과 복구된 파일 크기 비교:")
            print(f"원본 파일 크기: {os.path.getsize(input_file)} bytes")
            print(f"복구 파일 크기: {os.path.getsize(output_file)} bytes")
    finally:
        # 데이터베이스 연결 종료
        if conn:
            conn.close()

if __name__ == "__main__":
    main()
