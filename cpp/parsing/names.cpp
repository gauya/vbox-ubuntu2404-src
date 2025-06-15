#include <stdio.h>
// main.cpp
#include <iostream>
#include <cstdio> // printf를 위해

// MyMath 네임스페이스를 포함하는 헤더 파일이라고 가정
// MyString 네임스페이스를 포함하는 헤더 파일이라고 가정

// 위에서 직접 정의한 예시를 위해
namespace MyMath {
    int add(int a, int b) {
        return a + b;
    }
}

namespace MyString {
    void print(const char* str) {
        printf("MyString::print: %s\n", str);
    }
}


int main() {
    // MyMath 네임스페이스의 add 함수 호출
    int sum = MyMath::add(5, 3);
    std::cout << "Sum: " << sum << std::endl; // 출력: Sum: 8

    // MyString 네임스페이스의 print 함수 호출
    MyString::print("Hello from MyString!"); // 출력: MyString::print: Hello from MyString!

    // 만약 다른 라이브러리에도 print 함수가 있다면
    // global_print("Hello from global!"); // 이처럼 충돌 없이 사용 가능

    return 0;
}

// 다른 곳에 있을 수 있는 전역 print 함수
void global_print(const char* str) {
    printf("Global print: %s\n", str);
}
