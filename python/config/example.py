from config_manager import ConfigManager

# 설정 관리자 인스턴스 생성
config = ConfigManager('config.ini')

# 데이터베이스 설정 조회
db_config = config.get_section('database')
print("Database Config:", db_config)

# 특정 값 조회
db_host = config.get('database', 'host', 'localhost')
print("DB Host:", db_host)

# 서버 포트 조회 (정수형으로)
server_port = config.get_int('server', 'port', 8000)
print("Server Port:", server_port)

# SSL 사용 여부 조회 (불리언형으로)
use_ssl = config.get_boolean('database', 'ssl', False)
print("Use SSL:", use_ssl)

# 설정 값 수정
config.set('database', 'host', 'new.db.server.com')
config.set('server', 'port', '9090')

# 수정 후 확인
print("Updated DB Host:", config.get('database', 'host'))
print("Updated Server Port:", config.get_int('server', 'port'))
