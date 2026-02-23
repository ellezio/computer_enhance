#include "display.h"
#include "instruction.h"
#include <stddef.h>
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
uint16_t original_reg_table[8] = {0};

enum op_flag : uint16_t { ZF = 0x20, SF = 0x40 };
uint16_t op_flags = 0;
uint16_t original_op_flags = 0;

uint16_t ip = 0;
uint16_t original_ip = 0;

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

void print_executinon_change() {
  printf(" ;");

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
  case Operand_RelativeImmediate:
    return op.immediate;
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

int arithmetic_operation(struct instruction *instruction,
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
    return -1;
  }

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

  return 0;
}

int handle_jump(struct instruction *instruction) {
  switch (instruction->type) {
  case INST_JNZ: {
    if ((op_flags & ZF) == 0) {
      int16_t addr = get_value(instruction->operand[0]);
      ip += addr;
    }
  } break;

  default:
    printf("unknown jump operation");
    return -1;
  }

  return 0;
}

void update_state() {
  for (int i = 0; i < 8; ++i) {
    original_reg_table[i] = reg_table[i];
  }

  original_ip = ip;
  original_op_flags = op_flags;
}

ssize_t execute_instruction(struct instruction *instruction) {
  struct operand destination = instruction->operand[0];
  struct operand source = instruction->operand[1];

  ip += instruction->bsize;

  switch (instruction->type) {
  case INST_MOV: {
    save_value(destination, get_value(source));
  } break;

  case INST_ADD:
  case INST_SUB:
  case INST_CMP:
    if (arithmetic_operation(instruction, destination, source) < 0) {
      return -1;
    };
    break;

  case INST_JNZ:
    if (handle_jump(instruction) < 0) {
      return -1;
    }
    break;

  default:
    printf("unsupported instruction\n");
    return -1;
  }

  print_executinon_change();
  update_state();
  return ip;
}
