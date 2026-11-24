/*
 * cycles68k.c - Clock cycle cost of every 68000/68008 instruction
 *
 * Source of the timings: M68000 Family Programmer's Reference Manual and
 * MC68000 User's Manual, section 8 (tables 8-1 to 8-14). Every entry n(r/w)
 * is split into:
 *   in : internal cycles (n - 4 * word accesses on the 68000)
 *   pw : program words read (opcode, extension words, prefetch)
 *   rb/rw/rl, wb/ww/wl : data reads/writes by size
 *
 *   68000: cycles = in + 4 * (pw + rb + rw + 2*rl + wb + ww + 2*wl)
 *   68008: cycles = in + 4 * (2*pw + rb + 2*rw + 4*rl + wb + 2*ww + 4*wl)
 *
 * DIVU/DIVS with a register divisor use the exact timing algorithm by Jorge
 * Cwik (checked on a real QL: DIVU 100000/7 takes 122 cycles, as measured).
 *
 * Known approximations:
 *  - DIVU/DIVS with a memory divisor use the worst case (140 / 158).
 *  - MULU/MULS with a memory operand use n = 8 (average case).
 *  - BCHG/BCLR/BSET on Dn use the maximum (bit number >= 16).
 *  - Interrupt acknowledge does not include the E clock synchronisation of
 *    the autovector (VPA) cycle.
 *  - ZX8301 memory contention is handled separately (zx8301.c).
 */

#include "cycles68k.h"

uint16_t cyc_table[65536];
uint8_t  cyc_kind[65536];
uint8_t  cyc_pw[65536];
uint16_t cyc_interrupt;

static int cyc_cpu = CYC_CPU_68008;

enum { SZ_B = 0, SZ_W = 1, SZ_L = 2 };

enum {
	EA_DN, EA_AN, EA_AI, EA_PI, EA_PD, EA_D16, EA_D8X,
	EA_AW, EA_AL, EA_PC16, EA_PC8X, EA_IMM, EA_BAD
};

typedef struct {
	int in, pw, rb, rw, rl, wb, ww, wl;
} Acc;

static unsigned acc_cycles(const Acc *a)
{
	int units;
	if (cyc_cpu == CYC_CPU_68000)
		units = a->pw + a->rb + a->rw + 2 * a->rl +
			a->wb + a->ww + 2 * a->wl;
	else
		units = 2 * a->pw + a->rb + 2 * a->rw + 4 * a->rl +
			a->wb + 2 * a->ww + 4 * a->wl;
	return (unsigned)(a->in + 4 * units);
}

static Acc acc_make(int in, int pw)
{
	Acc a = { 0 };
	a.in = in;
	a.pw = pw;
	return a;
}

static void acc_read(Acc *a, int sz)
{
	if (sz == SZ_B) a->rb++;
	else if (sz == SZ_W) a->rw++;
	else a->rl++;
}

static void acc_write(Acc *a, int sz)
{
	if (sz == SZ_B) a->wb++;
	else if (sz == SZ_W) a->ww++;
	else a->wl++;
}

/* ------------------------------------------------------------------ */
/* Modos de direccionamiento                                          */
/* ------------------------------------------------------------------ */

static int ea_decode(int mode, int reg)
{
	switch (mode) {
	case 0: return EA_DN;
	case 1: return EA_AN;
	case 2: return EA_AI;
	case 3: return EA_PI;
	case 4: return EA_PD;
	case 5: return EA_D16;
	case 6: return EA_D8X;
	default:
		switch (reg) {
		case 0: return EA_AW;
		case 1: return EA_AL;
		case 2: return EA_PC16;
		case 3: return EA_PC8X;
		case 4: return EA_IMM;
		default: return EA_BAD;
		}
	}
}

static int ea_of(uint16_t op)            /* EA in bits 5-0 */
{
	return ea_decode((op >> 3) & 7, op & 7);
}

static int ea_is_mem(int ea)   { return ea >= EA_AI && ea <= EA_PC8X; }
static int ea_is_reg(int ea)   { return ea == EA_DN || ea == EA_AN; }
static int ea_is_ctrl(int ea)
{
	return ea == EA_AI || (ea >= EA_D16 && ea <= EA_PC8X);
}
/* alterable en memoria (destino de escritura) */
static int ea_is_mem_alt(int ea) { return ea >= EA_AI && ea <= EA_AL; }
static int ea_is_data_alt(int ea) { return ea == EA_DN || ea_is_mem_alt(ea); }

