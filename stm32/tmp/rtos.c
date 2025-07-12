#include "stm32f4xx.h" // 또는 stm32f3xx.h (사용하는 MCU에 맞게 변경하세요)

// FPU 지원 자동 감지 (Cortex-M4/M7/H7은 FPU가 있을 수 있음)
#if defined(__FPU_PRESENT) && (__FPU_PRESENT == 1)
    #define USE_FPU 1
#else
    #define USE_FPU 0
#endif

// 예시: 2개의 태스크를 가정합니다.
#define MAX_TASK 8
#define STACK_SIZE 256 // 스택 크기는 워드(4바이트) 단위 (uint32_t = 4바이트)
                       // STACK_SIZE * 4 (바이트)는 8의 배수여야 합니다. (256 * 4 = 1024, 8의 배수)

// 각 태스크 스택의 시작 주소가 8바이트 정렬되도록 선언
// GCC/Clang 컴파일러 사용 시:
uint32_t task_stacks[MAX_TASK][STACK_SIZE] __attribute__((aligned(8)));
// Keil MDK (ARM Compiler) 사용 시:
// __align(8) uint32_t task_stacks[MAX_TASK][STACK_SIZE];


// 태스크 컨트롤 블록 (TCB) 정의
// 각 태스크의 상태 정보를 저장합니다.
typedef struct {
    uint8_t  type;
    uint8_t  id;
    uint8_t  priority;          // 0~7 (TASK_OPPORTUNISTIC 용)
    uint8_t  status;            //
    uint32_t period;            // TASK_PERIODIC 용( > 0 ) ms
    uint32_t period_tick;
    void (*func)();             // 실행 함수 포인터
    uint32_t *stack_base;       // 태스크의 스택 시작 주소 (가장 낮은 주소, _stack[i]의 시작)
    uint32_t stack_size_words;  // 태스크 스택의 크기 (워드 단위)
    uint32_t psp;               // 태스크의 Process Stack Pointer (컨텍스트 스위칭 시 사용)

    uint32_t delay;             // > 0이면 SysTick에서 --
    uint32_t reserved[8];       // 32 byte (8 * 4 bytes)
} TCB;

// 태스크들을 관리할 TCB 배열 (예시)
// 실제 RTOS에서는 동적으로 할당하거나 리스트로 관리합니다.
TCB tasks[MAX_TASK];

// 현재 실행 중인 태스크의 인덱스 또는 포인터
// 여기서는 TCB 포인터를 직접 저장하는 방식으로 변경하여 어셈블리 접근을 단순화합니다.
TCB* current_task_tcb;
TCB* next_task_tcb;


// 태스크 함수 (무한 루프)
void task1_function(void) {
    // Task 1에서 할 일
    // 예: GPIOD->ODR ^= GPIO_PIN_12; (STM32F4 Discovery의 녹색 LED)
    // 실제 보드에 맞게 GPIO 핀 설정을 추가해야 합니다.
    volatile uint32_t counter = 0;
    while (1) {
        counter++;
        // 더미 지연
        for (volatile int i = 0; i < 1000000; i++);
    }
}

void task2_function(void) {
    // Task 2에서 할 일
    // 예: GPIOD->ODR ^= GPIO_PIN_13; (STM32F4 Discovery의 주황색 LED)
    // 실제 보드에 맞게 GPIO 핀 설정을 추가해야 합니다.
    volatile uint32_t counter = 0;
    while (1) {
        counter++;
        // 더미 지연
        for (volatile int i = 0; i < 1000000; i++);
    }
}

