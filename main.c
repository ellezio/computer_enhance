#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clocks.c"
#include "clocks.h"
#include "decode.c"
#include "display.c"
#include "execute.c"
#include "execute.h"
#include "memory.c"
#include "memory.h"

int main(int argc, char **argv) {
  bool execute = false;
  bool dump = false;
  char *fname = NULL;

  for (int arg_index = 1; arg_index < argc; ++arg_index) {
    char *arg = argv[arg_index];

    if (strcmp(arg, "--exec") == 0) {
      execute = true;
    } else if (strcmp(arg, "--dump") == 0) {
      dump = true;
    } else {
      fname = arg;
      break;
    }
  }

  if (fname == NULL) {
    fprintf(stderr, "Missing path to file.\n");
    exit(1);
  }

  if (execute) {
    printf("--- execute: %s ---\n", fname);
  } else {
    printf("; %s disassembly:\n", fname);
    printf("bits 16\n");
  }

  FILE *fd = fopen(fname, "rb");

  if (fd == 0) {
    fprintf(stderr, "Cannot open file \"%s\", errno = %d\n.", fname, errno);
    exit(1);
  }

  int16_t cnt = mem_load_file(0, fd);
  if (cnt < 0) {
    printf("error while loading file\n");
  }

  if (fclose(fd) != 0) {
    fprintf(stderr, "Failed to close file. errno = %d\n", errno);
    exit(1);
  }

  size_t ip = 0;
  while (ip < cnt) {
    struct instruction instruction = {0};
    if (!decode_instruction(ip, &instruction)) {
      break;
    }

    print_instruction(&instruction);

    if (execute) {
      struct execute_result exec_result = execute_instruction(&instruction);
      ip = exec_result.next_ip;
    } else {
      ip += instruction.bsize;
    }

    printf("\n");
  }

  if (execute) {
    print_registers_state();
  }

  if (dump) {
    FILE *fd = fopen("sim86_mem_dump.data", "wb");
    mem_dump(fd);
    fclose(fd);
  }

  return 0;
}
