
// mutex.c
#include "rtos.h"

extern void delay(uint32_t ms);
extern uint8_t current_task;
extern uint32_t get_tick();

void mutex_init(Mutex *m) {
    m->lock_count = 0;
    m->owner_task_id = 0xFF;
}

void mutex_lock(Mutex *m, uint8_t task_id) {
    while (1) {
        __disable_irq();
        if (m->lock_count == 0) {
            m->owner_task_id = task_id;
            m->lock_count = 1;
            __enable_irq();
            return;
        } else if (m->owner_task_id == task_id) {
            m->lock_count++;
            __enable_irq();
            return;
        }
        __enable_irq();
        task_monitors[task_id].wait_tick = get_tick();
        task_monitors[task_id].wait_desc = "MUTEX_WAIT";
        delay(1);
    }
}

int mutex_try_lock(Mutex *m, uint8_t task_id) {
    int success = 0;
    __disable_irq();
    if (m->lock_count == 0) {
        m->owner_task_id = task_id;
        m->lock_count = 1;
        success = 1;
    } else if (m->owner_task_id == task_id) {
        m->lock_count++;
        success = 1;
    }
    __enable_irq();
    return success;
}

int mutex_lock_timeout(Mutex *m, uint8_t task_id, uint32_t timeout_tick) {
    uint32_t start = get_tick();
    while ((get_tick() - start) < timeout_tick) {
        __disable_irq();
        if (m->lock_count == 0) {
            m->owner_task_id = task_id;
            m->lock_count = 1;
            __enable_irq();
            return 1;
        } else if (m->owner_task_id == task_id) {
            m->lock_count++;
            __enable_irq();
            return 1;
        }
        __enable_irq();
        task_monitors[task_id].wait_tick = get_tick();
        task_monitors[task_id].wait_desc = "MUTEX_TIMEOUT";
        delay(1);
    }
    return 0;
}

void mutex_unlock(Mutex *m, uint8_t task_id) {
    __disable_irq();
    if (m->owner_task_id == task_id && m->lock_count > 0) {
        m->lock_count--;
        if (m->lock_count == 0) {
            m->owner_task_id = 0xFF;
        }
    }
    __enable_irq();
}

