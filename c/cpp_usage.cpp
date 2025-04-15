// cpp_usage.cpp
#include <iostream>
#include <dlfcn.h>

// 함수 포인터 정의
typedef double (*power_func)(int, int);
typedef int (*get_cci_func)();
typedef void (*set_cci_func)(int);

int main() {
    void *handle;
    power_func power_ptr;
    get_cci_func get_cci_ptr;
    set_cci_func set_cci_ptr;
    char *error;

    // 라이브러리 로드
    handle = dlopen("./my_math_lib.so", RTLD_LAZY);
    if (!handle) {
        std::cerr << "Cannot open library: " << dlerror() << std::endl;
        return 1;
    }

    // power 함수 심볼 찾기
    power_ptr = (power_func)dlsym(handle, "c_power");
    if ((error = dlerror()) != nullptr) {
        std::cerr << "Cannot find symbol 'c_power': " << error << std::endl;
        dlclose(handle);
        return 1;
    }

    // get_cci 함수 심볼 찾기
    get_cci_ptr = (get_cci_func)dlsym(handle, "get_cci");
    if ((error = dlerror()) != nullptr) {
        std::cerr << "Cannot find symbol 'get_cci': " << error << std::endl;
        dlclose(handle);
        return 1;
    }

    // set_cci 함수 심볼 찾기
    set_cci_ptr = (set_cci_func)dlsym(handle, "set_cci");
    if ((error = dlerror()) != nullptr) {
        std::cerr << "Cannot find symbol 'set_cci': " << error << std::endl;
        dlclose(handle);
        return 1;
    }

    // 함수 호출 및 변수 접근
    double result = power_ptr(4, 2);
    std::cout << "4의 2제곱 (C++): " << result << std::endl;

    int current_cci = get_cci_ptr();
    std::cout << "cci 값 (C++): " << current_cci << std::endl;

    set_cci_ptr(400);
    current_cci = get_cci_ptr();
    std::cout << "cci 값 변경 후 (C++): " << current_cci << std::endl;

    // 라이브러리 언로드
    dlclose(handle);
    return 0;
}

// g++ cpp_usage.cpp -o cpp_usage -ldl
