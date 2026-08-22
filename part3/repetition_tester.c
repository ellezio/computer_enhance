#include <stdint.h>
#include <stdio.h>

enum test_mode : uint32_t {
  TestMode_Uninitialized,
  TestMode_Testing,
  TestMode_Completed,
  TestMode_Error,
};

enum repetition_value_type {
  RepValue_TestCount,

  RepValue_CPUTimer,
  RepValue_MemPageFaults,
  RepValue_ByteCount,

  RepValue_Count,
};

struct repetition_value {
  uint64_t E[RepValue_Count];
};

struct repetition_tester_results {
  struct repetition_value total;
  struct repetition_value min;
  struct repetition_value max;
};

struct repetition_tester {
  uint64_t target_processed_byte_count;
  uint64_t cpu_timer_freq;
  uint64_t try_for_time;
  uint64_t test_started_at;

  enum test_mode mode;
  bool print_new_minimums;
  uint32_t open_block_count;
  uint32_t close_block_count;

  struct repetition_value accumulated_on_this_test;
  struct repetition_tester_results results;
};

static double seconds_from_cpu_time(double cpu_time, uint64_t cpu_timer_freq) {
  double result = 0.0;

  if (cpu_timer_freq) {
    result = cpu_time / (double)cpu_timer_freq;
  }

  return result;
}

static void print_time(char const *label, double cpu_time,
                       uint64_t cpu_timer_freq, uint64_t byte_count) {
  printf("%s: %.0f", label, cpu_time);
  if (cpu_timer_freq) {
    double seconds = seconds_from_cpu_time(cpu_time, cpu_timer_freq);
    printf(" (%fms)", 1000.0f * seconds);

    if (byte_count) {
      double gigabyte = 1024.0f * 1024.0f * 1024.0f;
      double bandwidth = byte_count / (gigabyte * seconds);
      printf(" %fgb/s", bandwidth);
    }
  }

  fflush(stdout);
}

static void print_value(char const *label, struct repetition_value value,
                        uint64_t cpu_timer_freq) {
  uint64_t test_count = value.E[RepValue_TestCount];
  double divisor = test_count ? (double)test_count : 1;

  double E[RepValue_Count] = {};
  for (uint32_t E_index = 0; E_index < RepValue_Count; ++E_index) {
    E[E_index] = (double)value.E[E_index] / divisor;
  }

  printf("%s: %.0f", label, E[RepValue_CPUTimer]);
  if (cpu_timer_freq) {
    double seconds =
        seconds_from_cpu_time(E[RepValue_CPUTimer], cpu_timer_freq);
    printf("(%fms)", 1000.0f * seconds);

    if (E[RepValue_ByteCount] > 0) {
      double gigabyte = 1024.0f * 1024.0f * 1024.0f;
      double bandwidth = E[RepValue_ByteCount] / (gigabyte * seconds);
      printf(" %fgb/s", bandwidth);
    }
  }

  if (E[RepValue_MemPageFaults] > 0) {
    printf(" PF: %0.4f (%0.4fk/fault)", E[RepValue_MemPageFaults],
           E[RepValue_ByteCount] / (E[RepValue_MemPageFaults] * 1024.0));
  }
}

static void print_results(struct repetition_tester_results results,
                          uint64_t cpu_timer_freq) {
  print_value("Min", results.min, cpu_timer_freq);
  printf("\n");

  print_value("Max", results.max, cpu_timer_freq);
  printf("\n");

  print_value("Avg", results.total, cpu_timer_freq);
  printf("\n");
}

static void error(struct repetition_tester *tester, char const *message) {
  tester->mode = TestMode_Error;
  fprintf(stderr, "ERROR: %s\n", message);
}

static void new_test_wave(struct repetition_tester *tester,
                          uint64_t target_processed_byte_count,
                          uint64_t cpu_timer_freq, uint64_t seconds_to_try) {
  if (seconds_to_try == 0) {
    seconds_to_try = 10;
  }

  if (tester->mode == TestMode_Uninitialized) {
    tester->mode = TestMode_Testing;
    tester->target_processed_byte_count = target_processed_byte_count;
    tester->cpu_timer_freq = cpu_timer_freq;
    tester->print_new_minimums = true;
    tester->results.min.E[RepValue_CPUTimer] = (uint64_t)-1;
  } else if (tester->mode == TestMode_Completed) {
    tester->mode = TestMode_Testing;

    if (tester->target_processed_byte_count != target_processed_byte_count) {
      error(tester, "target_processed_byte_count changed");
    }

    if (tester->cpu_timer_freq != cpu_timer_freq) {
      error(tester, "cpu_timer_freq changed");
    }
  }

  tester->try_for_time = seconds_to_try * cpu_timer_freq;
  tester->test_started_at = read_cpu_timer();
}

static void begin_time(struct repetition_tester *tester) {
  ++tester->open_block_count;
  tester->accumulated_on_this_test.E[RepValue_MemPageFaults] -=
      read_os_page_fault_count();
  tester->accumulated_on_this_test.E[RepValue_CPUTimer] -= read_cpu_timer();
}

static void end_time(struct repetition_tester *tester) {
  ++tester->close_block_count;
  tester->accumulated_on_this_test.E[RepValue_MemPageFaults] +=
      read_os_page_fault_count();
  tester->accumulated_on_this_test.E[RepValue_CPUTimer] += read_cpu_timer();
}

static void count_bytes(struct repetition_tester *tester, uint64_t byte_count) {
  tester->accumulated_on_this_test.E[RepValue_ByteCount] += byte_count;
}

static bool is_testing(struct repetition_tester *tester) {
  if (tester->mode == TestMode_Testing) {
    uint64_t current_time = read_cpu_timer();

    if (tester->open_block_count) {
      if (tester->open_block_count != tester->close_block_count) {
        error(tester, "unbalanced begin_time/end_time");
      }

      if (tester->accumulated_on_this_test.E[RepValue_ByteCount] !=
          tester->target_processed_byte_count) {
        error(tester, "processed byte count mismach");
      }

      if (tester->mode == TestMode_Testing) {
        struct repetition_tester_results *results = &tester->results;

        tester->accumulated_on_this_test.E[RepValue_TestCount] = 1;
        for (uint32_t E_index = 0; E_index < RepValue_Count; ++E_index) {
          tester->results.total.E[E_index] +=
              tester->accumulated_on_this_test.E[E_index];
        }

        uint64_t elapsed_time =
            tester->accumulated_on_this_test.E[RepValue_CPUTimer];

        if (results->max.E[RepValue_CPUTimer] < elapsed_time) {
          results->max = tester->accumulated_on_this_test;
        }

        if (results->min.E[RepValue_CPUTimer] > elapsed_time) {
          results->min = tester->accumulated_on_this_test;

          tester->test_started_at = current_time;

          if (tester->print_new_minimums) {
            print_time("Min", (double)elapsed_time, tester->cpu_timer_freq,
                       tester->target_processed_byte_count);
            printf("               \r");
          }
        }

        tester->open_block_count = 0;
        tester->close_block_count = 0;
        tester->accumulated_on_this_test = (struct repetition_value){};
      }
    }

    if ((current_time - tester->test_started_at) > tester->try_for_time) {
      tester->mode = TestMode_Completed;

      printf("                                                          \r");
      print_results(tester->results, tester->cpu_timer_freq);
    }
  }

  bool result = tester->mode == TestMode_Testing;
  return result;
}
