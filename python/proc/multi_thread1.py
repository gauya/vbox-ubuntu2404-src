import threading
import time

def worker(num):
    """스레드에서 실행할 함수"""
    print(f"Worker {num} 시작")
    time.sleep(2)  # 2초간 대기 (I/O 작업 시뮬레이션)
    print(f"Worker {num} 종료")

# 스레드 생성
threads = []
for i in range(3):
    t = threading.Thread(target=worker, args=(i,))
    threads.append(t)
    t.start()  # 스레드 시작

# 모든 스레드가 종료될 때까지 기다림
time.sleep(1)
print("스레드 join 작업")
cnt=0
for t in threads:
	print(cnt)
	cnt = cnt+1
	t.join()

print("모든 스레드 작업 완료")