// 태스크 스택 초기화 함수 (최초 실행 시)
// stack_top: 해당 태스크 스택의 최상단 주소 (가장 높은 주소)
// task_func: 태스크 함수의 시작 주소
uint32_t* os_initialize_task_stack(uint32_t* stack_top, void (*task_func)(void)) {
    // ARM Cortex-M 인터럽트 스택 프레임 구조를 모방합니다.
    // 인터럽트에서 복귀할 때 pop되는 순서대로 채워넣습니다.
    // (High Address)
    // XPSR (PSR with Thumb bit set)
    // PC (Task Entry Point)
    // LR (EXC_RETURN value)
    // R12
    // R3
    // R2
    // R1
    // R0
    // (Low Address)

    // 스택 포인터를 4바이트씩 감소시키면서 레지스터 값들을 푸시합니다.
    // XPSR (Program Status Register) - Thumb 비트(bit 24)를 1로 설정 (0x01000000)
    *(--stack_top) = (uint32_t)0x01000000;
    // PC (Program Counter) - 태스크 함수의 시작 주소
    *(--stack_top) = (uint32_t)task_func;

    // LR (Link Register) - EXC_RETURN 값 설정
    // 이 값은 BX LR 명령으로 인터럽트에서 복귀할 때 하드웨어가 스택 복원 방식을 결정하는 데 사용됩니다.
    // 0xFFFFFFFD: 스레드 모드, PSP 사용, FPU 컨텍스트 없음
    // 0xFFFFFFED: 스레드 모드, PSP 사용, FPU 컨텍스트 있음 (Lazy Stacking)
    #if USE_FPU == 1
        *(--stack_top) = (uint32_t)0xFFFFFFED; // EXC_RETURN for FPU context (Lazy Stacking)
    #else
        *(--stack_top) = (uint32_t)0xFFFFFFFD; // EXC_RETURN for no FPU context
    #endif

    // R12, R3, R2, R1, R0 (일반 목적 레지스터) - 초기 값
    *(--stack_top) = (uint32_t)0x12121212; // R12
    *(--stack_top) = (uint32_t)0x03030303; // R3
    *(--stack_top) = (uint32_t)0x02020202; // R2
    *(--stack_top) = (uint32_t)0x01010101; // R1
    *(--stack_top) = (uint32_t)0x00000000; // R0

    // 나머지 R4-R11은 소프트웨어에 의해 저장/복원됩니다.
    // 이들은 처음에는 0으로 초기화되거나 임의의 값으로 채워질 수 있습니다.
    for (int i = 0; i < 8; i++) { // R4-R11
        *(--stack_top) = (uint32_t)0x04040404 + i; // 더미 값 (R4, R5, ...)
    }

    // FPU 레지스터 (S0-S31, FPSCR)는 Lazy Stacking을 사용할 경우 하드웨어가 자동으로 처리하므로
    // os_initialize_task_stack에서는 명시적으로 푸시하지 않습니다.

    return stack_top; // 초기화된 스택 포인터 반환 (이제 이 태스크의 SP로 사용될 주소)
}

// OS 초기화 함수
void os_init(void) {
    // TCB 구조체 초기화
    // task_stacks 배열 사용
    tasks[0].id = 0;
    tasks[0].func = task1_function;
    tasks[0].stack_base = &task_stacks[0][0]; // 스택 배열의 시작 주소
    tasks[0].stack_size_words = STACK_SIZE;   // 스택 크기 (워드 단위)
    tasks[0].psp = (uint32_t)os_initialize_task_stack(&task_stacks[0][STACK_SIZE - 1], task1_function);

    tasks[1].id = 1;
    tasks[1].func = task2_function;
    tasks[1].stack_base = &task_stacks[1][0];
    tasks[1].stack_size_words = STACK_SIZE;
    tasks[1].psp = (uint32_t)os_initialize_task_stack(&task_stacks[1][STACK_SIZE - 1], task2_function);

    // 최초 태스크 설정
    current_task_tcb = &tasks[0];
    next_task_tcb = &tasks[1];

    // PendSV 및 SysTick 우선순위 설정 (가장 낮은 우선순위로 설정)
    // SCB->SHPR3 (System Handler Priority Register 3)
    // PendSV: Bits 23:16, SysTick: Bits 31:24
    // 0xFF = 가장 낮은 우선순위 (STM32F3/F4에서 0xF0은 가장 낮은 서브 우선순위)
    SCB->SHPR3 |= (0xFFUL << 16); // PendSV 우선순위 설정
    SCB->SHPR3 |= (0xFFUL << 24); // SysTick 우선순위 설정

    // SysTick 타이머 설정 (주기 설정)
    // SystemCoreClock은 시스템 클럭 주파수입니다 (예: 168MHz for STM32F407).
    // 이 값은 system_stm32f4xx.c 파일에 정의되어 있습니다.
    SysTick->LOAD = (SystemCoreClock / 1000) - 1; // 1ms (1000Hz) 주기 설정
    SysTick->VAL = 0; // 카운터 초기화

    // 중요: SysTick 인터럽트 및 카운터는 start_first_task()에서 활성화합니다.
    // SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

// SysTick_Handler (startup_stm32f4xx.s 또는 startup_stm32f3xx.s 에 약한 심볼로 정의됨)
// 약한 심볼로 정의되어 있으므로, 우리는 이 함수를 재정의하여 사용할 수 있습니다.
void SysTick_Handler(void) {
    // 이곳에서 다음 실행할 태스크를 결정하는 스케줄링 로직이 들어갑니다.
    // 이 예제에서는 단순히 current_task_tcb와 next_task_tcb를 교환하는 것으로 스케줄링을 "모방"합니다.

    // 간단한 라운드 로빈 스케줄링 모방 (태스크 스위칭)
    // current_task_tcb는 현재 CPU를 점유하고 있는 태스크의 TCB 포인터
    // next_task_tcb는 다음에 실행될 태스크의 TCB 포인터
    // 이 두 포인터를 단순히 교환함으로써 다음 PendSV 인터럽트 시
    // 현재 실행 중인 태스크는 next_task_tcb에 저장되고, current_task_tcb의 태스크가 로드되도록 합니다.
    TCB* temp_tcb = current_task_tcb;
    current_task_tcb = next_task_tcb;
    next_task_tcb = temp_tcb;

    // PendSV 요청 (실제 컨텍스트 스위칭은 여기서 발생)
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}
