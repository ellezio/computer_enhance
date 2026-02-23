#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct mem {
  uint8_t *bytes;
  size_t size;
} mem_t;

int mem_init(mem_t *mem);
ssize_t mem_load_file(mem_t *mem, FILE *f);
void mem_free(mem_t *mem);
ssize_t mem_readn(mem_t *mem, size_t addr, uint8_t *buf, size_t n);

#endif // MEMORY_H
