#if !defined DECODE_H
#define DECODE_H

#include "instruction.h"
#include <stdint.h>

#define MOD_MEM 0b00
#define MOD_MEM8 0b01
#define MOD_MEM16 0b10
#define MOD_REG 0b11

enum instruction_encoding_size : uint8_t {
  BYTE = 1,
  WORD = 2,
};

enum instruction_encoding_extension : uint8_t {
  NOT_EXTENDED,
  EXTENDED,
};

struct instruction_encoding {
  enum instruction_encoding_extension extended;
  union {
    enum instruction_type type;
    enum instruction_type types[8];
  };

  enum instruction_encoding_size size;
  enum instruction_flag fields;
  uint8_t included_fields;
};

#endif
