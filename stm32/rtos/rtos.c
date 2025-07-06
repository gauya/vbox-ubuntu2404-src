#include "rtos.h"
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h" // 또는 stm32f3xx.h (사용하는 MCU에 맞게 변경하세요)
                       // 이 헤더는 SysTick, SCB, GPIO 관련 정의를 포함합니다.

#define TCB_ACTIVE  0x1
#define TCB_STANBY  0x2
#define TCB_SLEEP   0x4
#define TCB_DELAY_REQ 0x8
#define TCB_DELAY_RCH 0x10
#define TCB_TIME_RCH  0x20


uint32_t __stack_pool[ (MAX_TASK + 1) * DEF_STACK_SIZE];
uint8_t __next_task_id = 0; // 현재 최대 task 수

TCB __tcb[MAX_TASK];
uint8_t __current_task_id;

// FPU 포함 태스크 생성
void set_stack_from_tcb(TCB *tcb) {
    uint32_t *stack_top = tcb->stack;

    // 자동 복원되는 영역 (xPSR~R0)
    *(--stack_top) = 0x21000000;        // xPSR (Thumb)
    *(--stack_top) = (uint32_t)task_wrap_func; // PC
    *(--stack_top) = 0xFFFFFFED;        // LR (EXC_RETURN, use PSP + FPU active)
    *(--stack_top) = 0x12121212;        // R12
    *(--stack_top) = 0x03030303;        // R3
    *(--stack_top) = 0x02020202;        // R2
    *(--stack_top) = 0x01010101;        // R1
    *(--stack_top) = (uint32_t)tcb->context;        // R0

#if USE_FPU == 1
  #if defined(__FPU_DP) && (__FPU_DP == 1)
    // // Cortex-M7 등 DP FPU (D0~D31, 8byte씩 32개)
    for (int i = 31; i >= 0; i--) {   // S0 - S31  8byte * 32 = 256
        *(--stack_top) = 0xAAAA0000 + i; // 하위 32비트
        *(--stack_top) = 0xAAAA0000 + i; // 상위 32비트
    }
  #else
    // FPU 자동 저장 프레임 (S0~S15)
    for (int i = 15; i >= 0; i--) {   // S0 - S15
        *(--stack_top) = 0xAAAA0000 + i; 
    }
  #endif
    // FPU 자동 저장 프레임 (FPSCR + reserved)
    *(--stack_top) = 0x00000000;        // FPSCR
    *(--stack_top) = 0x00000000;        // reserved word
#endif

    // 수동 저장 대상 (R4~R11)
    for (int i = 0; i < 8; i++) {
        *(--stack_top) = 0xDEADBEEF;
    }

    tcb->sp = (uint32_t)stack_top;
}

int add_task(void (*func)(), void *context, uint32_t stack_size,uint32_t period, int priority) {
  static uint32_t *next_stack_ptr = __stack_pool; // only use add_task()

  if( stack_size < MIN_STACK_SIZE ) {
    stack_size = MIN_STACK_SIZE;
  }
  uint32_t aligned_size = (stack_size + 7) & ~7;  // 8바이트 정렬
  tcb->type = (period > 0)? 1 : 0; // 1: realtime( period > 0 ), 2: event driven( priority > 255 ) 
  if( priority < 0 || priority > 255 ) {
    tcb->type = 2; // event driven 
    priority = 255; 
  }

  TCB *tcb = __tcb[ __next_task_id ];

  if( aligned_size < MIN_STACK_SIZE ) {
    aligned_size = MIN_STACK_SIZE;
  } 
  tcb->status = 0;
  tcb->stack = stack_ptr;
  tcb->stack_size = aligned_size;
  tcb->period = period; // 0 이아니면 realtime function
  tcb->period_tick = period; // copy period
  tcb->priority = (uint8_t)priority; // check 0 - 255
  tcb->delay = 0;
  tcb->psp = 0;
  tcb->id = __next_task_id;
  tcb->func = func;
  tcb->context = context;

  if( tcb->period > 0 ) {
    tcb->type = 1;
    tcb->period_tick = tcb->period;
  }

  if( tcb->id == 0 ) {
    // maintance task
  }

  for (int j = 0; j < tcp->stack_size / sizeof(uint32_t); j++) {
      tcb->stack[j] = STACK_CANARY_PATTERN;
  }

  set_stack_from_tcb( tcb );

  next_stack_ptr += aligned_size;
  __next_task_id++;

  if( (__stack_pool - next_stack_ptr) >= (MAX_TASK * DEF_STACK_SIZE) ) {
    // stack overflow
    return -1;
  }

  return (int)tcb->id;
}

