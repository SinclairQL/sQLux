/*
 * (c) UQLX - see COPYRIGHT
 */

#include "QL68000.h"
#include "mmodes.h"

void abcd(void)
{
	w8 s, d, r;
	w8 *dx;
	uw16 abcd_lo, abcd_hi, abcd_res;
	int abcd_carry;

	if ((code & 8) != 0) {
		s = GetFromEA_b_m4();
		d = ModifyAtEA_b(4, (code >> 9) & 7);
	} else {
		dx = (w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO);
		d = *dx;
		s = (w8)reg[code & 7];
	}

	abcd_lo = (s & 0xF) + (d & 0xF) + (xflag ? 1 : 0);
	abcd_hi = (s & 0xF0) + (d & 0xF0);

	abcd_res = abcd_hi + abcd_lo;
	if (abcd_lo > 9) {
		abcd_res += 6;
	}
	abcd_carry = (abcd_res & 0x3F0) > 0x90;
	if (abcd_carry)
		abcd_res += 0x60;

	r = abcd_res;

	xflag = carry = abcd_carry ? 1 : 0;
	zero = (zero ? 1 : 0) & (r ? 0 : 1);
	negative = (r < 0) ? 1 : 0;

	if ((code & 8) != 0)
		RewriteEA_b(r);
	else
		*dx = r;
}

void add_b_dn(void)
{
	w8 r, s;
	w8 *d;
	s = GetFromEA_b[(code >> 3) & 7]();
	d = (w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO);
	r = *d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x80) == 0) && (((s | *d) & 0x80) != 0)) ||
			((s & *d & 0x80) != 0);
	overflow = ((0x80 & s & *d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (*d ^ 0x80) & (s ^ 0x80)) != 0);
	*d = r;
}

void add_b_dn_dn(void)
{
	w8 r, s;
	w8 *d;
	s = (w8)reg[code & 7];
	d = (w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO);
	r = *d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x80) == 0) && (((s | *d) & 0x80) != 0)) ||
			((s & *d & 0x80) != 0);
	overflow = ((0x80 & s & *d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (*d ^ 0x80) & (s ^ 0x80)) != 0);
	*d = r;
}

void add_w_dn(void)
{
	w16 r, s;
	w16 *d;
	s = GetFromEA_w[(code >> 3) & 7]();
	d = (w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO);
	r = *d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x8000) == 0) && (((s | *d) & 0x8000) != 0)) ||
			((s & *d & 0x8000) != 0);

	overflow = ((0x8000 & s & *d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (*d ^ 0x8000) & (s ^ 0x8000)) != 0);
	*d = r;
}

void add_w_dn_dn(void)
{
	w16 r, s;
	w16 *d;
	s = (w16)reg[code & 7];
	d = (w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO);
	r = *d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x8000) == 0) && (((s | *d) & 0x8000) != 0)) ||
			((s & *d & 0x8000) != 0);

	overflow = ((0x8000 & s & *d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (*d ^ 0x8000) & (s ^ 0x8000)) != 0);
	*d = r;
}

void add_l_dn(void)
{
	w32 r, s;
	w32 *d;
	s = GetFromEA_l[(code >> 3) & 7]();
	d = ((w32 *)((Ptr)reg + ((code >> 7) & 28)));
	r = *d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry =
		(((r & 0x80000000) == 0) && (((s | *d) & 0x80000000) != 0)) ||
		((s & *d & 0x80000000) != 0);

	overflow =
		((0x80000000 & s & *d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (*d ^ 0x80000000) & (s ^ 0x80000000)) != 0);
	*d = r;
}

void add_l_dn_dn(void)
{
	w32 r, s;
	w32 *d;
	s = reg[code & 7];
	d = ((w32 *)((Ptr)reg + ((code >> 7) & 28)));
	r = *d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry =
		(((r & 0x80000000) == 0) && (((s | *d) & 0x80000000) != 0)) ||
		((s & *d & 0x80000000) != 0);

	overflow =
		((0x80000000 & s & *d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (*d ^ 0x80000000) & (s ^ 0x80000000)) != 0);
	*d = r;
}

void add_b_ea(void)
{
	w8 r, s;
	w8 d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	s = *((w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO));
	r = d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x80) == 0) && (((s | d) & 0x80) != 0)) ||
			((s & d & 0x80) != 0);
	overflow = ((0x80 & s & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & (s ^ 0x80)) != 0);
	RewriteEA_b(r);
}

void add_w_ea(void)
{
	w16 r, s;
	w16 d;

	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	s = *((w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO));
	r = d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x8000) == 0) && (((s | d) & 0x8000) != 0)) ||
			((s & d & 0x8000) != 0);

	overflow = ((0x8000 & s & d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (d ^ 0x8000) & (s ^ 0x8000)) != 0);
	RewriteEA_w(r);
}

void add_l_ea(void)
{
	w32 r, s;
	w32 d;

	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	s = *((w32 *)((Ptr)reg + ((code >> 7) & 28)));
	r = d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry =
		(((r & 0x80000000) == 0) && (((s | d) & 0x80000000) != 0)) ||
		((s & d & 0x80000000) != 0);

	overflow =
		((0x80000000 & s & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & (s ^ 0x80000000)) != 0);
	RewriteEA_l(r);
}

// adda (a7)+,a7 messed up, compiler evalueated assignment in wrong order
void add_w_an(void)
{
	//w32 a1,t1,a2;
	w32 t1;
	//a1=aReg[7];
	t1 = LongFromWord(GetFromEA_w[(code >> 3) & 7]());

	*((w32 *)((Ptr)aReg + ((code >> 7) & 28))) +=
		t1; //LongFromWord(GetFromEA_w[(code>>3)&7]());
	//if (0xdedf == code )
	//  printf("a7 before: %x, after %x, temp %x\n",a1,aReg[7],t1);
}

void add_w_an_dn(void)
{
	*((w32 *)((Ptr)aReg + ((code >> 7) & 28))) += (w32)((w16)reg[code & 7]);
}

void add_l_an(void)
{
	w32 t1;
	t1 = GetFromEA_l[(code >> 3) & 7]();
	*((w32 *)((Ptr)aReg + ((code >> 7) & 28))) +=
		t1; //GetFromEA_l[(code>>3)&7]();
}

void add_l_an_dn(void)
{
	*((w32 *)((Ptr)aReg + ((code >> 7) & 28))) += reg[code & 7];
}

void addi_b(void)
{
	w8 r, s;
	w8 d;
	s = (w8)RW(pc++);
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	r = d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x80) == 0) && (((s | d) & 0x80) != 0)) ||
			((s & d & 0x80) != 0);
	overflow = ((0x80 & s & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & (s ^ 0x80)) != 0);
	RewriteEA_b(r);
}

