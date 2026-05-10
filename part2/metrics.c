#include <bits/types/struct_timeval.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <x86intrin.h>

uint64_t read_cpu_timer() { return __rdtsc(); }

uint64_t read_os_freq() { return 1000000; }

uint64_t read_os_timer() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec * read_os_freq() + tv.tv_usec;
}

uint64_t estimate_cpu_freq() {
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
  uint64_t cpu_freq = os_freq * cpu_elapsed / os_elapsed;

  return cpu_freq;
}
