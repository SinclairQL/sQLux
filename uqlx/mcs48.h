/*
 * mcs48.h - Intel MCS-48 (8048/8049) microcontroller core
 *
 * Instruction set, timing (1 or 2 machine cycles per instruction), timer,
 * event counter, external and timer interrupts, and the quasi-bidirectional
 * ports P1, P2 and BUS. External hardware is connected through callbacks.
 * One machine cycle is 15 oscillator periods (733333 cycles/s at 11 MHz).
 */

#ifndef MCS48_H
#define MCS48_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mcs48 mcs48_t;

typedef struct {
	void    *ctx;
	uint8_t (*p1_in)(void *ctx);            /* external P1 pins */
	uint8_t (*p2_in)(void *ctx);            /* external P2 pins */
	uint8_t (*bus_in)(void *ctx);           /* INS A,BUS */
	void    (*p1_out)(void *ctx, uint8_t v);
	void    (*p2_out)(void *ctx, uint8_t v);
	void    (*bus_out)(void *ctx, uint8_t v);
	uint8_t (*movx_rd)(void *ctx, uint8_t addr);
	void    (*movx_wr)(void *ctx, uint8_t addr, uint8_t v);
	int     (*t0_in)(void *ctx);
	int     (*t1_in)(void *ctx);
	int     (*int_in)(void *ctx);           /* INT pin level, 0 = asserted */
} mcs48_io_t;

struct mcs48 {
	/* program and data memory */
	const uint8_t *rom;
	uint16_t rom_mask;                      /* 0x7ff for 2K */
	uint8_t  ram[128];
	uint8_t  ram_mask;                      /* 0x7f for the 8049 */

	/* registers */
	uint16_t pc;
	uint8_t  a;
	uint8_t  psw;                           /* CY AC F0 BS 1 SP2 SP1 SP0 */
	uint8_t  f1;
	uint8_t  a11;                           /* memory bank flip-flop */
	uint8_t  a11_pending;                   /* SEL MB0/MB1 */

	/* ports */
	uint8_t  p1, p2, bus;

	/* timer / counter */
	uint8_t  timer;
	uint8_t  prescaler;
	uint8_t  timer_on, counter_on;
	uint8_t  tf;                            /* timer flag (JTF) */
	uint8_t  timer_irq_pending;
	uint8_t  last_t1;

	/* interrupts */
	uint8_t  xirq_en, tirq_en;
	uint8_t  in_irq;

	uint64_t cycles;                        /* machine cycles executed */
	mcs48_io_t io;
};

void mcs48_init(mcs48_t *c, const uint8_t *rom, uint16_t rom_size,
		const mcs48_io_t *io);
void mcs48_reset(mcs48_t *c);
/* Execute one instruction (servicing a pending interrupt first) and return
 * the number of machine cycles it took. */
int  mcs48_step(mcs48_t *c);

#ifdef __cplusplus
}
#endif

#endif /* MCS48_H */
