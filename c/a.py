# use_library.py
import ctypes
import os

# 공유 라이브러리 로드
lib_path = os.path.abspath("./a.so")
my_lib = ctypes.CDLL(lib_path)

# 함수 시그니처 정의 (인자 타입 및 반환 타입 명시)
my_lib.cpp_multiply_by_two.argtypes = [ctypes.c_int]
my_lib.cpp_multiply_by_two.restype = ctypes.c_int

my_lib.cpp_concatenate_strings.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
my_lib.cpp_concatenate_strings.restype = ctypes.c_char_p

# 함수 호출
result_multiply = my_lib.cpp_multiply_by_two(10)
print(f"Multiply by two result: {result_multiply}")

str1 = "Hello, "
str2 = "Python!"
result_concat_ptr = my_lib.cpp_concatenate_strings(str1.encode('utf-8'), str2.encode('utf-8'))
result_concat = ctypes.string_at(result_concat_ptr).decode('utf-8')
print(f"Concatenated string: {result_concat}")
