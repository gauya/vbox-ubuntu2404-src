// rtos.h
#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

#define MAX_TASK        8
#define TASK_STACK_SIZE 256
#define DEADLOCK_TIMEOUT_TICK 1000

// 스택 감시용 패턴
#define STACK_CANARY_PATTERN 0xA5A5A5A5

// ================= Task 종류 정의 =================
typedef enum {
    TASK_PERIODIC_CATCHUP,  // 1. 주기 + 누락된 횟수 보완
    TASK_PERIODIC_SKIP,     // 1. 주기만 중요, 지나치면 skip
    TASK_OPPORTUNISTIC,     // 2. 자주 실행되면 좋음 (우선순위로 관리)
    TASK_EVENT_DRIVEN       // 3. 외부 이벤트로 깨어남
} TaskType;

// ================= Task 상태/모니터링 구조 =================
typedef struct {
    TaskType type;
    uint8_t  priority;         // 0~7 (TASK_OPPORTUNISTIC 용)
    uint32_t period_tick;      // TASK_PERIODIC 용
    uint32_t last_run_tick;
    uint32_t missed_count;     // TASK_PERIODIC_CATCHUP 용
    uint8_t  active;           // TASK_EVENT_DRIVEN 용 (1이면 실행 요청됨)
    void (*func)();            // 실행 함수 포인터
} TaskControl;

extern TaskControl task_ctrl[MAX_TASK];
extern uint32_t*   task_stacks[MAX_TASK];
extern uint32_t    task_psp[MAX_TASK];
extern uint8_t     current_task;

void scheduler_init();
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
void pre_task(uint8_t task_id);
void post_task(uint8_t task_id);
void task_wrapper(void (*func)(), uint8_t task_id);
uint32_t task_check_stack_usage(uint32_t *stack_base, uint32_t stack_size);
void monitor_deadlock();
void monitor_waiting_tasks();
void monitor_stack();

#endif

