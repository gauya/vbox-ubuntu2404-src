// c_usage.c
#include <stdio.h>
#include <stdlib.h>
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
        fprintf(stderr, "Cannot open library: %s\n", dlerror());
        return 1;
    }

    // power 함수 심볼 찾기
    power_ptr = (power_func)dlsym(handle, "c_power");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Cannot find symbol 'c_power': %s\n", error);
        dlclose(handle);
        return 1;
    }

    // get_cci 함수 심볼 찾기
    get_cci_ptr = (get_cci_func)dlsym(handle, "get_cci");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Cannot find symbol 'get_cci': %s\n", error);
        dlclose(handle);
        return 1;
    }

    // set_cci 함수 심볼 찾기
    set_cci_ptr = (set_cci_func)dlsym(handle, "set_cci");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Cannot find symbol 'set_cci': %s\n", error);
        dlclose(handle);
        return 1;
    }

    // 함수 호출 및 변수 접근
    double result = power_ptr(3, 2);
    printf("3의 2제곱 (C): %f\n", result);

    int current_cci = get_cci_ptr();
    printf("cci 값 (C): %d\n", current_cci);

    set_cci_ptr(300);
    current_cci = get_cci_ptr();
    printf("cci 값 변경 후 (C): %d\n", current_cci);

    // 라이브러리 언로드
    dlclose(handle);
    return 0;
}
// gcc c_usage.c -o c_usage -ldl
