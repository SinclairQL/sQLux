/*
 * thor.c - CST Thor mouse port for sQLux
 *
 * Following the schematic of the Thor expansion board (sheet Printer and
 * Mouse Interfaces): the mouse is wired to the A side of an MC6821 PIA.
 *
 *   PA3-PA5  buttons, 0 = pressed
 *   PA6      X direction, latched by a 74LS74 on the clock of the X pulse
 *   PA7      Y direction, same for Y
 *   CA1      X pulse (one per count)
 *   CA2      Y pulse (one per count)
 *   IRQA     ZX8302 external interrupt (bit 4 of $18021)
 *
 * The registers are 4 bytes apart (RS0 = A2, RS1 = A3). The PIA is selected
 * by the I/O block with A7 A6 = 01, A5 = 0 and A4 = 0. The block is the
 * last 8K of the first megabyte (the 68008 has no A20-A23), and A8-A12 are
 * not decoded, so the PIA is repeated in every 256 byte page of it:
 * $FE040, $FE140 ... $FFF40. THOR_PIA_BASE is the first one.
 * Only the A side is wired to the mouse; the B side (printer) is just kept
 * as registers. A host count is delivered as a whole pulse: whatever the
 * edge selected in the control register, the flag is set.
 *
 * The sense of the direction bits (PA6 = 1 left, PA7 = 1 down), the pulses
 * and the address $FF40.w come from the ICE file manager, which works on the
 * real Thor. Not known: which button is which (here PA3 left, PA4 right,
 * PA5 middle).
 */

#include "thor.h"
#include "QL68000.h"

void ql_update_ipl(void); /* general.c */

#define PA_LEFT   0x08
#define PA_RIGHT  0x10
#define PA_MIDDLE 0x20
#define PA_XDIR   0x40
#define PA_YDIR   0x80

#define CR_C1_IRQEN 0x01
#define CR_DDR_OFF  0x04 /* 1 = the data register is selected */
#define CR_C2_IRQEN 0x08
#define CR_C2_OUT   0x20
#define CR_IRQ2     0x40
#define CR_IRQ1     0x80

#define INT_EXT     0x10
#define MAX_PENDING 255

int thor_mapped = 0;

static struct {
	uint8_t ora, ddra, cra;
	uint8_t orb, ddrb, crb;
	int dx, dy;          /* pending host motion, +y down */
	int xdir, ydir;      /* latched directions */
	int buttons;
} p;

static int clampi(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

/* IRQA is a level: asserted while a flag is set and its interrupt enabled */
static int irqa(void)
{
	return ((p.cra & CR_IRQ1) && (p.cra & CR_C1_IRQEN)) ||
	       ((p.cra & CR_IRQ2) && (p.cra & CR_C2_IRQEN) &&
		!(p.cra & CR_C2_OUT));
}

static void raise_int(void)
{
	if (irqa() && !(theInt & INT_EXT)) {
		theInt |= INT_EXT;
		ql_update_ipl();
	}
}

/* Deliver the next pulse of each axis when the previous one was read */
static void pump(void)
{
	if (p.dx && !(p.cra & CR_IRQ1)) {
		p.xdir = p.dx < 0; /* PA6 = 1 is left */
		p.dx -= p.dx > 0 ? 1 : -1;
		p.cra |= CR_IRQ1;
	}
	if (p.dy && !(p.cra & CR_IRQ2)) {
		p.ydir = p.dy > 0; /* PA7 = 1 is down */
		p.dy -= p.dy > 0 ? 1 : -1;
		p.cra |= CR_IRQ2;
	}
	raise_int();
}

void thor_init(void)
{
	p.ora = p.ddra = p.cra = p.orb = p.ddrb = p.crb = 0;
	p.dx = p.dy = p.xdir = p.ydir = p.buttons = 0;
	thor_mapped = 1;
}

int thor_is_reg(uint32_t addr)
{
	/* The 68008 has A0-A19 only, and the I/O decoding ignores A8-A12:
	 * the PIA answers in every 256 byte page of the last 8K */
	addr &= 0xFFFFF;
	return thor_mapped && (addr & 0xFE000) == THOR_IO_BASE &&
	       (addr & 0xF0) == (THOR_PIA_BASE & 0xF0);
}

static uint8_t pins_a(void)
{
	uint8_t v = PA_LEFT | PA_RIGHT | PA_MIDDLE; /* pulled up */

	if (p.buttons & 1)
		v &= ~PA_LEFT;
	if (p.buttons & 2)
		v &= ~PA_RIGHT;
	if (p.buttons & 4)
		v &= ~PA_MIDDLE;
	if (p.xdir)
		v |= PA_XDIR;
	if (p.ydir)
		v |= PA_YDIR;
	return v;
}

uint8_t thor_read_byte(uint32_t addr)
{
	uint8_t v;

	switch ((addr >> 2) & 3) {
	case 0:
		if (!(p.cra & CR_DDR_OFF))
			return p.ddra;
		v = (pins_a() & ~p.ddra) | (p.ora & p.ddra);
		p.cra &= ~(CR_IRQ1 | CR_IRQ2);
		pump();
		return v;
	case 1:
		return p.cra;
	case 2:
		return (p.crb & CR_DDR_OFF) ? p.orb : p.ddrb;
	default:
		return p.crb;
	}
}

void thor_write_byte(uint32_t addr, uint8_t d)
{
	switch ((addr >> 2) & 3) {
	case 0:
		if (p.cra & CR_DDR_OFF)
			p.ora = d;
		else
			p.ddra = d;
		break;
	case 1:
		p.cra = (p.cra & (CR_IRQ1 | CR_IRQ2)) | (d & 0x3F);
		raise_int();
		break;
	case 2:
		if (p.crb & CR_DDR_OFF)
			p.orb = d;
		else
			p.ddrb = d;
		break;
	default:
		p.crb = d & 0x3F;
		break;
	}
}

void thor_move(int mx, int my)
{
	p.dx = clampi(p.dx + mx, -MAX_PENDING, MAX_PENDING);
	p.dy = clampi(p.dy + my, -MAX_PENDING, MAX_PENDING);
	pump();
}

void thor_buttons(int mask)
{
	p.buttons = mask & 7;
}

/* The driver acknowledged the interrupt: raise it again if still asserted */
void thor_int_acked(void)
{
	if (thor_mapped)
		raise_int();
}
