#include <stdint.h>
#include <stdio.h>

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

int main() {
  randctx ctx = {0};
  rand_init(&ctx, 76915237468);

  FILE *f = fopen("points.json", "w");
  if (f == NULL) {
    fprintf(stderr, "can't open \"points.json\"");
    return 1;
  }

  fprintf(f, "{\"pairs\": [\n");
  for (int i = 0; i < 100; ++i) {
    double lat0 = rand_in_range(&ctx, -90, 90);
    double lon0 = rand_in_range(&ctx, -180, 180);
    double lat1 = rand_in_range(&ctx, -90, 90);
    double lon1 = rand_in_range(&ctx, -180, 180);
    char *sep = ",\n";
    if (i == 99) {
      sep = "\n";
    }

    fprintf(
        f,
        "  {\"lat0\":%.16f, \"lon0\":%.16f, \"lat1\":%.16f, \"lon1\":%.16f}%s",
        lat0, lon0, lat1, lon1, sep);
  }
  fprintf(f, "]}\n");
  fclose(f);
}
