import psycopg2

# dotenv
def test1():
    """
# .env (파일명은 반드시 .env)
DB_HOST=your_host
DB_PORT=5432
DB_NAME=your_db
DB_USER=your_username
DB_PASSWORD=your_password
"""
    #pip install python-dotenv
    import os,dotenv
    
    # .env 파일 로드
    dotenv.load_dotenv()
    
    # 환경 변수에서 값 읽기
    conn = psycopg2.connect(
        host=os.getenv("DB_HOST"),
        port=os.getenv("DB_PORT"),
        dbname=os.getenv("DB_NAME"),
        user=os.getenv("DB_USER"),
        password=os.getenv("DB_PASSWORD")
    )
    
    print("DB 연결 성공!")
    conn.close()

# json
def test2():
    """
{
  "database": {
    "host": "your_host",
    "port": 5432,
    "name": "your_db",
    "user": "your_username",
    "password": "your_password"
  }
}
"""
    # config.json 파일 읽기
    import json
    with open('.config.json', 'r') as f:
        config = json.load(f)
    
    db_config = config["database"]
    
    conn = psycopg2.connect(
        host=db_config["host"],
        port=db_config["port"],
        dbname=db_config["name"],
        user=db_config["user"],
        password=db_config["password"]
    )
    
    print("DB 연결 성공!")
    conn.close()


# ini 
def test3():
    """
[database]
host = your_host
port = 5432
name = your_db
user = your_username
password = your_password
"""
    import configparser
    
    # config.ini 파일 읽기
    config = configparser.ConfigParser()
    config.read('.config.ini')
    
    db_config = config["database"]
    
    conn = psycopg2.connect(
        host=db_config["host"],
        port=db_config["port"],
        dbname=db_config["name"],
        user=db_config["user"],
        password=db_config["password"]
    )
    
    print("DB 연결 성공!")
    conn.close()
    

if __name__ == '__main__':
    test1()
    test2()
    test3()
