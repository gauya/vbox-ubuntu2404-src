// semaphore.c
#include "semaphore.h"

extern void delay(uint32_t ms);
extern uint8_t current_task;
extern uint32_t get_tick();

// ========== Binary Semaphore ==========
void sem_init(Semaphore *s, uint8_t init_val) {
    s->value = init_val ? 1 : 0;
}

void sem_wait(Semaphore *s) {
    while (1) {
        OS_LOCK(_lock_);
        if (s->value) {
            s->value = 0;
            OS_UNLOCK(_lock_);
            return;
        }
        OS_UNLOCK(_lock_);
        delay(1);  // sleep 1 tick
    }
}

void sem_give(Semaphore *s) {
    OS_LOCK(_lock_);

    s->value = 1;
    
    OS_UNLOCK(_lock_);
}

void mutex_init(Mutex *m) {
    m->lock_count = 0;
    m->owner_task_id = 0xFF;
}

void mutex_lock(Mutex *m, uint8_t task_id) {
    while (1) {
        OS_LOCK(_lock_);
        if (m->lock_count == 0) {
            m->owner_task_id = task_id;
            m->lock_count = 1;
            OS_UNLOCK(_lock_);
            return;
        } else if (m->owner_task_id == task_id) {
            m->lock_count++;
            OS_UNLOCK(_lock_);
            return;
        }
        OS_UNLOCK(_lock_);
        task_monitors[task_id].wait_tick = get_tick();
        task_monitors[task_id].wait_desc = "MUTEX_WAIT";
        delay(1);
    }
}

int mutex_try_lock(Mutex *m, uint8_t task_id) {
    int success = 0;
    OS_LOCK(_lock_);
    if (m->lock_count == 0) {
        m->owner_task_id = task_id;
        m->lock_count = 1;
        success = 1;
    } else if (m->owner_task_id == task_id) {
        m->lock_count++;
        success = 1;
    }
    OS_UNLOCK(_lock_);
    return success;
}

int mutex_lock_timeout(Mutex *m, uint8_t task_id, uint32_t timeout_tick) {
    uint32_t start = get_tick();
    while ((get_tick() - start) < timeout_tick) {
        OS_LOCK(_lock_);
        if (m->lock_count == 0) {
            m->owner_task_id = task_id;
            m->lock_count = 1;
            OS_UNLOCK(_lock_);
            return 1;
        } else if (m->owner_task_id == task_id) {
            m->lock_count++;
            OS_UNLOCK(_lock_);
            return 1;
        }
        OS_UNLOCK(_lock_);
        task_monitors[task_id].wait_tick = get_tick();
        task_monitors[task_id].wait_desc = "MUTEX_TIMEOUT";
        delay(1);
    }
    return 0;
}

void mutex_unlock(Mutex *m, uint8_t task_id) {
    OS_LOCK(_lock_);
    if (m->owner_task_id == task_id && m->lock_count > 0) {
        m->lock_count--;
        if (m->lock_count == 0) {
            m->owner_task_id = 0xFF;
        }
    }
    OS_UNLOCK(_lock_);
}