static int ea_ext_words(int ea, int sz)
{
	switch (ea) {
	case EA_D16: case EA_D8X: case EA_AW: case EA_PC16: case EA_PC8X:
		return 1;
	case EA_AL:
		return 2;
	case EA_IMM:
		return sz == SZ_L ? 2 : 1;
	default:
		return 0;
	}
}

/* Internal cycles of the effective address calculation (table 8-1) */
static int ea_internal(int ea)
{
	return (ea == EA_PD || ea == EA_D8X || ea == EA_PC8X) ? 2 : 0;
}

/* Table 8-1: effective address calculation + operand read */
static void ea_read(Acc *a, int ea, int sz)
{
	a->pw += ea_ext_words(ea, sz);
	a->in += ea_internal(ea);
	if (ea_is_mem(ea))
		acc_read(a, sz);
}

/* ------------------------------------------------------------------ */
/* Costes especiales                                                  */
/* ------------------------------------------------------------------ */

/* Group 2 exception (TRAP, ILLEGAL, line A/F...): 34(4/3) */
static Acc acc_exception(void)
{
	Acc a = acc_make(6, 2);
	a.rl = 1;               /* vector */
	a.ww = 1;               /* SR */
	a.wl = 1;               /* PC */
	return a;
}

static void set(uint16_t op, const Acc *a, int kind)
{
	cyc_table[op] = (uint16_t)acc_cycles(a);
	cyc_kind[op] = (uint8_t)kind;
	/* Bcc/DBcc/Scc keep their cost in the dynamic part; their typical
	 * program fetch is used for memory contention. */
	if (kind == CYC_K_BCC || kind == CYC_K_DBCC)
		cyc_pw[op] = 2;
	else if (kind == CYC_K_SCC)
		cyc_pw[op] = 1;
	else
		cyc_pw[op] = (uint8_t)a->pw;
}

static void set_illegal(uint16_t op)
{
	Acc a = acc_exception();
	set(op, &a, CYC_K_NONE);
}

/* Precomputed costs of the dynamic instructions */
static unsigned c_bcc_taken, c_bcc_nt_b, c_bcc_nt_w;
static unsigned c_dbcc_true, c_dbcc_branch, c_dbcc_expired;
static unsigned c_scc_true, c_scc_false;
static unsigned c_movem_w, c_movem_l;

/* ------------------------------------------------------------------ */
/* Decoder by opcode line (bits 15-12)                               */
/* ------------------------------------------------------------------ */

/* OR/AND/SUB/ADD <ea>,Dn y Dn,<ea> ; allow_an: SUB/ADD admiten An */
static void op_dyadic(uint16_t op, int allow_an)
{
	int opmode = (op >> 6) & 7;
	int ea = ea_of(op);
	Acc a = acc_make(0, 1);

	if (opmode <= 2) {                          /* <ea>,Dn */
		int sz = opmode;
		if (ea == EA_BAD || (ea == EA_AN && (!allow_an || sz == SZ_B))) {
			set_illegal(op);
			return;
		}
		ea_read(&a, ea, sz);
		if (sz == SZ_L)
			a.in += (ea_is_reg(ea) || ea == EA_IMM) ? 4 : 2;
	} else {                                    /* Dn,<ea> */
		int sz = opmode - 4;
		if (!ea_is_mem_alt(ea)) {
			set_illegal(op);
			return;
		}
		ea_read(&a, ea, sz);
		acc_write(&a, sz);
	}
	set(op, &a, CYC_K_NONE);
}

/* ABCD / SBCD / ADDX / SUBX (register and -(An) forms) */
static void op_extended(uint16_t op, int sz)
{
	Acc a = acc_make(0, 1);
	if (op & 0x0008) {                  /* -(Ay),-(Ax) */
		a.in = 2;
		acc_read(&a, sz);
		acc_read(&a, sz);
		acc_write(&a, sz);
	} else if (sz == SZ_L) {
		a.in = 4;                       /* 8(1/0) */
	}
	set(op, &a, CYC_K_NONE);
}

