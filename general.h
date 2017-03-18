#ifndef _GENERAL_H
#define _GENERAL_H
#include "QLtypes.h"

void WriteHWByte(aw32 addr, aw8 d);
rw8 ReadHWByte(aw32 addr);
rw16 ReadHWWord(aw32 addr);
void WriteHWWord(aw32 addr, aw16 d);

#endif /* _GENERAL_H */
