#if !defined DISPLAY_H
#define DISPLAY_H

#include "instruction.h"

extern char reg_names[36][3];
extern char ea_names[8][8];

char *get_register_name(struct register_access register_);

#endif // DISPLAY_H
