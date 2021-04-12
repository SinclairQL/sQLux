
#include "QL68000.h"

rw8 GetFromEA_b_m0(void)
{
	return (w8)(reg[code & 7]);
}

rw8 GetFromEA_b_mBad(void)
{
	exception = 4;
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
	return 0;
}

rw32 AREGP GetEA_m2(ashort r)
{
	return aReg[r];
}

rw32 AREGP GetEA_m5(ashort r)
{
	return aReg[r] + (w16)RW(pc++);
}

rw32 AREGP GetEA_m6(ashort r)
{
	/*w16*/ shindex displ;

	displ = (w16)RW(pc++);
	if ((displ & 2048) != 0)
		return *(((w32 *)((Ptr)reg + ((displ >> 10) & 60)))) + aReg[r] +
		       (w32)((w8)displ);
	/*        register access */
	return (w32)(*((w16 *)((Ptr)reg + RWO + ((displ >> 10) & 60)))) +
	       aReg[r] + (w32)((w8)displ);
}

rw32 AREGP GetEA_m7(ashort r)
{
	/*w16*/ shindex displ;
	switch (r) {
	case 0:
		displ = (w16)RW(pc++);
		return displ;
	case 1:
		pc += 2;
		return RL((w32 *)(pc - 2));
	case 2:
		displ = (w16)RW(pc++);
		return (Ptr)pc - (Ptr)theROM - 2 + displ;
	case 3:
		displ = (w16)RW(pc++);
		if ((displ & 2048) != 0)
			return reg[(displ >> 12) & 15] + (Ptr)pc - (Ptr)theROM -
			       2 + (w32)((w8)displ);
		return (w32)((w16)(reg[(displ >> 12) & 15])) + (Ptr)pc -
		       (Ptr)theROM - 2 + (w32)((w8)displ);
	}
	exception = 4;
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
	return 0;
}

rw32 GetEA_m7_3(void)
{
	/*w16*/ shindex displ;

	displ = (w16)RW(pc++);
	if ((displ & 2048) != 0)
		return reg[(displ >> 12) & 15] + (Ptr)pc - (Ptr)theROM - 2 +
		       (w32)((w8)displ);
	return (w32)((w16)(reg[(displ >> 12) & 15])) + (Ptr)pc - (Ptr)theROM -
	       2 + (w32)((w8)displ);
}

rw8 GetFromEA_b_m2(void)
{
	return ReadByte(aReg[code & 7]);
}

rw8 GetFromEA_b_m3(void)
{
	register rw8 b;
	register shindex r;
	uw32 addr;

	r = code & 7;
	addr = aReg[r]++;
	if (r == 7)
		aReg[r]++;
	b = ReadByte(addr);

	return b;
}

/* added as special case for cmpm */
rw8 GetFromEA_rb_m3(ashort r)
{
	register rw8 b;
	uw32 addr;

	addr = aReg[r]++;
	if (r == 7)
		aReg[r]++;
	b = ReadByte(addr);

	return b;
}
rw16 GetFromEA_rw_m3(ashort r)
{
	uw32 addr;

	addr = aReg[r];
	aReg[r] += 2;

	return ReadWord(addr);
}
rw32 GetFromEA_rl_m3(ashort r)
{
	uw32 addr;

	addr = aReg[r];
	aReg[r] += 4;

	return ReadLong(addr);
}
/****/

rw8 GetFromEA_b_m4(void)
{
	register shindex r;

	r = code & 7;
	if (r == 7)
		aReg[r]--;
	return ReadByte(--aReg[r]);
}

rw8 GetFromEA_b_m5(void)
{
	return ReadByte(aReg[code & 7] + (w16)RW(pc++));
}

rw8 GetFromEA_b_m6(void)
{
	return ReadByte(GetEA_m6(code & 7));
}

rw8 GetFromEA_b_m7(void)
{
	register w32 addr;

	switch (code & 7) {
	case 0:
		return ReadByte((w32)((w16)RW(pc++)));
	case 1:
		addr = RL((w32 *)pc);
		pc += 2;
		return ReadByte(addr);
	case 2:
		addr = (uintptr_t)pc - (uintptr_t)theROM + (w16)RW(pc);
		pc++;
		return ReadByte(addr);
	case 3:
		return ReadByte(GetEA_m7_3());
	case 4:
		pc++;
		return RB((w8 *)pc - 1);
	}
	exception = 4;
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
	return 0;
}

rw16 GetFromEA_w_m0(void)
{
	return (w16)(reg[code & 7]);
}

rw16 GetFromEA_w_m1(void)
{
	return (w16)(aReg[code & 7]);
}

rw16 GetFromEA_w_m2(void)
{
	return ReadWord(aReg[code & 7]);
}

rw16 GetFromEA_w_m3(void)
{
	register rw16 t;
	register shindex r;
	uw32 addr;

	r = code & 7;
	addr = aReg[r];
	aReg[r] += 2;
	t = ReadWord(addr);

	return t;
}

rw16 GetFromEA_w_m4(void)
{
	return ReadWord(aReg[code & 7] -= 2);
}

rw16 GetFromEA_w_m5(void)
{
	return ReadWord(aReg[code & 7] + (w16)RW(pc++));
}

rw16 GetFromEA_w_m6()
{
	/*w16*/ shindex displ;
	displ = (w16)RW(pc++);
	return ReadWord(((displ & 2048) != 0) ?
				(*((w32 *)((Ptr)reg + ((displ >> 10) & 60))) +
				 aReg[code & 7] + (w32)((w8)displ)) :
				(w32) * (((w16 *)((Ptr)reg + RWO +
						  ((displ >> 10) & 60)))) +
					aReg[code & 7] + (w32)((w8)displ));
}

