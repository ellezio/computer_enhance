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
  printf(" ; %s:0x%x->0x%x", reg_name, old_value, *reg_ptr);
}

void execute_instruction(struct instruction *instruction) {
  struct operand destination = instruction->operand[0];
  struct operand source = instruction->operand[1];

  switch (destination.type) {

  case Operand_None: {
  } break;

  case Operand_Register: {
    set_register(destination.register_, source.immediate);
  } break;

  case Operand_Memory: {
  } break;

  case Operand_Immediate: {
  } break;

  case Operand_RelativeImmediate: {
  } break;
  }
}
