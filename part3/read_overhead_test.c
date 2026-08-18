#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

struct read_params {
  struct buffer dest;
  const char *filename;
};

typedef void read_overhead_test_func(struct repetition_tester *tester,
                                     struct read_params *params);

static void read_via_fread(struct repetition_tester *tester,
                           struct read_params *params) {
  while (is_testing(tester)) {
    FILE *file = fopen(params->filename, "rb");
    if (file != NULL) {
      begin_time(tester);
      int result = fread(params->dest.data, params->dest.size, 1, file);
      end_time(tester);

      if (result == 1) {
        count_bytes(tester, params->dest.size);
      } else {
        error(tester, "ERROR: fread failed");
      }

      fclose(file);
    } else {
      error(tester, "ERROR: fopen failed");
    }
  }
}

static void read_via_read(struct repetition_tester *tester,
                          struct read_params *params) {
  while (is_testing(tester)) {
    int fd = open(params->filename, O_RDONLY);
    if (fd > 0) {
      uint64_t remaining = params->dest.size;
      uint8_t *dest = params->dest.data;
      while (remaining > 0) {
        uint32_t read_size = INT32_MAX;
        if (read_size > remaining) {
          read_size = remaining;
        }

        begin_time(tester);
        ssize_t br = read(fd, dest, read_size);
        end_time(tester);

        if (br == read_size) {
          count_bytes(tester, br);
        } else {
          error(tester, "ERROR: read failed");
          break;
        }

        remaining -= read_size;
        dest += read_size;
      }
      close(fd);
    } else {
      error(tester, "ERROR: open failed");
    }
  }
}
