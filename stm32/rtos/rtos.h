#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

#define MAX_TASK        8

#define DEF_STACK_SIZE 256 // 8의 배수( Cortex-M 요구사항)
#define MIN_STACK_SIZE 80  // 64 + 16 : R4-R11, R0-R3, R12, LR, PC, PSR

#if USE_FPU == 1  // #if defined(__FPU_PRESENT) && (__FPU_PRESENT == 1)
#define DEF_STACK_SIZE 384
#define MIN_STACK_SIZE 164 // 64 + 64 + 4 + 32 : = , FPSCR, S0-S15

  #if defined(__FPU_DP) && (__FPU_DP == 1)  // M7,H7
#define DEF_STACK_SIZE 512 
#define MIN_STACK_SIZE 384 // 64 + 128 + 4 + 128 + 60 : =, S16-S31(D8-D15)
  #endif
#endif

#define DEADLOCK_TIMEOUT_TICK 1000

// 스택 감시용 패턴
#define STACK_CANARY_PATTERN 0xA5A5A5A5

// ================= Task 상태/모니터링 구조 =================
// active, realtime, delay, 
// B0: active
// B1: 
// 0:normal, 1:realtime, 2:event
typedef struct {
    uint8_t  type;             // 0: normal, 1: freq, 2: event
    uint8_t  id;
    uint8_t  priority;         // 0~7 (TASK_OPPORTUNISTIC 용)
    uint8_t  ctrlstat;         // 0: event 
    
    uint32_t period;           // TASK_PERIODIC 용( > 0 ) ms
    uint32_t period_tick;      // SysTick에서 decree, 0이되면 stat set b(1) 
    uint32_t delay;             // > 0이면 SysTick에서 -- stat set b(2)

    uint32_t *stack;
    uint32_t stack_size;
    uint32_t *sp;
    void (*func)();            // 실행 함수 포인터
    void *context;             // 

    uint32_t reserved[7];      // 64 byte
} TCB;

extern uint8_t current_task;

void stack_init();
void scheduler_start();
void schedule();
void task_set_priority(uint8_t task_id, uint8_t new_prio);
void task_wake(uint8_t task_id);

// ================= Mutex 구조 =================
typedef struct {
    volatile uint8_t owner_task_id;
    volatile uint8_t lock_count; // 0이면 lock 안됨
} Mutex;

void mutex_init(Mutex *m);
void mutex_lock(Mutex *m, uint8_t task_id);
void mutex_unlock(Mutex *m, uint8_t task_id);
int  mutex_try_lock(Mutex *m, uint8_t task_id);
int  mutex_lock_timeout(Mutex *m, uint8_t task_id, uint32_t timeout_tick);

// ================= Task/Monitor/Stack =================

__attribute__((naked)) void start_first_task(void);
void pre_task(uint8_t task_id);
void post_task(uint8_t task_id);
void task_wrapper(void (*func)(), uint8_t task_id);
uint32_t task_check_stack_usage(uint32_t *stack_base, uint32_t stack_size);
void monitor_deadlock();
void monitor_waiting_tasks();
void monitor_stack();

#endif // RTOS_H

