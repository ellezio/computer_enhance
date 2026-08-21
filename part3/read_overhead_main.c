#include "buffer.c"
#include "metrics.c"
#include "repetition_tester.c"

#include "read_overhead_test.c"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

struct test_function {
  char *name;
  read_overhead_test_func *func;
};

struct test_function test_functions[] = {
    {"read", read_via_read},
    {"fread", read_via_fread},
};

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  char *filename = argv[1];

  struct stat stat_;
  stat(filename, &stat_);

  struct buffer buf = {};
  buf.data = malloc(stat_.st_size);

  if (buf.data == NULL) {
    fprintf(stderr, "ERROR: failed to allocate %ld bytes\n", stat_.st_size);
    return 1;
  }

  buf.size = stat_.st_size;

  uint64_t cpu_time_freq = estimate_cpu_freq();

  struct read_params params = {};
  params.dest = buf;
  params.filename = filename;

  int arr_count = sizeof(test_functions) / sizeof(*test_functions);

  struct repetition_tester testers[arr_count][AllocType_count] = {};

  for (;;) {
    for (int func_index = 0; func_index < arr_count; ++func_index) {
      for (int alloc_type = 0; alloc_type < AllocType_count; ++alloc_type) {
        struct repetition_tester *tester = &testers[func_index][alloc_type];
        struct test_function test_func = test_functions[func_index];
        params.alloc_type = alloc_type;

        printf("\n--- %s%s%s ---\n", describe_allocation_type(alloc_type),
               alloc_type ? " + " : "", test_func.name);
        new_test_wave(tester, params.dest.size, cpu_time_freq, 0);
        test_func.func(tester, &params);
      }
    }
  }

  return 0;
}
