from ctypes import cdll, c_int, c_char_p, Structure, POINTER

class DnaData(Structure):
    _fields_ = [("id", c_int), ("sequence", c_char_p)]

# 라이브러리 로드
lib = cdll.LoadLibrary("./libdna.so")

# 함수 타입 설정 (매우 중요!)
lib.combine_dna.argtypes = [c_int, c_char_p, c_int, c_char_p]
lib.combine_dna.restype = DnaData

lib.combine_dna_struct.argtypes = [DnaData, DnaData]
lib.combine_dna_struct.restype = DnaData


# 함수 호출 (문자열은 bytes로 변환해야 함)
result = lib.combine_dna(1, b"ATGC", 2, b"GCTA")
print(f"Combined ID: {result.id}, Sequence: {result.sequence.decode()}")

# 구조체 사용 예시
dna1 = DnaData(1, b"ATGC")
dna2 = DnaData(2, b"GCTA")
result2 = lib.combine_dna_struct(dna1, dna2)
print(f"Combined ID (struct): {result2.id}, Sequence: {result2.sequence.decode()}")
