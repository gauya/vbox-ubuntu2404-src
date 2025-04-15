// my_library.cpp
#include <iostream>
#include <string>

// C++ 함수: 정수를 받아서 2배 값을 반환
int multiply_by_two(int num) {
    return num * 2;
}

// C++ 함수: 두 문자열을 연결하여 반환
std::string concatenate_strings(const std::string& str1, const std::string& str2) {
    return str1 + str2;
}

// C 스타일 인터페이스 (extern "C") - 필수
extern "C" {
    // 래핑 함수 1: multiply_by_two C 스타일로 노출
    int cpp_multiply_by_two(int num) {
        return multiply_by_two(num);
    }

    // 래핑 함수 2: concatenate_strings C 스타일로 노출
    const char* cpp_concatenate_strings(const char* str1, const char* str2) {
        std::string result = concatenate_strings(str1, str2);
        // 주의: 지역 변수의 포인터를 반환하면 안 됩니다.
        // 여기서는 간단하게 처리하기 위해 static 변수를 사용하지만,
        // 실제로는 메모리 관리에 더 주의해야 합니다.
        static std::string static_result;
        static_result = result;
        return static_result.c_str();
    }
}
