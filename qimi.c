/*
 * qimi.c - QJump QIMI mouse interface for sQLux
 *
 * QIMI replaces the ZX8302 socket and signals each mouse step with the
 * external interrupt of the ZX8302 (bit 4 of $18021). Registers, from Dave
 * Westbury's notes (sinclairql.net/djw/docs/manuals/qimi.html):
 *
 *   $1BF9C  buttons, bit 4 = right, bit 5 = left, 0 = pressed
 *   $1BFBC  movement, bit 2 = X step, bit 4 = X direction,
 *                     bit 5 = Y step, bit 0 = Y direction (1 = right, up)
 *   $1BFBE  reading or writing it enables the interrupts
 *
 * Each read of $1BFBC delivers one step of each axis and consumes it from
 * the motion accumulated from the host.
 */

#include "qimi.h"
#include "QL68000.h"

void ql_update_ipl(void); /* general.c */

#define BTN_RIGHT   0x10
#define BTN_LEFT    0x20
#define MOV_Y_DIR   0x01
#define MOV_X_STEP  0x04
#define MOV_X_DIR   0x10
#define MOV_Y_STEP  0x20
#define INT_EXT     0x10
#define MAX_PENDING 255

int qimi_mapped = 0;

static int dx, dy; /* host motion, +y down */
static int buttons;
static int enabled;

static int clampi(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

static void raise_int(void)
{
	if (enabled && (dx || dy) && !(theInt & INT_EXT)) {
		theInt |= INT_EXT;
		ql_update_ipl();
	}
}

void qimi_init(void)
{
	dx = dy = buttons = enabled = 0;
	qimi_mapped = 1;
}

int qimi_is_reg(uint32_t addr)
{
	if (!qimi_mapped)
		return 0;
	return addr == QIMI_BUTTONS || addr == QIMI_MOVEMENT ||
	       addr == QIMI_IRQ;
}

uint8_t qimi_read_byte(uint32_t addr)
{
	uint8_t v = 0;

	switch (addr) {
	case QIMI_BUTTONS:
		v = 0xFF;
		if (buttons & 1)
			v &= ~BTN_LEFT;
		if (buttons & 2)
			v &= ~BTN_RIGHT;
		return v;
	case QIMI_MOVEMENT:
		if (dx) {
			v |= MOV_X_STEP;
			if (dx > 0)
				v |= MOV_X_DIR;
			dx -= dx > 0 ? 1 : -1;
		}
		if (dy) {
			v |= MOV_Y_STEP;
			if (dy < 0)
				v |= MOV_Y_DIR; /* up */
			dy -= dy > 0 ? 1 : -1;
		}
		return v;
	case QIMI_IRQ:
		enabled = 1;
		raise_int();
		return 0;
	}
	return 0;
}

void qimi_write_byte(uint32_t addr, uint8_t d)
{
	(void)d;
	if (addr == QIMI_IRQ) {
		enabled = 1;
		raise_int();
	}
}

void qimi_move(int mx, int my)
{
	dx = clampi(dx + mx, -MAX_PENDING, MAX_PENDING);
	dy = clampi(dy + my, -MAX_PENDING, MAX_PENDING);
	raise_int();
}

void qimi_buttons(int mask)
{
	buttons = mask & 3;
}

/* The driver acknowledged the interrupt: ask for the next step if any */
void qimi_int_acked(void)
{
	if (qimi_mapped)
		raise_int();
}
