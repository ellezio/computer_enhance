#ifndef CLOCKS_H
#define CLOCKS_H

#include "instruction.h"
#include <stdbool.h>
#include <stdint.h>

struct instruction_timing {
  uint16_t base_min;
  uint16_t base_max;
  uint16_t ea;
  uint16_t min;
  uint16_t max;
};

struct timing_state {
  bool jumpTaken;
};

struct instruction_timing get_timing(struct instruction *,
                                     struct timing_state *);

#endif // CLOCKS_H
