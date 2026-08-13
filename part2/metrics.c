#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <x86intrin.h>

#ifndef PROFILER
#define PROFILER 1
#endif

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

#if PROFILER
struct measure_point {
  uint64_t hit_count;
  uint64_t exclusive_elapsed;
  uint64_t inclusive_elapsed;
  uint64_t processed_byte_count;
  const char *label;
};

uint64_t g_current_point_index = 0;
static struct measure_point profiler_points[4096];

struct time_block {
  uint64_t old_elapsed;
  uint64_t start;
  size_t point_index;
  uint64_t parent_index;
};

struct time_block new_time_block(const char *label, uint64_t point_index,
                                 uint64_t byte_count) {
  struct measure_point *point = profiler_points + point_index;
  point->label = label;
  ++point->hit_count;
  point->processed_byte_count += byte_count;

  struct time_block tb = {
      .old_elapsed = point->inclusive_elapsed,
      .point_index = point_index,
      .parent_index = g_current_point_index,
  };
  g_current_point_index = point_index;

  tb.start = read_cpu_timer();
  return tb;
}

#define CONCAT2(a, b) a##b
#define CONCAT(a, b) CONCAT2(a, b)
#define TIME_BANDWIDTH(name, byte_count)                                       \
  struct time_block CONCAT(Block, __LINE__)                                    \
      __attribute__((cleanup(cleanup))) =                                      \
          new_time_block(name, __COUNTER__ + 1, byte_count);

void cleanup(struct time_block *tb) {
  uint64_t elapsed = read_cpu_timer() - tb->start;

  struct measure_point *parent = profiler_points + tb->parent_index;
  struct measure_point *point = profiler_points + tb->point_index;

  if (tb->point_index != tb->parent_index) {
    parent->exclusive_elapsed -= elapsed;
    point->exclusive_elapsed += elapsed;
    point->inclusive_elapsed = tb->old_elapsed + elapsed;
  }

  g_current_point_index = tb->parent_index;
}

void print_measure_points(uint64_t total_cpu_elapsed) {
  for (size_t i = 0; i < (sizeof(profiler_points) / sizeof(*profiler_points));
       ++i) {
    struct measure_point *mp = profiler_points + i;
    if (mp->inclusive_elapsed) {
      printf("  %s[%lu]: %lu (%.2f%%", mp->label, mp->hit_count,
             mp->inclusive_elapsed,
             100.0 * (double)(mp->exclusive_elapsed) / total_cpu_elapsed);

      if (mp->inclusive_elapsed != mp->exclusive_elapsed) {
        printf(", w/children %.2f%%",
               100.0 * (double)mp->inclusive_elapsed / total_cpu_elapsed);
      }

      printf(")");

      if (mp->processed_byte_count) {
        double megabyte = 1024.0 * 1024.0;
        double gigabyte = megabyte * 1024.0;

        double seconds =
            (double)mp->inclusive_elapsed / (double)estimate_cpu_freq();
        double byte_per_second = (double)mp->processed_byte_count / seconds;
        double megabytes = (double)mp->processed_byte_count / megabyte;
        double gigabytes_per_second = byte_per_second / gigabyte;

        printf(" %.3fmb at %.2fgb/s", megabytes, gigabytes_per_second);
      }

      printf("\n");
    }
  }
}

#else

#define TIME_BANDWIDTH(...)
#define print_measure_points(...)

#endif

struct profiler {
  uint64_t start;
  uint64_t end;
};
static struct profiler s_profiler = {};

#define TIME_BLOCK(name) TIME_BANDWIDTH(name, 0)
#define TIME_FUNCTION TIME_BLOCK(__func__)

void print_profiler() {
  uint64_t total_cpu_elapsed = s_profiler.end - s_profiler.start;
  uint64_t cpu_freq = estimate_cpu_freq();
  printf("Total time: %.4fms (CPU freq %lu)\n",
         1000.0 * total_cpu_elapsed / (double)cpu_freq, cpu_freq);

  print_measure_points(total_cpu_elapsed);
}

void begin_profiling() { s_profiler.start = read_cpu_timer(); }
void end_profiling() {
  s_profiler.end = read_cpu_timer();
  print_profiler();
}
