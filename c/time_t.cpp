#include <stdio.h>
#include <fcntl.h>

#include <time.h>


struct timespec ts;
struct timespec* getntime( struct timespec *ts) {
  clockid_t clkid;
  int fd;

  fd = open("/dev/ptp0", O_RDWR);
  clkid = FD_TO_CLOCKID(fd);
  clock_gettime(clkid, ts);
  close(fd);
  
  return ts;
}

int main() {
  getntime(&ts);
  printf("Size of time_t: %zu %lu (%lu.%lu)bytes\n", sizeof(time_t),sizeof(ts), ts.tv_sec,ts.tv_nsec);
  return 0;
}