void addi_w(void)
{
	w16 r, s;
	w16 d;

	s = (w16)RW(pc++);
	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	r = d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x8000) == 0) && (((s | d) & 0x8000) != 0)) ||
			((s & d & 0x8000) != 0);

	overflow = ((0x8000 & s & d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (d ^ 0x8000) & (s ^ 0x8000)) != 0);
	RewriteEA_w(r);
}

void addi_l(void)
{
	w32 r, s;
	w32 d;

	/*s=*((w32*)pc);*/
	s = RL((Ptr)pc);
	pc += 2;
	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	r = d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry =
		(((r & 0x80000000) == 0) && (((s | d) & 0x80000000) != 0)) ||
		((s & d & 0x80000000) != 0);

	overflow =
		((0x80000000 & s & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & (s ^ 0x80000000)) != 0);
	RewriteEA_l(r);
}

void addq_b(void)
{
	w8 r, s;
	w8 d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	s = (code >> 9) & 7;
	if (s == 0)
		s = 8;
	r = d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x80) == 0) && ((d & 0x80) != 0));
	overflow = (0x80 & r & (d ^ 0x80)) != 0;
	RewriteEA_b(r);
}

void addq_w(void)
{
	w16 r, s;
	w16 d;

	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	s = (code >> 9) & 7;
	if (s == 0)
		s = 8;
	r = d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x8000) == 0) && ((d & 0x8000) != 0));
	overflow = (0x8000 & r & (d ^ 0x8000)) != 0;
	RewriteEA_w(r);
}

void addq_l(void)
{
	w32 r, s;
	w32 d;

	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	s = (code >> 9) & 7;
	if (s == 0)
		s = 8;
	r = d + s;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = (((r & 0x80000000) == 0) && ((d & 0x80000000) != 0));
	overflow = (0x80000000 & r & (d ^ 0x80000000)) != 0;
	RewriteEA_l(r);
}

void addq_an(void)
{
	register short s;
	s = (code >> 9) & 7;
	if (s == 0)
		s = 8;
	aReg[code & 7] += s;
}

void addq_4_an(void)
{
	aReg[code & 7] += 4;
}

void addx_b_r(void)
{
	w8 s, r;
	w8 *d;
	d = (w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO);
	s = (w8)reg[code & 7];
	r = *d + s;
	if (xflag)
		r++;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = (((r & 0x80) == 0) && (((s | *d) & 0x80) != 0)) ||
			((s & *d & 0x80) != 0);
	overflow = ((0x80 & s & *d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (*d ^ 0x80) & (s ^ 0x80)) != 0);
	*d = r;
}

void addx_w_r(void)
{
	w16 s, r;
	w16 *d;
	d = (w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO);
	s = (w16)reg[code & 7];
	r = *d + s;
	if (xflag)
		r++;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = (((r & 0x8000) == 0) && (((s | *d) & 0x8000) != 0)) ||
			((s & *d & 0x8000) != 0);

	overflow = ((0x8000 & s & *d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (*d ^ 0x8000) & (s ^ 0x8000)) != 0);
	*d = r;
}

void addx_l_r(void)
{
	w32 s, r;
	w32 *d;
	d = (w32 *)((Ptr)reg + ((code >> 7) & 28));
	s = reg[code & 7];
	r = *d + s;
	if (xflag)
		r++;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry =
		(((r & 0x80000000) == 0) && (((s | *d) & 0x80000000) != 0)) ||
		((s & *d & 0x80000000) != 0);

	overflow =
		((0x80000000 & s & *d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (*d ^ 0x80000000) & (s ^ 0x80000000)) != 0);
	*d = r;
}

void addx_b_m(void)
{
	w8 s, d, r;
	s = GetFromEA_b_m4();
	d = ModifyAtEA_b(4, (code >> 9) & 7);
	r = d + s;
	if (xflag)
		r++;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = (((r & 0x80) == 0) && (((s | d) & 0x80) != 0)) ||
			((s & d & 0x80) != 0);
	overflow = ((0x80 & s & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & (s ^ 0x80)) != 0);
	RewriteEA_b(r);
}

void addx_w_m(void)
{
	w16 s, d, r;

	s = GetFromEA_w_m4();
	d = ModifyAtEA_w(4, (code >> 9) & 7);
	r = d + s;
	if (xflag)
		r++;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = (((r & 0x8000) == 0) && (((s | d) & 0x8000) != 0)) ||
			((s & d & 0x8000) != 0);

	overflow = ((0x8000 & s & d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (d ^ 0x8000) & (s ^ 0x8000)) != 0);
	RewriteEA_w(r);
}

void addx_l_m(void)
{
	w32 s, d, r;

	s = GetFromEA_l_m4();
	d = ModifyAtEA_l(4, (code >> 9) & 7);
	r = d + s;
	if (xflag)
		r++;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry =
		(((r & 0x80000000) == 0) && (((s | d) & 0x80000000) != 0)) ||
		((s & d & 0x80000000) != 0);

	overflow =
		((0x80000000 & s & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & (s ^ 0x80000000)) != 0);
	RewriteEA_l(r);
}

void and_b_dn(void)
{
	register w8 *d;
	d = (w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO);
	*d = *d & GetFromEA_b[(code >> 3) & 7]();
	negative = *d < 0;
	zero = *d == 0;
	carry = overflow = false;
}

void and_w_dn(void)
{
	register w16 *d;
	d = (w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO);
	*d = *d & GetFromEA_w[(code >> 3) & 7]();
	negative = *d < 0;
	zero = *d == 0;
	carry = overflow = false;
}

void and_l_dn(void)
{
	register w32 *d;
	d = (w32 *)((Ptr)reg + ((code >> 7) & 28));
	*d = *d & GetFromEA_l[(code >> 3) & 7]();
	negative = *d < 0;
	zero = *d == 0;
	carry = overflow = false;
}

void and_l_dn_dn(void)
{
	register w32 *d;
	d = (w32 *)((Ptr)reg + ((code >> 7) & 28));
	(*d) &= reg[code & 7];
	negative = *d < 0;
	zero = *d == 0;
	carry = overflow = false;
}

void and_b_ea(void)
{
	register w8 d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	d = d & *((w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO));
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_b(d);
}

void and_w_ea(void)
{
	register w16 d;

	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	d = d & *((w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO));
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_w(d);
}

void and_l_ea(void)
{
	register w32 d;

	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	d = d & *((w32 *)((Ptr)reg + ((code >> 7) & 28)));
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_l(d);
}

void andi_b(void)
{
	register w8 d;
	d = (w8)RW(pc++);
	d = d & ModifyAtEA_b((code >> 3) & 7, code & 7);
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_b(d);
}

void andi_w(void)
{
	register w16 d;

	d = (w16)RW(pc++);
	d = d & ModifyAtEA_w((code >> 3) & 7, code & 7);
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_w(d);
}

void andi_l(void)
{
	register w32 d;

	d = RL((Ptr)pc); /* d=*((w32*)pc); */
	pc += 2;
	d = d & ModifyAtEA_l((code >> 3) & 7, code & 7);
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_l(d);
}

