#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define BUFFER_ALLOC_MMAP 1

#include "buffer.c"
#include "platform.c"
#include "repetition_tester.c"

#define DOUBLE_LOOP 0
#define DATA_ALIGMENT 1

typedef void asm_func(uint64_t inner_loop_counter, uint64_t outer_loop_counter,
                      char *data);

#if DOUBLE_LOOP
extern void double_loop_read(uint64_t inner_loop_counter,
                             uint64_t outer_loop_counter, char *data);
#endif

#if DATA_ALIGMENT
extern void read_align(uint64_t inner_loop_counter, uint64_t outer_loop_counter,
                       char *data);

extern void read_unalign(uint64_t inner_loop_counter,
                         uint64_t outer_loop_counter, char *data);
#endif

struct test_function {
  char *name;
  asm_func *func;
};

struct test_function test_functions[] = {
#if DOUBLE_LOOP
    {"double_loop_read", double_loop_read},
#endif

#if DATA_ALIGMENT
    {"read_align", read_align},
    {"read_unalign", read_unalign},
#endif
};

int main(int argc, char **argv) {
  struct buffer buffer = allocate_buffer(1024 * 1024 * 1024);
  if (buffer.data == 0) {
    fprintf(stderr, "ERROR: failed to allocate buffer\n");
    return 1;
  }

  for (size_t i = 0; i < buffer.size; ++i) {
    buffer.data[i] = i;
  }

  uint64_t read_amount = 256;
  uint64_t test_count = 32;

  uint64_t read_delta = 1024;
  uint64_t read_size = 4096;
  uint64_t read_record[test_count] = {};
  for (uint64_t i = 0; i < test_count; ++i) {
    read_record[i] = read_size;

    if (((read_size - 1) & read_size) == 0) {
      read_delta *= 2;
    }

    read_size += read_delta;
  }

  uint64_t cpu_time_freq = estimate_cpu_freq();

  size_t test_func_count = sizeof(test_functions) / sizeof(*test_functions);
  struct repetition_tester testers[test_count][test_func_count] = {};

  for (uint64_t i = 0; i < test_count; i++) {
    for (int test_index = 0; test_index < test_func_count; ++test_index) {
      struct repetition_tester *tester = &testers[i][test_index];
      struct test_function test_func = test_functions[test_index];
      uint64_t read_size = read_record[i];

      uint64_t outer_loop_count = buffer.size / read_size;
      uint64_t inner_loop_count = read_size / read_amount;
      uint64_t total_size = outer_loop_count * read_size;

      printf("\n--- %s | %ld KB ---\n", test_func.name, read_size / 1024);

      new_test_wave(tester, total_size, cpu_time_freq, 1);

      while (is_testing(tester)) {
        begin_time(tester);
        test_func.func(inner_loop_count, outer_loop_count, buffer.data);
        end_time(tester);
        count_bytes(tester, total_size);
      }
    }
  }

  for (int test_index = 0; test_index < test_func_count; ++test_index) {
    struct test_function test_func = test_functions[test_index];
    printf("func: %s\n", test_func.name);
    printf("size,gb/s\n");
    for (uint64_t i = 0; i < test_count; i++) {
      struct repetition_tester *tester = &testers[i][test_index];
      uint64_t read_size = read_record[i];

      double seconds = seconds_from_cpu_time(
          tester->results.min.E[RepValue_CPUTimer], cpu_time_freq);

      double gigabyte = 1024.0f * 1024.0f * 1024.0f;
      double bandwidth =
          tester->results.min.E[RepValue_ByteCount] / (gigabyte * seconds);

      printf("%lu,%f\n", read_size, bandwidth);
    }
  }
}
