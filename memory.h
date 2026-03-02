#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int16_t mem_load_file(uint16_t offset, FILE *f);
uint16_t mem_read_word(uint16_t addr);
void mem_save_word(uint16_t addr, uint16_t value);
int16_t mem_readn(uint16_t offset, uint8_t *buf, size_t n);
void mem_dump(FILE *f);

#endif // MEMORY_H
