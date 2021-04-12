#ifndef _QL_HARDWARE_H
#define _QL_HARDWARE_H

#include "QL68000.h"

extern int display_mode;
void queueKey(short m,short code,uw16 asciiChar);
void MReadKbd(void);

#endif /* _QL_HARDWARE_H */

