#include "instruction.h"
#include <stdint.h>
#include <stdio.h>

char reg_names[36][3] = {
    "al", "ah", "ax", "bl", "bh", "bx", "cl", "ch", "cx", "dl", "dh", "dx",
    "sp", "sp", "sp", "bp", "bp", "bp", "si", "si", "si", "di", "di", "di",
    "es", "es", "es", "cs", "cs", "cs", "ss", "ss", "ss", "ds", "ds", "ds",
};

char ea_names[8][8] = {
    "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx",
};

char *inst_get_name(struct instruction *inst) {
  switch (inst->type) {
  case INST_MOV:
    return "mov";
  case INST_ADD:
    return "add";
  case INST_SUB:
    return "sub";
  case INST_CMP:
    return "cmp";
  case INST_JO:
    return "jo";
  case INST_JNO:
    return "jno";
  case INST_JB:
    return "jb";
  case INST_JAE:
    return "jae";
  case INST_JZ:
    return "jz";
  case INST_JNZ:
    return "jnz";
  case INST_JBE:
    return "jbe";
  case INST_JA:
    return "ja";
  case INST_JS:
    return "js";
  case INST_JNS:
    return "jns";
  case INST_JP:
    return "jp";
  case INST_JPO:
    return "jpo";
  case INST_JL:
    return "jl";
  case INST_JGE:
    return "jge";
  case INST_JLE:
    return "jle";
  case INST_JG:
    return "jg";
  case INST_JMP:
    return "jmp";
  case INST_LOOP:
    return "loop";
  case INST_LOOPZ:
    return "loopz";
  case INST_LOOPNZ:
    return "loopnz";
  case INST_JCXZ:
    return "jcxz";
  case INST_PUSH:
    return "push";
  case INST_NOT_USED:
    return "not used";
  case INST_POP:
    return "pop";
  case INST_XCHG:
    return "xchg";
  case INST_IN:
    return "in";
  case INST_OUT:
    return "out";
  }

  return "";
}

char *get_register_name(struct register_access register_) {
  uint8_t pos = register_.type;
  uint8_t offset = register_.byte;
  return reg_names[3 * (pos - 1) + offset];
}

void print_operand(struct operand op) {
  switch (op.type) {

  case Operand_None: {
  } break;

  case Operand_Register: {
    struct register_access reg = op.register_;

    printf("%s", get_register_name(reg));
  } break;

  case Operand_Memory: {
    struct effective_address ea = op.memory;

    printf("[");
    if (ea.type == EffectiveAddress_Direct) {
      printf("%d", ea.displacement);

    } else {
      printf("%s", ea_names[ea.type - 1]);

      if (ea.displacement != 0) {
        if (ea.displacement > 0) {
          printf(" +");
        }

        printf(" %d", ea.displacement);
      }
    }

    printf("]");
  } break;

  case Operand_Immediate: {
    int16_t imm = op.immediate;
    printf("%d", imm);
  } break;

  case Operand_RelativeImmediate: {
    int16_t addr = op.immediate + 2;

    printf("$");
    if (addr >= 0) {
      printf("+");
    }

    printf("%d", addr);
  } break;
  }
}

void print_instruction(struct instruction *inst) {
  printf("%s ", inst_get_name(inst));

  uint8_t has_size_prefix =
      inst->operand[0].type != Operand_Register &&
      inst->operand[0].type != Operand_RelativeImmediate &&
      inst->operand[1].type != Operand_Register;

  if (has_size_prefix) {
    printf((inst->flags & F_W) ? "word " : "byte ");
  }

  if (inst->operand[0].type != Operand_None) {
    print_operand(inst->operand[0]);
  }

  if (inst->operand[1].type != Operand_None) {
    printf(", ");
    print_operand(inst->operand[1]);
  }
}
