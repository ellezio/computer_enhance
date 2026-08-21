#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct buffer {
  size_t size;
  char *data;
};

#define CONSTANT_STRING(String)                                                \
  (struct buffer) { sizeof(String) - 1, (char *)String }

bool buffer_is_equal(struct buffer b1, struct buffer b2) {
  if (b1.size != b2.size) {
    return false;
  }

  for (int i = 0; i < b1.size; ++i) {
    if (b1.data[i] != b2.data[i]) {
      return false;
    }
  }

  return true;
}

struct buffer allocate_buffer(size_t size) {
  struct buffer buffer = {};
  buffer.data = malloc(size);
  if (buffer.data) {
    buffer.size = size;
  } else {
    fprintf(stderr, "ERROR: failed to allocate %ld bytes\n", size);
  }

  return buffer;
}

void free_buffer(struct buffer *buffer) {
  if (buffer->data) {
    free(buffer->data);
  }

  *buffer = (struct buffer){};
}
