/*
 * (c) UQLX - see COPYRIGHT
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

void InitROM(void);
int EmulatorTable(void);
void save_regs(void *p);
void restore_regs(void *p);
bool LookFor(uint32_t *a, uint32_t w, int nMax);
int LoadMainRom(void);