static void line0(uint16_t op)
{
	int ea = ea_of(op);

	if ((op & 0x0138) == 0x0108) {                  /* MOVEP */
		Acc a = acc_make(0, 2);
		int n = (op & 0x0040) ? 4 : 2;
		if (op & 0x0080) a.wb = n; else a.rb = n;
		set(op, &a, CYC_K_NONE);
		return;
	}

	if (op & 0x0100) {                              /* dynamic bit */
		int t = (op >> 6) & 3;          /* 0 BTST 1 BCHG 2 BCLR 3 BSET */
		Acc a = acc_make(0, 1);
		if (ea == EA_DN) {
			static const int in_dn[4] = { 2, 4, 6, 4 };
			a.in = in_dn[t];
		} else if (t == 0 && ea != EA_AN && ea != EA_BAD) {
			ea_read(&a, ea, SZ_B);
		} else if (ea_is_mem_alt(ea)) {
			ea_read(&a, ea, SZ_B);
			acc_write(&a, SZ_B);
		} else {
			set_illegal(op);
			return;
		}
		set(op, &a, CYC_K_NONE);
		return;
	}

	if ((op & 0x0F00) == 0x0800) {                  /* static bit */
		int t = (op >> 6) & 3;
		Acc a = acc_make(0, 2);
		if (ea == EA_DN) {
			static const int in_dn[4] = { 2, 4, 6, 4 };
			a.in = in_dn[t];
		} else if (t == 0 && ea_is_mem(ea)) {
			ea_read(&a, ea, SZ_B);
		} else if (ea_is_mem_alt(ea)) {
			ea_read(&a, ea, SZ_B);
			acc_write(&a, SZ_B);
		} else {
			set_illegal(op);
			return;
		}
		set(op, &a, CYC_K_NONE);
		return;
	}

	{                                               /* inmediatos */
		int t = (op >> 9) & 7;  /* 0 OR 1 AND 2 SUB 3 ADD 5 EOR 6 CMP */
		int sz = (op >> 6) & 3;
		Acc a;

		if (t == 4 || t == 7 || sz == 3) {
			set_illegal(op);
			return;
		}
		if (ea == EA_IMM) {                     /* a CCR / SR */
			if ((t == 0 || t == 1 || t == 5) && sz != SZ_L) {
				a = acc_make(8, 3);     /* 20(3/0) */
				set(op, &a, CYC_K_NONE);
			} else {
				set_illegal(op);
			}
			return;
		}

		a = acc_make(0, 1 + (sz == SZ_L ? 2 : 1));
		if (ea == EA_DN) {
			if (sz == SZ_L)
				a.in = (t == 6) ? 2 : 4;
		} else if (ea_is_mem_alt(ea)) {
			ea_read(&a, ea, sz);
			if (t != 6)
				acc_write(&a, sz);
		} else {
			set_illegal(op);
			return;
		}
		set(op, &a, CYC_K_NONE);
	}
}

static void line_move(uint16_t op)
{
	int line = op >> 12;
	int sz = (line == 1) ? SZ_B : (line == 3) ? SZ_W : SZ_L;
	int src = ea_of(op);
	int dst = ea_decode((op >> 6) & 7, (op >> 9) & 7);
	Acc a = acc_make(0, 1);

	if (src == EA_BAD || (src == EA_AN && sz == SZ_B) ||
	    !(ea_is_data_alt(dst) || dst == EA_AN) ||
	    (dst == EA_AN && sz == SZ_B)) {
		set_illegal(op);
		return;
	}

	ea_read(&a, src, sz);
	if (!ea_is_reg(dst)) {
		/* Tables 8-2/8-3: write; no extra cycles for a -(An) destination */
		acc_write(&a, sz);
		a.pw += ea_ext_words(dst, sz);
		if (dst == EA_D8X)
			a.in += 2;
	}
	set(op, &a, CYC_K_NONE);
}

/* JMP/JSR/LEA/PEA from table 8-10: {in, pw} per control mode */
static int ctrl_index(int ea)
{
	switch (ea) {
	case EA_AI:   return 0;
	case EA_D16:  return 1;
	case EA_D8X:  return 2;
	case EA_AW:   return 3;
	case EA_AL:   return 4;
	case EA_PC16: return 5;
	case EA_PC8X: return 6;
	default:      return -1;
	}
}

