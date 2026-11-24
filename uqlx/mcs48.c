/*
 * mcs48.c - Intel MCS-48 (8048/8049) microcontroller core
 *
 * Reference: Intel MCS-48 Family User's Manual. All instructions take one or
 * two machine cycles. The timer counts machine cycles divided by 32; the
 * event counter counts high-to-low transitions of T1. Interrupts are taken
 * between instructions: the external interrupt (INT low, EN I) has priority
 * over the timer/counter overflow interrupt (EN TCNTI); both are blocked
 * while an interrupt routine runs, until RETR.
 */

#include <string.h>
#include "mcs48.h"

#define CY 0x80
#define AC 0x40
#define F0 0x20
#define BS 0x10

static inline uint8_t fetch(mcs48_t *c)
{
	uint8_t v = c->rom[c->pc & c->rom_mask];
	/* the program counter increments within 2K; A11 is kept */
	c->pc = (c->pc & 0x800) | ((c->pc + 1) & 0x7ff);
	return v;
}

static inline uint8_t *reg(mcs48_t *c, int r)
{
	return &c->ram[((c->psw & BS) ? 0x18 : 0x00) + r];
}

static inline uint8_t *ind(mcs48_t *c, int r)
{
	return &c->ram[*reg(c, r) & c->ram_mask];
}

static void push_pc(mcs48_t *c)
{
	uint8_t sp = c->psw & 7;
	c->ram[8 + 2 * sp] = (uint8_t)c->pc;
	c->ram[9 + 2 * sp] = (uint8_t)((c->psw & 0xf0) | ((c->pc >> 8) & 0x0f));
	c->psw = (uint8_t)((c->psw & 0xf8) | ((sp + 1) & 7));
}

static void pull_pc(mcs48_t *c, int restore_psw)
{
	uint8_t sp = (uint8_t)((c->psw - 1) & 7);
	uint8_t hi = c->ram[9 + 2 * sp];
	c->psw = (uint8_t)((c->psw & 0xf8) | sp);
	c->pc = (uint16_t)(((hi & 0x0f) << 8) | c->ram[8 + 2 * sp]);
	if (restore_psw)
		c->psw = (uint8_t)((hi & 0xf0) | (c->psw & 0x0f));
}

static void add(mcs48_t *c, uint8_t v, int carry)
{
	unsigned cin = carry ? ((c->psw & CY) ? 1u : 0u) : 0u;
	unsigned r = (unsigned)c->a + v + cin;
	unsigned h = (unsigned)(c->a & 0x0f) + (v & 0x0f) + cin;
	c->psw = (uint8_t)((c->psw & ~(CY | AC)) | ((r > 0xff) ? CY : 0) |
			   ((h > 0x0f) ? AC : 0));
	c->a = (uint8_t)r;
}

/* Conditional jump within the page of the address byte */
static void jcc(mcs48_t *c, int cond)
{
	uint16_t page = c->pc & 0xf00;
	uint8_t target = fetch(c);
	if (cond)
		c->pc = (uint16_t)(page | target);
}

static void jmp_call(mcs48_t *c, uint8_t op, int call)
{
	uint8_t lo = fetch(c);
	uint16_t addr = (uint16_t)(((op & 0xe0) << 3) | lo);
	if (call)
		push_pc(c);
	/* A11 comes from the memory bank flip-flop, except in interrupts */
	c->pc = (uint16_t)(addr | ((c->in_irq ? 0 : c->a11) ? 0x800 : 0));
}

static uint8_t port_in(mcs48_t *c, int p)
{
	/* quasi-bidirectional: an output latch at 1 lets the pin be read */
	if (p == 1)
		return c->p1 & (c->io.p1_in ? c->io.p1_in(c->io.ctx) : 0xff);
	return c->p2 & (c->io.p2_in ? c->io.p2_in(c->io.ctx) : 0xff);
}

static void port_out(mcs48_t *c, int p, uint8_t v)
{
	if (p == 1) {
		c->p1 = v;
		if (c->io.p1_out) c->io.p1_out(c->io.ctx, v);
	} else {
		c->p2 = v;
		if (c->io.p2_out) c->io.p2_out(c->io.ctx, v);
	}
}

