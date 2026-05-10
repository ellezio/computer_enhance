#include <stdint.h>
#include <stdio.h>

#include "metrics.c"

int main() {
  uint64_t os_freq = read_os_freq();
  uint64_t os_start = read_os_timer();
  uint64_t os_end = 0;
  uint64_t os_elapsed = 0;

  uint64_t cpu_start = read_cpu_timer();

  while (os_elapsed < os_freq) {
    os_end = read_os_timer();
    os_elapsed = os_end - os_start;
  }

  uint64_t cpu_end = read_cpu_timer();
  uint64_t cpu_elapsed = cpu_end - cpu_start;

  printf("os freq: %lu\n", os_freq);
  printf("os timer: %lu -> %lu = %lu elapsed\n", os_start, os_end, os_elapsed);
  printf("os seconds: %.4f\n", (double)os_elapsed / (double)os_freq);
  printf("cpu timer: %lu -> %lu = %lu elapsed\n", cpu_start, cpu_end,
         cpu_elapsed);

  printf("cpu timer: %lu (guessed)\n", os_freq * cpu_elapsed / os_elapsed);

  return 0;
}
