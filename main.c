#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.c"
#include "display.c"
#include "execute.c"

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

  uint8_t buf[2];
  unsigned long rb = 0;

  while ((fetch(buf, 1, fd)) != 0) {
    struct instruction instruction = decode_instruction(buf[0], fd);

    print_instruction(&instruction);

    if (execute) {
      execute_instruction(&instruction);
    }

    printf("\n");
  }

  if (execute) {
    print_registers_state();
  }

  if (fclose(fd) != 0) {
    fprintf(stderr, "Failed to close file. errno = %d\n", errno);
    exit(1);
  }

  return 0;
}
