import psycopg2
import sys

def save_mp3_to_db(file_path, song_name):
    # PostgreSQL 연결 설정
    conn = psycopg2.connect(
        host="localhost",
        database="gau",
        user="gau",
        password="qjemf"
    )
    
    try:
        # 파일을 바이너리 모드로 읽기
        with open(file_path, 'rb') as file:
            audio_data = file.read()
        
        # 커서 생성
        cur = conn.cursor()
        
        # SQL 쿼리 실행
        cur.execute(
            "INSERT INTO public.songs (audio_data, name) VALUES (%s, %s)",
            (psycopg2.Binary(audio_data), song_name)
            )
        
        # 변경사항 커밋
        conn.commit()
        print(f"성공적으로 저장되었습니다: {song_name}")
        
    except Exception as e:
        print(f"오류 발생: {e}")
        conn.rollback()
        
    finally:
        # 연결 종료
        if conn is not None:
            conn.close()

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("사용법: python save_mp3.py <mp3_file_path> <song_name>")
        sys.exit(1)
    
    file_path = sys.argv[1]
    song_name = sys.argv[2]
    
    save_mp3_to_db(file_path, song_name)