TCB *get_task( int id ) {
  if( id < 0 || id > __next_task_id ) {
    return __tcb[ __current_task_id ];
  }
  return __tcb[ id ];
}

void os_init() {
  // stack all set STACK_CANARY_PATTERN
  // add_task()
  /*
  if ( add_task() == -1 ) {
    while(1);
  }
  */
}

void os_start() {
  // SyTick, PendSV 우선순위 최하위로 설정 oxF0
  SCB->SHPR3 |= (0xFFUL << 16); // PendSV 
  SCB->SHPR3 |= (0xFFUL << 24); // SysTick

  // SysTick 1ms Start scadule
  SysTick_Config( SystemCoreClock / 1000 );
}

void delay(TCB *tcb, uint32_t ms) {
    tcb->state |= 0x4; // block
    tcb->delay = ms;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void udelay(uint32_t us) {
  // micro second delay ( 1-
}

volatile uint32_t _system_ticks;

uint32_t get_tick() {
  return _system_ticks;
}

void SysTick_Handler(void) {
    HAL_IncTick();  // HAL 라이브러리에서 제공하는 함수 호출

    _system_ticks++;

    TCB *tcb = __tcb;
    for (int i = 0; i < __next_task_id; i++) {
        if ( tcb->delay > 0) {
          tcb->delay--;
          if ( tcb->delay == 0 ) {
            tcb->ctrlstat |= 0b00001000;
          }
        }
        if ( tcb->period_tick > 0 ) {
          tcb->period_tick--;
          if ( tcb->period_tick == 0 ) {
            tcb->ctrlstat |= 0b00010000;
          }
        }
    }
    // PendSV
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void pre_task(uint8_t task_id) {
    task_ctrl[task_id].last_run_tick = get_tick();
}

void post_task(uint8_t task_id) {
    // 주기성 태스크 누락 보정
    if (task_ctrl[task_id].type == TASK_PERIODIC_CATCHUP &&
        task_ctrl[task_id].missed_count > 0)
    {
        task_ctrl[task_id].missed_count--;
    }
    if (task_ctrl[task_id].type == TASK_EVENT_DRIVEN) {
        task_ctrl[task_id].active = 0;
    }
}

void task_wrapper(void (*func)(), uint8_t task_id) {
    while (1) {
        pre_task(task_id);
        func();
        post_task(task_id);
    }
}

void schedule_next_task(void) {
    uint8_t best = __current_task_id;
    for (int i = 0; i < MAX_TASK; i++) {
        if (tasks[i].ready && tasks[i].priority >= tasks[best].priority) {
            best = i;
        }
    }
    __current_task_id = best;
}

// ========== SysTick for Delay Countdown ==========
void SysTick_Handler(void) {
    for (int i = 0; i < MAX_TASK; i++) {
        if (delay_vars[i] > 0) {
            delay_vars[i]--;
            if (delay_vars[i] == 0) {
                tasks[i].ready = 1;
            }
        }
    }
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void scheduler_start() {
    __current_task_id = 0;
    __set_PSP(__tcb[__current_task_id].stack);
    __set_CONTROL(0x03);
    __ISB();
    task_wrapper(__tcb[__current_task_id].func, __current_task_id);
}

void task_set_priority(uint8_t task_id, uint8_t new_prio) {
    if (task_id < task_count && task_ctrl[task_id].type == TASK_OPPORTUNISTIC) {
        task_ctrl[task_id].priority = new_prio;
    }
}

void task_wake(uint8_t task_id) {
    if (task_id < task_count && task_ctrl[task_id].type == TASK_EVENT_DRIVEN) {
        task_ctrl[task_id].active = 1;
    }
}

void critical_section() {
    interrupt_status = __get_PRIMASK();  // 현재 인터럽트 상태 저장 (PRIMASK 레지스터 읽기)
    __disable_irq();                 // 인터럽트 비활성화

    // ... (보호 구역 내 코드) ...

    if (!(interrupt_status & 0x01)) { // 인터럽트가 비활성화되지 않았다면
    //    __enable_irq();                 // 원래의 상태로 복원
      __set_PRIMASK(interrupt_status);
    }

}

// 우선순위 재조정-> 다음 실행할 task 결정
//
void schedule() {
    uint32_t now = get_tick();
    uint8_t best = __current_task_id;
    uint8_t best_score = 0;
    TCB *tcb = __tcb;

    for (int i = 0; i < __next_task_id; i++) {
        if (tcb->status | TCB_ACTIVE ) {
          best = i
          break;
        }

        switch (tcb->type) {
        case TASK_PERIODIC_CATCHUP:
            if (now - t->last_run_tick >= t->period_tick) {
                uint32_t miss = (now - t->last_run_tick) / t->period_tick;
                t->missed_count = (miss > 3) ? 3 : miss;
                if (t->missed_count > 0) {
                    best_task = i;
                    goto chosen;
                }
            }
            break;
        case TASK_PERIODIC_SKIP:
            if (now - t->last_run_tick >= t->period_tick) {
                best_task = i;
                goto chosen;
            }
            break;
        case TASK_EVENT_DRIVEN:
            if (t->active) {
                best_task = i;
                goto chosen;
            }
            break;
        case TASK_OPPORTUNISTIC:
            if (t->priority > best_score) {
                best_score = t->priority;
                best_task = i;
            }
            break;
        }
    }

chosen:
    if (best_task != __current_task_id) {
        task_psp[__current_task_id] = __get_PSP();
        __current_task_id = best_task;
        __set_PSP(task_psp[__current_task_id]);
    }
}

void delay(uint32_t ms) {
    delay_vars[__current_task_id] = ms;
    tasks[__current_task_id].ready = 0;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    __asm volatile("NOP");
}

// ========== Task Creation ==========
void create_task(int id, void (*task_func)(void), uint16_t stack_size, uint8_t priority) {
    tasks[id].stack_size = stack_size;
    tasks[id].stack_mem = malloc(stack_size * sizeof(uint32_t));
    uint32_t *sp = &tasks[id].stack_mem[stack_size];

    *(--sp) = 0x21000000; // xPSR
    *(--sp) = (uint32_t)task_func; // PC
    *(--sp) = 0xFFFFFFFD; // LR (return to thread using PSP)
    for (int i = 0; i < 5; i++) *(--sp) = 0; // R12, R3, R2, R1, R0
    for (int i = 0; i < 8; i++) *(--sp) = 0xDEADBEEF; // R4-R11

    tasks[id].sp = sp;
    tasks[id].priority = priority;
    tasks[id].ready = 1;
    delay_vars[id] = 0;
}

// FPU 포함 태스크 생성
void create_task_with_fpu(int id, void (*task_func)(void)) {
    uint32_t *stack_top = &stacks[id][STACK_SIZE - 1];

    // 자동 복원되는 영역 (xPSR~R0)
    *(--stack_top) = 0x21000000;        // xPSR (Thumb)
    *(--stack_top) = (uint32_t)task_func; // PC
    *(--stack_top) = 0xFFFFFFED;        // LR (EXC_RETURN, use PSP + FPU active)
    *(--stack_top) = 0x12121212;        // R12
    *(--stack_top) = 0x03030303;        // R3
    *(--stack_top) = 0x02020202;        // R2
    *(--stack_top) = 0x01010101;        // R1
    *(--stack_top) = 0x00000000;        // R0

    // FPU 자동 저장 프레임 (S0~S15 + FPSCR + reserved)
    *(--stack_top) = 0x00000000;        // FPSCR
    *(--stack_top) = 0x00000000;        // reserved word
    for (int i = 15; i >= 0; i--) {
        *(--stack_top) = 0xAAAA0000 + i; // S15~S0
    }

    // 수동 저장 대상 (R4~R11)
    for (int i = 0; i < 8; i++) {
        *(--stack_top) = 0xDEADBEEF;
    }

    tasks[id].sp = stack_top;
}


// ========== Example Idle Task ==========
void idle_task(void) {
    while (1) {
        __WFI(); // Wait For Interrupt
    }
}

// ========== Example Task ==========
void example_task(void) {
    while (1) {
        // Do something
        delay(1000); // sleep for 1000 ticks
    }
}

__attribute__((naked)) void restore_context(uint32_t psp) {
    __asm volatile (
        "MSR PSP, r0\n"
        "LDMIA R0!, {R4-R11}\n"
        "MSR PSP, R0\n"
        "BX LR\n"
    );
}

void monitor_task() {
    while (1) {
        uint32_t now = get_tick();
        uint8_t best = 255;
        uint8_t best_prio = 0;

        for (int i = 0; i < task_count; i++) {
            if (task_delay[i] > 0) continue;
            TaskControl *t = &task_ctrl[i];

            switch (t->type) {
                case TASK_PERIODIC_CATCHUP:
                    if (now - t->last_run_tick >= t->period_tick) {
                        uint32_t miss = (now - t->last_run_tick) / t->period_tick;
                        t->missed_count = (miss > 3) ? 3 : miss;
                        if (t->missed_count > 0) best = i;
                    }
                    break;
                case TASK_PERIODIC_SKIP:
                    if (now - t->last_run_tick >= t->period_tick) best = i;
                    break;
                case TASK_EVENT_DRIVEN:
                    if (t->active) best = i;
                    break;
                case TASK_OPPORTUNISTIC:
                    if (t->priority > best_prio) {
                        best_prio = t->priority;
                        best = i;
                    }
                    break;
            }
        }

        if (best != 255 && best != __current_task_id) {
            __set_PSP(task_psp[__current_task_id]);
            __set_CONTROL(0x03);
            __ISB();

            __current_task_id = best;
            if( first ) {
              task_ctrl[__current_task_id].func();
            } else {
              restore_context( task_psp[__current_task_id] );
            }
        }
    }
}

attribute((naked)) void start_first_task(void) {
__asm volatile (
    "LDR R0, =task_stack_ptrs\n"    // R0 = 배열 주소
    "LDR R1, =__current_task_id\n"       // R1 = __current_task_id index 주소
    "LDR R2, [R1]\n"                // R2 = 현재 태스크 ID
    "LSLS R2, R2, #2\n"             // 인덱싱을 위한 *4
    "ADD R0, R0, R2\n"
    "LDR R0, [R0]\n"                // R0 = PSP 값
    "MSR PSP, R0         \n"        // PSP 설정
    "MOV R0, #0x03       \n"        // Thread mode, PSP 사용
    "MSR CONTROL, R0     \n"
    "ISB                 \n"
    "DSB                 \n"
    "LDMIA R0!, {R4-R11} \n"        // 수동으로 R4-R11 복원
    "BX LR               \n"        // 하드웨어가 자동으로 나머지 (R0-R3, R12, LR, PC, xPSR) 복원
);

__attribute__((naked)) void start_first_task(void) {
__asm volatile (
    "LDR R0, =__tcb\n"              // tasks 배열 주소 로드
    "LDR R1, =__current_task_id\n"  // __current_task_id 주소 로드
    "LDR R2, [R1]\n"                // __current_task_id 값 로드
    "LSLS R2, R2, #2\n"             // 인덱스 계산 (4바이트 단위)
    "ADD R0, R0, R2\n"              // 태스크 스택 포인터 주소 계산
    "LDR R0, [R0]\n"                // 첫 태스크의 스택 포인터 로드
    "MSR PSP, R0\n"                 // PSP에 스택 포인터 설정
    "MSR CONTROL, R3\n"          // CONTROL 레지스터에 설정 적용
    "ISB\n"                     // 명령어 동기화
    "POP {R4-R11}\n"            // FPU 없는 경우 기본 레지스터만 복원
    "POP {R0-R3, R12, LR, PC, xPSR}\n"  // 컨텍스트 복원 후 실행
);
}

os_start_first_task:
    ; 이 함수는 최초로 태스크를 시작할 때 main 함수에서 호출됩니다.
    ; 여기서 Main Stack Pointer(MSP)에서 Process Stack Pointer(PSP)로 전환하고,
    ; 첫 태스크의 컨텍스트를 로드하여 실행을 시작합니다.

    ; 1. 첫 태스크의 스택 포인터를 로드합니다.
    LDR R0, =__current_task_sp  ; __current_task_sp 변수의 주소를 R0에 로드
    LDR R0, [R0]              ; __current_task_sp 값을 R0에 로드 (이것이 첫 태스크의 스택 포인터)

    ; 2. 첫 태스크의 R4-R11 레지스터를 복원합니다.
    LDMIA R0!, {R4-R11}       ; 첫 태스크의 R4-R11을 스택에서 복원하고 R0를 업데이트(증가)합니다.

    ; 3. 업데이트된 R0 (첫 태스크의 SP)를 PSP에 저장합니다.
    MSR PSP, R0               ; Process Stack Pointer를 R0 (첫 태스크의 SP)로 설정

    ; 4. CONTROL 레지스터를 설정하여 PSP를 사용하고 비특권 모드로 전환합니다.
    ; CONTROL 레지스터 (ARMv7-M Architecture Reference Manual 참조):
    ; Bit 1 (SPSEL): 0 = MSP 사용, 1 = PSP 사용
    ; Bit 0 (nPRIV): 0 = 특권 모드, 1 = 비특권 모드 (RTOS에서는 보통 태스크를 비특권 모드로 실행)
    MOV R0, #0x03             ; PSP 사용 (bit 1 설정), 비특권 모드 (bit 0 설정)
    MSR CONTROL, R0           ; CONTROL 레지스터에 설정 값 적용

    ; 5. ISB (Instruction Synchronization Barrier) 및 DSB (Data Synchronization Barrier)
    ; 캐시, 파이프라인 등의 동기화를 보장하여 레지스터 변경 사항이 즉시 적용되도록 합니다.
    ISB
    DSB

    ; 6. BX LR 명령을 통해 첫 태스크 함수로 점프합니다.
    ; 이 과정은 PendSV_Handler에서 복귀하는 것과 유사하게 작동합니다.
    ; 즉, 스택에 미리 저장된 R0-R3, R12, LR, PC, XPSR 값이 PSP에서 팝되면서
    ; 첫 태스크가 마치 인터럽트에서 복귀하는 것처럼 실행을 시작합니다.
    BX LR


// Cortex-M4/M7 (FPU 지원) vs M3/M0 (미지원) 자동 감지
#if defined(__FPU_PRESENT) && (__FPU_PRESENT == 1)
    #define USE_FPU 1
#else
    #define USE_FPU 0
#endif

__attribute__((naked)) void start_first_task(void) {
    __asm volatile (
        "LDR R0, =__tcb\n"              // tasks 배열 주소 로드
        "LDR R1, =__current_task_id\n"       // __current_task_id 주소 로드
        "LDR R2, [R1]\n"                // __current_task_id 값 로드
        "LSLS R2, R2, #2\n"             // 인덱스 계산 (4바이트 단위)
        "ADD R0, R0, R2\n"              // 태스크 스택 포인터 주소 계산
        "LDR R0, [R0]\n"                // 첫 태스크의 스택 포인터 로드
        "MSR PSP, R0\n"                 // PSP에 스택 포인터 설정

#if USE_FPU == 1
        "    MRS R3, CONTROL\n"         // CONTROL 레지스터 확인
        "    ORR R3, R3, #0x04\n"       // FPCA (FPU Context Active) 비트 설정 (bit 2)
        "    MSR CONTROL, R3\n"         // FPU 사용 활성화
  #if defined(__FPU_DP) && (__FPU_DP == 1)
        "    MOV R3, #0x07\n"           // PSP, 비특권 모드, FPU DP 활성화
  #else
        "    MOV R3, #0x03\n"           // PSP, 비특권 모드, FPU SP 활성화
  #endif
#else
        "    MOV R3, #0x03\n"           // PSP, 비특권 모드, FPU 비활성화
#endif
        "    MSR CONTROL, R3\n"          // CONTROL 레지스터에 설정 적용
        "    ISB\n"                     // 명령어 동기화

#if USE_FPU == 1
        "    POP {R4-R11}\n"            // 기본 레지스터 복원
        "    VLDMIA R0!, {S0-S31}\n"    // FPU 레지스터 복원 (S0-S31)
        "    ADD SP, SP, #8\n"          // FPSCR + reserved 스킵
#else
        "    POP {R4-R11}\n"            // FPU 없는 경우 기본 레지스터만 복원
#endif

        "POP {R0-R3, R12, LR, PC, xPSR}\n"  // 컨텍스트 복원 후 실행
    );
}

__attribute__((naked)) void go_to_monitor() {
    __asm volatile (
        "MRS R0, PSP\n"
        "STMDB R0!, {R4-R11}\n"
        "STR R0, %[psp]\n"
        "MOV R0, #0\n"
        "MSR CONTROL, R0\n"
        "ISB\n"
        "BL monitor_task\n"
        : [psp] "=m" (task_psp[__current_task_id])
        :
        : "r0"
    );
}

__attribute__((naked)) void PendSV_Handler(void) {
    __asm volatile (
        // Save context of current task
        "MRS R0, PSP\n"
#if USE_FPU == 1        
        "TST LR, #0x10\n"  // FPU가 없는 코어에서는 비트 4가 항상 1
        "IT EQ\n"
#if defined(__FPU_DP) && (__FPU_DP == 1)  // M7,H7        
        "VSTMDBEQ R0!, {D8-D15}\n"
#else
        "VSTMDBEQ R0!, {S16-S31}\n"
#endif // __FPU_DP
#endif // USE_FPU == 1
        "STMDB R0!, {R4-R11}\n"

        "LDR R1, =tasks\n"
        "LDR R2, =__current_task_id\n"
        "LDR R3, [R2]\n"
        "LSLS R3, R3, #6\n"
        "ADD R1, R1, R3\n"
        "STR R0, [R1]\n"

        // Find next task
        "BL schedule_next_task\n"

        // Load next task context
        "LDR R1, =tasks\n"
        "LDR R2, =__current_task_id\n"
        "LDR R3, [R2]\n"
        "LSLS R3, R3, #6\n"
        "ADD R1, R1, R3\n"
        "LDR R0, [R1]\n"

        "LDMIA R0!, {R4-R11}\n"
#if USE_FPU == 1        
        "TST LR, #0x10\n"
        "IT EQ\n"
#if defined(__FPU_DP) && (__FPU_DP == 1)        
        "VLDMIAEQ R0!, {D8-D15}\n"
#else
        "VLDMIAEQ R0!, {S16-S31}\n"
#endif // __FPU_DP
#endif
        "MSR PSP, R0\n"
        "BX LR\n"
    );
}

#if 0
__set_PSP(task_psp[__current_task_id]);
__set_CONTROL(0x03);
__ISB()

/ C/C++ 코드에 해당하는 어셈블리 코드 (PendSV_Handler 내부)

// 1. __set_PSP(task_psp[__current_task_id]);  // PSP 설정

// task_psp[__current_task_id] 주소를, R0 레지스터에 로드 (C/C++ 컴파일러에 따라 다름)
// (task_psp 배열, __current_task_id 변수 등은, 외부에서 정의되었다고 가정)
LDR R0, =task_psp  // task_psp 배열의 주소 로드
LDR R1, =__current_task_id // __current_task_id 변수의 주소 로드
LDR R2, [R1]       // __current_task_id 값 로드 (현재 태스크의 ID)
LSLS R2, R2, #2     // R2 = __current_task_id * 4 (uint32_t 배열)
ADD R0, R0, R2      // R0 = task_psp[__current_task_id]의 주소
LDR R0, [R0]       // R0 = task_psp[__current_task_id]의 값 (PSP 값)

// PSP 레지스터에, R0 값 저장
MSR PSP, R0        // PSP = R0  (PSP 설정)

// 2. __set_CONTROL(0x03);  // CONTROL 레지스터 설정 (PSP, User mode)

// 0x03 값을, R0 레지스터에 로드
MOV R0, #0x03

// CONTROL 레지스터에, R0 값 저장
MSR CONTROL, R0    // CONTROL = R0  (CONTROL 레지스터 설정)

// 3. __ISB();  // Instruction Synchronization Barrier (ISB)

// ISB 명령어 실행 (ARM 아키텍처)
ISB  // Instruction Synchronization Barrier

#endif

