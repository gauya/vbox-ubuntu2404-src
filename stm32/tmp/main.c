#include "stm32f4xx.h" // 또는 stm32f3xx.h (사용하는 MCU에 맞게 변경하세요)
                       // 이 헤더는 SysTick, SCB, GPIO 관련 정의를 포함합니다.

// 어셈블리 파일에 정의된 함수를 C 파일에서 사용하기 위해 extern 선언
extern void start_first_task(void);

// os_init 함수는 C 파일에 정의되어 있으므로 extern 선언 필요 없음
extern void os_init(void);

// 태스크 함수들은 이미 위에 정의되어 있습니다.

int main(void) {
    // 1. 시스템 클럭 및 주변장치 초기화
    // 이 부분은 실제 STM32 펌웨어 프로젝트에서 CubeMX 등으로 생성된 코드를 포함해야 합니다.
    // 예: SystemClock_Config(), MX_GPIO_Init() 등
    // 여기서는 컨텍스트 스위칭 로직에 집중하므로 생략합니다.
    // 실제 보드에서 LED를 사용하려면 해당 GPIO 핀을 출력으로 초기화해야 합니다.

    // 2. OS 초기화: 태스크 스택 준비 및 PendSV 우선순위 설정
    // SysTick 타이머는 설정하되, 인터럽트 활성화는 start_first_task()에서 수행합니다.
    os_init();

    // 3. 최초 태스크 시작
    // 이 함수가 호출되면 CPU는 MSP에서 PSP로 전환하고,
    // os_initialize_task_stack에서 설정한 첫 태스크의 컨텍스트를 로드하여 실행을 시작합니다.
    // 또한, 이 함수 내에서 SysTick 인터럽트가 활성화됩니다.
    start_first_task();

    // 여기까지 코드가 도달하면 안 됩니다.
    // 첫 태스크가 무한 루프에 들어가기 때문에, start_first_task 호출 후
    // 이 main 함수의 나머지 부분은 실행되지 않습니다.
    while (1) {
        // 프로그램이 여기에 도달하면 오류입니다.
    }
}

