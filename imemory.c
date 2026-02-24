#include "imemory.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define MEM_INIT_SIZE 1024
#define MEM_GROW_SIZE 1024

int imem_init(imem_t *imem) {
  uint8_t *bytes = calloc(sizeof *bytes, MEM_INIT_SIZE);
  if (bytes == NULL) {
    return -1;
  }
  imem->bytes = bytes;
  imem->size = MEM_INIT_SIZE;
  return 0;
}

int imem_grow(imem_t *imem) {
  size_t new_size = imem->size + MEM_GROW_SIZE;
  imem->bytes = realloc(imem->bytes, new_size);
  if (imem->bytes == NULL) {
    return -1;
  }
  imem->size = new_size;
  return 0;
}

void imem_free(imem_t *imem) {
  free(imem->bytes);
  imem->bytes = NULL;
  imem->size = 0;
}

ssize_t imem_load_file(imem_t *mem, FILE *f) {
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

ssize_t imem_readn(imem_t *imem, size_t addr, uint8_t *buf, size_t n) {
  if (addr >= imem->size) {
    return -1;
  }

  if (addr + n > imem->size) {
    n = imem->size - addr;
  }

  int i = 0;
  for (; i < n; ++i) {
    buf[i] = imem->bytes[addr + i];
  }

  return i;
}
