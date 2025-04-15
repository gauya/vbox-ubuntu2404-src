// my_math_lib.cpp
#include <iostream>
#include <cmath>

// 전역 변수
int cci = 100;

// a의 b제곱을 계산하는 C++ 함수
double power(int a, int b) {
    return std::pow(static_cast<double>(a), static_cast<double>(b));
}

// C 스타일 인터페이스 (extern "C")
extern "C" {
    // 전역 변수 cci에 접근하는 함수 (C와 Python에서 사용)
    int get_cci() {
        return cci;
    }

    // 전역 변수 cci를 설정하는 함수 (C와 Python에서 사용)
    void set_cci(int new_cci) {
        cci = new_cci;
    }

    // power 함수를 C 스타일로 래핑 (C와 Python에서 사용)
    double c_power(int a, int b) {
        return power(a, b);
    }
}
// g++ -std=c++11 -fPIC -shared my_math_lib.cpp -o my_math_lib.so
