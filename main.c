#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.c"
#include "display.c"
#include "execute.c"
#include "memory.c"
#include "memory.h"

int main(int argc, char **argv) {
  bool execute = false;
  char *fname = NULL;

  for (int arg_index = 1; arg_index < argc; ++arg_index) {
    char *arg = argv[arg_index];

    if (strcmp(arg, "--exec") == 0) {
      execute = true;
    } else {
      fname = arg;
      break;
    }
  }

  if (fname == NULL) {
    fprintf(stderr, "Missing path to file.\n");
    exit(1);
  }

  FILE *fd = fopen(fname, "rb");

  if (fd == 0) {
    fprintf(stderr, "Cannot open file \"%s\", errno = %d\n.", fname, errno);
    exit(1);
  }

  if (execute) {
    printf("--- execute: %s ---\n", fname);
  } else {
    printf("; %s disassembly:\n", fname);
    printf("bits 16\n");
  }

  mem_t mem = {0};
  mem_init(&mem);
  ssize_t cnt = mem_load_file(&mem, fd);
  if (cnt < 0) {
    printf("error while loading file\n");
  }

  size_t ip = 0;
  while (ip < cnt) {
    struct instruction instruction = {0};
    if (!decode_instruction(&mem, ip, &instruction)) {
      break;
    }

    print_instruction(&instruction);

    if (execute) {
      ip = execute_instruction(&instruction);
      if (ip < 0) {
        break;
      }
    } else {
      ip += instruction.bsize;
    }

    printf("\n");
  }

  mem_free(&mem);

  if (execute) {
    print_registers_state();
  }

  if (fclose(fd) != 0) {
    fprintf(stderr, "Failed to close file. errno = %d\n", errno);
    exit(1);
  }

  return 0;
}
