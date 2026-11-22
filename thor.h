/*
 * thor.h - CST Thor mouse port (MC6821 PIA) for sQLux
 */
#ifndef THOR_H
#define THOR_H

#include <stdint.h>

#define THOR_IO_BASE  0xFE000 /* I/O block, the RAM must stay below */
#define THOR_PIA_BASE 0xFE040 /* PIA registers every 4 bytes (RS0 = A2, RS1 = A3) */

extern int thor_mapped;

void    thor_init(void);
int     thor_is_reg(uint32_t addr);
uint8_t thor_read_byte(uint32_t addr);
void    thor_write_byte(uint32_t addr, uint8_t d);
void    thor_move(int dx, int dy);
void    thor_buttons(int mask); /* bit 0 left, 1 right, 2 middle */
void    thor_int_acked(void);   /* the ZX8302 interrupt was cleared */

#endif
