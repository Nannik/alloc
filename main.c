#include <stdio.h>

#include "mem.h"

int main() {
  char *buf1 = mem(5);
  char *buf2 = mem(10);
  char *buf3 = mem(5);

  unmem(buf2);

  char *buf4 = mem(5);
  char *buf5 = mem(5);

  return 0;
}
