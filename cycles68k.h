/*
 * cycles68k.h - Clock cycle cost of every 68000/68008 instruction
 *
 * Every entry n(r/w) of the Motorola timing tables is split into internal
 * cycles and bus accesses. On the 68000 a word access takes 4 cycles; on the
 * 68008 (8 bit data bus) a word takes two 4 cycle byte accesses, a byte one
 * and a long word four. The same description therefore produces the 68000
 * table (which can be checked against the Tom Harte ProcessorTests) and the
 * 68008 table used for the QL.
 */

#ifndef CYCLES68K_H
#define CYCLES68K_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CYC_CPU_68000 0
#define CYC_CPU_68008 1

/* Instructions whose cost depends on the CPU state */
enum {
	CYC_K_NONE = 0,
	CYC_K_BCC,      /* Bcc: taken / not taken (.b or .w)           */
	CYC_K_DBCC,     /* DBcc: cc true / branch / counter expired     */
	CYC_K_SCC,      /* Scc Dn: true / false                        */
	CYC_K_SHIFT,    /* shift with count in a register: +2n         */
	CYC_K_MOVEM_W,  /* MOVEM.W: cost per register                  */
	CYC_K_MOVEM_L,  /* MOVEM.L: cost per register                  */
	CYC_K_MULU,     /* MULU Dn: +2 per set bit of the operand      */
	CYC_K_MULS,     /* MULS Dn: +2 per 01/10 transition            */
	CYC_K_DIVU,     /* DIVU Dn,Dn: depends on the quotient bits    */
	CYC_K_DIVS      /* DIVS Dn,Dn: depends on the quotient bits    */
};

/* Fixed cost of every opcode (for dynamic ones, the fixed part) */
extern uint16_t cyc_table[65536];
/* Dynamic kind of every opcode (CYC_K_NONE for most of them) */
extern uint8_t  cyc_kind[65536];
/* Program words read by every opcode (opcode, extension words and
 * prefetch), used for memory contention when code runs from RAM */
extern uint8_t  cyc_pw[65536];
/* Cost of accepting an autovectored interrupt */
extern uint16_t cyc_interrupt;

/* Build the tables for CYC_CPU_68000 or CYC_CPU_68008 */
void cycles_init(int cpu);

/*
 * Dynamic part of the cost of an opcode with cyc_kind != CYC_K_NONE.
 *   cond  : result of the cc condition (Bcc, DBcc and Scc only)
 *   dregs : D0-D7 before the instruction is executed
 *   ext   : word following the opcode (MOVEM register mask)
 */
unsigned cycles_dynamic(uint16_t op, int cond, const int32_t *dregs,
			uint16_t ext);

#ifdef __cplusplus
}
#endif

#endif /* CYCLES68K_H */
