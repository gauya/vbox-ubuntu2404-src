from contextlib import contextmanager

@contextmanager
def open_file(name, mode):
    f = open(name, mode)
    try:
        yield f        # with 블록 내부에서 사용할 값
    finally:
        f.close()      # 블록이 끝나면 자동으로 실행

# 사용 예:
with open_file("2.py", "rb") as f:
    data = f.read()
    print(len(data))
