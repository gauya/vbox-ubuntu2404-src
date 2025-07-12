    .syntax unified
    .cpu cortex-m4  ; 또는 cortex-m3 (사용하는 MCU에 맞게 변경하세요)
    .thumb

    .global PendSV_Handler      ; PendSV_Handler 함수를 전역으로 선언
    .global start_first_task    ; start_first_task 함수를 전역으로 선언

    // FPU 지원 여부에 따른 조건부 컴파일
    #if defined(__FPU_PRESENT) && (__FPU_PRESENT == 1)
        #define USE_FPU 1
    #else
        #define USE_FPU 0
    #endif

.type PendSV_Handler, %function
PendSV_Handler:
    ; 이 함수는 PendSV 인터럽트가 발생했을 때 호출됩니다.
    ; 여기서 현재 태스크의 컨텍스트를 저장하고, 다음 태스크의 컨텍스트를 복원합니다.

    ; 1. 현재 실행 중인 태스크의 컨텍스트 저장
    MRS R0, PSP         ; Process Stack Pointer(PSP)를 R0 레지스터에 로드 (현재 태스크의 스택 포인터)

    ; R4-R11 레지스터를 현재 태스크의 스택에 푸시합니다.
    ; Cortex-M의 하드웨어 스택 프레임은 R0-R3, R12, LR, PC, XPSR을 자동으로 푸시합니다.
    ; FPU가 활성화되어 있고 lazy stacking이 사용되는 경우, FPU 레지스터(S0-S15, FPSCR)는
    ; FPU 명령이 실행될 때만 하드웨어적으로 스택에 푸시됩니다.
    ; 따라서 여기서는 R4-R11만 소프트웨어로 푸시합니다.
    STMDB R0!, {R4-R11} ; R4-R11을 R0가 가리키는 주소에 저장하고 R0를 업데이트(감소)합니다 (Full Descending Stack).

    ; 현재 태스크의 스택 포인터(SP)를 current_task_tcb->psp 변수에 저장합니다.
    ; current_task_tcb는 TCB* 타입이므로, TCB 구조체의 psp 멤버 오프셋을 사용합니다.
    ; TCB 구조체 정의에 따라 psp 멤버의 오프셋은 24바이트입니다.
    LDR R1, =current_task_tcb ; current_task_tcb 변수의 주소를 R1에 로드
    LDR R1, [R1]              ; current_task_tcb (TCB* 타입) 값을 R1에 로드
    STR R0, [R1, #24]         ; R0 (업데이트된 현재 SP)를 current_task_tcb->psp에 저장

    ; 2. 다음 실행할 태스크의 스택 포인터(SP)를 로드하여 컨텍스트 복원 준비
    LDR R1, =next_task_tcb    ; next_task_tcb 변수의 주소를 R1에 로드
    LDR R1, [R1]              ; next_task_tcb (TCB* 타입) 값을 R1에 로드
    LDR R0, [R1, #24]         ; next_task_tcb->psp 값을 R0에 로드 (이것이 다음 태스크의 새로운 스택 포인터)

    ; 다음 태스크의 컨텍스트 복원
    ; R4-R11 레지스터를 다음 태스크의 스택에서 팝합니다.
    LDMIA R0!, {R4-R11} ; R4-R11을 스택에서 복원하고 R0를 업데이트(증가)합니다 (Increment After).

    ; R0 (새로운 SP)를 PSP에 저장합니다.
    MSR PSP, R0         ; Process Stack Pointer를 R0 (새로운 SP)로 설정

    ; BX LR을 통해 인터럽트에서 복귀합니다.
    ; Cortex-M은 EXCEPTION RETURN 메커니즘을 사용하여 나머지 레지스터 (R0-R3, R12, LR, PC, XPSR)를
    ; PSP (우리가 설정한 새로운 SP)에서 자동으로 복원합니다.
    ; FPU가 활성화된 경우, LR의 EXC_RETURN 값에 따라 FPU 레지스터도 하드웨어적으로 복원됩니다.
    BX LR               ; 인터럽트에서 복귀

.type start_first_task, %function
__attribute__((naked)) void start_first_task(void) {
    __asm volatile (
        ; 1. 현재 실행할 첫 태스크의 TCB 포인터를 가져옵니다.
        ; current_task_tcb는 TCB* 타입의 전역 변수입니다.
        "LDR R0, =current_task_tcb\n" ; current_task_tcb 변수의 주소를 R0에 로드
        "LDR R0, [R0]\n"              ; current_task_tcb (TCB* 타입) 값을 R0에 로드 (첫 태스크의 TCB 주소)

        ; 2. 첫 태스크의 PSP 값을 TCB에서 로드합니다.
        ; TCB 구조체 내 psp 멤버의 오프셋은 24바이트입니다.
        "LDR R0, [R0, #24]\n"         ; R0 = current_task_tcb->psp

        ; 3. PSP 레지스터를 첫 태스크의 스택 포인터로 설정합니다.
        "MSR PSP, R0\n"

        ; 4. CONTROL 레지스터를 설정하여 PSP를 사용하고 비특권 모드로 전환합니다.
        ; FPU가 활성화된 경우 FPCA 비트도 설정합니다.
        ; CONTROL 레지스터 비트:
        ; Bit 0 (nPRIV): 0 = 특권 모드, 1 = 비특권 모드
        ; Bit 1 (SPSEL): 0 = MSP 사용, 1 = PSP 사용
        ; Bit 2 (FPCA): 0 = FPU 컨텍스트 비활성, 1 = FPU 컨텍스트 활성 (M4/M7/H7 FPU 전용)
        "MOV R3, #0x03\n"             ; PSP 사용 (bit 1), 비특권 모드 (bit 0) = 0b011
        "#if USE_FPU == 1\n"
        "   ORR R3, R3, #0x04\n"      ; FPCA 비트 (bit 2) 추가 = 0b100. 결과: 0b111 (0x07)
        "#endif\n"
        "MSR CONTROL, R3\n"           ; CONTROL 레지스터에 설정 값 적용
        "ISB\n"                       ; 명령어 동기화 장벽 (레지스터 변경 즉시 반영)

        ; 5. 첫 태스크의 R4-R11 레지스터를 스택에서 복원합니다.
        "POP {R4-R11}\n"              ; PSP에서 R4-R11을 팝하여 복원

        ; 6. SysTick 인터럽트 및 카운터를 활성화합니다.
        ; SysTick->CTRL 레지스터:
        ; Bit 0 (ENABLE): 1 = 카운터 활성화
        ; Bit 1 (TICKINT): 1 = 인터럽트 활성화
        ; Bit 2 (CLKSOURCE): 1 = 프로세서 클럭 사용 (Cortex-M에서 가장 일반적)
        "LDR R0, =0xE000E010\n"       ; SysTick CTRL 레지스터 주소
        "LDR R1, =0x07\n"             ; 0x07 = 0b111 (ENABLE | TICKINT | CLKSOURCE)
        "STR R1, [R0]\n"              ; SysTick->CTRL = 0x07

        ; 7. BX LR을 통해 첫 태스크 함수로 점프합니다.
        ; os_initialize_task_stack에서 LR에 설정된 EXC_RETURN 값에 따라
        ; 하드웨어적으로 PSP에서 나머지 레지스터 (R0-R3, R12, LR, PC, XPSR)를 팝하고
        ; FPU가 활성화된 경우 FPU 레지스터도 자동으로 팝하여 태스크를 시작합니다.
        "BX LR\n"
    );
}