void andi_to_ccr(void)
{
	register uw16 d;
	d = RW(pc++);
	carry = carry && ((d & 1) != 0);
	overflow = overflow && ((d & 2) != 0);
	zero = zero && ((d & 4) != 0);
	negative = negative && ((d & 8) != 0);
	xflag = xflag && ((d & 16) != 0);
}

void andi_to_sr(void)
{
	register w16 d;
	d = (w16)RW(pc++);
	if (supervisor) {
		d &= GetSR();
		PutSR(d);
	} else {
		exception = 8;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void asl_m(void)
{
	register w16 d;

	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	xflag = carry = (d & 0x8000) != 0;
	d <<= 1;
	negative = d < 0;
	zero = d == 0;
	overflow = carry != negative;
	RewriteEA_w(d);
}

void asr_m(void)
{
	register w16 d;

	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	xflag = carry = (d & 1) != 0;
	negative = d < 0;
	d >>= 1; /* attenzione: deve essere un signed shift !! */
	zero = d == 0;
	overflow = false;
	RewriteEA_w(d);
}

void bcc_l(void)
{
	if (ConditionTrue[(code >> 8) & 15]())
		SetPC((Ptr)pc - (Ptr)theROM + (w16)RW(pc));
	else
		pc++;
}

void beq_l(void)
{
	if (zero)
		SetPC((Ptr)pc - (Ptr)theROM + (w16)RW(pc));
	else
		pc++;
}

void bne_l(void)
{
	if (!zero)
		SetPC((Ptr)pc - (Ptr)theROM + (w16)RW(pc));
	else
		pc++;
}

void bcc_bad(void)
{
	if (ConditionTrue[(code >> 8) & 15]()) {
		SetPC((Ptr)pc - (Ptr)theROM +
		      (w8)code); /* cause address error */
	}
}

void bcc_s(void)
{
	if (ConditionTrue[(code >> 8) & 15]())
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void beq_s(void)
{
	if (zero)
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void bne_s(void)
{
	if (!zero)
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void bcs_s(void)
{
	if (carry)
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void bccc_s(void)
{
	if (!carry)
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void bpl_s(void)
{
	if (!negative)
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void bmi_s(void)
{
	if (negative)
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void bge_s(void)
{
	if ((negative && overflow) || (!(negative || overflow)))
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void blt_s(void)
{
	if ((negative && (!overflow)) || ((!negative) && overflow))
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void bgt_s(void)
{
	if ((!zero) && ((negative && overflow) || (!(negative || overflow))))
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void ble_s(void)
{
	if (zero || (negative && (!overflow)) || ((!negative) && overflow))
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
		pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void bra_l(void)
{
	SetPC((Ptr)pc - (Ptr)theROM + (w16)RW(pc));
}

void bra_s(void)
{
#ifdef DEBUG
	SetPC((Ptr)pc - (Ptr)theROM + (w8)code);
#else
	pc = (uw16 *)((Ptr)pc + (w8)code);
#ifdef TRACE
	CheckTrace();
#endif
#endif
}

void bchg_d(void)
{
	w32 mask = 1;
	w8 d;
	short EAmode;
	short bit;
	EAmode = (code >> 3) & 7;
	bit = reg[code >> 9];
	if (EAmode != 0) {
		mask <<= (short)reg[code >> 9] & 7;
		d = ModifyAtEA_b(EAmode, code & 7);
		zero = (d & (w8)mask) == 0;
		RewriteEA_b(d ^ (w8)mask);
	} else {
		mask <<= (short)reg[code >> 9] & 31;
		zero = (reg[code & 7] & mask) == 0;
		reg[code & 7] ^= mask;
	}
}

void bchg_s(void)
{
	w32 mask = 1;
	w8 d;
	short EAmode;
	short bit;
	EAmode = (code >> 3) & 7;
	bit = RW(pc++);
	if (EAmode != 0) {
		mask <<= bit & 7;
		d = ModifyAtEA_b(EAmode, code & 7);
		zero = (d & (w8)mask) == 0;
		RewriteEA_b(d ^ (w8)mask);
	} else {
		mask <<= bit & 31;
		zero = (reg[code & 7] & mask) == 0;
		reg[code & 7] ^= mask;
	}
}

void bclr_d(void)
{
	w32 mask = 1;
	w8 d;
	short EAmode;
	short bit;
	EAmode = (code >> 3) & 7;
	bit = reg[code >> 9];
	if (EAmode != 0) {
		mask <<= (short)reg[code >> 9] & 7;
		d = ModifyAtEA_b(EAmode, code & 7);
		zero = (d & (w8)mask) == 0;
		if (!zero)
			d ^= (w8)mask;
		RewriteEA_b(d);
	} else {
		mask <<= (short)reg[code >> 9] & 31;
		zero = (reg[code & 7] & mask) == 0;
		if (!zero)
			reg[code & 7] ^= mask;
	}
}

void bclr_s(void)
{
	w32 mask = 1;
	w8 d;
	short EAmode;
	short bit;
	EAmode = (code >> 3) & 7;
	bit = RW(pc++);
	if (EAmode != 0) {
		mask <<= bit & 7;
		d = ModifyAtEA_b(EAmode, code & 7);
		zero = (d & (w8)mask) == 0;
		if (!zero)
			d ^= (w8)mask;
		RewriteEA_b(d);
	} else {
		mask <<= bit & 31;
		zero = (reg[code & 7] & mask) == 0;
		if (!zero)
			reg[code & 7] ^= mask;
	}
}

void bsr(void)
{
	w16 displ;
	w32 oldPC;

	oldPC = (uintptr_t)pc - (uintptr_t)theROM;
	if ((displ = (w16)(((w8)(code & 255)))) == 0) {
		displ = (w16)RW(pc);
		oldPC += 2;
	}
	WriteLong((*m68k_sp) -= 4, oldPC);
#ifdef BACKTRACE
	SetPCB((Ptr)pc - (Ptr)theROM + displ, BSR);
#else
	SetPC((Ptr)pc - (Ptr)theROM + displ);
#endif
}

void bset_d(void)
{
	w32 mask = 1;
	w8 d;
	short EAmode;
	short bit;
	EAmode = (code >> 3) & 7;
	bit = reg[code >> 9];
	if (EAmode != 0) {
		mask <<= (short)reg[code >> 9] & 7;
		d = ModifyAtEA_b(EAmode, code & 7);
		zero = (d & (w8)mask) == 0;
		if (zero)
			d |= (w8)mask;
		RewriteEA_b(d);
	} else {
		mask <<= (short)reg[code >> 9] & 31;
		zero = (reg[code & 7] & mask) == 0;
		if (zero)
			reg[code & 7] |= mask;
	}
}

void bset_s(void)
{
	w32 mask = 1;
	w8 d;
	short EAmode;
	short bit;
	EAmode = (code >> 3) & 7;
	bit = RW(pc++);
	if (EAmode != 0) {
		mask <<= bit & 7;
		d = ModifyAtEA_b(EAmode, code & 7);
		zero = (d & (w8)mask) == 0;
		if (zero)
			d |= (w8)mask;
		RewriteEA_b(d);
	} else {
		mask <<= bit & 31;
		zero = (reg[code & 7] & mask) == 0;
		if (zero)
			reg[code & 7] |= mask;
	}
}

void btst_d(void)
{
	w32 mask = 1;
	short EAmode;
	short bit;
	EAmode = (code >> 3) & 7;
	bit = reg[code >> 9];
	if (EAmode != 0) {
		mask <<= (short)reg[code >> 9] & 7;
		zero = (GetFromEA_b[EAmode]() & mask) == 0;
	} else {
		mask <<= (short)reg[code >> 9] & 31;
		zero = (reg[code & 7] & mask) == 0;
	}
}

void btst_s(void)
{
	w32 mask = 1;
	short EAmode;
	short bit;
	EAmode = (code >> 3) & 7;
	bit = RW(pc++);
	if (EAmode) {
		mask <<= bit & 7;
		zero = (GetFromEA_b[EAmode]() & mask) == 0;
	} else {
		mask <<= bit & 31;
		zero = (reg[code & 7] & mask) == 0;
	}
}

void chk(void)
{
	w16 *d;
	w16 ea;
	d = (w16 *)(((Ptr)reg) + ((code >> 7) & 0x1c) + RWO);
	ea = GetFromEA_w[(code >> 3) & 7]();
	if (*d < 0) {
		negative = true;
		exception = 6;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	} else {
		if (*d > ea) {
			negative = false;
			exception = 6;
			extraFlag = true;
			nInst2 = nInst;
			nInst = 0;
		}
	}
}

void clr_b(void)
{
	ARCALL(PutToEA_b, (code >> 3) & 7, code & 7, 0);
	/*((void (*)(short,w8)REGP2)PutToEA_b[(code>>3)&7])(code&7,0);*/
	/*PUT_TOEA_B((code>>3)&7,code&7,0);*/
	negative = overflow = carry = false;
	zero = true;
}

void clr_w(void)
{
	ARCALL(PutToEA_w, (code >> 3) & 7, code & 7, 0);
	/*PUT_TOEA_W((code>>3)&7,code&7,0);*/
	negative = overflow = carry = false;
	zero = true;
}

void clr_l(void)
{
	ARCALL(PutToEA_l, (code >> 3) & 7, code & 7, 0);
	/*PUT_TOEA_L((code>>3)&7,code&7,0);*/
	negative = overflow = carry = false;
	zero = true;
}

void cmp_b(void)
{
	w8 r, s, d;
	s = GetFromEA_b[(code >> 3) & 7]();
	d = *((w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80) == 0) && (((s | r) & 0x80) != 0)) ||
		((s & r & 0x80) != 0);
	overflow = ((0x80 & (s ^ 0x80) & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & s) != 0);
}

void cmp_b_dan(void)
{
	w8 r, s, d;
	s = ReadByte(aReg[code & 7] + (w16)RW(pc++));
	d = *((w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80) == 0) && (((s | r) & 0x80) != 0)) ||
		((s & r & 0x80) != 0);
	overflow = ((0x80 & (s ^ 0x80) & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & s) != 0);
}

void cmp_b_dn(void)
{
	w8 r, s, d;
	s = (w8)reg[code & 7];
	d = *((w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80) == 0) && (((s | r) & 0x80) != 0)) ||
		((s & r & 0x80) != 0);
	overflow = ((0x80 & (s ^ 0x80) & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & s) != 0);
}

void cmp_w(void)
{
	w16 r, s, d;
	s = GetFromEA_w[(code >> 3) & 7]();
	d = *((w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x8000) == 0) && (((s | r) & 0x8000) != 0)) ||
		((s & r & 0x8000) != 0);

	overflow = ((0x8000 & (s ^ 0x8000) & d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (d ^ 0x8000) & s) != 0);
}

void cmp_w_dn(void)
{
	w16 r, s, d;
	s = (w16)reg[code & 7];
	d = *((w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x8000) == 0) && (((s | r) & 0x8000) != 0)) ||
		((s & r & 0x8000) != 0);

	overflow = ((0x8000 & (s ^ 0x8000) & d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (d ^ 0x8000) & s) != 0);
}

void cmp_l(void)
{
	w32 r, s, d;
	s = GetFromEA_l[(code >> 3) & 7]();
	d = *((w32 *)((Ptr)reg + ((code >> 7) & 28))); /* d=reg[(code>>9)&7] */
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow =
		((0x80000000 & (s ^ 0x80000000) & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & s) != 0);
}

void cmp_l_dn(void)
{
	w32 r, s, d;
	s = reg[code & 7];
	d = *((w32 *)((Ptr)reg + ((code >> 7) & 28))); /* d=reg[(code>>9)&7] */
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow =
		((0x80000000 & (s ^ 0x80000000) & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & s) != 0);
}

void cmpa_w(void)
{
	w32 r, s, d;
	s = LongFromWord(GetFromEA_w[(code >> 3) & 7]());
	d = *((w32 *)((Ptr)aReg + ((code >> 7) & 28)));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow =
		((0x80000000 & (s ^ 0x80000000) & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & s) != 0);
}

void cmpa_l(void)
{
	w32 r, s, d;
	s = GetFromEA_l[(code >> 3) & 7]();
	d = *((w32 *)((Ptr)aReg + ((code >> 7) & 28)));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow =
		((0x80000000 & (s ^ 0x80000000) & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & s) != 0);
}

void cmpa_l_an(void)
{
	w32 r, s, d;
	s = aReg[code & 7];
	d = *((w32 *)((Ptr)aReg + ((code >> 7) & 28)));
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow =
		((0x80000000 & (s ^ 0x80000000) & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & s) != 0);
}

void cmpi_b(void)
{
	w8 r, s, d;
	s = (w8)RW(pc++);
	d = GetFromEA_b[(code >> 3) & 7]();
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80) == 0) && (((s | r) & 0x80) != 0)) ||
		((s & r & 0x80) != 0);
	overflow = ((0x80 & (s ^ 0x80) & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & s) != 0);
}

void cmpi_w(void)
{
	w16 r, s, d;
	s = (w16)RW(pc++);
	d = GetFromEA_w[(code >> 3) & 7]();
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x8000) == 0) && (((s | r) & 0x8000) != 0)) ||
		((s & r & 0x8000) != 0);

	overflow = ((0x8000 & (s ^ 0x8000) & d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (d ^ 0x8000) & s) != 0);
}

void cmpi_l(void)
{
	w32 r, s, d;
	/* s=*((w32*)pc); */
	s = RL((Ptr)pc);

	pc += 2;
	d = GetFromEA_l[(code >> 3) & 7]();
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow =
		((0x80000000 & (s ^ 0x80000000) & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & s) != 0);
}

void cmpm_b(void)
{
	w8 r, s, d;
	/*printf("call cmpm\n");*/

	s = GetFromEA_b_m3();
	/*code>>=9; */ /* una delle uniche modifiche di 'code' !! !!!*/
	d = GetFromEA_rb_m3((code >> 9) & 7);

	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80) == 0) && (((s | r) & 0x80) != 0)) ||
		((s & r & 0x80) != 0);
	overflow = ((0x80 & (s ^ 0x80) & d & (r ^ 0x80)) != 0) ||
		   ((0x80 & r & (d ^ 0x80) & s) != 0);
}

