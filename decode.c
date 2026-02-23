#include "decode.h"
#include "instruction.h"
#include "memory.h"
#include <stdbool.h>
#include <stdint.h>

#define MOD(bits) (0b##bits << 6)
#define REG(bits) (0b##bits << 3)
#define RM(bits) (0b##bits)
#define XXX(bits) (0b##bits << 3)
#define SR(bits) (0b##bits << 3)

// clang-format off
struct instruction_encoding instructions[256] = {
    // MOV - RM to/from REG
    [0x88] = {NOT_EXTENDED, INST_MOV, WORD, MOD | REG | RM},
    [0x89] = {NOT_EXTENDED, INST_MOV, WORD, F_W | MOD | REG | RM},
    [0x8A] = {NOT_EXTENDED, INST_MOV, WORD, F_D | MOD | REG | RM},
    [0x8B] = {NOT_EXTENDED, INST_MOV, WORD, F_D | F_W | MOD | REG | RM},

    // MOV - IMM to RM
    [0xC6] = {NOT_EXTENDED, INST_MOV, WORD, MOD | RM | DATA},
    [0xC7] = {NOT_EXTENDED, INST_MOV, WORD, F_W | MOD | RM | DATA},

    // MOV - IMM to REG
    [0xB0] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | REG | DATA, REG(000)},
    [0xB1] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | REG | DATA, REG(001)},
    [0xB2] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | REG | DATA, REG(010)},
    [0xB3] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | REG | DATA, REG(011)},
    [0xB4] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | REG | DATA, REG(100)},
    [0xB5] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | REG | DATA, REG(101)},
    [0xB6] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | REG | DATA, REG(110)},
    [0xB7] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | REG | DATA, REG(111)},
    [0xB8] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | F_W | REG | DATA, REG(000)},
    [0xB9] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | F_W | REG | DATA, REG(001)},
    [0xBA] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | F_W | REG | DATA, REG(010)},
    [0xBB] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | F_W | REG | DATA, REG(011)},
    [0xBC] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | F_W | REG | DATA, REG(100)},
    [0xBD] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | F_W | REG | DATA, REG(101)},
    [0xBE] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | F_W | REG | DATA, REG(110)},
    [0xBF] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | F_W | REG | DATA, REG(111)},

    // MOV - MEM to ACC
    [0xA0] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | REG | RM, REG(000) | RM(110)},
    [0xA1] = {NOT_EXTENDED, INST_MOV, BYTE, F_D | F_W | REG | RM, REG(000) | RM(110)},

    // MOV - ACC to MEM
    [0xA2] = {NOT_EXTENDED, INST_MOV, BYTE, REG | RM, REG(000) | RM(110)},
    [0xA3] = {NOT_EXTENDED, INST_MOV, BYTE, F_W | REG | RM, REG(000) | RM(110)},

    // MOV - RM to SR
    [0x8E] = {NOT_EXTENDED, INST_MOV, WORD, F_D | MOD | SR | RM},

    // MOV - SR to RM
    [0x8C] = {NOT_EXTENDED, INST_MOV, WORD, MOD | SR | RM},

    // PUSH - RM
    [0xFF] = {EXTENDED, .types = {[0x6] = INST_PUSH}, WORD, F_W | MOD | RM},

    // PUSH - REG
    [0x50] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | REG, REG(000)},
    [0x51] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | REG, REG(001)},
    [0x52] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | REG, REG(010)},
    [0x53] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | REG, REG(011)},
    [0x54] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | REG, REG(100)},
    [0x55] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | REG, REG(101)},
    [0x56] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | REG, REG(110)},
    [0x57] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | REG, REG(111)},

    // PUSH - SR
    [0x06] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | SR, SR(00)},
    [0x0E] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | SR, SR(01)},
    [0x16] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | SR, SR(10)},
    [0x1E] = {NOT_EXTENDED, INST_PUSH, BYTE, F_D | F_W | SR, SR(11)},

    // POP - RM
    [0x8F] = {EXTENDED, .types = {INST_POP}, WORD, F_W | MOD | RM},

    // POP - REG
    [0x58] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | REG, REG(000)},
    [0x59] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | REG, REG(001)},
    [0x5A] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | REG, REG(010)},
    [0x5B] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | REG, REG(011)},
    [0x5C] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | REG, REG(100)},
    [0x5D] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | REG, REG(101)},
    [0x5E] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | REG, REG(110)},
    [0x5F] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | REG, REG(111)},

    // POP - SR
    [0x07] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | SR, SR(00)},
    [0x0F] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | SR, SR(01)},
    [0x17] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | SR, SR(10)},
    [0x1F] = {NOT_EXTENDED, INST_POP, BYTE, F_D | F_W | SR, SR(11)},

    // XCHG - RM with REG
    [0x86] = {NOT_EXTENDED, INST_XCHG, WORD, MOD | REG | RM},
    [0x87] = {NOT_EXTENDED, INST_XCHG, WORD, F_W | MOD | REG | RM},

    // XCHG - REG with ACC
    [0x90] = {NOT_EXTENDED, INST_XCHG, BYTE, F_W | MOD | REG | RM, MOD(11) | REG(000) | RM(000)},
    [0x91] = {NOT_EXTENDED, INST_XCHG, BYTE, F_W | MOD | REG | RM, MOD(11) | REG(001) | RM(000)},
    [0x92] = {NOT_EXTENDED, INST_XCHG, BYTE, F_W | MOD | REG | RM, MOD(11) | REG(010) | RM(000)},
    [0x93] = {NOT_EXTENDED, INST_XCHG, BYTE, F_W | MOD | REG | RM, MOD(11) | REG(011) | RM(000)},
    [0x94] = {NOT_EXTENDED, INST_XCHG, BYTE, F_W | MOD | REG | RM, MOD(11) | REG(100) | RM(000)},
    [0x95] = {NOT_EXTENDED, INST_XCHG, BYTE, F_W | MOD | REG | RM, MOD(11) | REG(101) | RM(000)},
    [0x96] = {NOT_EXTENDED, INST_XCHG, BYTE, F_W | MOD | REG | RM, MOD(11) | REG(110) | RM(000)},
    [0x97] = {NOT_EXTENDED, INST_XCHG, BYTE, F_W | MOD | REG | RM, MOD(11) | REG(111) | RM(000)},

    // IN - Fixed port
    [0xE4] = {NOT_EXTENDED, INST_IN, BYTE, F_D | REG | DATA8, REG(000)},
    [0xE5] = {NOT_EXTENDED, INST_IN, BYTE, F_D | F_W | REG | DATA8, REG(000)},

    // IN - Variable port
    [0xEC] = {NOT_EXTENDED, INST_IN, BYTE, F_D | RM_ALWAYS_W | MOD | REG | RM, MOD(11) | REG(000) | RM(010)},
    [0xED] = {NOT_EXTENDED, INST_IN, BYTE, F_D | F_W | RM_ALWAYS_W | MOD | REG | RM, MOD(11) | REG(000) | RM(010)},

    // OUT - Fixed port
    [0xE6] = {NOT_EXTENDED, INST_OUT, BYTE, REG | DATA8, REG(000)},
    [0xE7] = {NOT_EXTENDED, INST_OUT, BYTE, F_W | REG | DATA8, REG(000)},

    // OUT - Variable port
    [0xEE] = {NOT_EXTENDED, INST_OUT, BYTE, RM_ALWAYS_W | MOD | REG | RM, MOD(11) | REG(000) | RM(010)},
    [0xEF] = {NOT_EXTENDED, INST_OUT, BYTE, F_W | RM_ALWAYS_W | MOD | REG | RM, MOD(11) | REG(000) | RM(010)},

    // ADD - RM with REG to either
    [0x00] = {NOT_EXTENDED, INST_ADD, WORD, MOD | REG | RM},
    [0x01] = {NOT_EXTENDED, INST_ADD, WORD, F_W | MOD | REG | RM},
    [0x02] = {NOT_EXTENDED, INST_ADD, WORD, F_D | MOD | REG | RM},
    [0x03] = {NOT_EXTENDED, INST_ADD, WORD, F_W | F_D | MOD | REG | RM},

    // ADD, SUB, CMP - IMM to RM
    [0x80] = {EXTENDED,
              .types = {INST_ADD, [0x5] = INST_SUB, [0x7] = INST_CMP},
              WORD, MOD | RM | DATA},
    [0x81] = {EXTENDED,
              .types = {INST_ADD, [0x5] = INST_SUB, [0x7] = INST_CMP},
              WORD, F_W | MOD | RM | DATA},
    [0x82] = {EXTENDED,
              .types = {INST_ADD, [0x5] = INST_SUB, [0x7] = INST_CMP},
              WORD, F_S | MOD | RM | DATA},
    [0x83] = {EXTENDED,
              .types = {INST_ADD, [0x5] = INST_SUB, [0x7] = INST_CMP},
              WORD, F_W | F_S | MOD | RM | DATA},

    // ADD - IMM to ACC
    [0x04] = {NOT_EXTENDED, INST_ADD, BYTE, F_D | REG | DATA, REG(000)},
    [0x05] = {NOT_EXTENDED, INST_ADD, BYTE, F_D | F_W | REG | DATA, REG(000)},

    // SUB - RM with REG to either
    [0x28] = {NOT_EXTENDED, INST_SUB, WORD, MOD | REG | RM},
    [0x29] = {NOT_EXTENDED, INST_SUB, WORD, F_W | MOD | REG | RM},
    [0x2A] = {NOT_EXTENDED, INST_SUB, WORD, F_D | MOD | REG | RM},
    [0x2B] = {NOT_EXTENDED, INST_SUB, WORD, F_W | F_D | MOD | REG | RM},

    // SUB - IMM to ACC
    [0x2C] = {NOT_EXTENDED, INST_SUB, BYTE, F_D | REG | DATA, REG(000)},
    [0x2D] = {NOT_EXTENDED, INST_SUB, BYTE, F_D | F_W | REG | DATA, REG(000)},

    // CMP - RM with REG to either
    [0x38] = {NOT_EXTENDED, INST_CMP, WORD, MOD | REG | RM},
    [0x39] = {NOT_EXTENDED, INST_CMP, WORD, F_W | MOD | REG | RM},
    [0x3A] = {NOT_EXTENDED, INST_CMP, WORD, F_D | MOD | REG | RM},
    [0x3B] = {NOT_EXTENDED, INST_CMP, WORD, F_W | F_D | MOD | REG | RM},

    // CMP - IMM to ACC
    [0x3C] = {NOT_EXTENDED, INST_CMP, BYTE, F_D | REG | DATA, REG(000)},
    [0x3D] = {NOT_EXTENDED, INST_CMP, BYTE, F_D | F_W | REG | DATA, REG(000)},

    // JMP / LOOP
    [0x70] = {NOT_EXTENDED, INST_JO, BYTE, ADDR},
    [0x71] = {NOT_EXTENDED, INST_JNO, BYTE, ADDR},
    [0x72] = {NOT_EXTENDED, INST_JB, BYTE, ADDR},
    [0x73] = {NOT_EXTENDED, INST_JAE, BYTE, ADDR},
    [0x74] = {NOT_EXTENDED, INST_JZ, BYTE, ADDR},
    [0x75] = {NOT_EXTENDED, INST_JNZ, BYTE, ADDR},
    [0x76] = {NOT_EXTENDED, INST_JBE, BYTE, ADDR},
    [0x77] = {NOT_EXTENDED, INST_JA, BYTE, ADDR},
    [0x78] = {NOT_EXTENDED, INST_JS, BYTE, ADDR},
    [0x79] = {NOT_EXTENDED, INST_JNS, BYTE, ADDR},
    [0x7A] = {NOT_EXTENDED, INST_JP, BYTE, ADDR},
    [0x7B] = {NOT_EXTENDED, INST_JPO, BYTE, ADDR},
    [0x7C] = {NOT_EXTENDED, INST_JL, BYTE, ADDR},
    [0x7D] = {NOT_EXTENDED, INST_JGE, BYTE, ADDR},
    [0x7E] = {NOT_EXTENDED, INST_JLE, BYTE, ADDR},
    [0x7F] = {NOT_EXTENDED, INST_JG, BYTE, ADDR},
    [0xE0] = {NOT_EXTENDED, INST_LOOPNZ, BYTE, ADDR},
    [0xE1] = {NOT_EXTENDED, INST_LOOPZ, BYTE, ADDR},
    [0xE2] = {NOT_EXTENDED, INST_LOOP, BYTE, ADDR},
    [0xE3] = {NOT_EXTENDED, INST_JCXZ, BYTE, ADDR},
};
// clang-format on