static void line4(uint16_t op)
{
	int ea = ea_of(op);
	int sz = (op >> 6) & 3;
	Acc a;

	/* Instructions without operands */
	switch (op) {
	case 0x4AFC: set_illegal(op); return;                   /* ILLEGAL */
	case 0x4E70: a = acc_make(128, 1); set(op, &a, 0); return; /* RESET */
	case 0x4E71: a = acc_make(0, 1);   set(op, &a, 0); return; /* NOP */
	case 0x4E72: a = acc_make(4, 0);   set(op, &a, 0); return; /* STOP */
	case 0x4E73:                                            /* RTE */
	case 0x4E77:                                            /* RTR */
		a = acc_make(0, 2); a.rw = 1; a.rl = 1;
		set(op, &a, 0); return;
	case 0x4E75:                                            /* RTS */
		a = acc_make(0, 2); a.rl = 1;
		set(op, &a, 0); return;
	case 0x4E76: a = acc_make(0, 1);   set(op, &a, 0); return; /* TRAPV */
	default: break;
	}

	if ((op & 0xFFF0) == 0x4E40) {                          /* TRAP */
		a = acc_exception();
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFFF8) == 0x4E50) {                          /* LINK */
		a = acc_make(0, 2); a.wl = 1;
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFFF8) == 0x4E58) {                          /* UNLK */
		a = acc_make(0, 1); a.rl = 1;
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFFF0) == 0x4E60) {                          /* MOVE USP */
		a = acc_make(0, 1);
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFF80) == 0x4E80) {                          /* JSR / JMP */
		static const int jmp_in[7] = { 0, 2, 2, 2, 0, 2, 2 };
		static const int jmp_pw[7] = { 2, 2, 3, 2, 3, 2, 3 };
		static const int jsr_in[7] = { 0, 2, 6, 2, 0, 2, 6 };
		static const int jsr_pw[7] = { 2, 2, 2, 2, 3, 2, 2 };
		int i = ctrl_index(ea);
		if (i < 0) { set_illegal(op); return; }
		if (op & 0x0040) {                              /* JMP */
			a = acc_make(jmp_in[i], jmp_pw[i]);
		} else {                                        /* JSR */
			a = acc_make(jsr_in[i], jsr_pw[i]);
			a.wl = 1;
		}
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFFF8) == 0x4840) {                          /* SWAP */
		a = acc_make(0, 1);
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFFC0) == 0x4840) {                          /* PEA */
		int i = ctrl_index(ea);
		if (i < 0) { set_illegal(op); return; }
		a = acc_make((ea == EA_D8X || ea == EA_PC8X) ? 4 : 0,
			     1 + ea_ext_words(ea, SZ_L));
		a.wl = 1;
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFFB8) == 0x4880) {                          /* EXT */
		a = acc_make(0, 1);
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFB80) == 0x4880) {                          /* MOVEM */
		int to_reg = (op & 0x0400) != 0;
		int is_l = (op & 0x0040) != 0;
		int ok = to_reg ? (ea_is_ctrl(ea) || ea == EA_PI)
				: ((ea_is_ctrl(ea) && ea <= EA_AL) || ea == EA_PD);
		if (!ok) { set_illegal(op); return; }
		a = acc_make((ea == EA_D8X || ea == EA_PC8X) ? 2 : 0,
			     2 + ea_ext_words(ea, SZ_W));
		if (to_reg)
			a.rw = 1;       /* extra read at the end (table 8-10) */
		set(op, &a, is_l ? CYC_K_MOVEM_L : CYC_K_MOVEM_W);
		return;
	}
	if ((op & 0x01C0) == 0x01C0) {                          /* LEA */
		int i = ctrl_index(ea);
		if (i < 0) { set_illegal(op); return; }
		a = acc_make((ea == EA_D8X || ea == EA_PC8X) ? 4 : 0,
			     1 + ea_ext_words(ea, SZ_L));
		set(op, &a, 0);
		return;
	}
	if ((op & 0x01C0) == 0x0180) {                          /* CHK */
		if (ea == EA_BAD || ea == EA_AN) { set_illegal(op); return; }
		a = acc_make(6, 1);
		ea_read(&a, ea, SZ_W);
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFFC0) == 0x40C0) {                          /* MOVE from SR */
		if (!ea_is_data_alt(ea)) { set_illegal(op); return; }
		if (ea == EA_DN) {
			a = acc_make(2, 1);
		} else {
			a = acc_make(0, 1);
			ea_read(&a, ea, SZ_W);
			acc_write(&a, SZ_W);
		}
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFFC0) == 0x44C0 || (op & 0xFFC0) == 0x46C0) { /* to CCR/SR */
		if (ea == EA_BAD || ea == EA_AN) { set_illegal(op); return; }
		a = acc_make(4, 2);
		ea_read(&a, ea, SZ_W);
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFFC0) == 0x4800) {                          /* NBCD */
		if (ea == EA_DN) {
			a = acc_make(2, 1);
		} else if (ea_is_mem_alt(ea)) {
			a = acc_make(0, 1);
			ea_read(&a, ea, SZ_B);
			acc_write(&a, SZ_B);
		} else {
			set_illegal(op);
			return;
		}
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFFC0) == 0x4AC0) {                          /* TAS */
		if (ea == EA_DN) {
			a = acc_make(0, 1);
		} else if (ea_is_mem_alt(ea)) {
			a = acc_make(2, 1);
			ea_read(&a, ea, SZ_B);
			acc_write(&a, SZ_B);
		} else {
			set_illegal(op);
			return;
		}
		set(op, &a, 0);
		return;
	}
	if ((op & 0xFF00) == 0x4A00 && sz != 3) {               /* TST */
		if (!ea_is_data_alt(ea)) { set_illegal(op); return; }
		a = acc_make(0, 1);
		ea_read(&a, ea, sz);
		set(op, &a, 0);
		return;
	}
	if (sz != 3 && ((op & 0xFF00) == 0x4000 || (op & 0xFF00) == 0x4200 ||
			(op & 0xFF00) == 0x4400 || (op & 0xFF00) == 0x4600)) {
		/* NEGX / CLR / NEG / NOT */
		if (ea == EA_DN) {
			a = acc_make(sz == SZ_L ? 2 : 0, 1);
		} else if (ea_is_mem_alt(ea)) {
			a = acc_make(0, 1);
			ea_read(&a, ea, sz);
			acc_write(&a, sz);
		} else {
			set_illegal(op);
			return;
		}
		set(op, &a, 0);
		return;
	}

	set_illegal(op);
}

