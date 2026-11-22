/*
 * ice.c - Eidersoft mICE bus mouse for sQLux
 *
 * The mICE cartridge is a 16K ROM at $C000 that also shows the mouse at
 * $FFFE/$FFFF (A0 is not decoded):
 *
 *   D0-D2  buttons
 *   D3     always 0
 *   D4, D7 horizontal quadrature signals
 *   D5, D6 vertical quadrature signals
 *
 * It is a bus mouse: the bits are the raw encoder phototransistors, which
 * follow 00 01 11 10 00 ... in one direction and 00 10 11 01 00 ... in the
 * other. The software has to poll it. As the USB to bus mouse adapter does,
 * one phase step is made per pending count every millisecond.
 *
 * Not known from the hardware: which bit of each pair leads, the sense of
 * the buttons (here 1 = pressed) and the order of the buttons; see the
 * defines below.
 */

#include <stdio.h>

#include "ice.h"
#include "zx8301.h"

extern uint64_t ql_cycles; /* iexl_general.c */

#define BTN_LEFT   0x01
#define BTN_RIGHT  0x02
#define BTN_MIDDLE 0x04
#define BUTTON_MASK 0x07
#define X_A        0x10
#define X_B        0x80
#define Y_A        0x20
#define Y_B        0x40
#define STEP_HZ    1000
#define MAX_PENDING 255

#define ICE_ROM_CRC32 0xC843CB0Eu /* roms/mICE_21.rom */

int ice_mapped = 0;

static const uint8_t gray[4] = {0, 1, 3, 2}; /* bit 0 = A, bit 1 = B */

static struct {
	int dx, dy;       /* pending host motion, +y down */
	int px, py;       /* phase 0-3 */
	int buttons;
	uint64_t last;    /* cycle of the last phase step */
} m;

static int clampi(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

void ice_init(void)
{
	m.dx = m.dy = m.px = m.py = m.buttons = 0;
	m.last = ql_cycles;
	ice_mapped = 1;
}

int ice_is_reg(uint32_t addr)
{
	return ice_mapped && (addr == ICE_REG0 || addr == ICE_REG1);
}

/* Make the phase steps due since the last read */
static void advance(uint64_t now)
{
	uint64_t period = ZX_CPU_HZ / STEP_HZ;
	uint64_t n;

	if (!m.dx && !m.dy) {
		m.last = now;
		return;
	}
	n = (now - m.last) / period;
	if (!n)
		return;
	m.last += n * period;
	while (n-- && (m.dx || m.dy)) {
		if (m.dx) {
			m.px = (m.px + (m.dx > 0 ? 1 : 3)) & 3;
			m.dx -= m.dx > 0 ? 1 : -1;
		}
		if (m.dy) {
			m.py = (m.py + (m.dy > 0 ? 1 : 3)) & 3;
			m.dy -= m.dy > 0 ? 1 : -1;
		}
	}
	if (!m.dx && !m.dy)
		m.last = now;
}

uint8_t ice_read_byte(uint32_t addr)
{
	uint8_t v;

	(void)addr;
	advance(ql_cycles);
	v = m.buttons & BUTTON_MASK;
	if (gray[m.px] & 1)
		v |= X_A;
	if (gray[m.px] & 2)
		v |= X_B;
	if (gray[m.py] & 1)
		v |= Y_A;
	if (gray[m.py] & 2)
		v |= Y_B;
	return v;
}

void ice_move(int dx, int dy)
{
	if (!m.dx && !m.dy)
		m.last = ql_cycles;
	m.dx = clampi(m.dx + dx, -MAX_PENDING, MAX_PENDING);
	m.dy = clampi(m.dy + dy, -MAX_PENDING, MAX_PENDING);
}

void ice_buttons(int mask)
{
	m.buttons = mask & BUTTON_MASK;
}

static uint32_t crc32(const uint8_t *p, uint32_t n)
{
	uint32_t c = 0xFFFFFFFFu;

	while (n--) {
		c ^= *p++;
		for (int k = 0; k < 8; k++)
			c = (c >> 1) ^ (0xEDB88320u & -(c & 1));
	}
	return ~c;
}

void ice_check_rom(const uint8_t *rom, uint32_t size)
{
	if (!rom || crc32(rom, size) != ICE_ROM_CRC32)
		fprintf(stderr,
			"SQLUX WARNING: MOUSE = ICE, but the ROM in ROMPORT is not the original mICE_21.rom\n");
}
