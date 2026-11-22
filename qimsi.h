/*
 * qimsi.h - QIMSI PS/2 mouse interface for sQLux
 *
 * The registers sit in the ROM port ($FED0-$FEDF) and are read only.
 * Based on qimsi.js from QLRun.
 */
#ifndef QIMSI_H
#define QIMSI_H

#include <stdint.h>

#define QIMSI_REG_BASE 0xFED0
#define QIMSI_REG_END  0xFEE0

extern int qimsi_mapped;

void    qimsi_init(void);
uint8_t qimsi_read_byte(uint32_t addr);
void    qimsi_move(int dx, int dy);
void    qimsi_scroll(int dz);
void    qimsi_buttons(int mask); /* bit 0 left, 1 right, 2 middle */

#endif
