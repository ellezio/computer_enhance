#include "memory.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MEM_SIZE 65536
#define BUF_SIZE 1024

uint8_t memory[MEM_SIZE] = {0};

int16_t mem_load_file(uint16_t offset, FILE *f) {
  uint8_t buf[BUF_SIZE];
  size_t rb = 0;
  while ((rb = fread(buf, 1, BUF_SIZE, f)) != 0) {
    if (rb < 0) {
      return -1;
    }

    for (size_t i = 0; i < rb; ++i) {
      memory[offset + i] = buf[i];
    }

    offset = rb;
  }

  return offset;
}

uint16_t mem_read_word(uint16_t addr) {
  return memory[addr] | (memory[addr + 1] << 8);
}

void mem_save_word(uint16_t addr, uint16_t value) {
  printf("[0x%X] save: %d\n", addr, value);
  memory[addr] = value;
  memory[addr + 1] = value >> 8;
}

int16_t mem_readn(uint16_t offset, uint8_t *buf, size_t n) {
  if (offset + n > MEM_SIZE) {
    n = MEM_SIZE - offset;
  }

  int i = 0;
  for (; i < n; ++i) {
    buf[i] = memory[offset + i];
  }

  return i;
}

void mem_dump(FILE *f) { fwrite(memory, MEM_SIZE, 1, f); }
