#pragma once

#include <stddef.h>

extern int display_mode;
void queueKey(short m,short code, uint16_t asciiChar);
void MReadKbd(void);
void KbdCmd(void);

