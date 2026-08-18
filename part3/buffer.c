#include <stdbool.h>
#include <stddef.h>

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
