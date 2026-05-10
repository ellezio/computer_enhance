#include "haversine_formula.c"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct randctx {
  uint64_t a;
  uint64_t b;
  uint64_t c;
  uint64_t d;
} randctx;

#define rot(x, k) (((x) << (k)) | ((x) >> (64 - (k))))

uint64_t rand_val(randctx *ctx) {
  uint64_t e = ctx->a - rot(ctx->b, 7);
  ctx->a = ctx->b ^ rot(ctx->c, 13);
  ctx->b = ctx->c + rot(ctx->d, 37);
  ctx->c = ctx->d + e;
  ctx->d = e + ctx->a;
  return ctx->d;
}

void rand_init(randctx *ctx, uint64_t seed) {
  uint64_t i;
  ctx->a = 0xf1ea5eed, ctx->b = ctx->c = ctx->d = seed;
  for (i = 0; i < 20; ++i) {
    (void)rand_val(ctx);
  }
}

double rand_in_range(randctx *ctx, double min, double max) {
  double t = (double)rand_val(ctx) / (double)UINT64_MAX;
  return (1.0 - t) * min + t * max;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s <seed> <count>\n", argv[0]);
    exit(1);
  }

  int seed = atoi(argv[1]);
  int count = atoi(argv[2]);

  randctx ctx = {0};
  rand_init(&ctx, seed);

  char fname[100];
  sprintf(fname, "%d-points.json", count);
  FILE *json_file = fopen(fname, "w");
  if (json_file == NULL) {
    fprintf(stderr, "can't open \"%d-points.json\"", count);
    return 1;
  }

  sprintf(fname, "%d-answer.f64", count);
  FILE *answer_file = fopen(fname, "w");
  if (answer_file == NULL) {
    fprintf(stderr, "can't open \"%d-answer.json\"", count);
    return 1;
  }

  double sum = 0;
  double sum_coef = 1.0 / count;

  fprintf(json_file, "{\"pairs\": [\n");
  for (int i = 0; i < count; ++i) {
    double lat0 = rand_in_range(&ctx, -90, 90);
    double lon0 = rand_in_range(&ctx, -180, 180);
    double lat1 = rand_in_range(&ctx, -90, 90);
    double lon1 = rand_in_range(&ctx, -180, 180);
    char *sep = ",\n";
    if (i == count - 1) {
      sep = "\n";
    }

    double earth_radius = 6372.8;
    double hav = reference_haversine(lon0, lat0, lon1, lat1, earth_radius);
    sum += hav * sum_coef;

    fprintf(json_file,
            "  {\"lat0\":%.16f, \"lon0\":%.16f, \"lat1\":%.16f, "
            "\"lon1\":%.16f}%s",
            lat0, lon0, lat1, lon1, sep);
  }
  fprintf(json_file, "]}\n");
  fwrite(&sum, sizeof(sum), 1, answer_file);

  fclose(json_file);
  fclose(answer_file);

  fprintf(stdout, "Random seed: %d\n", seed);
  fprintf(stdout, "Pair count: %d\n", count);
  fprintf(stdout, "Expected sum: %.16f\n", sum);
}
