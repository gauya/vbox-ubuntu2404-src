import psycopg2
from psycopg2 import sql

# DB 연결 설정
conn = psycopg2.connect(
    host="localhost",
    database="gau",
    user="gau",
    password="qjemf"  # 실제 비밀번호로 변경
)
cursor = conn.cursor()

# 1. 테이블 생성 (이미 존재하면 무시)
create_table_query = """
CREATE TABLE IF NOT EXISTS test_table (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100),
    age INT,
    score FLOAT
);
"""
cursor.execute(create_table_query)
conn.commit()

# 2. 기존 데이터 조회
cursor.execute("SELECT * FROM test_table")
rows = cursor.fetchall()
print("=== 기존 데이터 ===")
for row in rows:
    print(row)

# 3. 데이터 수정 (예: 모든 age +1, score * 1.1)
update_query = """
UPDATE test_table
SET age = age + 1, score = score * 1.1
WHERE id = %s;
"""

# 4. 새 데이터 추가 (예시)
insert_query = """
INSERT INTO test_table (name, age, score) VALUES (%s, %s, %s);
"""
new_data = [("Alice", 25, 88.5), ("Bob", 30, 92.0)]
cursor.executemany(insert_query, new_data)

# 5. 수정된 데이터 확인
cursor.execute("SELECT * FROM test_table ORDER BY id DESC LIMIT 5")
print("\n=== 수정/추가 후 데이터 ===")
for row in cursor.fetchall():
    print(row)

# 변경사항 저장 및 연결 종료
conn.commit()
cursor.close()
conn.close()
