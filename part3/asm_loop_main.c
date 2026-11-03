#include <stdint.h>
#include <stdio.h>

#include "buffer.c"
#include "platform.c"
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
extern void ConditionalNOP(uint64_t size, char *data);
#endif

typedef void asm_func(uint64_t size, char *data);

struct test_function {
  char *name;
  asm_func *func;
};

struct test_function test_functions[] = {
#if NOP_LOOP
    {"MOVAllBytes", MOVAllBytes},       {"NOPAllBytes", NOPAllBytes},
    {"CMPAllBytes", CMPAllBytes},       {"DECAllBytes", DECAllBytes},
#endif

#if MULTINOP_LOOPS
    {"NOP3x1AllBytes", NOP3x1AllBytes}, {"NOP1x3AllBytes", NOP1x3AllBytes},
    {"NOP1x9AllBytes", NOP1x9AllBytes},
#endif

#if TEST_COND_JUMP
    {"ConditionalNOP", ConditionalNOP},
#endif
};

#if TEST_COND_JUMP
enum branch_pattern {
  BranchPattern_NeverTaken,
  BranchPattern_AlwaysTaken,
  BranchPattern_Every2,
  BranchPattern_Every3,
  BranchPattern_Every4,
  BranchPattern_CRTRandom,
  BranchPattern_OSRandom,

  BranchPattern_Count,
};

static char const *fill_with_branch_pattern(enum branch_pattern pattern,
                                            struct buffer buffer) {
  char const *pattern_name = "UNKNOWN";

  if (pattern == BranchPattern_OSRandom) {
    pattern_name = "OS Random";
    read_os_random_bytes(buffer);
  } else {
    for (int idx = 0; idx < buffer.size; idx++) {
      switch (pattern) {
      default:
        break;

      case BranchPattern_NeverTaken: {
        pattern_name = "Never Taken";
        buffer.data[idx] = 0;
      } break;

      case BranchPattern_AlwaysTaken: {
        pattern_name = "Always Taken";
        buffer.data[idx] = 1;
      } break;

      case BranchPattern_Every2: {
        pattern_name = "Every 2";
        buffer.data[idx] = (idx % 2) == 0;
      } break;

      case BranchPattern_Every3:
        pattern_name = "Every 3";
        buffer.data[idx] = (idx % 3) == 0;
        break;

      case BranchPattern_Every4:
        pattern_name = "Every 4";
        buffer.data[idx] = (idx % 4) == 0;
        break;

      case BranchPattern_CRTRandom:
        pattern_name = "CRT Random";
        buffer.data[idx] = (uint8_t)rand();
        break;
      }
    }
  }

  return pattern_name;
}
#else
enum branch_pattern {
  BranchPattern_None,
  BranchPattern_Count,
};

static char const *fill_with_branch_pattern(enum branch_pattern pattern,
                                            struct buffer buffer) {
  return 0;
}
#endif

int main(int argc, char **argv) {
  struct buffer buffer = allocate_buffer(1024 * 1024 * 1024);
  if (buffer.data == 0) {
    fprintf(stderr, "ERROR: failed to allocate buffer\n");
    return 1;
  }

  size_t test_func_count = sizeof(test_functions) / sizeof(*test_functions);
  uint64_t cpu_time_freq = estimate_cpu_freq();

  struct repetition_tester testers[BranchPattern_Count][test_func_count] = {};

  for (;;) {
    for (int test_index = 0; test_index < test_func_count; ++test_index) {
      for (int pattern_index = 0; pattern_index < BranchPattern_Count;
           pattern_index++) {
        struct repetition_tester *tester = &testers[pattern_index][test_index];
        struct test_function test_func = test_functions[test_index];

        printf("\n--- %s", test_func.name);

        char const *pattern_name =
            fill_with_branch_pattern(pattern_index, buffer);
        if (pattern_name != 0) {
          printf(", %s", pattern_name);
        }

        printf(" ---\n");

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
}
