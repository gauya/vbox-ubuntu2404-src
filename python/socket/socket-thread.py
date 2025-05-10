import threading
import time
import socket

def worker():
    """간단한 소켓 연결을 맺고 끊는 스레드"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect(('localhost', 12345)) # 예시 주소
        print("스레드: 소켓 연결 성공")
        time.sleep(3)
    except Exception as e:
        print(f"스레드: 소켓 오류 발생: {e}")
    finally:
        sock.close()
        print("스레드: 소켓 닫음")

# 스레드 생성 및 시작
t = threading.Thread(target=worker)
t.start()

# 메인 스레드는 즉시 종료
print("메인 스레드 종료")

# t.join() # 이 부분을 주석 처리하면 자원 낭비 가능성 발생
