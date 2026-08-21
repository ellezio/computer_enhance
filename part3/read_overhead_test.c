#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

enum allocation_type {
  AllocType_none,
  AllocType_malloc,
  AllocType_count,
};

struct read_params {
  enum allocation_type alloc_type;
  struct buffer dest;
  const char *filename;
};

typedef void read_overhead_test_func(struct repetition_tester *tester,
                                     struct read_params *params);

static char const *describe_allocation_type(enum allocation_type alloc_type) {
  char const *result;
  switch (alloc_type) {
  case AllocType_none: {
    result = "";
  } break;

  case AllocType_malloc: {
    result = "malloc";
  } break;

  default: {
    result = "UNKNOWN";
  } break;
  }

  return result;
}

static void handle_allocation(struct read_params *params,
                              struct buffer *buffer) {
  switch (params->alloc_type) {
  case AllocType_none:
    break;

  case AllocType_malloc: {
    *buffer = allocate_buffer(params->dest.size);
  }; break;

  default: {
    fprintf(stderr, "ERROR: Unrecognized allocation type\n");
  } break;
  }
}

static void handle_deallocation(struct read_params *params,
                                struct buffer *buffer) {
  switch (params->alloc_type) {
  case AllocType_none:
    break;

  case AllocType_malloc: {
    free_buffer(buffer);
  }; break;

  default: {
    fprintf(stderr, "ERROR: Unrecognized allocation type\n");
  } break;
  }
}

static void read_via_fread(struct repetition_tester *tester,
                           struct read_params *params) {
  while (is_testing(tester)) {
    FILE *file = fopen(params->filename, "rb");
    if (file != NULL) {
      struct buffer dest_buffer = params->dest;
      handle_allocation(params, &dest_buffer);

      begin_time(tester);
      int result = fread(dest_buffer.data, dest_buffer.size, 1, file);
      end_time(tester);

      if (result == 1) {
        count_bytes(tester, params->dest.size);
      } else {
        error(tester, "ERROR: fread failed");
      }

      handle_deallocation(params, &dest_buffer);
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
      struct buffer dest_buffer = params->dest;
      handle_allocation(params, &dest_buffer);

      uint64_t remaining = dest_buffer.size;
      uint8_t *dest = dest_buffer.data;
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

      handle_deallocation(params, &dest_buffer);
      close(fd);
    } else {
      error(tester, "ERROR: open failed");
    }
  }
}
