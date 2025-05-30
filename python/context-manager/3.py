import time
from contextlib import contextmanager

@contextmanager
def timer():
    start = time.time()
    yield
    end = time.time()
    print(f"⏱ 실행 시간: {end - start:.3f}초")

# 사용 예:
with timer():
    time.sleep(1.2)  # 일부러 대기

