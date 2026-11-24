/*
 * qimi.h - QJump QIMI mouse interface for sQLux
 *
 * Registers in the internal I/O area, see docs/sqlux.md.
 */
#ifndef QIMI_H
#define QIMI_H

#include <stdint.h>

#define QIMI_BUTTONS  0x1BF9C
#define QIMI_MOVEMENT 0x1BFBC
#define QIMI_IRQ      0x1BFBE

extern int qimi_mapped;

void    qimi_init(void);
int     qimi_is_reg(uint32_t addr);
uint8_t qimi_read_byte(uint32_t addr);
void    qimi_write_byte(uint32_t addr, uint8_t d);
void    qimi_move(int dx, int dy);
void    qimi_buttons(int mask); /* bit 0 left, 1 right */
void    qimi_int_acked(void);   /* the ZX8302 interrupt was cleared */

#endif
