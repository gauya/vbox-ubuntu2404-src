# python_usage.py
import ctypes
import os

# .so 라이브러리 로드
lib_path = os.path.abspath("./my_math23_lib.so")
my_lib = ctypes.CDLL(lib_path)

# 함수 시그니처 정의
my_lib.c_power.argtypes = [ctypes.c_int, ctypes.c_int]
my_lib.c_power.restype = ctypes.c_double

my_lib.get_cci.restype = ctypes.c_int
my_lib.set_cci.argtypes = [ctypes.c_int]

# 함수 호출 및 변수 접근
result = my_lib.c_power(2, 3)
print(f"2의 3제곱 (Python): {result}")

current_cci = my_lib.get_cci()
print(f"cci 값 (Python): {current_cci}")

my_lib.set_cci(200)
current_cci = my_lib.get_cci()
print(f"cci 값 변경 후 (Python): {current_cci}")