static void line5(uint16_t op)
{
	int ea = ea_of(op);
	int sz = (op >> 6) & 3;
	Acc a;

	if (sz == 3) {
		if (ea == EA_AN) {                              /* DBcc */
			a = acc_make(0, 0);
			set(op, &a, CYC_K_DBCC);
		} else if (ea == EA_DN) {                       /* Scc Dn */
			a = acc_make(0, 0);
			set(op, &a, CYC_K_SCC);
		} else if (ea_is_mem_alt(ea)) {                 /* Scc <ea> */
			a = acc_make(0, 1);
			ea_read(&a, ea, SZ_B);
			acc_write(&a, SZ_B);
			set(op, &a, 0);
		} else {
			set_illegal(op);
		}
		return;
	}

	/* ADDQ / SUBQ */
	a = acc_make(0, 1);
	if (ea == EA_DN) {
		if (sz == SZ_L) a.in = 4;
	} else if (ea == EA_AN) {
		if (sz == SZ_B) { set_illegal(op); return; }
		a.in = 4;
	} else if (ea_is_mem_alt(ea)) {
		ea_read(&a, ea, sz);
		acc_write(&a, sz);
	} else {
		set_illegal(op);
		return;
	}
	set(op, &a, 0);
}

static void line6(uint16_t op)
{
	int cc = (op >> 8) & 15;
	Acc a;

	if (cc == 0) {                                          /* BRA */
		a = acc_make(2, 2);
		set(op, &a, 0);
	} else if (cc == 1) {                                   /* BSR */
		a = acc_make(2, 2);
		a.wl = 1;
		set(op, &a, 0);
	} else {                                                /* Bcc */
		a = acc_make(0, 0);
		set(op, &a, CYC_K_BCC);
	}
}

