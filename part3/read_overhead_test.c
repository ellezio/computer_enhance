#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

enum allocation_type {
  AllocType_none,
  AllocType_malloc,
  AllocType_mmap,
  AllocType_mmapLargePages,

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

  case AllocType_mmap: {
    result = "mmap";
  } break;

  case AllocType_mmapLargePages: {
    result = "mmap (large)";
  } break;

  default: {
    result = "UNKNOWN";
  } break;
  }

  return result;
}

static void handle_allocation(struct repetition_tester *tester,
                              struct read_params *params,
                              struct buffer *buffer) {
  switch (params->alloc_type) {
  case AllocType_none:
    break;

  case AllocType_malloc: {
    *buffer = allocate_buffer(params->dest.size);
  }; break;

  case AllocType_mmap:
  case AllocType_mmapLargePages: {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;

    if (params->alloc_type == AllocType_mmapLargePages) {
      flags |= MAP_HUGETLB | MAP_HUGE_2MB;
    }

    uint8_t *alloc_data =
        mmap(NULL, params->dest.size, PROT_READ | PROT_WRITE, flags, -1, 0);

    if ((ssize_t)alloc_data >= 0) {
      buffer->size = params->dest.size;
      buffer->data = alloc_data;
    } else {
      error(tester, "allocation failed");
      printf("errno = %d\n", errno);
    }

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

  case AllocType_mmap:
  case AllocType_mmapLargePages: {
    size_t alloc_size = buffer->size;
    if (params->alloc_type == AllocType_mmapLargePages) {
      int64_t page_size = (1 << 21);
      alloc_size = (alloc_size + page_size - 1) & ~(page_size - 1);
    }
    munmap(buffer->data, alloc_size);
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
      handle_allocation(tester, params, &dest_buffer);

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
      handle_allocation(tester, params, &dest_buffer);

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

static void write_all_bytes(struct repetition_tester *tester,
                            struct read_params *params) {
  while (is_testing(tester)) {
    struct buffer buffer = params->dest;
    handle_allocation(tester, params, &buffer);

    begin_time(tester);
    for (uint64_t idx = 0; idx < buffer.size; ++idx) {
      buffer.data[idx] = (uint8_t)idx;
    }
    end_time(tester);

    count_bytes(tester, buffer.size);

    handle_deallocation(params, &buffer);
  }
}