static void bus_out(mcs48_t *c, uint8_t v)
{
	c->bus = v;
	if (c->io.bus_out) c->io.bus_out(c->io.ctx, v);
}

static int t_in(mcs48_t *c, int t)
{
	if (t == 0)
		return c->io.t0_in ? c->io.t0_in(c->io.ctx) : 0;
	return c->io.t1_in ? c->io.t1_in(c->io.ctx) : 0;
}

static int int_asserted(mcs48_t *c)
{
	return c->io.int_in ? (c->io.int_in(c->io.ctx) == 0) : 0;
}

void mcs48_init(mcs48_t *c, const uint8_t *rom, uint16_t rom_size,
		const mcs48_io_t *io)
{
	memset(c, 0, sizeof(*c));
	c->rom = rom;
	c->rom_mask = (uint16_t)(rom_size - 1);
	c->ram_mask = 0x7f;
	if (io)
		c->io = *io;
	mcs48_reset(c);
}

void mcs48_reset(mcs48_t *c)
{
	c->pc = 0;
	c->psw = 0x08;
	c->a11 = c->a11_pending = 0;
	c->f1 = 0;
	c->xirq_en = c->tirq_en = 0;
	c->in_irq = 0;
	c->timer_on = c->counter_on = 0;
	c->tf = 0;
	c->timer_irq_pending = 0;
	c->prescaler = 0;
	/* ports float high after reset */
	port_out(c, 1, 0xff);
	port_out(c, 2, 0xff);
	c->bus = 0xff;
	c->last_t1 = (uint8_t)t_in(c, 1);
}

static void timer_tick(mcs48_t *c, int cycles)
{
	while (cycles--) {
		int inc = 0;
		if (c->timer_on) {
			if (++c->prescaler == 32) {
				c->prescaler = 0;
				inc = 1;
			}
		} else if (c->counter_on) {
			uint8_t t1 = (uint8_t)(t_in(c, 1) ? 1 : 0);
			if (c->last_t1 && !t1)
				inc = 1;
			c->last_t1 = t1;
		}
		if (inc && ++c->timer == 0) {
			c->tf = 1;
			if (c->tirq_en)
				c->timer_irq_pending = 1;
		}
	}
}

