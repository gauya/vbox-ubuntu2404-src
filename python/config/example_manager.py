import time
import logging
from config_manager import ConfigManager

# 로깅 설정
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

# 설정 변경 콜백 함수
def config_changed_callback():
    logging.info("설정이 변경되었습니다! 새로운 값:")
    logging.info(f"DB Host: {config.get('database', 'host')}")
    logging.info(f"Server Port: {config.get_int('server', 'port')}")

# ConfigManager 인스턴스 생성 (with 문 사용)
with ConfigManager('config.ini') as config:
    # 변경 콜백 등록
    config.register_callback(config_changed_callback)
    
    # 초기 설정 출력
    print("초기 데이터베이스 설정:", config.get_section('database'))
    print("초기 서버 설정:", config.get_section('server'))
    
    # 설정 값 수정 예제
    config.set('database', 'host', 'new.db.example.com')
    config.set('server', 'port', '9090')
    
    # 수동으로 설정 변경 테스트를 위해 대기
    print("설정 파일을 직접 수정하고 변경 사항을 관찰하세요...")
    print(f"감지 중인 파일: {config.config_path}")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("감지 종료")
