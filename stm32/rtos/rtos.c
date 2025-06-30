#include "rtos.h"
#include <stdio.h>
#include <stdlib.h>

TaskControl task_ctrl[MAX_TASK];
uint32_t*   task_stacks[MAX_TASK];
uint32_t    task_psp[MAX_TASK];
uint32_t    task_delay[MAX_TASK];
uint8_t     current_task = 0;
uint8_t     task_count = 0;

extern uint32_t get_tick();

void task_register(TaskType type, void (*func)(), uint32_t period_tick, uint8_t prio) {
    if (task_count >= MAX_TASK) return;
    task_ctrl[task_count].type = type;
    task_ctrl[task_count].func = func;
    task_ctrl[task_count].period_tick = period_tick;
    task_ctrl[task_count].priority = prio;
    task_ctrl[task_count].last_run_tick = 0;
    task_ctrl[task_count].missed_count = 0;
    task_ctrl[task_count].active = (type == TASK_EVENT_DRIVEN);
    task_delay[task_count] = 0;
    task_count++;
}

void delay(uint32_t tick) {
    task_delay[current_task] = tick;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    while (task_delay[current_task] > 0); // sleep
}

void SysTick_Handler(void) {
    for (int i = 0; i < task_count; i++) {
        if (task_delay[i] > 0) task_delay[i]--;
    }
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

void scheduler_init() {
    for (int i = 0; i < task_count; i++) {
        task_stacks[i] = (uint32_t*)malloc(TASK_STACK_SIZE);
        for (int j = 0; j < TASK_STACK_SIZE / sizeof(uint32_t); j++) {
            task_stacks[i][j] = STACK_CANARY_PATTERN;
        }

        uint32_t *stk = task_stacks[i] + TASK_STACK_SIZE / 4;
        *(--stk) = 0x01000000;                        // xPSR
        *(--stk) = (uint32_t)task_wrapper;           // PC
        *(--stk) = 0; // LR
        *(--stk) = 0; // R12
        *(--stk) = 0; // R3
        *(--stk) = 0; // R2
        *(--stk) = 0; // R1
        *(--stk) = (uint32_t)task_ctrl[i].func;      // R0
        for (int r = 0; r < 8; r++) *(--stk) = 0;
        task_psp[i] = (uint32_t)stk;
    }
}

void scheduler_start() {
    current_task = 0;
    __set_PSP(task_psp[current_task]);
    __set_CONTROL(0x03);
    __ISB();
    task_wrapper(task_ctrl[current_task].func, current_task);
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

void schedule() {
    uint32_t now = get_tick();
    uint8_t best_task = current_task;
    uint8_t best_score = 0;

    for (int i = 0; i < task_count; i++) {
        if (task_delay[i] > 0) continue;
        TaskControl *t = &task_ctrl[i];

        switch (t->type) {
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
    if (best_task != current_task) {
        task_psp[current_task] = __get_PSP();
        current_task = best_task;
        __set_PSP(task_psp[current_task]);
    }
}

void PendSV_Handler(void) {
    __asm volatile(
        "MRS R0, PSP\n"
        "STMDB R0!, {R4-R11}\n"
        "LDR R1, =task_psp\n"
        "LDR R2, =current_task\n"
        "LDR R3, [R2]\n"
        "STR R0, [R1, R3, LSL #2]\n"
        "BL schedule\n"
        "LDR R3, [R2]\n"
        "LDR R0, [R1, R3, LSL #2]\n"
        "LDMIA R0!, {R4-R11}\n"
        "MSR PSP, R0\n"
        "BX LR\n"
    );
}