static void line8_c(uint16_t op, int is_and)
{
	int opmode = (op >> 6) & 7;
	int mode = (op >> 3) & 7;
	int ea = ea_of(op);
	Acc a;

	if (opmode == 3 || opmode == 7) {               /* DIV / MUL */
		if (ea == EA_BAD || ea == EA_AN) { set_illegal(op); return; }
		if (!is_and && ea == EA_DN) {
			/* Exact time in cycles_dynamic() (Cwik's algorithm, which
			 * already includes the 68000 opcode fetch); the table only
			 * holds what the 68008 adds to that fetch. */
			a = acc_make(0, 1);
			set(op, &a, opmode == 3 ? CYC_K_DIVU : CYC_K_DIVS);
			cyc_table[op] = (uint16_t)(acc_cycles(&a) - 4);
		} else if (!is_and) {
			/* DIVU 140(1/0)+ / DIVS 158(1/0)+ (worst case) */
			a = acc_make(opmode == 3 ? 136 : 154, 1);
			ea_read(&a, ea, SZ_W);
			set(op, &a, 0);
		} else {
			/* MULU/MULS 38+2n(1/0)+ */
			a = acc_make(34, 1);
			ea_read(&a, ea, SZ_W);
			if (ea == EA_DN) {
				set(op, &a, opmode == 3 ? CYC_K_MULU : CYC_K_MULS);
			} else {
				a.in += 16;     /* n = 8 on average */
				set(op, &a, 0);
			}
		}
		return;
	}
	if (opmode == 4 && mode <= 1) {                 /* ABCD / SBCD */
		a = acc_make(2, 1);
		if (mode == 1) {
			op_extended(op, SZ_B);
			return;
		}
		set(op, &a, 0);
		return;
	}
	if (is_and && ((opmode == 5 && mode <= 1) ||
		       (opmode == 6 && mode == 1))) {   /* EXG */
		a = acc_make(2, 1);
		set(op, &a, 0);
		return;
	}
	op_dyadic(op, 0);                               /* OR / AND */
}

static void line9_d(uint16_t op)                        /* SUB / ADD */
{
	int opmode = (op >> 6) & 7;
	int mode = (op >> 3) & 7;
	int ea = ea_of(op);
	Acc a;

	if (opmode == 3 || opmode == 7) {                       /* ADDA/SUBA */
		int sz = (opmode == 3) ? SZ_W : SZ_L;
		if (ea == EA_BAD) { set_illegal(op); return; }
		a = acc_make(0, 1);
		ea_read(&a, ea, sz);
		if (sz == SZ_W)
			a.in += 4;
		else
			a.in += (ea_is_reg(ea) || ea == EA_IMM) ? 4 : 2;
		set(op, &a, 0);
		return;
	}
	if (opmode >= 4 && mode <= 1) {                         /* ADDX/SUBX */
		op_extended(op, opmode - 4);
		return;
	}
	op_dyadic(op, 1);
}

static void lineB(uint16_t op)                          /* CMP / EOR */
{
	int opmode = (op >> 6) & 7;
	int ea = ea_of(op);
	Acc a;

	if (opmode == 3 || opmode == 7) {                       /* CMPA */
		if (ea == EA_BAD) { set_illegal(op); return; }
		a = acc_make(2, 1);
		ea_read(&a, ea, opmode == 3 ? SZ_W : SZ_L);
		set(op, &a, 0);
		return;
	}
	if (opmode <= 2) {                                      /* CMP */
		if (ea == EA_BAD || (ea == EA_AN && opmode == SZ_B)) {
			set_illegal(op);
			return;
		}
		a = acc_make(opmode == SZ_L ? 2 : 0, 1);
		ea_read(&a, ea, opmode);
		set(op, &a, 0);
		return;
	}
	if (ea == EA_AN) {                                      /* CMPM */
		int sz = opmode - 4;
		a = acc_make(0, 1);
		acc_read(&a, sz);
		acc_read(&a, sz);
		set(op, &a, 0);
		return;
	}
	/* EOR Dn,<ea> */
	{
		int sz = opmode - 4;
		if (ea == EA_DN) {
			a = acc_make(sz == SZ_L ? 4 : 0, 1);
		} else if (ea_is_mem_alt(ea)) {
			a = acc_make(0, 1);
			ea_read(&a, ea, sz);
			acc_write(&a, sz);
		} else {
			set_illegal(op);
			return;
		}
		set(op, &a, 0);
	}
}

static void lineE(uint16_t op)                          /* shifts */
{
	int sz = (op >> 6) & 3;
	Acc a;

	if (sz == 3) {                                  /* en memoria, .W */
		int ea = ea_of(op);
		if ((op & 0x0800) || !ea_is_mem_alt(ea)) {
			set_illegal(op);
			return;
		}
		a = acc_make(0, 1);
		ea_read(&a, ea, SZ_W);
		acc_write(&a, SZ_W);
		set(op, &a, 0);
		return;
	}

	a = acc_make(sz == SZ_L ? 4 : 2, 1);            /* 6+2n / 8+2n (1/0) */
	if (op & 0x0020) {
		set(op, &a, CYC_K_SHIFT);               /* contador en Dn */
	} else {
		int n = (op >> 9) & 7;
		a.in += 2 * (n ? n : 8);
		set(op, &a, 0);
	}
}

