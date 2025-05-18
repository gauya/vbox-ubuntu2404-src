#include <stdio.h>

int add(int a, int b) {
  return a + b;
}

const char* greet(const char* name) {
  static char buffer[256];
  snprintf(buffer, sizeof(buffer), "Hello, %s!", name);
  return buffer;
}
