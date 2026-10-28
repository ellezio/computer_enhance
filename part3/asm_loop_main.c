#include <stdint.h>
#include <stdio.h>

#include "buffer.c"
#include "metrics.c"
#include "repetition_tester.c"

#define NOP_LOOP 0
#define MULTINOP_LOOPS 0
#define TEST_COND_JUMP 1

#if NOP_LOOP
extern void MOVAllBytesASM(uint64_t size, char *data);
extern void NOPAllBytesASM(uint64_t size, char *data);
extern void CMPAllBytesASM(uint64_t size, char *data);
extern void DECAllBytesASM(uint64_t size, char *data);
#endif

#if MULTINOP_LOOPS
extern void NOP3x1AllBytes(uint64_t size, char *data);
extern void NOP1x3AllBytes(uint64_t size, char *data);
extern void NOP1x9AllBytes(uint64_t size, char *data);
#endif

#if TEST_COND_JUMP
extern void TESTJumpTakeJmp(uint64_t size, char *data);
extern void TESTJumpNotTakeJmp(uint64_t size, char *data);
#endif

typedef void asm_func(uint64_t size, char *data);

struct test_function {
  char *name;
  asm_func *func;
};

struct test_function test_functions[] = {
#if NOP_LOOP
    {"MOVAllBytes", MOVAllBytes},
    {"NOPAllBytes", NOPAllBytes},
    {"CMPAllBytes", CMPAllBytes},
    {"DECAllBytes", DECAllBytes},
#endif

#if MULTINOP_LOOPS
    {"NOP3x1AllBytes", NOP3x1AllBytes},
    {"NOP1x3AllBytes", NOP1x3AllBytes},
    {"NOP1x9AllBytes", NOP1x9AllBytes},
#endif

#if TEST_COND_JUMP
    {"TESTJumpTakeJmp", TESTJumpTakeJmp},
    {"TESTJumpNotTakeJmp", TESTJumpNotTakeJmp},
#endif
};

int main(int argc, char **argv) {
  struct buffer buffer = allocate_buffer(1024 * 1024 * 1024);
  if (buffer.data == 0) {
    fprintf(stderr, "ERROR: failed to allocate buffer\n");
    return 1;
  }

  size_t test_func_count = sizeof(test_functions) / sizeof(*test_functions);
  uint64_t cpu_time_freq = estimate_cpu_freq();

  struct repetition_tester testers[test_func_count] = {};

  for (;;) {
    for (int test_index = 0; test_index < test_func_count; ++test_index) {
      struct repetition_tester *tester = &testers[test_index];
      struct test_function test_func = test_functions[test_index];

      printf("\n--- %s ---\n", test_func.name);
      new_test_wave(tester, buffer.size, cpu_time_freq, 0);

      while (is_testing(tester)) {
        begin_time(tester);
        test_func.func(buffer.size, buffer.data);
        end_time(tester);
        count_bytes(tester, buffer.size);
      }
    }
  }
}