void cycles_init(int cpu)
{
	Acc a;
	unsigned op;

	cyc_cpu = cpu;

	/* Precomputed dynamic costs */
	a = acc_make(2, 2); c_bcc_taken = acc_cycles(&a);       /* 10(2/0) */
	a = acc_make(4, 1); c_bcc_nt_b = acc_cycles(&a);        /*  8(1/0) */
	a = acc_make(4, 2); c_bcc_nt_w = acc_cycles(&a);        /* 12(2/0) */
	a = acc_make(4, 2); c_dbcc_true = acc_cycles(&a);       /* 12(2/0) */
	a = acc_make(2, 2); c_dbcc_branch = acc_cycles(&a);     /* 10(2/0) */
	a = acc_make(2, 3); c_dbcc_expired = acc_cycles(&a);    /* 14(3/0) */
	a = acc_make(2, 1); c_scc_true = acc_cycles(&a);        /*  6(1/0) */
	a = acc_make(0, 1); c_scc_false = acc_cycles(&a);       /*  4(1/0) */
	a = acc_make(0, 0); a.rw = 1; c_movem_w = acc_cycles(&a);
	a = acc_make(0, 0); a.rl = 1; c_movem_l = acc_cycles(&a);

	/* Autovectored interrupt: 44(5/3), the IACK cycle is a byte cycle */
	a = acc_exception();
	a.in = 12;
	a.rb = 1;
	cyc_interrupt = (uint16_t)acc_cycles(&a);

	for (op = 0; op < 65536; op++) {
		uint16_t o = (uint16_t)op;
		cyc_kind[o] = CYC_K_NONE;
		switch (o >> 12) {
		case 0x0: line0(o); break;
		case 0x1: case 0x2: case 0x3: line_move(o); break;
		case 0x4: line4(o); break;
		case 0x5: line5(o); break;
		case 0x6: line6(o); break;
		case 0x7:
			if (o & 0x0100) {
				set_illegal(o);
			} else {
				a = acc_make(0, 1);             /* MOVEQ */
				set(o, &a, 0);
			}
			break;
		case 0x8: line8_c(o, 0); break;
		case 0x9: case 0xD: line9_d(o); break;
		case 0xB: lineB(o); break;
		case 0xC: line8_c(o, 1); break;
		case 0xE: lineE(o); break;
		default: set_illegal(o); break;         /* lines A and F */
		}
	}
}

static unsigned popcount16(unsigned v)
{
	unsigned n = 0;
	v &= 0xFFFF;
	while (v) {
		v &= v - 1;
		n++;
	}
	return n;
}

/* DIVU: 68000 cycles (algorithm by Jorge Cwik) */
static unsigned divu_cycles(uint32_t dividend, uint16_t divisor)
{
	uint32_t hdivisor = (uint32_t)divisor << 16;
	unsigned mcycles = 38;
	int i;

	if ((dividend >> 16) >= divisor)        /* desbordamiento (o /0) */
		return 10;
	for (i = 0; i < 15; i++) {
		uint32_t temp = dividend;
		dividend <<= 1;
		if (temp & 0x80000000u) {
			dividend -= hdivisor;
		} else {
			mcycles += 2;
			if (dividend >= hdivisor) {
				dividend -= hdivisor;
				mcycles--;
			}
		}
	}
	return mcycles * 2;
}

/* DIVS: 68000 cycles (algorithm by Jorge Cwik) */
static unsigned divs_cycles(int32_t dividend, int16_t divisor)
{
	uint32_t adividend = dividend < 0 ? 0u - (uint32_t)dividend : (uint32_t)dividend;
	uint32_t adivisor = divisor < 0 ? (uint32_t)(-(int32_t)divisor) : (uint32_t)divisor;
	unsigned mcycles = 6;
	uint32_t aquot;
	int i;

	if (dividend < 0)
		mcycles++;
	if (adivisor == 0 || (adividend >> 16) >= adivisor)
		return (mcycles + 2) * 2;
	aquot = adividend / adivisor;
	mcycles += 55;
	if (divisor >= 0) {
		if (dividend >= 0)
			mcycles--;
		else
			mcycles++;
	}
	for (i = 0; i < 15; i++) {
		if (!(aquot & 0x8000))
			mcycles++;
		aquot <<= 1;
	}
	return mcycles * 2;
}

