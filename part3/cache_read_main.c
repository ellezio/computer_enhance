#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define BUFFER_ALLOC_MMAP 1

#include "buffer.c"
#include "platform.c"
#include "repetition_tester.c"

typedef void asm_func(uint64_t size, char *data, uint64_t mask);

extern void read_mask(uint64_t size, char *data, uint64_t mask);

struct test_function {
  char *name;
  asm_func *func;
};

struct test_function test_functions[] = {{"read_mask", read_mask}};

int main(int argc, char **argv) {
  struct buffer buffer = allocate_buffer(1024 * 1024 * 1024);
  if (buffer.data == 0) {
    fprintf(stderr, "ERROR: failed to allocate buffer\n");
    return 1;
  }

  uint32_t bit_count = log2(1024 * 1024) + 1;

  size_t test_func_count = sizeof(test_functions) / sizeof(*test_functions);
  uint64_t cpu_time_freq = estimate_cpu_freq();

  struct repetition_tester testers[bit_count][test_func_count] = {};

  for (;;) {
    for (int test_index = 0; test_index < test_func_count; ++test_index) {
      for (uint64_t i = 0; i < bit_count; i++) {
        struct repetition_tester *tester = &testers[i][test_index];
        struct test_function test_func = test_functions[test_index];

        printf("\n--- %s | %d KB ---\n", test_func.name, (1024 << i) / 1024);

        new_test_wave(tester, buffer.size, cpu_time_freq, 1);

        while (is_testing(tester)) {
          begin_time(tester);
          test_func.func(buffer.size, buffer.data, (1024 << i) - 1);
          end_time(tester);
          count_bytes(tester, buffer.size);
        }
      }
    }
  }
}
