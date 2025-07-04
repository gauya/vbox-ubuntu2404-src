#include <errno.h>

extern void _exit(int status);
extern int _write(int file, char *ptr, int len);

extern void *_sbrk(int incr);
extern int _close(int file);
extern int _read(int file, char *ptr, int len);

extern int _lseek(int file, int ptr, int dir) ;
extern int _getpid(void);
extern int _kill(int pid, int sig);