unsigned cycles_dynamic(uint16_t op, int cond, const int32_t *dregs,
			uint16_t ext)
{
	switch (cyc_kind[op]) {
	case CYC_K_BCC:
		if (cond)
			return c_bcc_taken;
		return (op & 0xFF) ? c_bcc_nt_b : c_bcc_nt_w;

	case CYC_K_DBCC:
		if (cond)
			return c_dbcc_true;
		/* Dn.W reaches -1: the loop terminates */
		return ((uint16_t)dregs[op & 7] == 0) ? c_dbcc_expired
						      : c_dbcc_branch;

	case CYC_K_SCC:
		return cond ? c_scc_true : c_scc_false;

	case CYC_K_SHIFT:
		return 2u * ((uint32_t)dregs[(op >> 9) & 7] & 63u);

	case CYC_K_MOVEM_W:
		return c_movem_w * popcount16(ext);

	case CYC_K_MOVEM_L:
		return c_movem_l * popcount16(ext);

	case CYC_K_MULU:
		return 2u * popcount16((uint32_t)dregs[op & 7]);

	case CYC_K_MULS: {
		uint32_t x = ((uint32_t)dregs[op & 7] & 0xFFFF) << 1;
		return 2u * popcount16((x ^ (x >> 1)) & 0xFFFF);
	}

	case CYC_K_DIVU:
		return divu_cycles((uint32_t)dregs[(op >> 9) & 7],
				   (uint16_t)dregs[op & 7]);

	case CYC_K_DIVS:
		return divs_cycles(dregs[(op >> 9) & 7], (int16_t)dregs[op & 7]);

	default:
		return 0;
	}
}

#ifdef CYCLES68K_TOOL
/*
 * Standalone tool:
 *   cc -DCYCLES68K_TOOL -o cycles_tool cycles68k.c
 *   ./cycles_tool 68000 table68000.bin     (dump table + kinds)
 *   ./cycles_tool 68008                    (print a few samples)
 */
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	int cpu = (argc > 1 && !strcmp(argv[1], "68000")) ? CYC_CPU_68000
							  : CYC_CPU_68008;
	cycles_init(cpu);

	if (argc > 2) {
		FILE *f = fopen(argv[2], "wb");
		if (!f) { perror(argv[2]); return 1; }
		fwrite(cyc_table, sizeof(cyc_table), 1, f);
		fwrite(cyc_kind, sizeof(cyc_kind), 1, f);
		fclose(f);
		printf("%s table written to %s\n", cpu ? "68008" : "68000",
		       argv[2]);
		return 0;
	}

	{
		static const struct { uint16_t op; const char *txt; } samples[] = {
			{ 0x4E71, "NOP" },
			{ 0x3010, "MOVE.W (A0),D0" },
			{ 0x2010, "MOVE.L (A0),D0" },
			{ 0x10D8, "MOVE.B (A0)+,(A0)+" },
			{ 0x3228, "MOVE.W d16(A0),D1" },
			{ 0xD081, "ADD.L D1,D0" },
			{ 0xD090, "ADD.L (A0),D0" },
			{ 0x0680, "ADDI.L #imm,D0" },
			{ 0x5280, "ADDQ.L #1,D0" },
			{ 0x0812, "BTST #n,(A2)" },
			{ 0x4E75, "RTS" },
			{ 0x4EB9, "JSR abs.L" },
			{ 0x4E40, "TRAP #0" },
			{ 0xE348, "LSL.W #1,D0" },
		};
		unsigned i;
		printf("CPU %s\n", cpu ? "68008" : "68000");
		for (i = 0; i < sizeof(samples) / sizeof(samples[0]); i++)
			printf("  %04X %-22s %3u\n", samples[i].op, samples[i].txt,
			       cyc_table[samples[i].op]);
		printf("  Bcc.B taken / not taken: %u / %u\n", c_bcc_taken,
		       c_bcc_nt_b);
		printf("  DBcc branch / expired / cc true: %u / %u / %u\n",
		       c_dbcc_branch, c_dbcc_expired, c_dbcc_true);
		printf("  interrupt: %u\n", cyc_interrupt);
	}
	return 0;
}
#endif
