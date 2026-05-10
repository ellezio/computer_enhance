#include <stdio.h>

#include "metrics.c"

int main() {
  uint64_t os_freq = read_os_freq();
  uint64_t os_start = read_os_timer();
  uint64_t os_end = 0;
  uint64_t os_elapsed = 0;
  while (os_elapsed < os_freq) {
    os_end = read_os_timer();
    os_elapsed = os_end - os_start;
  }

  printf("os freq: %lu\n", os_freq);
  printf("os timer: %lu -> %lu = %lu elapsed\n", os_start, os_end, os_elapsed);
  printf("os seconds: %.4f\n", (double)os_elapsed / (double)os_freq);

  return 0;
}
