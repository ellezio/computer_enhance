/*
 * memory for instructions
 */

#ifndef IMEMORY_H
#define IMEMORY_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct imem {
  uint8_t *bytes;
  size_t size;
} imem_t;

int imem_init(imem_t *imem);
ssize_t imem_load_file(imem_t *imem, FILE *f);
void imem_free(imem_t *imem);
ssize_t imem_readn(imem_t *imem, size_t addr, uint8_t *buf, size_t n);

#endif // IMEMORY_H
