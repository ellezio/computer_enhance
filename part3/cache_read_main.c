#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define BUFFER_ALLOC_MMAP 1

#include "buffer.c"
#include "platform.c"
#include "repetition_tester.c"

extern void double_loop_read(uint64_t inner_loop_counter,
                             uint64_t outer_loop_counter, char *data);

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
  uint64_t test_count = 64;
  uint64_t read_delta = 1024;
  uint64_t read_size = 4096;

  uint64_t cpu_time_freq = estimate_cpu_freq();

  struct repetition_tester testers[test_count] = {};

  for (;;) {
    for (uint64_t i = 0; i < test_count; i++) {
      struct repetition_tester *tester = &testers[i];

      uint64_t outer_loop_count = buffer.size / read_size;
      uint64_t inner_loop_count = read_size / read_amount;
      uint64_t total_size = outer_loop_count * read_size;

      printf("\n--- %s | %ld KB ---\n", "double_loop_read", read_size);

      new_test_wave(tester, total_size, cpu_time_freq, 1);

      while (is_testing(tester)) {
        begin_time(tester);
        double_loop_read(inner_loop_count, outer_loop_count, buffer.data);
        end_time(tester);
        count_bytes(tester, total_size);
      }

      if (((read_size - 1) & read_size) == 0) {
        read_delta *= 2;
      }

      read_size += read_delta;
    }
  }
}
