#include "memory.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define MEM_INIT_SIZE 1024
#define MEM_GROW_SIZE 1024

int mem_init(mem_t *mem) {
  uint8_t *bytes = calloc(sizeof *bytes, MEM_INIT_SIZE);
  if (bytes == NULL) {
    return -1;
  }
  mem->bytes = bytes;
  mem->size = MEM_INIT_SIZE;
  return 0;
}

int mem_grow(mem_t *mem) {
  size_t new_size = mem->size + MEM_GROW_SIZE;
  mem->bytes = realloc(mem->bytes, new_size);
  if (mem->bytes == NULL) {
    return -1;
  }
  mem->size = new_size;
  return 0;
}

void mem_free(mem_t *mem) {
  free(mem->bytes);
  mem->bytes = NULL;
  mem->size = 0;
}

ssize_t mem_load_file(mem_t *mem, FILE *f) {
  uint8_t buf[BUF_SIZE];
  size_t rb = 0, offset = 0;
  while ((rb = fread(buf, 1, BUF_SIZE, f)) != 0) {
    if (rb < 0) {
      return -1;
    }

    for (size_t i = 0; i < rb; ++i) {
      mem->bytes[offset + i] = buf[i];
    }

    offset = rb;
  }

  return offset;
}

ssize_t mem_readn(mem_t *mem, size_t addr, uint8_t *buf, size_t n) {
  if (addr >= mem->size) {
    return -1;
  }

  if (addr + n > mem->size) {
    n = mem->size - addr;
  }

  int i = 0;
  for (; i < n; ++i) {
    buf[i] = mem->bytes[addr + i];
  }

  return i;
}
