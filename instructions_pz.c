/*
 * (c) UQLX - see COPYRIGHT
 */

#include "QL68000.h"

#ifdef DEBUG
extern int trace_rts;
#endif

void pea(void)
{
	register w32 ea;
	ea = ARCALL(GetEA, (code >> 3) & 7,
		    (code & 7)); /* first compute effective address */
	/* ea=GET_EA((code>>3)&7,code&7);*/
	(*m68k_sp) -= 4; /* then
							     push onto the stack */
	WriteLong(*m68k_sp, ea);
}

void reset(void)
{
	if (!supervisor) {
		exception = 8;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void rol_m(void)
{
	register uw16 d;
	;
	d = (uw16)ModifyAtEA_w((code >> 3) & 7, code & 7);
	carry = (d & 0x8000) != 0;
	d <<= 1;
	if (carry)
		d |= 1;
	negative = (d & 0x8000) != 0;
	zero = d == 0;
	overflow = false;
	RewriteEA_w((w16)d);
}

void ror_m(void)
{
	register uw16 d;
	;
	d = (uw16)ModifyAtEA_w((code >> 3) & 7, code & 7);
	carry = negative = (d & 1) != 0;
	d >>= 1;
	if (carry)
		d |= 0x8000;
	zero = d == 0;
	overflow = false;
	RewriteEA_w((w16)d);
}

void roxl_m(void)
{
	register uw16 d;
	;
	d = (uw16)ModifyAtEA_w((code >> 3) & 7, code & 7);
	carry = (d & 0x8000) != 0;
	d <<= 1;
	if (xflag)
		d |= 1;
	xflag = carry;
	negative = (d & 0x8000) != 0;
	zero = d == 0;
	overflow = false;
	RewriteEA_w((w16)d);
}

void roxr_m(void)
{
	register uw16 d;
	;
	d = (uw16)ModifyAtEA_w((code >> 3) & 7, code & 7);
	carry = (d & 1) != 0;
	negative = xflag;
	d >>= 1;
	if (xflag)
		d |= 0x8000;
	xflag = carry;
	zero = d == 0;
	overflow = false;
	RewriteEA_w((w16)d);
}

void rte(void)
{
	register w16 sr;
	if (supervisor) {
		sr = ReadWord(*m68k_sp);
#ifdef BACKTRACE
		SetPCB(ReadLong((*m68k_sp) + 2), RTE);
#else
		SetPC(ReadLong((*m68k_sp) + 2));
#endif
		(*m68k_sp) += 6;
		ExceptionOut();
		PutSR(sr);

#ifdef DEBUG
		if (trace_rts-- > 0)
			dbginfo("returned by RTE\n");
#endif
	} else {
		exception = 8;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void rtr(void)
{
	register w16 cc;
	cc = ReadWord((*m68k_sp));
#ifdef BACKTRACE
	SetPCB(ReadLong((*m68k_sp) + 2), RTR);
#else
	SetPC(ReadLong((*m68k_sp) + 2));
#endif
	(*m68k_sp) += 6;
	xflag = (cc & 16) != 0;
	negative = (cc & 8) != 0;
	zero = (cc & 4) != 0;
	overflow = (cc & 2) != 0;
	carry = (cc & 1) != 0;

#ifdef DEBUG
	if (trace_rts-- > 0)
		dbginfo("returned by RTR\n");
#endif
}

void rts(void)
{
#ifdef BACKTRACE
	SetPCB((uw16 *)((Ptr)theROM + (ReadLong(*m68k_sp) & ADDR_MASK)), RTS);
#else
	/* uggly cast to avoid warning */
	if ((((char)(int)(pc = (uw16 *)((Ptr)theROM +
					(ReadLong(*m68k_sp) & ADDR_MASK)))) &
	     1) != 0) {
		exception = 3;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
		readOrWrite = 16;
		badAddress = ReadLong(*m68k_sp);
		badCodeAddress = true;
	}
#endif
	(*m68k_sp) += 4;

#ifdef DEBUG
	if (trace_rts-- > 0)
		dbginfo("returned from RTS\n");
#endif

#ifdef TRACE
	CheckTrace();
#endif
}

void sbcd(void)
{
	w8 s, d, r;
	w8 s2, d2, r2;
	w8 *dx;
	if ((code & 8) != 0) {
		s = GetFromEA_b_m4();
		d = ModifyAtEA_b(4, (code >> 9) & 7);
	} else {
		dx = (w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO);
		d = *dx;
		s = (w8)reg[code & 7];
	}
	s2 = ((s & 0x0f) > 9 ? 9 : (s & 0x0f));
	s >>= 4;
	if (s >= 9)
		s2 += 90;
	else
		s2 += s * 10;
	d2 = ((d & 0x0f) > 9 ? 9 : (d & 0x0f));
	d >>= 4;
	if (d >= 9)
		d2 += 90;
	else
		d2 += d * 10;
	r2 = d2 - s2;
	if (xflag)
		r2--;
	if (xflag = carry = r2 < 0)
		r2 += 100;
	zero = zero && r2 == 0;
	r = (r2 % 10) + ((r2 / 10) << 4);
	if ((code & 8) != 0)
		RewriteEA_b(r);
	else
		*dx = r;
}

void scc(void)
{
	ARCALL(PutToEA_b, (code >> 3) & 7, code & 7,
	       ConditionTrue[(code >> 8) & 15]() ? 0xff : 0);
	/* PUT_TOEA_B((code>>3)&7,code&7,ConditionTrue[(code>>8)&15]()? 0xff:0);*/
}

void st(void)
{
	ARCALL(PutToEA_b, (code >> 3) & 7, code & 7, 0xff);
	/*PUT_TOEA_B((code>>3)&7,code&7,0xff);*/
}

void sf(void)
{
	ARCALL(PutToEA_b, (code >> 3) & 7, code & 7, 0);
	/* PUT_TOEA_B((code>>3)&7,code&7,0); */
}

void stop(void)
{
	pc++;
	if (supervisor) {
		PutSR(RW(pc - 1));
		if (exception == 0) /* to avoid mess with interrupts */
		{
			if (supervisor) {
				stopped = true;
				nInst = nInst2 = 0;
			} else {
				exception = 8;
				extraFlag = true;
				nInst2 = nInst;
				nInst = 0;
			}
		}
	} else {
		exception = 8;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void sub_b_dn(void)
{
	w8 r, s;
	w8 *d;
	s = GetFromEA_b[(code >> 3) & 7]();
	d = (w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO);
	r = *d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((*d & 0x80) == 0) && (((s | r) & 0x80) != 0)) ||
			((s & r & 0x80) != 0);
	overflow = ((0x80 & (s ^ 0x80) & *d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (*d ^ 0x80) & s) != 0);
	*d = r;
}

void sub_w_dn(void)
{
	w16 r, s;
	w16 *d;
	s = GetFromEA_w[(code >> 3) & 7]();
	d = (w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO);
	r = *d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((*d & 0x8000) == 0) && (((s | r) & 0x8000) != 0)) ||
			((s & r & 0x8000) != 0);

	overflow = ((0x8000 & (s ^ 0x8000) & *d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (*d ^ 0x8000) & s) != 0);
	*d = r;
}

void sub_l_dn(void)
{
	w32 r, s;
	w32 *d;
	s = GetFromEA_l[(code >> 3) & 7]();
	d = (w32 *)((Ptr)reg + ((code >> 7) & 28));
	r = *d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry =
		(((*d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow = ((0x80000000 & (s ^ 0x80000000) & *d & (r ^ 0x80000000)) !=
		    0) ||
		   ((0x80000000 & r & (*d ^ 0x80000000) & s) != 0);
	*d = r;
}

void sub_b_ea(void)
{
	w8 r, s;
	w8 d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	s = *((w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((d & 0x80) == 0) && (((s | r) & 0x80) != 0)) ||
			((s & r & 0x80) != 0);
	overflow = ((0x80 & (s ^ 0x80) & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & s) != 0);
	RewriteEA_b(r);
}

void sub_w_ea(void)
{
	w16 r, s;
	w16 d;
	;
	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	s = *((w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((d & 0x8000) == 0) && (((s | r) & 0x8000) != 0)) ||
			((s & r & 0x8000) != 0);

	overflow = ((0x8000 & (s ^ 0x8000) & d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (d ^ 0x8000) & s) != 0);
	RewriteEA_w(r);
}

void sub_l_ea(void)
{
	w32 r, s;
	w32 d;
	;
	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	s = *((w32 *)((Ptr)reg + ((code >> 7) & 28)));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry =
		(((d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow =
		((0x80000000 & (s ^ 0x80000000) & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & s) != 0);
	RewriteEA_l(r);
}

void sub_w_an(void)
{
	w32 t1 = LongFromWord(GetFromEA_w[(code >> 3) & 7]());
	*((w32 *)((Ptr)aReg + ((code >> 7) & 28))) -=
		t1; //LongFromWord(GetFromEA_w[(code>>3)&7]());
}

void sub_l_an(void)
{
	w32 t1 = GetFromEA_l[(code >> 3) & 7]();
	*((w32 *)((Ptr)aReg + ((code >> 7) & 28))) -=
		t1; //GetFromEA_l[(code>>3)&7]();
}

void subi_b(void)
{
	w8 r, s;
	w8 d;
	s = (w8)RW(pc++);
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((d & 0x80) == 0) && (((s | r) & 0x80) != 0)) ||
			((s & r & 0x80) != 0);
	overflow = ((0x80 & (s ^ 0x80) & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & s) != 0);
	RewriteEA_b(r);
}

void subi_w(void)
{
	w16 r, s;
	w16 d;
	;
	s = (w16)RW(pc++);
	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((d & 0x8000) == 0) && (((s | r) & 0x8000) != 0)) ||
			((s & r & 0x8000) != 0);

	overflow = ((0x8000 & (s ^ 0x8000) & d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (d ^ 0x8000) & s) != 0);
	RewriteEA_w(r);
}

void subi_l(void)
{
	w32 r, s;
	w32 d;
	;
	s = RL((w32 *)pc);
	pc += 2;
	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry =
		(((d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow =
		((0x80000000 & (s ^ 0x80000000) & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & s) != 0);
	RewriteEA_l(r);
}

void subq_b(void)
{
	w8 r, s;
	w8 d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	s = (code >> 9) & 7;
	if (s == 0)
		s = 8;
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = ((d & 0x80) == 0) && ((r & 0x80) != 0);
	overflow = (0x80 & d & (r ^ 0x80)) != 0;
	RewriteEA_b(r);
}

void subq_w(void)
{
	w16 r, s;
	w16 d;
	;
	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	s = (code >> 9) & 7;
	if (s == 0)
		s = 8;
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = ((d & 0x8000) == 0) && ((r & 0x8000) != 0);
	overflow = (0x8000 & d & (r ^ 0x8000)) != 0;
	RewriteEA_w(r);
}

void subq_l(void)
{
	w32 r, s;
	w32 d;
	;
	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	s = (code >> 9) & 7;
	if (s == 0)
		s = 8;
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = ((d & 0x80000000) == 0) && ((r & 0x80000000) != 0);
	overflow = (0x80000000 & d & (r ^ 0x80000000)) != 0;
	RewriteEA_l(r);
}

void subq_an(void)
{
	short s;
	s = (code >> 9) & 7;
	if (s == 0)
		s = 8;
	aReg[code & 7] -= s;
}

void subq_4_an(void)
{
	aReg[code & 7] -= 4;
}

void subx_b_r(void)
{
	w8 s, r;
	w8 *d;
	d = (w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO);
	s = (w8)reg[code & 7];
	r = *d - s;
	if (xflag)
		r--;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = (((*d & 0x80) == 0) && (((s | r) & 0x80) != 0)) ||
			((s & r & 0x80) != 0);
	overflow = ((0x80 & (s ^ 0x80) & *d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (*d ^ 0x80) & s) != 0);
	*d = r;
}

void subx_w_r(void)
{
	w16 s, r;
	w16 *d;
	d = (w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO);
	s = (w16)reg[code & 7];
	r = *d - s;
	if (xflag)
		r--;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = (((*d & 0x8000) == 0) && (((s | r) & 0x8000) != 0)) ||
			((s & r & 0x8000) != 0);

	overflow = ((0x8000 & (s ^ 0x8000) & *d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (*d ^ 0x8000) & s) != 0);
	*d = r;
}

void subx_l_r(void)
{
	w32 s, r;
	w32 *d;
	d = (w32 *)((Ptr)reg + ((code >> 7) & 28));
	s = reg[code & 7];
	r = *d - s;
	if (xflag)
		r--;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry =
		(((*d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow = ((0x80000000 & (s ^ 0x80000000) & *d & (r ^ 0x80000000)) !=
		    0) ||
		   ((0x80000000 & r & (*d ^ 0x80000000) & s) != 0);
	*d = r;
}

void subx_b_m(void)
{
	w8 s, d, r;
	s = GetFromEA_b_m4();
	d = ModifyAtEA_b(4, (code >> 9) & 7);
	r = d - s;
	if (xflag)
		r--;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = (((d & 0x80) == 0) && (((s | r) & 0x80) != 0)) ||
			((s & r & 0x80) != 0);
	overflow = ((0x80 & (s ^ 0x80) & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & s) != 0);
	RewriteEA_b(r);
}

void subx_w_m(void)
{
	w16 s, d, r;
	;
	s = GetFromEA_w_m4();
	d = ModifyAtEA_w(4, (code >> 9) & 7);
	r = d - s;
	if (xflag)
		r--;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = (((d & 0x8000) == 0) && (((s | r) & 0x8000) != 0)) ||
			((s & r & 0x8000) != 0);

	overflow = ((0x8000 & (s ^ 0x8000) & d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (d ^ 0x8000) & s) != 0);
	RewriteEA_w(r);
}

void subx_l_m(void)
{
	w32 s, d, r;
	;
	s = GetFromEA_l_m4();
	d = ModifyAtEA_l(4, (code >> 9) & 7);
	r = d - s;
	if (xflag)
		r--;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry =
		(((d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow =
		((0x80000000 & (s ^ 0x80000000) & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & s) != 0);
	RewriteEA_l(r);
}

void swap(void)
{
	register w32 d;
	register w32 *r;
	r = &(reg[code & 7]);
	d = *r << 16;
	d += (*r >> 16) & 0x0ffff;
	*r = d;
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void tas(void)
{
	register w8 d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	RewriteEA_b(d | 0x80);
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void trap(void)
{
#ifdef IE_XL
	qlux_table[code]();
#else
	exception = 32 + (code & 15);
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
#endif
}

void trapv(void)
{
	if (overflow) {
		exception = 7;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void tst_b(void)
{
	register w8 d;
	d = GetFromEA_b[(code >> 3) & 7]();
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void tst_w(void)
{
	register w16 d;
	d = GetFromEA_w[(code >> 3) & 7]();
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void tst_l(void)
{
	register w32 d;
	d = GetFromEA_l[(code >> 3) & 7]();
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void unlk(void)
{
	register w32 *r;
	r = &(aReg[code & 7]);
	(*m68k_sp) = *r;
	*r = ReadLong(*m68k_sp);
	(*m68k_sp) += 4;
}

/* register shifts */

void asr_b_i(void)
{
	register w8 *d;
	register short c;
	d = ((w8 *)(&(reg[code & 7]))) + RBO;
	negative = *d < 0;
	if ((c = (code >> 9) & 7) != 0) {
		carry = xflag = (*d & ((uw8)1 << (c - 1))) != 0;
		(*d) >>= c;
	} else {
		carry = xflag = (*d & 128) != 0;
		*d = negative ? -1 : 0;
	}
	zero = *d == 0;
	overflow = false;
}

void asl_b_i(void)
{
	w8 *d;
	short c;
	uw8 mask;
	d = ((w8 *)(&(reg[code & 7]))) + RBO;
	if ((c = (code >> 9) & 7) != 0) {
		carry = xflag = (*d & ((uw8)128 >> (c - 1))) != 0;
		negative = *d < 0;
		mask = 255 << (7 - c);
		if (negative)
			overflow = (mask & (uw8)(*d)) != mask;
		else
			overflow = (mask & (uw8)(*d)) != 0;
		(*d) <<= c;
		zero = *d == 0;
		negative = *d < 0;
	} else {
		carry = xflag = (*d & 128) != 0;
		overflow = *d != 0;
		*d = 0;
		zero = true;
		negative = false;
	}
}

void asr_w_i(void)
{
	register w16 *d;
	register short c;
	d = (w16 *)(((w8 *)(&(reg[code & 7]))) + RWO);
	negative = *d < 0;
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = xflag = (*d & ((w16)1 << (c - 1))) != 0;
	(*d) >>= c;
	zero = *d == 0;
	overflow = false;
}

void asl_w_i(void)
{
	w16 *d;
	short c;
	uw16 mask;
	d = (w16 *)(((w8 *)(&(reg[code & 7]))) + RWO);
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = xflag = (*d & ((uw16)0x8000 >> (c - 1))) != 0;
	negative = *d < 0;
	mask = 0xff80 << (8 - c);
	if (negative)
		overflow = (mask & (uw16)(*d)) != mask;
	else
		overflow = (mask & (uw16)(*d)) != 0;
	(*d) <<= c;
	zero = *d == 0;
	negative = *d < 0;
}

void asr_l_i(void)
{
	register w32 *d;
	register short c;
	d = &(reg[code & 7]);
	negative = *d < 0;
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = xflag = (*d & ((w32)1 << (c - 1))) != 0;
	(*d) >>= c;
	zero = *d == 0;
	overflow = false;
}

void asl_l_i(void)
{
	w32 *d;
	short c;
	uw32 mask;
	d = &(reg[code & 7]);
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = xflag = (*d & ((uw32)0x80000000 >> (c - 1))) != 0;
	negative = *d < 0;
	mask = 0xff800000 << (8 - c);
	if (negative)
		overflow = (mask & (uw32)(*d)) != mask;
	else
		overflow = (mask & (uw32)(*d)) != 0;
	(*d) <<= c;
	zero = *d == 0;
	negative = *d < 0;
}

void asr_b_r(void)
{
	register w8 *d;
	register uw8 c;
	d = (w8 *)((Ptr)reg + ((code & 7) << 2) + RBO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	negative = *d < 0;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
	} else {
		if (c <= 8)
			carry = xflag = (*d & ((uw8)1 << (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 8) {
			*d >>= c;
			zero = *d == 0;
		} else {
			*d = negative ? -1 : 0;
			zero = !negative;
		}
	}
	overflow = false;
}

void asl_b_r(void)
{
	w8 *d;
	uw8 c;
	uw8 mask;
	d = (w8 *)((Ptr)reg + ((code & 7) << 2) + RBO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	negative = *d < 0;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
		overflow = false;
	} else {
		if (c <= 8)
			carry = xflag = (*d & ((uw8)0x80 >> (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 8) {
			mask = 255 << (7 - c);
			if (negative)
				overflow = (mask & (uw8)(*d)) != mask;
			else
				overflow = (mask & (uw8)(*d)) != 0;
			*d <<= c;
			zero = *d == 0;
			negative = *d < 0;
		} else {
			overflow = *d != 0;
			*d = 0;
			zero = true;
			negative = false;
		}
	}
}

void asr_w_r(void)
{
	register w16 *d;
	register uw8 c;
	d = (w16 *)((Ptr)reg + ((code & 7) << 2) + RWO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	negative = *d < 0;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
	} else {
		if (c <= 16)
			carry = xflag = (*d & ((uw16)1 << (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 16) {
			*d >>= c;
			zero = *d == 0;
		} else {
			*d = negative ? -1 : 0;
			zero = !negative;
		}
	}
	overflow = false;
}

void asl_w_r(void)
{
	w16 *d;
	uw8 c;
	uw16 mask;
	d = (w16 *)((Ptr)reg + ((code & 7) << 2) + RWO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	negative = *d < 0;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
		overflow = false;
	} else {
		if (c <= 16)
			carry = xflag = (*d & ((uw16)0x8000 >> (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 16) {
			mask = 65535 << (15 - c);
			if (negative)
				overflow = (mask & (uw16)(*d)) != mask;
			else
				overflow = (mask & (uw16)(*d)) != 0;
			*d <<= c;
			zero = *d == 0;
			negative = *d < 0;
		} else {
			overflow = *d != 0;
			*d = 0;
			zero = true;
			negative = false;
		}
	}
}

void asr_l_r(void)
{
	register w32 *d;
	register uw8 c;
	d = &(reg[code & 7]);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	negative = *d < 0;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
	} else {
		if (c <= 32)
			carry = xflag = (*d & ((uw32)1 << (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 32) {
			*d >>= c;
			zero = *d == 0;
		} else {
			*d = negative ? -1 : 0;
			zero = !negative;
		}
	}
	overflow = false;
}

void asl_l_r(void)
{
	w32 *d;
	uw8 c;
	uw32 mask;
	d = &(reg[code & 7]);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	negative = *d < 0;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
		overflow = false;
	} else {
		if (c <= 32)
			carry = xflag =
				(*d & ((uw32)0x80000000 >> (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 32) {
			mask = 0xffffffff << (31 - c);
			if (negative)
				overflow = (mask & (uw32)(*d)) != mask;
			else
				overflow = (mask & (uw32)(*d)) != 0;
			*d <<= c;
			zero = *d == 0;
			negative = *d < 0;
		} else {
			overflow = *d != 0;
			*d = 0;
			zero = true;
			negative = false;
		}
	}
}

void lsr_b_i(void)
{
	register uw8 *d;
	register short c;
	d = ((uw8 *)(&(reg[code & 7]))) + RBO;
	if ((c = (code >> 9) & 7) != 0) {
		carry = xflag = (*d & ((uw8)1 << (c - 1))) != 0;
		(*d) >>= c;
		zero = *d == 0;
	} else {
		carry = xflag = (*d & 128) != 0;
		*d = 0;
		zero = true;
	}
	negative = overflow = false;
}

void lsr1_b(void)
{
	register uw8 *d;
	d = ((uw8 *)(&(reg[code & 7]))) + RBO;
	carry = xflag = (*d & ((uw8)1)) != 0;
	(*d) >>= 1;
	zero = *d == 0;
	negative = overflow = false;
}

void lsl_b_i(void)
{
	register uw8 *d;
	register short c;
	d = ((uw8 *)(&(reg[code & 7]))) + RBO;
	if ((c = (code >> 9) & 7) != 0) {
		carry = xflag = (*d & ((uw8)128 >> (c - 1))) != 0;
		(*d) <<= c;
		zero = *d == 0;
		negative = (w8)(*d) < 0;
	} else {
		carry = xflag = (*d & 1) != 0;
		*d = 0;
		zero = true;
		negative = false;
	}
	overflow = false;
}

void lsl1_b(void)
{
	register uw8 *d;
	d = ((uw8 *)(&(reg[code & 7]))) + RBO;
	carry = xflag = (*d & ((uw8)0x80)) != 0;
	(*d) <<= 1;
	zero = *d == 0;
	negative = (w8)(*d) < 0;
	overflow = false;
}

void lsr_w_i(void)
{
	register uw16 *d;
	register short c;
	d = (uw16 *)((Ptr)reg + ((code & 7) << 2) + RWO);
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = xflag = (*d & ((uw16)1 << (c - 1))) != 0;
	(*d) >>= c;
	zero = *d == 0;
	negative = overflow = false;
}

void lsr1_w(void)
{
	register uw16 *d;
	d = (uw16 *)((Ptr)reg + ((code & 7) << 2) + RWO);
	carry = xflag = (*d & ((uw16)1)) != 0;
	(*d) >>= 1;
	zero = *d == 0;
	negative = overflow = false;
}

void lsl_w_i(void)
{
	register uw16 *d;
	register short c;
	d = (uw16 *)((Ptr)reg + ((code & 7) << 2) + RWO);
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = xflag = (*d & ((uw16)0x8000 >> (c - 1))) != 0;
	(*d) <<= c;
	zero = *d == 0;
	negative = (w16)(*d) < 0;
	overflow = false;
}

void lsl1_w(void)
{
	register uw16 *d;
	d = (uw16 *)((Ptr)reg + ((code & 7) << 2) + RWO);
	carry = xflag = (*d & ((uw16)0x8000)) != 0;
	(*d) <<= 1;
	zero = *d == 0;
	negative = (w16)(*d) < 0;
	overflow = false;
}

void lsr_l_i(void)
{
	register uw32 *d;
	register short c;
	d = (uw32 *)(&(reg[code & 7]));
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = xflag = (*d & ((uw32)1 << (c - 1))) != 0;
	(*d) >>= c;
	zero = *d == 0;
	negative = overflow = false;
}

void lsr1_l(void)
{
	register uw32 *d;
	d = (uw32 *)(&(reg[code & 7]));
	carry = xflag = (*d & ((uw32)1)) != 0;
	(*d) >>= 1;
	zero = *d == 0;
	negative = overflow = false;
}

void lsl_l_i(void)
{
	register uw32 *d;
	register short c;
	d = (uw32 *)(&(reg[code & 7]));
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = xflag = (*d & ((uw32)0x80000000 >> (c - 1))) != 0;
	(*d) <<= c;
	zero = *d == 0;
	negative = (w32)(*d) < 0;
	overflow = false;
}

void lsl1_l(void)
{
	register uw32 *d;
	d = (uw32 *)(&(reg[code & 7]));
	carry = xflag = ((*d) & ((uw32)0x80000000)) != 0;
	(*d) <<= 1;
	zero = *d == 0;
	negative = (w32)(*d) < 0;
	overflow = false;
}

void lsl2_l(void)
{
	register uw32 *d;
	d = (uw32 *)(&(reg[code & 7]));
	carry = xflag = ((*d) & 0x40000000l) != 0;
	(*d) <<= 2;
	zero = *d == 0;
	negative = (w32)(*d) < 0;
	overflow = false;
}

void lsr_b_r(void)
{
	register uw8 *d;
	register uw8 c;
	d = (uw8 *)((Ptr)reg + ((code & 7) << 2) + RBO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
		negative = (w8)(*d) < 0;
	} else {
		if (c <= 8)
			carry = xflag = (*d & ((uw8)1 << (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 8) {
			*d >>= c;
			zero = *d == 0;
		} else {
			*d = 0;
			zero = true;
		}
		negative = false;
	}
	overflow = false;
}

void lsl_b_r(void)
{
	register uw8 *d;
	register uw8 c;
	d = (uw8 *)((Ptr)reg + ((code & 7) << 2) + RBO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
		negative = (w8)(*d) < 0;
	} else {
		if (c <= 8)
			carry = xflag = (*d & ((uw8)0x80 >> (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 8) {
			*d <<= c;
			zero = *d == 0;
			negative = (w8)(*d) < 0;
		} else {
			*d = 0;
			zero = true;
			negative = false;
		}
	}
	overflow = false;
}

void lsr_w_r(void)
{
	register uw16 *d;
	register uw8 c;
	d = (uw16 *)((Ptr)reg + ((code & 7) << 2) + RWO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
		negative = (w16)(*d) < 0;
	} else {
		if (c <= 16)
			carry = xflag = (*d & ((uw16)1 << (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 16) {
			*d >>= c;
			zero = *d == 0;
		} else {
			*d = 0;
			zero = true;
		}
		negative = false;
	}
	overflow = false;
}

void lsl_w_r(void)
{
	register uw16 *d;
	register uw8 c;
	d = (uw16 *)((Ptr)reg + ((code & 7) << 2) + RWO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
		negative = (w16)(*d) < 0;
	} else {
		if (c <= 16)
			carry = xflag = (*d & ((uw16)0x8000 >> (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 16) {
			*d <<= c;
			zero = *d == 0;
			negative = (w16)(*d) < 0;
		} else {
			*d = 0;
			zero = true;
			negative = false;
		}
	}
	overflow = false;
}

void lsr_l_r(void)
{
	register uw32 *d;
	register uw8 c;
	d = (uw32 *)(&(reg[code & 7]));
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
		negative = (w32)(*d) < 0;
	} else {
		if (c <= 32)
			carry = xflag = (*d & ((uw32)1 << (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 32) {
			*d >>= c;
			zero = *d == 0;
		} else {
			*d = 0;
			zero = true;
		}
		negative = false;
	}
	overflow = false;
}

void lsl_l_r(void)
{
	register uw32 *d;
	register uw8 c;
	d = (uw32 *)(&(reg[code & 7]));
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0) {
		carry = false;
		zero = *d == 0;
		negative = (w32)(*d) < 0;
	} else {
		if (c <= 32)
			carry = xflag =
				(*d & ((uw32)0x80000000 >> (c - 1))) != 0;
		else
			carry = xflag = false;
		if (c < 32) {
			*d <<= c;
			zero = *d == 0;
			negative = (w32)(*d) < 0;
		} else {
			*d = 0;
			zero = true;
			negative = false;
		}
	}
	overflow = false;
}

void ror_b_i(void)
{
	register uw8 *d;
	register short c;
	d = ((uw8 *)(&(reg[code & 7]))) + RBO;
	if ((c = (code >> 9) & 7) != 0)
		(*d) = ((*d) >> c) | ((*d) << (8 - c));
	overflow = false;
	carry = negative = (*d & 128) != 0;
	zero = *d == 0;
}

void rol_b_i(void)
{
	register uw8 *d;
	register short c;
	d = ((uw8 *)(&(reg[code & 7]))) + RBO;
	if ((c = (code >> 9) & 7) != 0)
		(*d) = ((*d) << c) | ((*d) >> (8 - c));
	carry = (*d & 1) != 0;
	zero = *d == 0;
	negative = (*d & 128) != 0;
	overflow = false;
}

void ror_w_i(void)
{
	register uw16 *d;
	register short c;
	d = (uw16 *)(((uw8 *)(&(reg[code & 7]))) + RWO);
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	(*d) = ((*d) >> c) | ((*d) << (16 - c));
	overflow = false;
	carry = negative = (*d & 0x8000) != 0;
	zero = *d == 0;
}

void rol_w_i(void)
{
	register uw16 *d;
	register short c;
	d = (uw16 *)(((uw8 *)(&(reg[code & 7]))) + RWO);
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	(*d) = ((*d) << c) | ((*d) >> (16 - c));
	carry = (*d & 1) != 0;
	zero = *d == 0;
	negative = (*d & 0x8000) != 0;
	overflow = false;
}

void ror_l_i(void)
{
	register uw32 *d;
	register short c;
	d = (uw32 *)(&(reg[code & 7]));
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	(*d) = ((*d) >> c) | ((*d) << (32 - c));
	overflow = false;
	carry = negative = (*d & 0x80000000) != 0;
	zero = *d == 0;
}

void rol_l_i(void)
{
	register uw32 *d;
	register short c;
	d = (uw32 *)(&(reg[code & 7]));
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	(*d) = ((*d) << c) | ((*d) >> (32 - c));
	carry = (*d & 1) != 0;
	zero = *d == 0;
	negative = (*d & 0x80000000) != 0;
	overflow = false;
}

void ror_b_r(void)
{
	register uw8 *d;
	register uw8 c;
	d = (uw8 *)((Ptr)reg + ((code & 7) << 2) + RBO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		c &= 7;
		if (c != 0)
			(*d) = ((*d) >> c) | ((*d) << (8 - c));
		carry = (*d & 128) != 0;
	}
	overflow = false;
	negative = (*d & 128) != 0;
	zero = *d == 0;
}

void rol_b_r(void)
{
	register uw8 *d;
	register uw8 c;
	d = (uw8 *)((Ptr)reg + ((code & 7) << 2) + RBO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		c &= 7;
		if (c != 0)
			(*d) = ((*d) << c) | ((*d) >> (8 - c));
		carry = (*d & 1) != 0;
	}
	overflow = false;
	negative = (*d & 128) != 0;
	zero = *d == 0;
}

void ror_w_r(void)
{
	register uw16 *d;
	register uw8 c;
	d = (uw16 *)((Ptr)reg + ((code & 7) << 2) + RWO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		c &= 15;
		if (c != 0)
			(*d) = ((*d) >> c) | ((*d) << (16 - c));
		carry = (*d & 0x8000) != 0;
	}
	overflow = false;
	negative = (*d & 0x8000) != 0;
	zero = *d == 0;
}

void rol_w_r(void)
{
	register uw16 *d;
	register uw8 c;
	d = (uw16 *)((Ptr)reg + ((code & 7) << 2) + RWO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		c &= 15;
		if (c != 0)
			(*d) = ((*d) << c) | ((*d) >> (16 - c));
		carry = (*d & 1) != 0;
	}
	overflow = false;
	negative = (*d & 0x8000) != 0;
	zero = *d == 0;
}

void ror_l_r(void)
{
	register uw32 *d;
	register uw8 c;
	d = (uw32 *)(&(reg[code & 7]));
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		c &= 31;
		if (c != 0)
			(*d) = ((*d) >> c) | ((*d) << (32 - c));
		carry = (*d & 0x80000000) != 0;
	}
	overflow = false;
	negative = (*d & 0x80000000) != 0;
	zero = *d == 0;
}

void rol_l_r(void)
{
	register uw32 *d;
	register uw8 c;
	d = (uw32 *)(&(reg[code & 7]));
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		c &= 31;
		if (c != 0)
			(*d) = ((*d) << c) | ((*d) >> (32 - c));
		carry = (*d & 1) != 0;
	}
	overflow = false;
	negative = (*d & 0x80000000) != 0;
	zero = *d == 0;
}

void roxr_b_i(void)
{
	uw8 *d;
	uw8 temp;
	short c;
	d = ((uw8 *)(&(reg[code & 7]))) + RBO;
	if ((c = (code >> 9) & 7) != 0) {
		carry = ((*d) & ((uw8)1 << (c - 1))) != 0;
		temp = (*d) << 1;
		if (xflag)
			temp |= 1;
		(*d) = ((*d) >> c) | (temp << (8 - c));
	} else {
		carry = ((*d) & 128) != 0;
		(*d) <<= 1;
		if (xflag)
			(*d) |= 1;
	}
	xflag = carry;
	overflow = false;
	negative = (*d & 128) != 0;
	zero = *d == 0;
}

void roxl_b_i(void)
{
	uw8 *d;
	uw8 temp;
	short c;
	d = ((uw8 *)(&(reg[code & 7]))) + RBO;
	if ((c = (code >> 9) & 7) != 0) {
		carry = ((*d) & ((uw8)128 >> (c - 1))) != 0;
		temp = (*d) >> 1;
		if (xflag)
			temp |= 128;
		(*d) = ((*d) << c) | (temp >> (8 - c));
	} else {
		carry = ((*d) & 1) != 0;
		(*d) >>= 1;
		if (xflag)
			(*d) |= 128;
	}
	xflag = carry;
	overflow = false;
	negative = (*d & 128) != 0;
	zero = *d == 0;
}

void roxr_w_i(void)
{
	uw16 *d;
	uw16 temp;
	short c;
	d = (uw16 *)(((uw8 *)(&(reg[code & 7]))) + RWO);
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = ((*d) & ((uw16)1 << (c - 1))) != 0;
	temp = (*d) << 1;
	if (xflag)
		temp |= 1;
	(*d) = ((*d) >> c) | (temp << (16 - c));
	xflag = carry;
	overflow = false;
	negative = (*d & 0x8000) != 0;
	zero = *d == 0;
}

void roxl_w_i(void)
{
	uw16 *d;
	uw16 temp;
	short c;
	d = (uw16 *)(((uw8 *)(&(reg[code & 7]))) + RWO);
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = ((*d) & ((uw16)0x8000 >> (c - 1))) != 0;
	temp = (*d) >> 1;
	if (xflag)
		temp |= 0x8000;
	(*d) = ((*d) << c) | (temp >> (16 - c));
	xflag = carry;
	overflow = false;
	negative = (*d & 0x8000) != 0;
	zero = *d == 0;
}

void roxr_l_i(void)
{
	uw32 *d;
	uw32 temp;
	short c;
	d = (uw32 *)(&(reg[code & 7]));
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = ((*d) & ((uw32)1 << (c - 1))) != 0;
	temp = (*d) << 1;
	if (xflag)
		temp |= 1;
	(*d) = ((*d) >> c) | (temp << (32 - c));
	xflag = carry;
	overflow = false;
	negative = (*d & 0x80000000) != 0;
	zero = *d == 0;
}

void roxl_l_i(void)
{
	uw32 *d;
	uw32 temp;
	short c;
	d = (uw32 *)(&(reg[code & 7]));
	if ((c = (code >> 9) & 7) == 0)
		c = 8;
	carry = ((*d) & ((uw32)0x80000000 >> (c - 1))) != 0;
	temp = (*d) >> 1;
	if (xflag)
		temp |= 0x80000000;
	(*d) = ((*d) << c) | (temp >> (32 - c));
	xflag = carry;
	overflow = false;
	negative = (*d & 0x80000000) != 0;
	zero = *d == 0;
}

void roxr_b_r(void)
{
	uw8 *d;
	uw8 temp;
	short c;
	d = ((uw8 *)(&(reg[code & 7]))) + RBO;
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		if ((c %= 9) != 0) {
			carry = ((*d) & ((uw8)1 << (c - 1))) != 0;
			temp = (*d) << 1;
			if (xflag)
				temp |= 1;
			(*d) = ((*d) >> c) | (temp << (8 - c));
			xflag = carry;
		} else
			carry = xflag;
	}
	overflow = false;
	negative = (*d & 128) != 0;
	zero = *d == 0;
}

void roxl_b_r(void)
{
	uw8 *d;
	uw8 temp;
	short c;
	d = ((uw8 *)(&(reg[code & 7]))) + RBO;
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		if ((c %= 9) != 0) {
			carry = ((*d) & ((uw8)128 >> (c - 1))) != 0;
			temp = (*d) >> 1;
			if (xflag)
				temp |= 128;
			(*d) = ((*d) << c) | (temp >> (8 - c));
			xflag = carry;
		} else
			carry = xflag;
	}
	overflow = false;
	negative = (*d & 128) != 0;
	zero = *d == 0;
}

void roxr_w_r(void)
{
	uw16 *d;
	uw16 temp;
	short c;
	d = (uw16 *)(((uw8 *)(&(reg[code & 7]))) + RWO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		if ((c %= 17) != 0) {
			carry = ((*d) & ((uw16)1 << (c - 1))) != 0;
			temp = (*d) << 1;
			if (xflag)
				temp |= 1;
			(*d) = ((*d) >> c) | (temp << (16 - c));
			xflag = carry;
		} else
			carry = xflag;
	}
	overflow = false;
	negative = (*d & 0x8000) != 0;
	zero = *d == 0;
}

void roxl_w_r(void)
{
	uw16 *d;
	uw16 temp;
	short c;
	d = (uw16 *)(((uw8 *)(&(reg[code & 7]))) + RWO);
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		if ((c %= 17) != 0) {
			carry = ((*d) & ((uw16)0x8000 >> (c - 1))) != 0;
			temp = (*d) >> 1;
			if (xflag)
				temp |= 0x8000;
			(*d) = ((*d) << c) | (temp >> (16 - c));
			xflag = carry;
		} else
			carry = xflag;
	}
	overflow = false;
	negative = (*d & 0x8000) != 0;
	zero = *d == 0;
}

void roxr_l_r(void)
{
	uw32 *d;
	uw32 temp;
	short c;
	d = (uw32 *)(&(reg[code & 7]));
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		if ((c %= 33) != 0) {
			carry = ((*d) & ((uw32)1 << (c - 1))) != 0;
			temp = (*d) << 1;
			if (xflag)
				temp |= 1;
			(*d) = ((*d) >> c) | (temp << (32 - c));
			xflag = carry;
		} else
			carry = xflag;
	}
	overflow = false;
	negative = (*d & 0x80000000) != 0;
	zero = *d == 0;
}

void roxl_l_r(void)
{
	uw32 *d;
	uw32 temp;
	short c;
	d = (uw32 *)(&(reg[code & 7]));
	c = *((uw8 *)((Ptr)reg + ((code >> 7) & 28) + RBO)) & 63;
	if (c == 0)
		carry = false;
	else {
		if ((c %= 33) != 0) {
			carry = ((*d) & ((uw32)0x80000000 >> (c - 1))) != 0;
			temp = (*d) >> 1;
			if (xflag)
				temp |= 0x80000000;
			(*d) = ((*d) << c) | (temp >> (32 - c));
			xflag = carry;
		} else
			carry = xflag;
	}
	overflow = false;
	negative = (*d & 0x80000000) != 0;
	zero = *d == 0;
}
