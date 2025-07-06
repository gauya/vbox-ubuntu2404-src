#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

// 위에서 정의한 Semaphore 구조체 및 함수 포함

// 가상 환경 설정
#include <unistd.h>
#define delay(ms) usleep((ms) * 1000)

typedef struct {
  volatile uint8_t value;
} Semaphore;

void sem_init(Semaphore *s, uint8_t init_val) {
    s->value = init_val ? 1 : 0;
}

void sem_wait(Semaphore *s) {
    while (1) {
        __disable_irq();
        if (s->value) {
            s->value = 0;
            __enable_irq();
            return;
        }
        __enable_irq();
        delay(1);  // sleep 1 tick
    }
}

void sem_give(Semaphore *s) {
    __disable_irq();
    s->value = 1;
    __enable_irq();
}

// 공유 자원 (critical section)
std::atomic<int> shared_resource = 0;

// 세마포어 정의
Semaphore my_semaphore;

// 태스크 (task) 또는, 스레드 (thread) 함수
void task1() {
    for (int i = 0; i < 5; ++i) {
        sem_wait(&my_semaphore);  // 세마포어 획득
        shared_resource++;       // 공유 자원 접근 (critical section)
        std::cout << "Task 1: shared_resource = " << shared_resource << std::endl;
        sem_give(&my_semaphore);  // 세마포어 반환
        delay(10);  // 짧은 시간 대기
    }
}

void task2() {
    for (int i = 0; i < 5; ++i) {
        sem_wait(&my_semaphore);  // 세마포어 획득
        shared_resource--;       // 공유 자원 접근
        std::cout << "Task 2: shared_resource = " << shared_resource << std::endl;
        sem_give(&my_semaphore);  // 세마포어 반환
        delay(15);  // 짧은 시간 대기
    }
}

int main() {
    // 세마포어 초기화 (사용 가능 상태로)
    sem_init(&my_semaphore, 1);

    // 태스크 (스레드) 생성
    std::thread t1(task1);
    std::thread t2(task2);

    // 태스크 (스레드) 종료 대기
    t1.join();
    t2.join();

    std::cout << "Final shared_resource = " << shared_resource << std::endl;

    return 0;
}
