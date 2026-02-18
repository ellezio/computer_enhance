#include "display.h"
#include "instruction.h"
#include <stdint.h>
#include <stdio.h>

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

enum op_flag : uint16_t { ZF = 0x20, SF = 0x40 };
uint16_t op_flags = 0;

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
}

void set_register(struct register_access reg, uint16_t value) {
  uint16_t *reg_ptr = &reg_table[reg.type - 1];
  uint16_t old_value = *reg_ptr;

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

  char *reg_name = get_register_name(reg);
  printf(" %s:0x%x->0x%x", reg_name, old_value, *reg_ptr);
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

uint16_t get_value(struct operand op) {
  switch (op.type) {

  case Operand_None: {
  } break;

  case Operand_Register: {
    return get_reg_value(op.register_);
  } break;

  case Operand_Memory: {
  } break;

  case Operand_Immediate:
    return op.immediate;

  case Operand_RelativeImmediate: {
  } break;
  }

  return 0;
}

void save_value(struct operand op, uint16_t value) {
  switch (op.type) {

  case Operand_None: {
  } break;

  case Operand_Register: {
    set_register(op.register_, value);
  } break;

  case Operand_Memory: {
  } break;

  case Operand_Immediate: {
  } break;

  case Operand_RelativeImmediate: {
  } break;
  }
}

void print_flags_change(uint16_t old_flags, uint16_t new_flags) {
  printf(" flags:");

  if (old_flags & ZF)
    printf("Z");

  if (old_flags & SF)
    printf("S");

  printf("->");

  if (new_flags & ZF)
    printf("Z");

  if (new_flags & SF)
    printf("S");
}

void arithmetic_operation(struct instruction *instruction,
                          struct operand destination, struct operand source) {
  uint16_t dv = get_value(destination);
  uint16_t sv = get_value(source);
  uint16_t v = 0;

  switch (instruction->type) {

  case INST_ADD: {
    v = dv + sv;
    save_value(destination, v);
  } break;

  case INST_SUB: {
    v = dv - sv;
    save_value(destination, v);
  } break;

  case INST_CMP: {
    v = dv - sv;
  } break;

  default:
    printf("unknown arithmetic operation");
    return;
  }

  uint16_t old_flags = op_flags;

  if (v == 0) {
    op_flags = op_flags | ZF;
  } else {
    op_flags = op_flags & ~ZF;
  }

  if (v & 0x8000) {
    op_flags = op_flags | SF;
  } else {
    op_flags = op_flags & ~SF;
  }

  print_flags_change(old_flags, op_flags);
}

void execute_instruction(struct instruction *instruction) {
  struct operand destination = instruction->operand[0];
  struct operand source = instruction->operand[1];

  printf(" ;");

  switch (instruction->type) {
  case INST_MOV: {
    save_value(destination, get_value(source));
  } break;

  case INST_ADD:
  case INST_SUB:
  case INST_CMP:
    arithmetic_operation(instruction, destination, source);
    break;

  default:
    printf("unsupported instruction\n");
    return;
  }
}
