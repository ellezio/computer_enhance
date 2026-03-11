#ifndef EXECUTE_H
#define EXECUTE_H

#include "clocks.h"
#include <stdbool.h>
#include <stdint.h>

struct execute_result {
  uint16_t next_ip;
  bool unimplemented;
  struct instruction_timing timing;
};

struct execute_result execute_instruction(struct instruction *instruction);

#endif // !EXECUTE_H