int mcs48_step(mcs48_t *c)
{
	int cyc;
	uint8_t op, v;

	/* interrupts are recognised between instructions */
	if (!c->in_irq) {
		int vec = -1;
		if (c->xirq_en && int_asserted(c))
			vec = 3;
		else if (c->timer_irq_pending) {
			c->timer_irq_pending = 0;
			vec = 7;
		}
		if (vec >= 0) {
			push_pc(c);
			c->pc = (uint16_t)vec;
			c->in_irq = 1;
			c->cycles += 2;
			timer_tick(c, 2);
			return 2;
		}
	}

	op = fetch(c);
	cyc = 1;

	switch (op) {
	/* ---- accumulator ---- */
	case 0x03: add(c, fetch(c), 0); cyc = 2; break;             /* ADD A,#d  */
	case 0x13: add(c, fetch(c), 1); cyc = 2; break;             /* ADDC A,#d */
	case 0x60: case 0x61: add(c, *ind(c, op & 1), 0); break;    /* ADD A,@R  */
	case 0x70: case 0x71: add(c, *ind(c, op & 1), 1); break;    /* ADDC A,@R */
	case 0x68: case 0x69: case 0x6a: case 0x6b:
	case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		add(c, *reg(c, op & 7), 0); break;                  /* ADD A,Rr  */
	case 0x78: case 0x79: case 0x7a: case 0x7b:
	case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		add(c, *reg(c, op & 7), 1); break;                  /* ADDC A,Rr */

	case 0x53: c->a &= fetch(c); cyc = 2; break;                /* ANL A,#d */
	case 0x50: case 0x51: c->a &= *ind(c, op & 1); break;
	case 0x58: case 0x59: case 0x5a: case 0x5b:
	case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		c->a &= *reg(c, op & 7); break;
	case 0x43: c->a |= fetch(c); cyc = 2; break;                /* ORL A,#d */
	case 0x40: case 0x41: c->a |= *ind(c, op & 1); break;
	case 0x48: case 0x49: case 0x4a: case 0x4b:
	case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		c->a |= *reg(c, op & 7); break;
	case 0xd3: c->a ^= fetch(c); cyc = 2; break;                /* XRL A,#d */
	case 0xd0: case 0xd1: c->a ^= *ind(c, op & 1); break;
	case 0xd8: case 0xd9: case 0xda: case 0xdb:
	case 0xdc: case 0xdd: case 0xde: case 0xdf:
		c->a ^= *reg(c, op & 7); break;

	case 0x17: c->a++; break;                                   /* INC A  */
	case 0x07: c->a--; break;                                   /* DEC A  */
	case 0x27: c->a = 0; break;                                 /* CLR A  */
	case 0x37: c->a = (uint8_t)~c->a; break;                    /* CPL A  */
	case 0x47: c->a = (uint8_t)((c->a << 4) | (c->a >> 4)); break; /* SWAP */
	case 0x57: {                                                /* DA A   */
		unsigned r = c->a;
		if ((r & 0x0f) > 9 || (c->psw & AC)) {
			r += 6;
			if (r > 0xff) c->psw |= CY;
		}
		if (((r >> 4) & 0x0f) > 9 || (c->psw & CY)) {
			r += 0x60;
			c->psw |= CY;
		}
		c->a = (uint8_t)r;
		break;
	}
	case 0x77: c->a = (uint8_t)((c->a >> 1) | (c->a << 7)); break; /* RR  */
	case 0xe7: c->a = (uint8_t)((c->a << 1) | (c->a >> 7)); break; /* RL  */
	case 0x67: {                                                /* RRC A */
		uint8_t cy = c->a & 1;
		c->a = (uint8_t)((c->a >> 1) | ((c->psw & CY) ? 0x80 : 0));
		c->psw = (uint8_t)((c->psw & ~CY) | (cy ? CY : 0));
		break;
	}
	case 0xf7: {                                                /* RLC A */
		uint8_t cy = c->a & 0x80;
		c->a = (uint8_t)((c->a << 1) | ((c->psw & CY) ? 1 : 0));
		c->psw = (uint8_t)((c->psw & ~CY) | (cy ? CY : 0));
		break;
	}

	/* ---- registers and data memory ---- */
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		(*reg(c, op & 7))++; break;                         /* INC Rr */
	case 0x10: case 0x11: (*ind(c, op & 1))++; break;           /* INC @R */
	case 0xc8: case 0xc9: case 0xca: case 0xcb:
	case 0xcc: case 0xcd: case 0xce: case 0xcf:
		(*reg(c, op & 7))--; break;                         /* DEC Rr */

	case 0xf8: case 0xf9: case 0xfa: case 0xfb:
	case 0xfc: case 0xfd: case 0xfe: case 0xff:
		c->a = *reg(c, op & 7); break;                      /* MOV A,Rr */
	case 0xf0: case 0xf1: c->a = *ind(c, op & 1); break;        /* MOV A,@R */
	case 0x23: c->a = fetch(c); cyc = 2; break;                 /* MOV A,#d */
	case 0xa8: case 0xa9: case 0xaa: case 0xab:
	case 0xac: case 0xad: case 0xae: case 0xaf:
		*reg(c, op & 7) = c->a; break;                      /* MOV Rr,A */
	case 0xa0: case 0xa1: *ind(c, op & 1) = c->a; break;        /* MOV @R,A */
	case 0xb8: case 0xb9: case 0xba: case 0xbb:
	case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		*reg(c, op & 7) = fetch(c); cyc = 2; break;         /* MOV Rr,#d */
	case 0xb0: case 0xb1: {                                     /* MOV @R,#d */
		uint8_t *p = ind(c, op & 1);
		*p = fetch(c);
		cyc = 2;
		break;
	}
	case 0x28: case 0x29: case 0x2a: case 0x2b:
	case 0x2c: case 0x2d: case 0x2e: case 0x2f: {               /* XCH A,Rr */
		uint8_t *p = reg(c, op & 7);
		v = *p; *p = c->a; c->a = v;
		break;
	}
	case 0x20: case 0x21: {                                     /* XCH A,@R */
		uint8_t *p = ind(c, op & 1);
		v = *p; *p = c->a; c->a = v;
		break;
	}
	case 0x30: case 0x31: {                                     /* XCHD A,@R */
		uint8_t *p = ind(c, op & 1);
		v = *p;
		*p = (uint8_t)((v & 0xf0) | (c->a & 0x0f));
		c->a = (uint8_t)((c->a & 0xf0) | (v & 0x0f));
		break;
	}
	case 0xc7: c->a = (uint8_t)(c->psw | 0x08); break;          /* MOV A,PSW */
	case 0xd7: c->psw = (uint8_t)(c->a | 0x08); break;          /* MOV PSW,A */

	case 0xa3:                                                  /* MOVP A,@A */
		c->a = c->rom[((c->pc & 0xf00) | c->a) & c->rom_mask];
		cyc = 2;
		break;
	case 0xe3:                                                  /* MOVP3 A,@A */
		c->a = c->rom[(0x300 | c->a) & c->rom_mask];
		cyc = 2;
		break;
	case 0x80: case 0x81:                                       /* MOVX A,@R */
		c->a = c->io.movx_rd ? c->io.movx_rd(c->io.ctx, *reg(c, op & 1))
				     : 0xff;
		cyc = 2;
		break;
	case 0x90: case 0x91:                                       /* MOVX @R,A */
		if (c->io.movx_wr)
			c->io.movx_wr(c->io.ctx, *reg(c, op & 1), c->a);
		cyc = 2;
		break;

	/* ---- flags ---- */
	case 0x97: c->psw &= (uint8_t)~CY; break;                   /* CLR C  */
	case 0xa7: c->psw ^= CY; break;                             /* CPL C  */
	case 0x85: c->psw &= (uint8_t)~F0; break;                   /* CLR F0 */
	case 0x95: c->psw ^= F0; break;                             /* CPL F0 */
	case 0xa5: c->f1 = 0; break;                                /* CLR F1 */
	case 0xb5: c->f1 ^= 1; break;                               /* CPL F1 */
	case 0xc5: c->psw &= (uint8_t)~BS; break;                   /* SEL RB0 */
	case 0xd5: c->psw |= BS; break;                             /* SEL RB1 */
	case 0xe5: c->a11 = 0; break;                               /* SEL MB0 */
	case 0xf5: c->a11 = 1; break;                               /* SEL MB1 */

	/* ---- branches ---- */
	case 0x04: case 0x24: case 0x44: case 0x64:
	case 0x84: case 0xa4: case 0xc4: case 0xe4:
		jmp_call(c, op, 0); cyc = 2; break;                 /* JMP  */
	case 0x14: case 0x34: case 0x54: case 0x74:
	case 0x94: case 0xb4: case 0xd4: case 0xf4:
		jmp_call(c, op, 1); cyc = 2; break;                 /* CALL */
	case 0x83: pull_pc(c, 0); cyc = 2; break;                   /* RET  */
	case 0x93: pull_pc(c, 1); c->in_irq = 0; cyc = 2; break;    /* RETR */
	case 0xb3: {                                                /* JMPP @A */
		uint16_t page = c->pc & 0xf00;
		c->pc = (uint16_t)(page | c->rom[(page | c->a) & c->rom_mask]);
		cyc = 2;
		break;
	}
	case 0xe8: case 0xe9: case 0xea: case 0xeb:
	case 0xec: case 0xed: case 0xee: case 0xef: {               /* DJNZ */
		uint8_t *p = reg(c, op & 7);
		(*p)--;
		jcc(c, *p != 0);
		cyc = 2;
		break;
	}
	case 0xf6: jcc(c, c->psw & CY); cyc = 2; break;             /* JC  */
	case 0xe6: jcc(c, !(c->psw & CY)); cyc = 2; break;          /* JNC */
	case 0xc6: jcc(c, c->a == 0); cyc = 2; break;               /* JZ  */
	case 0x96: jcc(c, c->a != 0); cyc = 2; break;               /* JNZ */
	case 0x36: jcc(c, t_in(c, 0)); cyc = 2; break;              /* JT0 */
	case 0x26: jcc(c, !t_in(c, 0)); cyc = 2; break;             /* JNT0 */
	case 0x56: jcc(c, t_in(c, 1)); cyc = 2; break;              /* JT1 */
	case 0x46: jcc(c, !t_in(c, 1)); cyc = 2; break;             /* JNT1 */
	case 0xb6: jcc(c, c->psw & F0); cyc = 2; break;             /* JF0 */
	case 0x76: jcc(c, c->f1); cyc = 2; break;                   /* JF1 */
	case 0x16: {                                                /* JTF */
		int t = c->tf;
		c->tf = 0;
		jcc(c, t);
		cyc = 2;
		break;
	}
	case 0x86: jcc(c, int_asserted(c)); cyc = 2; break;         /* JNI */
	case 0x12: case 0x32: case 0x52: case 0x72:
	case 0x92: case 0xb2: case 0xd2: case 0xf2:                 /* JBb */
		jcc(c, (c->a >> (op >> 5)) & 1);
		cyc = 2;
		break;

	/* ---- timer / counter ---- */
	case 0x42: c->a = c->timer; break;                          /* MOV A,T */
	case 0x62: c->timer = c->a; break;                          /* MOV T,A */
	case 0x55: c->timer_on = 1; c->counter_on = 0;              /* STRT T */
		c->prescaler = 0; break;
	case 0x45: c->counter_on = 1; c->timer_on = 0;              /* STRT CNT */
		c->last_t1 = (uint8_t)(t_in(c, 1) ? 1 : 0); break;
	case 0x65: c->timer_on = c->counter_on = 0; break;          /* STOP TCNT */
	case 0x25: c->tirq_en = 1; break;                           /* EN TCNTI */
	case 0x35: c->tirq_en = 0; c->timer_irq_pending = 0; break; /* DIS TCNTI */
	case 0x05: c->xirq_en = 1; break;                           /* EN I  */
	case 0x15: c->xirq_en = 0; break;                           /* DIS I */
	case 0x75: break;                                           /* ENT0 CLK */

	/* ---- ports ---- */
	case 0x08: c->a = c->io.bus_in ? c->io.bus_in(c->io.ctx) : 0xff;
		cyc = 2; break;                                     /* INS A,BUS */
	case 0x09: c->a = port_in(c, 1); cyc = 2; break;            /* IN A,P1 */
	case 0x0a: c->a = port_in(c, 2); cyc = 2; break;            /* IN A,P2 */
	case 0x02: bus_out(c, c->a); cyc = 2; break;                /* OUTL BUS,A */
	case 0x39: port_out(c, 1, c->a); cyc = 2; break;            /* OUTL P1,A */
	case 0x3a: port_out(c, 2, c->a); cyc = 2; break;            /* OUTL P2,A */
	case 0x88: bus_out(c, c->bus | fetch(c)); cyc = 2; break;   /* ORL BUS,#d */
	case 0x98: bus_out(c, c->bus & fetch(c)); cyc = 2; break;   /* ANL BUS,#d */
	case 0x89: port_out(c, 1, c->p1 | fetch(c)); cyc = 2; break; /* ORL P1,#d */
	case 0x8a: port_out(c, 2, c->p2 | fetch(c)); cyc = 2; break; /* ORL P2,#d */
	case 0x99: port_out(c, 1, c->p1 & fetch(c)); cyc = 2; break; /* ANL P1,#d */
	case 0x9a: port_out(c, 2, c->p2 & fetch(c)); cyc = 2; break; /* ANL P2,#d */
	/* 8243 expander (not fitted on the QL) */
	case 0x0c: case 0x0d: case 0x0e: case 0x0f:                 /* MOVD A,Pp */
		c->a = 0x0f; cyc = 2; break;
	case 0x3c: case 0x3d: case 0x3e: case 0x3f:                 /* MOVD Pp,A */
	case 0x8c: case 0x8d: case 0x8e: case 0x8f:                 /* ORLD Pp,A */
	case 0x9c: case 0x9d: case 0x9e: case 0x9f:                 /* ANLD Pp,A */
		cyc = 2; break;

	case 0x00:                                                  /* NOP */
	default:                                                    /* undefined */
		break;
	}

	c->cycles += (uint64_t)cyc;
	timer_tick(c, cyc);
	return cyc;
}
