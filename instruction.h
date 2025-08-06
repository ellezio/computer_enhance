#if !defined INST_H
#define INST_H

#include <stdint.h>

enum instruction_flag : uint16_t {
  F_NONE = 0,
  F_W = 1 << 0,
  F_D = 1 << 1,
  F_S = 1 << 2,
  F_V = 1 << 3,
  F_Z = 1 << 4,
  MOD = 1 << 5,
  REG = 1 << 6,
  RM = 1 << 7,
  SR = 1 << 8,
  DATA = 1 << 9,
  DATA8 = 1 << 10,
  ADDR = 1 << 11,
  RM_ALWAYS_W = 1 << 12,
};

enum instruction_type : uint8_t {
  INST_NOT_USED,
  INST_MOV,
  INST_ADD,
  INST_SUB,
  INST_CMP,
  INST_JO,
  INST_JNO,
  INST_JB,
  INST_JAE,
  INST_JZ,
  INST_JNZ,
  INST_JBE,
  INST_JA,
  INST_JS,
  INST_JNS,
  INST_JP,
  INST_JPO,
  INST_JL,
  INST_JGE,
  INST_JLE,
  INST_JG,
  INST_JMP,
  INST_LOOP,
  INST_LOOPZ,
  INST_LOOPNZ,
  INST_JCXZ,
  INST_PUSH,
  INST_POP,
  INST_XCHG,
  INST_IN,
  INST_OUT,
};

enum operand_type {
  Operand_None,
  Operand_Register,
  Operand_Memory,
  Operand_Immediate,
  Operand_RelativeImmediate,
};

enum register_type {
  Reg_None,

  Reg_A,
  Reg_B,
  Reg_C,
  Reg_D,
  Reg_SP,
  Reg_BP,
  Reg_SI,
  Reg_DI,

  Reg_ES,
  Reg_CS,
  Reg_SS,
  Reg_DS,
};

enum register_byte {
  RegByte_Low,
  RegByte_High,
  RegByte_All,
};

struct register_access {
  enum register_type type;
  enum register_byte byte;
};

enum effective_address_type {
  EffectiveAddress_Direct,

  EffectiveAddress_BX_SI,
  EffectiveAddress_BX_DI,
  EffectiveAddress_BP_SI,
  EffectiveAddress_BP_DI,

  EffectiveAddress_SI,
  EffectiveAddress_DI,
  EffectiveAddress_BP,
  EffectiveAddress_BX,
};

struct effective_address {
  enum effective_address_type type;
  int16_t displacement;
};

struct operand {
  enum operand_type type;
  union {
    struct register_access register_;
    struct effective_address memory;
    int16_t immediate;
  };
};

struct instruction {
  enum instruction_type type;
  enum instruction_flag flags;
  struct operand operand[2];
};

#endif
