#include "execute.h"
#include "clocks.h"
#include "display.h"
#include "instruction.h"
#include "memory.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

/*
0 - al, ah, ax
1 - bl, bh, bx
2 - cl, ch, cx
3 - dl, dh, dx
4 - sp
5 - bp
6 - si
7 - di
*/
uint16_t reg_table[8] = {0};
uint16_t original_reg_table[8] = {0};

enum op_flag : uint16_t { ZF = 0x20, SF = 0x40 };
uint16_t op_flags = 0;
uint16_t original_op_flags = 0;

uint16_t ip = 0;
uint16_t original_ip = 0;

uint16_t total_clocks = 0;

void print_registers_state() {
  printf("\nFinal registers:\n");

  printf("\tax: 0x%x (%d)\n", reg_table[0], reg_table[0]);
  printf("\tbx: 0x%x (%d)\n", reg_table[1], reg_table[1]);
  printf("\tcx: 0x%x (%d)\n", reg_table[2], reg_table[2]);
  printf("\tdx: 0x%x (%d)\n", reg_table[3], reg_table[3]);
  printf("\tsp: 0x%x (%d)\n", reg_table[4], reg_table[4]);
  printf("\tbp: 0x%x (%d)\n", reg_table[5], reg_table[5]);
  printf("\tsi: 0x%x (%d)\n", reg_table[6], reg_table[6]);
  printf("\tdi: 0x%x (%d)\n", reg_table[7], reg_table[7]);

  printf("\tip: 0x%x (%d)\n", ip, ip);
}

void print_executinon_change(struct instruction_timing *timing) {
  printf(" ;");

  if (timing != NULL) {
    // NOTE:
    // Range is omited because currently I do not implemented instruction that
    // have ranged clocks
    printf(" Clocks: +%d = %d", timing->min, total_clocks);
    if (timing->ea != 0)
      printf(" (%d + %dea)", timing->base_min, timing->ea);
    printf(" |");
  }

  for (int i = 0; i < 8; ++i) {
    if (original_reg_table[i] != reg_table[i]) {
      printf(" %s:0x%x->0x%x", reg_names[i * 3 + 2], original_reg_table[i],
             reg_table[i]);
    }
  }

  if (original_ip != ip) {
    printf(" ip:0x%x->0x%x", original_ip, ip);
  }

  if (original_op_flags != op_flags) {
    printf(" flags:");

    if (original_op_flags & ZF)
      printf("Z");

    if (original_op_flags & SF)
      printf("S");

    printf("->");

    if (op_flags & ZF)
      printf("Z");

    if (op_flags & SF)
      printf("S");
  }
}

void set_register(struct register_access reg, uint16_t value) {
  uint16_t *reg_ptr = &reg_table[reg.type - 1];

  switch (reg.byte) {
  case RegByte_Low: {
    *reg_ptr = (*reg_ptr & 0xFF00) | (0x00FF & value);
  } break;

  case RegByte_High: {
    *reg_ptr = (*reg_ptr & 0x00FF) | (0xFF00 & (value << 8));
  } break;

  case RegByte_All: {
    *reg_ptr = value;
  } break;
  }
}

uint16_t get_reg_value(struct register_access reg) {
  uint16_t value = reg_table[reg.type - 1];

  switch (reg.byte) {
  case RegByte_Low:
    return 0x00FF & value;

  case RegByte_High:
    return value & 0xFF00;

  case RegByte_All:
    return value;
  }
}

uint16_t get_memory_value(struct effective_address memaddr) {
  switch (memaddr.type) {
  case EffectiveAddress_Direct: {
    return mem_read_word(memaddr.displacement);
  }

  case EffectiveAddress_BX: {
    struct register_access bx_reg = {Reg_B, RegByte_All};
    uint16_t bx_val = get_reg_value(bx_reg);
    return mem_read_word(bx_val + memaddr.displacement);
  }

  case EffectiveAddress_BP_SI: {
    uint16_t bp_val =
        get_reg_value((struct register_access){Reg_BP, RegByte_All});
    uint16_t si_val =
        get_reg_value((struct register_access){Reg_SI, RegByte_All});

    return mem_read_word(bp_val + si_val);
  } break;

  default:
    break;
  }

  return 0;
}