rw16 GetFromEA_w_m7(void)
{
	register w32 addr;

	switch (code & 7) {
	case 0:
		return ReadWord((w32)((w16)RW(pc++)));
	case 1:
		addr = RL((w32 *)pc);
		pc += 2;
		return ReadWord(addr);
	case 2:
		addr = (uintptr_t)pc - (uintptr_t)theROM + (w16)RW(pc);
		pc++;
		return ReadWord(addr);
	case 3:
		return ReadWord(GetEA_m7_3());
	case 4:
		return (w16)RW(pc++);
	}
	exception = 4;
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
	return 0;
}

rw32 GetFromEA_l_m0(void)
{
	return reg[code & 7];
}

rw32 GetFromEA_l_m1(void)
{
	return aReg[code & 7];
}

rw32 GetFromEA_l_m2(void)
{
	return ReadLong(aReg[code & 7]);
}

rw32 GetFromEA_l_m3(void)
{
	register rw32 t;
	register shindex r;
	uw32 addr;

	r = code & 7;
	addr = aReg[r];
	aReg[r] += 4;
	t = ReadLong(addr);

	return t;
}

rw32 GetFromEA_l_m4(void)
{
	return ReadLong(aReg[code & 7] -= 4);
}

rw32 GetFromEA_l_m5(void)
{
	return ReadLong(aReg[code & 7] + (w16)RW(pc++));
}

rw32 GetFromEA_l_m6(void)
{
	return ReadLong(GetEA_m6(code & 7));
}

rw32 GetFromEA_l_m7(void)
{
	register w32 addr;

	switch (code & 7) {
	case 0:
		return ReadLong((w32)((w16)RW(pc++)));
	case 1:
		addr = RL((w32 *)pc); /* *((w32*)pc); */
		pc += 2;
		return ReadLong(addr);
	case 2:
		addr = (uintptr_t)pc - (uintptr_t)theROM + (w16)RW(pc);
		pc++;
		return ReadLong(addr);
	case 3:
		return ReadLong(GetEA_m7_3());
	case 4:
		pc += 2;
		return RL((w32 *)(pc - 2));
	}
	exception = 4;
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
	return 0;
}

void AREGP PutToEA_b_m0(ashort r, aw8 d)
{
	*((w8 *)((Ptr)(&(reg[r])) + RBO)) = d;
}

void AREGP PutToEA_b_mBad(ashort r, aw8 d)
{
	exception = 4;
	extraFlag = true;
	nInst2 = nInst;
	nInst = 0;
}

void AREGP PutToEA_b_m2(ashort r, aw8 d)
{
	WriteByte(aReg[r], d);
}

void AREGP PutToEA_b_m3(ashort r, aw8 d)
{
	uw32 addr = aReg[r];
	aReg[r]++;
	if (r == 7)
		aReg[r]++;

	WriteByte(addr, d);
}

void AREGP PutToEA_b_m4(ashort r, aw8 d)
{
	if (r == 7)
		aReg[r]--;
	WriteByte(--aReg[r], d);
}

void AREGP PutToEA_b_m5(ashort r, aw8 d)
{
	WriteByte(aReg[r] + (w16)RW(pc++), d);
}

void AREGP PutToEA_b_m6(ashort r, aw8 d)
{
	WriteByte(GetEA_m6(r), d);
}

void AREGP PutToEA_b_m7(ashort r, aw8 d)
{
	WriteByte(GetEA_m7(r), d);
}

void AREGP PutToEA_w_m0(ashort r, aw16 d)
{
	//*( (w16*)( (Ptr)(&(reg[r])) + RWO ) )=d;
	reg[r] = (reg[r] & 0xFFFF0000) | d;
}

void AREGP PutToEA_w_m1(ashort r, aw16 d)
{
	//*( (w16*)( (Ptr)(&(aReg[r])) + RWO ) )=d;
	aReg[r] = (aReg[r] & 0xFFFF0000) | d;
}

void AREGP PutToEA_w_m2(ashort r, aw16 d)
{
	WriteWord(aReg[r], d);
}

void AREGP PutToEA_w_m3(ashort r, aw16 d)
{
	uw32 addr = aReg[r];

	aReg[r] += 2;
	WriteWord(addr, d);
}

void AREGP PutToEA_w_m4(ashort r, aw16 d)
{
	WriteWord(aReg[r] -= 2, d);
}

void AREGP PutToEA_w_m5(ashort r, aw16 d)
{
	WriteWord(aReg[r] + (w16)RW(pc++), d);
}

void AREGP PutToEA_w_m6(ashort r, aw16 d)
{
	WriteWord(GetEA_m6(r), d);
}

void AREGP PutToEA_w_m7(ashort r, aw16 d)
{
	WriteWord(GetEA_m7(r), d);
}

void AREGP PutToEA_l_m0(ashort r, aw32 d)
{
	reg[r] = d;
}

void AREGP PutToEA_l_m1(ashort r, aw32 d)
{
	aReg[r] = d;
}

void AREGP PutToEA_l_m2(ashort r, aw32 d)
{
	WriteLong(aReg[r], d);
}

void AREGP PutToEA_l_m3(ashort r, aw32 d)
{
	uw32 addr = aReg[r];
	aReg[r] += 4;
	WriteLong(addr, d);
}

void AREGP PutToEA_l_m4(ashort r, aw32 d)
{
	WriteLong(aReg[r] -= 4, d);
}

void AREGP PutToEA_l_m5(ashort r, aw32 d)
{
	WriteLong(aReg[r] + (w16)RW(pc++), d);
}

void AREGP PutToEA_l_m6(ashort r, aw32 d)
{
	WriteLong(GetEA_m6(r), d);
}

void AREGP PutToEA_l_m7(ashort r, aw32 d)
{
	WriteLong(GetEA_m7(r), d);
}
