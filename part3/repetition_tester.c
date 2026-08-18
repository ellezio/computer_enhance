#include <stdint.h>
#include <stdio.h>

enum test_mode : uint32_t {
  TestMode_Uninitialized,
  TestMode_Testing,
  TestMode_Completed,
  TestMode_Error,
};

struct repetition_tester_results {
  uint64_t test_count;
  uint64_t total_time;
  uint64_t min_time;
  uint64_t max_time;
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
  uint64_t time_accumulated_on_this_test;
  uint64_t bytes_accumulated_on_this_test;

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

static void print_results(struct repetition_tester_results results,
                          uint64_t cpu_timer_freq, uint64_t byte_count) {
  print_time("Min", (double)results.min_time, cpu_timer_freq, byte_count);
  printf("\n");

  print_time("Max", (double)results.max_time, cpu_timer_freq, byte_count);
  printf("\n");

  if (results.test_count) {
    print_time("Avg", (double)results.total_time / (double)results.test_count,
               cpu_timer_freq, byte_count);
    printf("\n");
  }
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
    tester->results.min_time = (uint64_t)-1;
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
  tester->time_accumulated_on_this_test -= read_cpu_timer();
}

static void end_time(struct repetition_tester *tester) {
  ++tester->close_block_count;
  tester->time_accumulated_on_this_test += read_cpu_timer();
}

static void count_bytes(struct repetition_tester *tester, uint64_t byte_count) {
  tester->bytes_accumulated_on_this_test += byte_count;
}

static bool is_testing(struct repetition_tester *tester) {
  if (tester->mode == TestMode_Testing) {
    uint64_t current_time = read_cpu_timer();

    if (tester->open_block_count) {
      if (tester->open_block_count != tester->close_block_count) {
        error(tester, "unbalanced begin_time/end_time");
      }

      if (tester->bytes_accumulated_on_this_test !=
          tester->target_processed_byte_count) {
        error(tester, "processed byte count mismach");
      }

      if (tester->mode == TestMode_Testing) {
        struct repetition_tester_results *results = &tester->results;
        uint64_t elapsed_time = tester->time_accumulated_on_this_test;
        results->test_count += 1;
        results->total_time += elapsed_time;

        if (results->max_time < elapsed_time) {
          results->max_time = elapsed_time;
        }

        if (results->min_time > elapsed_time) {
          results->min_time = elapsed_time;

          tester->test_started_at = current_time;

          if (tester->print_new_minimums) {
            print_time("Min", (double)elapsed_time, tester->cpu_timer_freq,
                       tester->target_processed_byte_count);
            printf("               \r");
          }
        }

        tester->open_block_count = 0;
        tester->close_block_count = 0;
        tester->time_accumulated_on_this_test = 0;
        tester->bytes_accumulated_on_this_test = 0;
      }
    }

    if ((current_time - tester->test_started_at) > tester->try_for_time) {
      tester->mode = TestMode_Completed;

      printf("                                                          \r");
      print_results(tester->results, tester->cpu_timer_freq,
                    tester->target_processed_byte_count);
    }
  }

  bool result = tester->mode == TestMode_Testing;
  return result;
}