void cmpm_w(void)
{
	w16 r, s, d;
	/*printf("call cmpm_w\n");*/
	s = GetFromEA_w_m3();
	/*code>>=9;*/ /* una delle uniche modifiche di 'code' !! !!!*/
	d = GetFromEA_rw_m3((code >> 9) & 7);
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x8000) == 0) && (((s | r) & 0x8000) != 0)) ||
		((s & r & 0x8000) != 0);

	overflow = ((0x8000 & (s ^ 0x8000) & d & (r ^ 0x8000)) != 0) ||
		   ((0x8000 & r & (d ^ 0x8000) & s) != 0);
}

void cmpm_l(void)
{
	w32 r, s, d;
	/*printf("call cmpm_l\n");*/
	s = GetFromEA_l_m3();
	/*code>>=9;*/ /* una delle uniche modifiche di 'code' !! !!!*/
	d = GetFromEA_rl_m3((code >> 9) & 7);
	r = d - s;
	negative = r < 0;
	zero = r == 0;
	carry = (((d & 0x80000000) == 0) && (((s | r) & 0x80000000) != 0)) ||
		((s & r & 0x80000000) != 0);

	overflow =
		((0x80000000 & (s ^ 0x80000000) & d & (r ^ 0x80000000)) != 0) ||
		((0x80000000 & r & (d ^ 0x80000000) & s) != 0);
}

