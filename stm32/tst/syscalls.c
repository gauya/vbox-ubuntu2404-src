#include <errno.h>

void _exit(int status) {
    while (1); // 무한 루프, 시스템 종료 없음
}

int _write(int file, char *ptr, int len) {
    (void)file; (void)ptr; (void)len;
    return -1; // 더미 반환, 필요 시 UART로 출력 구현
}

void *_sbrk(int incr) {
    extern char _end; // 링커 스크립트에서 정의된 힙 끝
    static char *heap_end = &_end;
    char *prev_heap_end = heap_end;
    heap_end += incr;
    return (void *)prev_heap_end;
}

int _close(int file) {
    (void)file;
    return -1; // 더미 반환
}

int _read(int file, char *ptr, int len) {
    (void)file; (void)ptr; (void)len;
    return -1; // 더미 반환
}

int _lseek(int file, int ptr, int dir) {
    (void)file; (void)ptr; (void)dir;
    return -1; // 더미 반환
}

int _getpid(void) { return 1; }
int _kill(int pid, int sig) { (void)pid; (void)sig; return -1; }
