import psycopg2
import pandas as pd
from io import StringIO

# CSV 파일 경로 (예: data.csv)
csv_file = "data.csv"  # name,age,score 형식의 CSV 파일

# PostgreSQL 연결
conn = psycopg2.connect(
    host="localhost",
    database="gau",
    user="gau",
    password="qjemf"
)
cursor = conn.cursor()

# 1. 테이블 생성 (없는 경우)
create_table_query = """
CREATE TABLE IF NOT EXISTS test_table (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100),
    age INT,
    score FLOAT
);
"""
cursor.execute(create_table_query)

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

# 6. CSV → DataFrame → PostgreSQL 일괄 삽입
df = pd.read_csv(csv_file)

# StringIO를 이용한 고속 삽입 (COPY 사용)
buffer = StringIO()
df.to_csv(buffer, index=False, header=False) # header = True
buffer.seek(0)

try:
    cursor.copy_expert(
        "COPY test_table (name, age, score) FROM STDIN WITH CSV header",
        buffer
    )
    conn.commit()
    print(f"{len(df)}행 삽입 성공!")
except Exception as e:
    conn.rollback()
    print("오류 발생:", e)
finally:
    cursor.close()
    conn.close()

# sudo chown postgres:postgres data.csv
# sudo chmod 644 data.csv
# psql -h localhost -U gau -d gau -c "\COPY test_table(name, age, score) FROM 'data.csv' DELIMITER ',' CSV HEADER"