void dbcc(void)
{
	if (ConditionTrue[(code >> 8) & 15]())
		pc++;
	else {
		if (((*((uw16 *)((Ptr)(&(reg[code & 7])) + RWO)))--) == 0)
			pc++;
		else {
#ifdef DEBUG
			SetPC((Ptr)pc - (Ptr)theROM + (w16)RW(pc));
#else
			pc = (uw16 *)((Ptr)pc + (w16)RW(pc));
#ifdef TRACE
			CheckTrace();
#endif
#endif

			if ((uintptr_t)pc & 1) {
				exception = 3;
				extraFlag = true;
				nInst2 = nInst;
				nInst = 0;
				readOrWrite = 16;
				badAddress = (Ptr)pc - (Ptr)theROM;
				badCodeAddress = true;
			}
		}
	}
}

void dbf(void)
{
	if (((*((uw16 *)((Ptr)(&(reg[code & 7])) + RWO)))--) == 0)
		pc++;
	else {
#ifdef DEBUG
		SetPC((Ptr)pc - (Ptr)theROM + (w16)RW(pc));
#else
		pc = (uw16 *)((Ptr)pc + (w16)RW(pc));
#ifdef TRACE
		CheckTrace();
#endif
#endif
		if ((uintptr_t)pc & 1) {
			exception = 3;
			extraFlag = true;
			nInst2 = nInst;
			nInst = 0;
			readOrWrite = 16;
			badAddress = (Ptr)pc - (Ptr)theROM;
			badCodeAddress = true;
		}
	}
}

