#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>
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

struct measure_point {
  uint64_t hit_count;
  uint64_t exclusive_elapsed;
  uint64_t inclusive_elapsed;
  const char *label;
};

struct profiler {
  struct measure_point points[4096];
  uint64_t start;
  uint64_t end;
};
static struct profiler s_profiler = {};
uint64_t g_current_point_index = 0;

struct time_block {
  uint64_t old_elapsed;
  uint64_t start;
  size_t point_index;
  uint64_t parent_index;
};

struct time_block new_time_block(const char *label, uint64_t point_index) {
  struct measure_point *point = s_profiler.points + point_index;
  point->label = label;
  ++point->hit_count;

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
#define TIME_BLOCK(name)                                                       \
  struct time_block CONCAT(Block, __LINE__)                                    \
      __attribute__((cleanup(cleanup))) =                                      \
          new_time_block(name, __COUNTER__ + 1);

#define TIME_FUNCTION TIME_BLOCK(__func__)

void cleanup(struct time_block *tb) {
  uint64_t elapsed = read_cpu_timer() - tb->start;

  struct measure_point *parent = s_profiler.points + tb->parent_index;
  struct measure_point *point = s_profiler.points + tb->point_index;

  if (tb->point_index != tb->parent_index) {
    parent->exclusive_elapsed -= elapsed;
    point->exclusive_elapsed += elapsed;
    point->inclusive_elapsed = tb->old_elapsed + elapsed;
  }

  g_current_point_index = tb->parent_index;
}

void print_profiler() {
  double total = (double)(s_profiler.end - s_profiler.start) / 100.0;
  for (size_t i = 0;
       i < (sizeof(s_profiler.points) / sizeof(*s_profiler.points)); ++i) {
    struct measure_point *mp = s_profiler.points + i;
    if (mp->inclusive_elapsed) {
      printf("%s[%lu]: %lu (%.2f%%", mp->label, mp->hit_count,
             mp->inclusive_elapsed, (double)(mp->exclusive_elapsed) / total);

      if (mp->inclusive_elapsed != mp->exclusive_elapsed) {
        printf(", w/children %.2f%%", (double)mp->inclusive_elapsed / total);
      }

      printf(")\n");
    }
  }
}

void begin_profiling() { s_profiler.start = read_cpu_timer(); }
void end_profiling() {
  s_profiler.end = read_cpu_timer();
  print_profiler();
}
