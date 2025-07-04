// startup.c
extern int main();

void Reset_Handler(void) {
    extern unsigned int __bss_start__;
    extern unsigned int __bss_end__;
    unsigned int *src, *dst;

    // BSS 초기화
    for (dst = &__bss_start__; dst < &__bss_end__; dst++) {
        *dst = 0;
    }

    // main 함수 호출
    main();
    while (1); // 무한 루프
}

void Default_Handler(void) {
    while (1);
}

// 인터럽트 벡터 테이블
void (* const vectors[])(void) __attribute__((section(".vectors"))) = {
    (void (*)(void))0x20002000, // 초기 MSP 값 (RAM 상단)
    Reset_Handler,
    Default_Handler, // 나머지 인터럽트 핸들러
    // 다른 핸들러 추가 필요
};