void divs(void)
{
	w32 *d;
	w32 r;
	w16 s;
	d = (w32 *)((Ptr)reg + ((code >> 7) & 28));
	s = GetFromEA_w[(code >> 3) & 7]();
	if (s != 0) {
		r = *d / s;
		if (r < -32768 || r > 32767)
			overflow = true;
		else {
			zero = r == 0;
			negative = r < 0;
			overflow = carry = false;
			*((w16 *)((Ptr)d + UW_RWO)) = *d - r * s;
			*((w16 *)((Ptr)d + RWO)) = (w16)r;
		}
	} else {
		exception = 5;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void divu(void)
{
	uw32 *d;
	uw32 r;
	uw16 s;
	d = (uw32 *)((Ptr)reg + ((code >> 7) & 28));
	s = GetFromEA_w[(code >> 3) & 7]();
	if (s != 0) {
		r = *d / s;
		if (r > 65535)
			overflow = true;
		else {
			zero = r == 0;
			negative = (w32)r < 0;
			overflow = carry = false;
			*((uw16 *)((Ptr)d + UW_RWO)) = *d - r * s;
			*((uw16 *)((Ptr)d + RWO)) = (uw16)r;
		}
	} else {
		exception = 5;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void eor_b(void)
{
	register w8 d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	d = d ^ *((w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO));
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_b(d);
}

void eor_w(void)
{
	register w16 d;

	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	d = d ^ *((w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO));
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_w(d);
}

void eor_l(void)
{
	register w32 d;

	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	d = d ^ *((w32 *)((Ptr)reg + ((code >> 7) & 28)));
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_l(d);
}

void eori_b(void)
{
	register w8 d;
	d = (w8)RW(pc++);
	d = d ^ ModifyAtEA_b((code >> 3) & 7, code & 7);
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_b(d);
}

void eori_w(void)
{
	register w16 d;

	d = (w16)RW(pc++);
	d = d ^ ModifyAtEA_w((code >> 3) & 7, code & 7);
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_w(d);
}

void eori_l(void)
{
	register w32 d;

	d = RL((w32 *)pc);
	pc += 2;
	d = d ^ ModifyAtEA_l((code >> 3) & 7, code & 7);
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_l(d);
}

void eori_to_ccr(void)
{
	register uw16 d;
	d = RW(pc++);
	if ((d & 1) != 0)
		carry = !carry;
	if ((d & 2) != 0)
		overflow = !overflow;
	if ((d & 4) != 0)
		zero = !zero;
	if ((d & 8) != 0)
		negative = !negative;
	if ((d & 16) != 0)
		xflag = !xflag;
}

void eori_to_sr(void)
{
	register w16 d;
	d = (w16)RW(pc++);
	if (supervisor) {
		PutSR(GetSR() ^ d);
	} else {
		exception = 8;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void exg_d(void)
{
	w32 t;
	short r1, r2;
	r1 = code & 7;
	r2 = (code >> 9) & 7;
	t = reg[r1];
	reg[r1] = reg[r2];
	reg[r2] = t;
}

void exg_a(void)
{
	w32 t;
	short r1, r2;
	r1 = code & 7;
	r2 = (code >> 9) & 7;
	t = aReg[r1];
	aReg[r1] = aReg[r2];
	aReg[r2] = t;
}

void exg_ad(void)
{
	w32 t;
	short r1, r2;
	r1 = code & 7;
	r2 = (code >> 9) & 7;
	t = aReg[r1];
	aReg[r1] = reg[r2];
	reg[r2] = t;
}

void ext_w(void)
{
	register w16 *dn;
	dn = (w16 *)(RWO + (Ptr)(&reg[code & 7]));

	*dn = WordFromByte((w8)(*dn));
	zero = *dn == 0;
	negative = *dn < 0;
	overflow = carry = false;
}

void ext_l(void)
{
	register w32 *dn;
	dn = &reg[code & 7];
	*dn = (w32)((w16)(*dn));
	zero = *dn == 0;
	negative = *dn < 0;
	overflow = carry = false;
}

void illegal(void)
{
	exception = 4;
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
	pc--; /* ???? sembra che al QL piaccia cos=EC! Altrimenti i
	   breakpoint non funzionano */
}

void jmp(void)
{
	SetPC(ARCALL(GetEA, (code >> 3) & 7, (code & 7)));
	/*SetPC(GET_EA((code>>3)&7,code&7));*/
}

void jsr(void)
{
	w32 ea;

	ea = ARCALL(GetEA, (code >> 3) & 7, (code & 7));
	/* ea=GET_EA((code>>3)&7,(code&7));*/
	WriteLong((*m68k_sp) -= 4, (w32)((Ptr)pc - (Ptr)theROM));
#ifdef BACKTRACE
	SetPCB(ea, JSR);
#else
	SetPC(ea);
#endif
}

void jsr_displ(void)
{
	register w32 ea;
	ea = (uintptr_t)pc - (uintptr_t)theROM;
	WriteLong((*m68k_sp) -= 4, ea + 2);
	SetPC(ea + (w16)RW(pc));
}

void lea(void)
{
	*((w32 *)((Ptr)aReg + ((code >> 7) & 28))) =
		ARCALL(GetEA, (code >> 3) & 7, (code & 7));
}

void link(void)
{
	register w32 *r;
	r = &(aReg[code & 7]);
	WriteLong((*m68k_sp) -= 4, *r);
	*r = (*m68k_sp);
	(*m68k_sp) += (w16)RW(pc++);
}

void lsl_m(void)
{
	register uw16 d;

	d = (uw16)ModifyAtEA_w((code >> 3) & 7, code & 7);
	carry = xflag = (d & 0x8000) != 0;
	d <<= 1;
	negative = (d & 0x8000) != 0;
	zero = d == 0;
	overflow = false;
	RewriteEA_w((w16)d);
}

void lsr_m(void)
{
	register uw16 d;

	d = (uw16)ModifyAtEA_w((code >> 3) & 7, code & 7);
	carry = xflag = (d & 1) != 0;
	d >>= 1;
	zero = d == 0;
	negative = overflow = false;
	RewriteEA_w((w16)d);
}

void move_b(void)
{
	register w8 d;
	d = GetFromEA_b[(code >> 3) & 7]();
	ARCALL(PutToEA_b, (code >> 6) & 7, (code >> 9) & 7, d);
	/*PUT_TOEA_B((code>>6)&7,(code>>9)&7,d);*/
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_b_from_dn(void)
{
	register w8 d;
	d = *((w8 *)(&reg[code & 7]) + RBO);
	ARCALL(PutToEA_b, (code >> 6) & 7, (code >> 9) & 7, d);
	/*PUT_TOEA_B((code>>6)&7,(code>>9)&7,d);*/
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_b_to_dn(void)
{
	register w8 d;
	d = *((w8 *)reg + ((code >> 7) & 28) + RBO) =
		GetFromEA_b[(code >> 3) & 7]();
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_b_reg(void)
{
	register w8 d;
	d = *((w8 *)reg + ((code >> 7) & 28) + RBO) =
		*((w8 *)(&reg[code & 7]) + RBO);
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_w(void)
{
	register w16 d;
	d = GetFromEA_w[(code >> 3) & 7]();
	ARCALL(PutToEA_w, (code >> 6) & 7, (code >> 9) & 7, d);
	/*PUT_TOEA_W((code>>6)&7,(code>>9)&7,d);*/
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_w_from_dn(void)
{
	register w16 d;
	d = (w16)reg[code & 7];
	ARCALL(PutToEA_w, (code >> 6) & 7, (code >> 9) & 7, d);
	/*PUT_TOEA_W((code>>6)&7,(code>>9)&7,d);*/
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_w_to_dn(void)
{
	register w16 d;
	d = GetFromEA_w[(code >> 3) & 7]();
	*((w16 *)((Ptr)reg + RWO + ((code >> 7) & 28))) = d;
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_w_reg(void)
{
	register w16 d;
	d = *((w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO)) =
		*(w16 *)((Ptr)(&reg[code & 7]) + RWO);
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_l(void)
{
	register w32 d;
	d = GetFromEA_l[((code >> 3) & 7)]();
	ARCALL(PutToEA_l, ((code >> 6) & 7), ((code >> 9) & 7), d);
	/*PUT_TOEA_L(((code>>6)&7),((code>>9)&7),d);*/
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_l_from_dn(void)
{
	register w32 d;
	d = reg[code & 7];
	ARCALL(PutToEA_l, (code >> 6) & 7, (code >> 9) & 7, d);
	/*PUT_TOEA_L((code>>6)&7,(code>>9)&7,d);*/
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_l_to_dn(void)
{
	register w32 d;
	d = GetFromEA_l[(code >> 3) & 7]();
	*((w32 *)((Ptr)reg + ((code >> 7) & 28))) = d;
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}
void move_l_reg(void)
{
	register w32 d;
	d = *((w32 *)((Ptr)reg + ((code >> 7) & 28))) = reg[code & 7];
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void move_to_ccr(void)
{
	register w16 x;
	x = GetFromEA_w[(code >> 3) & 7]();
	carry = (x & 1) != 0;
	overflow = (x & 2) != 0;
	zero = (x & 4) != 0;
	negative = (x & 8) != 0;
	xflag = (x & 16) != 0;
}

void move_to_sr(void)
{
	register w16 x;
	x = GetFromEA_w[(code >> 3) & 7]();
	if (supervisor)
		PutSR(x);
	else {
		exception = 8;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void move_from_sr(void)
{
	ARCALL(PutToEA_w, (code >> 3) & 7, code & 7, GetSR());
	/*PUT_TOEA_W((code>>3)&7,code&7,GetSR());*/
}

void move_to_usp(void)
{
	if (supervisor)
		usp = aReg[code & 7];
	else {
		exception = 8;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void move_from_usp(void)
{
	if (supervisor)
		aReg[code & 7] = usp;
	else {
		exception = 8;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void movea_w(void)
{
	*((w32 *)((Ptr)aReg + ((code >> 7) & 28))) =
		LongFromWord(GetFromEA_w[(code >> 3) & 7]());
}

void movea_l(void)
{
	*((w32 *)((Ptr)aReg + ((code >> 7) & 28))) =
		GetFromEA_l[(code >> 3) & 7]();
}

void movea_l_an(void)
{
	*((w32 *)((Ptr)aReg + ((code >> 7) & 28))) = aReg[code & 7];
}

void movem_save_w(void)
{
	register uw16 mask;
	register w32 ea;
	register short i;
	w8 eaMode;

	mask = RW(pc++);
	eaMode = ((w8)code >> 3) & 7;
	if (eaMode == 4) /* predecrement mode */
	{
		ea = aReg[eaMode = (code & 7)];
		if ((ea & 1) != 0)
			WriteWord(ea, 0); /* bad address */
		else {
			for (i = 15; mask != 0; mask >>= 1, i--) {
				if ((mask & 1) != 0)
					WriteWord(ea -= 2, (w16)reg[i]);
			}
			aReg[eaMode] = ea;
		}
	} else {
		ea = ARCALL(GetEA, eaMode, (code & 7));
		if ((ea & 1) != 0)
			WriteWord(ea, 0); /* bad address */
		else {
			for (i = 0; mask != 0; mask >>= 1, i++) {
				if ((mask & 1) != 0) {
					WriteWord(ea, (w16)reg[i]);
					ea += 2;
				}
			}
		}
	}
}

void movem_save_l(void)
{
	register uw16 mask;
	register w32 ea;
	register short i;
	w8 eaMode;

	mask = RW(pc++);
	eaMode = ((w8)code >> 3) & 7;
	if (eaMode == 4) /* predecrement mode */
	{
		ea = aReg[eaMode = (code & 7)];
		if ((ea & 1) != 0)
			WriteLong(ea, 0); /* bad address */
		else {
			for (i = 15; mask != 0; mask >>= 1, i--) {
				if ((mask & 1) != 0)
					WriteLong(ea -= 4, reg[i]);
			}
			aReg[eaMode] = ea;
		}
	} else {
		ea = ARCALL(GetEA, eaMode, (code & 7));
		if ((ea & 1) != 0)
			WriteLong(ea, 0); /* bad address */
		else {
			for (i = 0; mask != 0; mask >>= 1, i++) {
				if ((mask & 1) != 0) {
					WriteLong(ea, reg[i]);
					ea += 4;
				}
			}
		}
	}
}

void movem_load_w(void)
{
	register uw16 mask;
	register w32 ea;
	register short i;
	w8 eaMode, eaReg;

	mask = RW(pc++);
	eaMode = ((w8)code >> 3) & 7;
	eaReg = code & 7;
	ea = (eaMode == 3) ? aReg[eaReg] : ARCALL(GetEA, eaMode, (eaReg));
	if ((ea & 1) != 0)
		ReadWord(ea); /* bad address */
	else {
		for (i = 0; mask != 0; mask >>= 1, i++) {
			if ((mask & 1) != 0) {
				reg[i] = LongFromWord(ReadWord(ea));
				ea += 2;
			}
		}
		if (eaMode == 3)
			aReg[eaReg] = ea;
	}
}

void movem_load_l(void)
{
	register uw16 mask;
	register w32 ea;
	register short i;
	w8 eaMode, eaReg;

	mask = RW(pc++);
	eaMode = ((w8)code >> 3) & 7;
	eaReg = code & 7;
	ea = (eaMode == 3) ? aReg[eaReg] : ARCALL(GetEA, eaMode, (eaReg));
	if ((ea & 1) != 0)
		ReadLong(ea); /* bad address */
	else {
		for (i = 0; mask != 0; mask >>= 1, i++) {
			if ((mask & 1) != 0) {
				reg[i] = ReadLong(ea);
				ea += 4;
			}
		}
		if (eaMode == 3)
			aReg[eaReg] = ea;
	}
}

void movep_w_mr(void) /* word from memory */
{
	register w32 ea;
	register w8 *dn;

#if 0
  printf("movep\n");
  DbgInfo();
  /*BackTrace(10);*/
#endif

	ea = aReg[code & 7] + (w32)((w16)RW(pc++));
#ifdef QM_BIG_ENDIAN
	dn = (w8 *)reg + ((code >> 7) & 28) + RWO;
	*dn++ = ReadByte(ea);
	ea += 2;
	*dn = ReadByte(ea);
#else

	dn = (w8 *)reg + ((code >> 7) & 28) + MSB_W;
	*dn-- = ReadByte(ea);
	ea += 2;
	*dn = ReadByte(ea);

#endif
}

void movep_l_mr(void) /* long from memory */
{
	register w32 ea;
	register w8 *dn;

	/*  printf("movep\n"); */

	ea = aReg[code & 7] + (w32)((w16)RW(pc++));
#ifdef QM_BIG_ENDIAN
	dn = (w8 *)reg + ((code >> 7) & 28);
	*dn++ = ReadByte(ea);
	ea += 2;
	*dn++ = ReadByte(ea);
	ea += 2;
	*dn++ = ReadByte(ea);
	ea += 2;
	*dn = ReadByte(ea);
#else
	dn = (w8 *)reg + ((code >> 7) & 28) + MSB_L;
	*dn-- = ReadByte(ea);
	ea += 2;
	*dn-- = ReadByte(ea);
	ea += 2;
	*dn-- = ReadByte(ea);
	ea += 2;
	*dn = ReadByte(ea);
#endif
}

void movep_w_rm(void) /* word to memory */
{
	register w32 ea;
	register w8 *dn;

	/*    printf("movep\n");
   DbgInfo(); */
	/*BackTrace(10);*/

	ea = aReg[code & 7] + (w32)((w16)RW(pc++));
#ifdef QM_BIG_ENDIAN
	dn = (w8 *)reg + ((code >> 7) & 28) + RWO;
	WriteByte(ea, *dn++);
	ea += 2;
	WriteByte(ea, *dn);
#else

	dn = (w8 *)reg + ((code >> 7) & 28) + MSB_W;
	WriteByte(ea, *dn--);
	ea += 2;
	WriteByte(ea, *dn);

#endif
}

void movep_l_rm(void) /* long to memory */
{
	register w32 ea;
	register w8 *dn;

	/* printf("movep\n"); */

	ea = aReg[code & 7] + (w32)((w16)RW(pc++));
#ifdef QM_BIG_ENDIAN
	dn = (w8 *)reg + ((code >> 7) & 28);
	WriteByte(ea, *dn++);
	ea += 2;
	WriteByte(ea, *dn++);
	ea += 2;
	WriteByte(ea, *dn++);
	ea += 2;
	WriteByte(ea, *dn);
#else
	dn = (w8 *)reg + ((code >> 7) & 28) + MSB_L;
	WriteByte(ea, *dn--);
	ea += 2;
	WriteByte(ea, *dn--);
	ea += 2;
	WriteByte(ea, *dn--);
	ea += 2;
	WriteByte(ea, *dn);
#endif
}

void moveq(void)
{
	register w32 d;
	d = LongFromByte((w8)code);
	*((w32 *)((Ptr)reg + ((code >> 7) & 28))) = d;
	negative = d < 0;
	zero = d == 0;
	overflow = carry = false;
}

void muls(void)
{
	register w32 *d;
	d = (w32 *)((Ptr)reg + ((code >> 7) & 28));
	*d = *((w16 *)((Ptr)d + RWO)) * (w32)GetFromEA_w[(code >> 3) & 7]();
	zero = *d == 0;
	negative = *d < 0;
	overflow = carry = false;
}

void mulu(void)
{
	register uw32 *d;
	d = (uw32 *)((Ptr)reg + ((code >> 7) & 28));
	*d = *((uw16 *)((Ptr)d + RWO)) *
	     (uw32)((uw16)GetFromEA_w[(code >> 3) & 7]());
	zero = *d == 0;
	negative = (w32)(*d) < 0;
	overflow = carry = false;
}

void nbcd(void)
{
	w8 d, r;
	uw16 nbcd_lo, nbcd_hi, nbcd_res;
	int nbcd_carry;

	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	nbcd_lo = -(d & 0xF) - (xflag ? 1 : 0);
	nbcd_hi = -(d & 0xF0);

	if (nbcd_lo > 9) {
		nbcd_lo -= 6;
	}
	nbcd_res = nbcd_hi + nbcd_lo;
	nbcd_carry = (nbcd_res & 0x1F0) > 0x90;
	if (nbcd_carry)
		nbcd_res -= 0x60;

	r = nbcd_res & 0xFF;

	/* Set the flags */
	zero = (zero ? 1 : 0) & (r ? 0 : 1);
	negative = r < 0 ? 1 : 0;
	xflag = carry = nbcd_carry ? 1 : 0;

	RewriteEA_b(r);
}

void neg_b(void)
{
	w8 r, d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	r = -d;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = ((d | r) & 0x80) != 0;
	overflow = (0x80 & r & d) != 0;
	RewriteEA_b(r);
}

void neg_w(void)
{
	w16 r, d;

	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	r = -d;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = ((d | r) & 0x8000) != 0;
	overflow = (0x8000 & r & d) != 0;
	RewriteEA_w(r);
}

void neg_l(void)
{
	w32 r, d;

	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	r = -d;
	negative = r < 0;
	zero = r == 0;
	xflag = carry = ((d | r) & 0x80000000) != 0;
	overflow = (0x80000000 & r & d) != 0;
	RewriteEA_l(r);
}

void negx_b(void)
{
	w8 r, d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	r = -d;
	if (xflag)
		r--;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = ((d | r) & 0x80) != 0;
	overflow = (0x80 & r & d) != 0;
	RewriteEA_b(r);
}

void negx_w(void)
{
	w16 r, d;

	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	r = -d;
	if (xflag)
		r--;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = ((d | r) & 0x8000) != 0;
	overflow = (0x8000 & r & d) != 0;
	RewriteEA_w(r);
}

void negx_l(void)
{
	w32 r, d;

	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	r = -d;
	if (xflag)
		r--;
	negative = r < 0;
	zero = zero && r == 0;
	xflag = carry = ((d | r) & 0x80000000) != 0;
	overflow = (0x80000000 & r & d) != 0;
	RewriteEA_l(r);
}

void nop(void)
{
}

void not_b(void)
{
	register w8 d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7) ^ 0xff;
	zero = d == 0;
	negative = d < 0;
	overflow = carry = false;
	RewriteEA_b(d);
}

void not_w(void)
{
	register w16 d;

	d = ModifyAtEA_w((code >> 3) & 7, code & 7) ^ 0xffff;
	zero = d == 0;
	negative = d < 0;
	overflow = carry = false;
	RewriteEA_w(d);
}

void not_l(void)
{
	register w32 d;

	d = ModifyAtEA_l((code >> 3) & 7, code & 7) ^ 0xffffffff;
	zero = d == 0;
	negative = d < 0;
	overflow = carry = false;
	RewriteEA_l(d);
}

void or_b_dn(void)
{
	register w8 *d;
	d = (w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO);
	*d = *d | GetFromEA_b[(code >> 3) & 7]();
	negative = *d < 0;
	zero = *d == 0;
	carry = overflow = false;
}

void or_w_dn(void)
{
	register w16 *d;
	d = (w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO);
	*d = *d | GetFromEA_w[(code >> 3) & 7]();
	negative = *d < 0;
	zero = *d == 0;
	carry = overflow = false;
}

void or_l_dn(void)
{
	register w32 *d;
	d = (w32 *)((Ptr)reg + ((code >> 7) & 28));
	*d = *d | GetFromEA_l[(code >> 3) & 7]();
	negative = *d < 0;
	zero = *d == 0;
	carry = overflow = false;
}

void or_b_ea(void)
{
	register w8 d;
	d = ModifyAtEA_b((code >> 3) & 7, code & 7);
	d = d | *((w8 *)((Ptr)reg + ((code >> 7) & 28) + RBO));
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_b(d);
}

void or_w_ea(void)
{
	register w16 d;

	d = ModifyAtEA_w((code >> 3) & 7, code & 7);
	d = d | *((w16 *)((Ptr)reg + ((code >> 7) & 28) + RWO));
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_w(d);
}

void or_l_ea(void)
{
	register w32 d;

	d = ModifyAtEA_l((code >> 3) & 7, code & 7);
	d = d | *((w32 *)((Ptr)reg + ((code >> 7) & 28)));
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_l(d);
}

void ori_b(void)
{
	register w8 d;
	d = (w8)RW(pc++);
	d = d | ModifyAtEA_b((code >> 3) & 7, code & 7);
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_b(d);
}

void ori_w(void)
{
	register w16 d;

	d = (w16)RW(pc++);
	d = d | ModifyAtEA_w((code >> 3) & 7, code & 7);
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_w(d);
}

void ori_l(void)
{
	register w32 d;

	d = RL((w32 *)pc);
	pc += 2;
	d = d | ModifyAtEA_l((code >> 3) & 7, code & 7);
	negative = d < 0;
	zero = d == 0;
	carry = overflow = false;
	RewriteEA_l(d);
}

void ori_to_ccr(void)
{
	register w16 d;
	d = (w16)RW(pc++);
	carry = carry || ((d & 1) != 0);
	overflow = overflow || ((d & 2) != 0);
	zero = zero || ((d & 4) != 0);
	negative = negative || ((d & 8) != 0);
	xflag = xflag || ((d & 16) != 0);
}

void ori_to_sr(void)
{
	register w16 d;
	d = (w16)RW(pc++);
	if (supervisor) {
		PutSR(GetSR() | d);
	} else {
		exception = 8;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void code1111(void)
{
#ifdef IE_XL
	qlux_table[code]();
#else
	exception = 11;
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
#endif
}

void InvalidCode(void)
{
	exception = 4;
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
	pc--; /* ???? sembra che al QL piaccia cos=EC! Altrimenti i
	   breakpoint non funzionano */
}

void code1010(void)
{
#ifdef IE_XL
	qlux_table[code]();
#else
	exception = 10;
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
#endif
}
