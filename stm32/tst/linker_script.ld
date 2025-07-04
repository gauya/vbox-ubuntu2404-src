/* linker_script.ld (수정된 버전) */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 256K
    RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 64K
}

SECTIONS
{
    .text :
    {
        *(.vectors)         /* 인터럽트 벡터 테이블 */
        *(.text)           /* 코드 섹션 */
        *(.rodata)         /* 읽기 전용 데이터 */
        _etext = .;        /* 텍스트 섹션 끝 */
    } > FLASH

    .data :
    {
        _sdata = .;        /* 데이터 섹션 시작 */
        *(.data)           /* 초기화된 데이터 */
        _edata = .;        /* 데이터 섹션 끝 */
    } > RAM AT > FLASH

    .bss :
    {
        __bss_start__ = .; /* Newlib 호환 심볼 */
        *(.bss)
        *(COMMON)
        __bss_end__ = .;   /* Newlib 호환 심볼 */
    } > RAM

    _end = __bss_end__;    /* 힙 시작점, BSS 끝과 일치 */
    PROVIDE(_heap_start = _end);
    PROVIDE(_heap_end = ORIGIN(RAM) + LENGTH(RAM));
}
