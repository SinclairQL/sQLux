/*
 * ice.h - Eidersoft mICE bus mouse for sQLux
 *
 * The mouse sits in the last two bytes of the ROM cartridge ($FFFE and
 * $FFFF, A0 is not decoded).
 */
#ifndef ICE_H
#define ICE_H

#include <stdint.h>

#define ICE_REG0 0xFFFE
#define ICE_REG1 0xFFFF

extern int ice_mapped;

void    ice_init(void);
int     ice_is_reg(uint32_t addr);
uint8_t ice_read_byte(uint32_t addr);
void    ice_move(int dx, int dy);
void    ice_buttons(int mask); /* bit 0 left, 1 right, 2 middle */

/* Check the cartridge in the ROM port is the original mICE ROM */
void    ice_check_rom(const uint8_t *rom, uint32_t size);

#endif
