#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

struct haversine_pair {
  double lat0;
  double lon0;
  double lat1;
  double lon1;
};

#include "metrics.c"

#include "buffer.c"
#include "haversine_formula.c"
#include "json_parse.c"

struct prof {
  uint64_t startup;
  uint64_t read;
  uint64_t misc_setup;
  uint64_t parse;
  uint64_t sum;
  uint64_t misc_output;
};

void prof_print(struct prof prof) {
  double total = prof.startup + prof.read + prof.misc_setup + prof.parse +
                 prof.sum + prof.misc_output;

  uint64_t cpu_freq = estimate_cpu_freq();
  double time_elapsed = total / cpu_freq;
  printf("Total time: %.4fms (CPU freq %lu)\n", time_elapsed, cpu_freq);

  total /= 100;
  printf(" Startup: %lu (%.2f%%)\n", prof.startup,
         (double)prof.startup / total);
  printf(" Read: %lu (%.2f%%)\n", prof.read, (double)prof.read / total);
  printf(" MiscSetup: %lu (%.2f%%)\n", prof.misc_setup,
         (double)prof.misc_setup / total);
  printf(" Parse: %lu (%.2f%%)\n", prof.parse, (double)prof.parse / total);
  printf(" Sum: %lu (%.2f%%)\n", prof.sum, (double)prof.sum / total);
  printf(" MiscOutput: %lu (%.2f%%)\n", prof.misc_output,
         (double)prof.misc_output / total);
}

struct buffer read_file(char *filename) {
  TIME_FUNCTION

  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    fprintf(stderr, "ERROR: failed to open file %s", filename);
    return (struct buffer){};
  }
  struct stat stat_;
  stat(filename, &stat_);

  struct buffer buffer = {};
  buffer.data = malloc(stat_.st_size);
  if (buffer.data == NULL) {
    fprintf(stderr, "ERROR: failed to allocate %ld bytes", stat_.st_size);
    return (struct buffer){};
  }
  buffer.size = stat_.st_size;

  if (fread(buffer.data, buffer.size, 1, file) != 1) {
    fprintf(stderr, "ERROR: failed to read file");
    free(buffer.data);
    buffer = (struct buffer){};
  }

  fclose(file);

  return buffer;
}

double sum_haversine_distance(struct haversine_pair *pairs, uint64_t count) {
  TIME_FUNCTION

  double sum = 0;
  double sum_coef = 1.0 / count;
  double earth_radius = 6372.8;

  for (uint64_t i = 0; i < count; ++i) {
    struct haversine_pair pair = pairs[i];
    uint64_t hav = reference_haversine(pair.lon0, pair.lat0, pair.lon1,
                                       pair.lat1, earth_radius);
    sum += hav * sum_coef;
  }

  return sum;
}

int main(int argc, char **args) {
  begin_profiling();

  struct prof prof = {};

  uint64_t start = read_cpu_timer();
  char *filename = args[1];
  uint64_t end = read_cpu_timer();
  prof.startup = end - start;

  start = read_cpu_timer();
  struct buffer json = read_file(filename);
  end = read_cpu_timer();
  prof.read = end - start;

  if (json.data == NULL) {
    return 1;
  }

  start = read_cpu_timer();
  int minimum_json_pair_encoding = 8 * 4;
  int maximum_pair_count = json.size / minimum_json_pair_encoding;
  struct haversine_pair *pairs = malloc(maximum_pair_count * sizeof(*pairs));
  end = read_cpu_timer();
  prof.misc_setup = end - start;

  start = read_cpu_timer();
  uint64_t pairs_count = parse_haversine_json_pairs(json, pairs);
  end = read_cpu_timer();
  prof.parse = end - start;

  start = read_cpu_timer();
  double sum = sum_haversine_distance(pairs, pairs_count);
  end = read_cpu_timer();
  prof.sum = end - start;

  // start = read_cpu_timer();
  // printf("input size: %lu\n", json.size);
  // printf("pair count: %lu\n", pairs_count);
  // printf("haversine sum: %.16f\n\n", sum);
  // end = read_cpu_timer();
  // prof.misc_output = end - start;
  //
  end_profiling();
  // printf("\n");
  // prof_print(prof);
}