uint16_t get_value(struct operand op) {
  switch (op.type) {

  case Operand_None: {
  } break;

  case Operand_Register: {
    return get_reg_value(op.register_);
  } break;

  case Operand_Memory: {
    return get_memory_value(op.memory);
  } break;

  case Operand_Immediate:
  case Operand_RelativeImmediate:
    return op.immediate;
  }

  return 0;
}

void store_to_memory(struct effective_address memaddr, uint16_t value) {
  switch (memaddr.type) {
  case EffectiveAddress_Direct: {
    mem_save_word(memaddr.displacement, value);
  } break;

  case EffectiveAddress_BX: {
    struct register_access bx_reg = {Reg_B, RegByte_All};
    uint16_t bx_val = get_reg_value(bx_reg);
    mem_save_word(bx_val + memaddr.displacement, value);
  } break;

  case EffectiveAddress_BP: {
    struct register_access bp_reg = {Reg_BP, RegByte_All};
    uint16_t bp_val = get_reg_value(bp_reg);
    mem_save_word(bp_val + memaddr.displacement, value);
  } break;

  case EffectiveAddress_BP_SI: {
    uint16_t bp_val =
        get_reg_value((struct register_access){Reg_BP, RegByte_All});
    uint16_t si_val =
        get_reg_value((struct register_access){Reg_SI, RegByte_All});

    mem_save_word(bp_val + si_val, value);
  } break;

  default:
    break;
  }
}

void save_value(struct operand op, uint16_t value) {
  switch (op.type) {

  case Operand_None: {
  } break;

  case Operand_Register: {
    set_register(op.register_, value);
  } break;

  case Operand_Memory: {
    store_to_memory(op.memory, value);
  } break;

  case Operand_Immediate: {
  } break;

  case Operand_RelativeImmediate: {
  } break;
  }
}

void update_state() {
  for (int i = 0; i < 8; ++i) {
    original_reg_table[i] = reg_table[i];
  }

  original_ip = ip;
  original_op_flags = op_flags;
}

void update_flags(uint16_t value, uint16_t width) {
  uint16_t signBit = width == 1 ? 1 << 7 : 1 << 15;

  op_flags = 0;
  op_flags |= value == 0 ? ZF : 0;
  op_flags |= value & signBit ? SF : 0;
}

uint16_t mask_value(uint16_t value, uint16_t width) {
  return value & (width == 1 ? 0xff : 0xffff);
}

struct execute_result execute_instruction(struct instruction *instruction) {
  struct execute_result result = {};
  struct timing_state state = {};
  struct operand destination = instruction->operand[0];
  struct operand source = instruction->operand[1];

  ip += instruction->bsize;
  result.next_ip = ip;

  uint16_t width = instruction->flags & F_W ? 2 : 1;
  uint16_t value_dst = get_value(destination);
  uint16_t value_src = get_value(source);

  switch (instruction->type) {
  case INST_MOV: {
    save_value(destination, value_src);
  } break;

  case INST_ADD: {
    uint16_t v = mask_value(
        mask_value(value_dst, width) + mask_value(value_src, width), width);

    update_flags(v, width);
    save_value(destination, v);
  } break;

  case INST_SUB: {
    uint16_t v = mask_value(
        mask_value(value_dst, width) - mask_value(value_src, width), width);

    update_flags(v, width);
    save_value(destination, v);
  } break;

  case INST_CMP: {
    uint16_t v = mask_value(
        mask_value(value_dst, width) - mask_value(value_src, width), width);

    update_flags(v, width);
  } break;

  case INST_JNZ: {
    if ((op_flags & ZF) == 0) {
      int16_t addr = get_value(instruction->operand[0]);
      ip += addr;
    }
  } break;

  default:
    printf("unsupported instruction\n");
    result.unimplemented = true;
    return result;
  }

  result.timing = get_timing(instruction, &state);
  total_clocks += result.timing.min;
  print_executinon_change(&result.timing);
  update_state();

  return result;
}