#undef MOD
#undef REG
#undef RM
#undef XXX
#undef SR

struct operand get_register_operand(uint8_t reg_index, uint8_t wide) {
  struct register_access reg_table[] = {
      {Reg_A, RegByte_Low},  {Reg_C, RegByte_Low},
      {Reg_D, RegByte_Low},  {Reg_B, RegByte_Low},

      {Reg_A, RegByte_High}, {Reg_C, RegByte_High},
      {Reg_D, RegByte_High}, {Reg_B, RegByte_High},

      {Reg_A, RegByte_All},  {Reg_C, RegByte_All},
      {Reg_D, RegByte_All},  {Reg_B, RegByte_All},

      {Reg_SP, RegByte_All}, {Reg_BP, RegByte_All},
      {Reg_SI, RegByte_All}, {Reg_DI, RegByte_All},
  };

  uint8_t offset = (wide == 0 ? 0 : 1) * 8;
  struct register_access reg = reg_table[reg_index + offset];
  struct operand op = {Operand_Register, reg};
  return op;
}

struct operand get_segment_register_operand(uint8_t sr_index) {
  struct register_access sr_table[] = {
      {Reg_ES, RegByte_All},
      {Reg_CS, RegByte_All},
      {Reg_SS, RegByte_All},
      {Reg_DS, RegByte_All},
  };

