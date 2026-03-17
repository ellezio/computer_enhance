#include "clocks.h"
#include "instruction.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

uint16_t calc_ea(struct operand op) {
  switch (op.memory.type) {
  case EffectiveAddress_Direct:
    return 6;

  case EffectiveAddress_BP_DI:
  case EffectiveAddress_BX_SI:
    if (op.memory.displacement == 0) {
      return 7;
    }
    return 11;

  case EffectiveAddress_BP_SI:
  case EffectiveAddress_BX_DI:
    if (op.memory.displacement == 0) {
      return 8;
    }
    return 12;

  case EffectiveAddress_SI:
  case EffectiveAddress_DI:
  case EffectiveAddress_BP:
  case EffectiveAddress_BX:
    if (op.memory.displacement == 0) {
      return 5;
    }
    return 9;
  }
}

void update_timing(struct instruction_timing *state, uint16_t base_min,
                   uint16_t base_max, uint16_t ea) {
  state->base_min = base_min;
  state->base_max = base_max;
  state->ea = ea;
  state->min = base_min + ea;
  state->max = base_max + ea;
}

struct instruction_timing get_timing(struct instruction *instruction,
                                     struct timing_state *state) {
  struct instruction_timing result = {0};

  struct operand op_dst = instruction->operand[0];
  struct operand op_src = instruction->operand[1];

  bool isSrcRegister = op_src.type == Operand_Register;
  bool isDstRegister = op_dst.type == Operand_Register;
  bool isSrcMemory = op_src.type == Operand_Memory;
  bool isDstMemory = op_dst.type == Operand_Memory;
  bool isSrcImmediate = op_src.type == Operand_Immediate;

  uint16_t ea = 0;
  if (isDstMemory)
    ea = calc_ea(op_dst);
  if (isSrcMemory)
    ea = calc_ea(op_src);

  switch (instruction->type) {
  case INST_MOV: {
    if (isDstRegister && isSrcRegister) {
      update_timing(&result, 2, 2, ea);
    }

    if (isDstRegister && isSrcMemory) {
      update_timing(&result, 8, 8, ea);
    }

    if (isDstMemory && isSrcRegister) {
      update_timing(&result, 9, 9, ea);
    }

    if (isDstRegister && isSrcImmediate) {
      update_timing(&result, 4, 4, ea);
    }

    if (isDstMemory && isSrcImmediate) {
      update_timing(&result, 10, 10, ea);
    }
  } break;

  case INST_ADD:
  case INST_SUB: {
    if (isDstRegister && isSrcRegister) {
      update_timing(&result, 3, 3, ea);
    }

    if (isDstRegister && isSrcMemory) {
      update_timing(&result, 9, 9, ea);
    }

    if (isDstMemory && isSrcRegister) {
      update_timing(&result, 16, 16, ea);
    }

    if (isDstRegister && isSrcImmediate) {
      update_timing(&result, 4, 4, ea);
    }

    if (isDstMemory && isSrcImmediate) {
      update_timing(&result, 17, 17, ea);
    }
  } break;

  case INST_CMP: {
    if (isDstRegister && isSrcRegister) {
      update_timing(&result, 3, 3, ea);
    }

    if (isDstRegister && isSrcMemory) {
      update_timing(&result, 9, 9, ea);
    }

    if (isDstMemory && isSrcRegister) {
      update_timing(&result, 9, 9, ea);
    }

    if (isDstRegister && isSrcImmediate) {
      update_timing(&result, 4, 4, ea);
    }

    if (isDstMemory && isSrcImmediate) {
      update_timing(&result, 10, 10, ea);
    }
  } break;

  case INST_JNZ: {
    uint16_t v = state->jumpTaken ? 16 : 4;
    update_timing(&result, v, v, ea);
  } break;

  default:
    break;
  }

  return result;
}
