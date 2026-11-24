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
 *   $1BFBE  reading or writing it acknowledges the step and enables the
 *           interrupts
 *
 * In the circuit (QIMI schematic, and the MAME emulation) each axis has a
 * flip-flop for the step, set by an edge of the mouse signal and cleared only
 * by an access to $1BFBE, and a flip-flop for the direction, sampled at every
 * edge. EXTI is high while a step flag is set. So reading $1BFBC has no side
 * effects, and the movement that arrives while a step is pending is not
 * counted by the hardware. Here it is accumulated instead, and delivered one
 * step per axis for each acknowledge.
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

static int dx, dy; /* host motion not delivered yet, +y down */
static int buttons;
static int enabled;
static uint8_t status; /* value of $1BFBC */

static int clampi(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

/* Set the step flag of the axes that have motion and no step pending */
static void latch_steps(void)
{
	if (dx && !(status & MOV_X_STEP)) {
		status |= MOV_X_STEP;
		if (dx > 0)
			status |= MOV_X_DIR;
		else
			status &= ~MOV_X_DIR;
		dx -= dx > 0 ? 1 : -1;
	}
	if (dy && !(status & MOV_Y_STEP)) {
		status |= MOV_Y_STEP;
		if (dy < 0)
			status |= MOV_Y_DIR; /* up */
		else
			status &= ~MOV_Y_DIR;
		dy -= dy > 0 ? 1 : -1;
	}
}

/* EXTI is high while a step is pending and the interrupts are enabled */
static void raise_int(void)
{
	if (enabled && (status & (MOV_X_STEP | MOV_Y_STEP)) &&
	    !(theInt & INT_EXT)) {
		theInt |= INT_EXT;
		ql_update_ipl();
	}
}

/* Access to $1BFBE: clears the step flags, the direction stays */
static void acknowledge(void)
{
	enabled = 1;
	status &= ~(MOV_X_STEP | MOV_Y_STEP);
	theInt &= ~INT_EXT;
	latch_steps();
	ql_update_ipl();
	raise_int();
}

void qimi_init(void)
{
	dx = dy = buttons = enabled = 0;
	status = 0;
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
	uint8_t v;

	switch (addr) {
	case QIMI_BUTTONS:
		v = 0xFF;
		if (buttons & 1)
			v &= ~BTN_LEFT;
		if (buttons & 2)
			v &= ~BTN_RIGHT;
		return v;
	case QIMI_MOVEMENT:
		return status;
	case QIMI_IRQ:
		acknowledge();
		return 0;
	}
	return 0;
}

void qimi_write_byte(uint32_t addr, uint8_t d)
{
	(void)d;
	if (addr == QIMI_IRQ)
		acknowledge();
}

void qimi_move(int mx, int my)
{
	/* the direction is sampled at every edge, even with a step pending */
	if (mx && (status & MOV_X_STEP)) {
		if (mx > 0)
			status |= MOV_X_DIR;
		else
			status &= ~MOV_X_DIR;
	}
	if (my && (status & MOV_Y_STEP)) {
		if (my < 0)
			status |= MOV_Y_DIR;
		else
			status &= ~MOV_Y_DIR;
	}
	dx = clampi(dx + mx, -MAX_PENDING, MAX_PENDING);
	dy = clampi(dy + my, -MAX_PENDING, MAX_PENDING);
	latch_steps();
	raise_int();
}

void qimi_buttons(int mask)
{
	buttons = mask & 3;
}

/* The driver cleared the ZX8302 interrupt: EXTI is still high if a step is
 * pending */
void qimi_int_acked(void)
{
	if (qimi_mapped)
		raise_int();
}
