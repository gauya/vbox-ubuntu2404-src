// semaphore.h
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>

// ========== Binary Semaphore ==========
typedef struct {
    volatile uint8_t value;
} Semaphore;

void sem_init(Semaphore *s, uint8_t init_val);
void sem_wait(Semaphore *s);
void sem_give(Semaphore *s);

// ========== Recursive Mutex ==========
typedef struct {
    volatile uint8_t owner_task_id;
    volatile uint8_t lock_count;    // 0이면 잠기지 않음
} Mutex;

void mutex_init(Mutex *m);
void mutex_lock(Mutex *m, uint8_t task_id);
void mutex_unlock(Mutex *m, uint8_t task_id);

#endif // SEMAPHORE_H

