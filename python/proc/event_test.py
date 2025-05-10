import threading
import time

class Worker(threading.Thread):
    def __init__(self, num, duration, completion_event):
        super().__init__()
        self.num = num
        self.duration = duration
        self.completion_event = completion_event

    def run(self):
        print(f"Worker {self.num} 시작")
        time.sleep(self.duration)
        print(f"Worker {self.num} 종료")
        self.completion_event.set()  # 작업 완료 이벤트 설정

# 이벤트 객체 생성
events = [threading.Event() for _ in range(3)]

# 스레드 생성
threads = [
    Worker(1, 3600, events[0]),
    Worker(2, 1, events[1]),
    Worker(3, 1, events[2])
]

# 스레드 시작
for t in threads:
    t.start()

# 각 스레드의 완료를 기다리고 필요한 자원 정리 (가상)
for i, t in enumerate(threads):
    print(f"Worker {i+1} join 대기...")
    events[i].wait()  # 해당 스레드의 완료 이벤트 대기
    print(f"Worker {i+1} 작업 완료. 관련 자원 정리.")
    # 여기에 자원 정리 코드 추가
    t.join() # 스레드 객체 join (선택 사항, 자원 정리가 목적이면 필수는 아님)

print("모든 스레드 작업 완료 및 자원 정리 완료")