  struct register_access sr = sr_table[sr_index];
  struct operand op = {Operand_Register, sr};
  return op;
}

enum effective_address_type get_effective_address_type(uint8_t index,
                                                       uint8_t mod) {
  if (mod == MOD_MEM && index == 0b110) {
    return EffectiveAddress_Direct;
  }

  enum effective_address_type ea_table[] = {
      EffectiveAddress_BX_SI, EffectiveAddress_BX_DI,
      EffectiveAddress_BP_SI, EffectiveAddress_BP_DI,

      EffectiveAddress_SI,    EffectiveAddress_DI,
      EffectiveAddress_BP,    EffectiveAddress_BX,
  };

  return ea_table[index];
}

bool decode_instruction(mem_t *mem, uint8_t ip, struct instruction *inst) {
  uint8_t buf[2];
  uint8_t ipo = ip; /* instruction pointer with offset */

  if ((mem_readn(mem, ipo, buf, 1)) <= 0) {
    return false;
  }
  ipo += 1;

  inst->bsize = 1;
  struct instruction_encoding inst_encoding = instructions[buf[0]];

  if (inst_encoding.size == WORD) {
    ipo += mem_readn(mem, ipo, buf, 1);
    inst->bsize++;
  }

  if (inst_encoding.extended == EXTENDED) {
    uint8_t ext_bits = (buf[0] >> 3) & 0x7;
    inst->type = inst_encoding.types[ext_bits];
  } else {
    inst->type = inst_encoding.type;
  }

  inst->flags = inst_encoding.fields;

  uint8_t D = inst_encoding.fields & F_D;
  uint8_t W = inst_encoding.fields & F_W;
  uint8_t S = inst_encoding.fields & F_S;
  uint8_t rm_is_w = (inst_encoding.fields & RM_ALWAYS_W) || W;

  uint8_t fields =
      (inst_encoding.size & BYTE) ? inst_encoding.included_fields : buf[0];

  struct operand *reg_op = &inst->operand[D ? 0 : 1];
  struct operand *rm_op = &inst->operand[D ? 1 : 0];

  if (inst_encoding.fields & REG) {
    uint8_t reg_index = (fields & 0x38) >> 3;
    *reg_op = get_register_operand(reg_index, W);
  }

  if (inst_encoding.fields & SR) {
    uint8_t sr_index = (fields & 0x18) >> 3;
    *reg_op = get_segment_register_operand(sr_index);
  }

  if (inst_encoding.fields & RM) {
    uint8_t mod = (fields & 0xC0) >> 6;
    uint8_t rm_index = fields & 0x7;

    if (mod == MOD_REG) {
      *rm_op = get_register_operand(rm_index, rm_is_w);

    } else {
      rm_op->type = Operand_Memory;
      rm_op->memory.type = get_effective_address_type(rm_index, mod);

      uint8_t is_direct_address = rm_op->memory.type == EffectiveAddress_Direct;

      if (mod == MOD_MEM8) {
        ipo += mem_readn(mem, ipo, buf, 1);
        inst->bsize++;
        rm_op->memory.displacement = (int8_t)buf[0];

      } else if (mod == MOD_MEM16 || is_direct_address) {
        ipo += mem_readn(mem, ipo, buf, 2);
        inst->bsize += 2;
        rm_op->memory.displacement = (buf[1] << 8) | buf[0];
      }
    }
  }

  struct operand *imm_op = &inst->operand[0];
  if (imm_op->type) {
    imm_op = &inst->operand[1];
  }

  uint8_t is_imm_wide = (inst_encoding.fields & DATA) &&
                        (inst_encoding.fields & F_W) &&
                        !(inst_encoding.fields & F_S);

  if (is_imm_wide) {
    ipo += mem_readn(mem, ipo, buf, 2);
    inst->bsize += 2;
    imm_op->type = Operand_Immediate;
    imm_op->immediate = (buf[1] << 8) | buf[0];

  } else if (inst_encoding.fields & (DATA | DATA8)) {
    ipo += mem_readn(mem, ipo, buf, 1);
    inst->bsize++;
    imm_op->type = Operand_Immediate;

    if (inst_encoding.fields & DATA) {
      imm_op->immediate = (int8_t)buf[0];

    } else if (inst_encoding.fields & DATA8) {
      imm_op->immediate = (uint8_t)buf[0];
    }
  }

  if (inst_encoding.fields & ADDR) {
    ipo += mem_readn(mem, ipo, buf, 1);
    inst->bsize++;
    imm_op->type = Operand_RelativeImmediate;
    imm_op->immediate = (int8_t)buf[0];
  }

  return true;
}
