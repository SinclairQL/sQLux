/*
 * (c) UQLX - see COPYRIGHT
 */

// memory access and addressing modes for slow emulation engine
// hw access for slow and fast versions

// want globally visible definitions and little inlining
#define vml
#define STATIC
#define INLINE

#include "QL68000.h"
#include "sqlux_bdi.h"
#include "dummies.h"
#include "unixstuff.h"

#include <signal.h>
#include <time.h>

void debug(char *);
void debug2(char *, long);

extern void vmMarkScreen(uw32 /*addr*/);

#ifdef DEBUG
int trace_rts = 0;
#endif

inline void REGP1 WriteHWByte(aw32 addr, aw8 d);
inline rw8 REGP1 ReadHWByte(aw32 addr);
rw16 ReadHWWord(aw32 addr);
void WriteHWWord(aw32 addr, aw16 d);

#include "memaccess.h"
#include "mmodes.h"

INLINE rw32 GetEA_m2(ashort) AREGP;
INLINE rw32 GetEA_m5(ashort) AREGP;
INLINE rw32 GetEA_m6(ashort) AREGP;
INLINE rw32 GetEA_m7(ashort) AREGP;
INLINE rw32 GetEA_mBad(ashort) AREGP;
rw32 (*GetEA[8])(ashort) /*AREGP*/ = { GetEA_mBad, GetEA_mBad, GetEA_m2,
				       GetEA_mBad, GetEA_mBad, GetEA_m5,
				       GetEA_m6,   GetEA_m7 };

INLINE rw8 GetFromEA_b_m0(void);
INLINE rw8 GetFromEA_b_mBad(void);
INLINE rw8 GetFromEA_b_m2(void);
INLINE rw8 GetFromEA_b_m3(void);
INLINE rw8 GetFromEA_b_m4(void);
INLINE rw8 GetFromEA_b_m5(void);
INLINE rw8 GetFromEA_b_m6(void);
INLINE rw8 GetFromEA_b_m7(void);
rw8 (*GetFromEA_b[8])(void) = { GetFromEA_b_m0, GetFromEA_b_mBad,
				GetFromEA_b_m2, GetFromEA_b_m3,
				GetFromEA_b_m4, GetFromEA_b_m5,
				GetFromEA_b_m6, GetFromEA_b_m7 };

INLINE rw16 GetFromEA_w_m0(void);
INLINE rw16 GetFromEA_w_m1(void);
INLINE rw16 GetFromEA_w_m2(void);
INLINE rw16 GetFromEA_w_m3(void);
INLINE rw16 GetFromEA_w_m4(void);
INLINE rw16 GetFromEA_w_m5(void);
INLINE rw16 GetFromEA_w_m6(void);
INLINE rw16 GetFromEA_w_m7(void);
rw16 (*GetFromEA_w[8])(void) = { GetFromEA_w_m0, GetFromEA_w_m1, GetFromEA_w_m2,
				 GetFromEA_w_m3, GetFromEA_w_m4, GetFromEA_w_m5,
				 GetFromEA_w_m6, GetFromEA_w_m7 };

INLINE rw32 GetFromEA_l_m0(void);
INLINE rw32 GetFromEA_l_m1(void);
INLINE rw32 GetFromEA_l_m2(void);
INLINE rw32 GetFromEA_l_m3(void);
INLINE rw32 GetFromEA_l_m4(void);
INLINE rw32 GetFromEA_l_m5(void);
INLINE rw32 GetFromEA_l_m6(void);
INLINE rw32 GetFromEA_l_m7(void);
rw32 (*GetFromEA_l[8])(void) = { GetFromEA_l_m0, GetFromEA_l_m1, GetFromEA_l_m2,
				 GetFromEA_l_m3, GetFromEA_l_m4, GetFromEA_l_m5,
				 GetFromEA_l_m6, GetFromEA_l_m7 };

INLINE void PutToEA_b_m0(ashort, aw8) AREGP;
INLINE void PutToEA_b_mBad(ashort, aw8) AREGP;
INLINE void PutToEA_b_m2(ashort, aw8) AREGP;
INLINE void PutToEA_b_m3(ashort, aw8) AREGP;
INLINE void PutToEA_b_m4(ashort, aw8) AREGP;
INLINE void PutToEA_b_m5(ashort, aw8) AREGP;
INLINE void PutToEA_b_m6(ashort, aw8) AREGP;
INLINE void PutToEA_b_m7(ashort, aw8) AREGP;
void (*PutToEA_b[8])(ashort, aw8) /*REGP2*/ = { PutToEA_b_m0, PutToEA_b_mBad,
						PutToEA_b_m2, PutToEA_b_m3,
						PutToEA_b_m4, PutToEA_b_m5,
						PutToEA_b_m6, PutToEA_b_m7 };

INLINE void PutToEA_w_m0(ashort, aw16) AREGP;
INLINE void PutToEA_w_m1(ashort, aw16) AREGP;
INLINE void PutToEA_w_m2(ashort, aw16) AREGP;
INLINE void PutToEA_w_m3(ashort, aw16) AREGP;
INLINE void PutToEA_w_m4(ashort, aw16) AREGP;
INLINE void PutToEA_w_m5(ashort, aw16) AREGP;
INLINE void PutToEA_w_m6(ashort, aw16) AREGP;
INLINE void PutToEA_w_m7(ashort, aw16) AREGP;
void (*PutToEA_w[8])(ashort, aw16) /*REGP2*/ = { PutToEA_w_m0, PutToEA_w_m1,
						 PutToEA_w_m2, PutToEA_w_m3,
						 PutToEA_w_m4, PutToEA_w_m5,
						 PutToEA_w_m6, PutToEA_w_m7 };

INLINE void PutToEA_l_m0(ashort, aw32) AREGP;
INLINE void PutToEA_l_m1(ashort, aw32) AREGP;
INLINE void PutToEA_l_m2(ashort, aw32) AREGP;
INLINE void PutToEA_l_m3(ashort, aw32) AREGP;
INLINE void PutToEA_l_m4(ashort, aw32) AREGP;
INLINE void PutToEA_l_m5(ashort, aw32) AREGP;
INLINE void PutToEA_l_m6(ashort, aw32) AREGP;
INLINE void PutToEA_l_m7(ashort, aw32) AREGP;
void (*PutToEA_l[8])(ashort, aw32) /*REGP2*/ = { PutToEA_l_m0, PutToEA_l_m1,
						 PutToEA_l_m2, PutToEA_l_m3,
						 PutToEA_l_m4, PutToEA_l_m5,
						 PutToEA_l_m6, PutToEA_l_m7 };

#ifdef DEBUG
#define TRR                                                                    \
	{                                                                      \
		trace_rts = 20;                                                \
	}
#else
#define TRR
#endif

w8 ReadRTClock(w32 addr)
{
	w32 t;
	GetDateTime(&t);
	/*  t-=qlClock; */
	while (addr++ < 0x18003l)
		t >>= 8;
	prep_rtc_emu();
	return (w8)t;
}

void FrameInt(void)
{
	if ((intReg & 8) != 0) /* controlla che sia abilitato */
	{
		theInt = 8;
		intReg ^= 8;
		pendingInterrupt = 2;
		*((uw8 *)theROM + 0x280a0l) = 16;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

inline void REGP1 WriteInt(aw8 d)
{
	intReg = d;
}

w8 IntRead(void)
{
	register w8 t;

	t = theInt;
	theInt = 0;
	return t;
}

void WriteHWByte(aw32 addr, aw8 d)
{
	/*printf("write HWreg at %x val=%x\n",addr-0x18000,d);*/

	switch (addr) {
	case 0x018063: /* Display control */
		SetDisplay(d, true);
	case 0x018000:
	case 0x018001:
		/* ignore write to real-time clock registers */
	case 0x018023:
		/* ignore write to no reg */
		break;
	case 0x018002:
		if (d != 16) {
			debug2("Write to transmit control >", d);
			debug2("at pc-2 ", (Ptr)pc - (Ptr)theROM - 2);
			/*TRR;*/
		}
		break;
	case 0x018003:
		debugIPC("Write to IPC link > ", d);
		debugIPC("at (PC-2) ", (Ptr)pc - (Ptr)theROM - 2);
		/*TRR;*/
		break;
	case 0x018020:
		WriteMdvControl(d); /*TRR;*/
		break;
	case 0x018021:
		WriteInt(d);
		break;
	case 0x018022:
		debug2("Write to MDV/RS232 data >", d); /*TRR;*/
		break;
	case 0x018100:
		SQLUXBDISelect(d);
		break;
	case 0x018101:
		SQLUXBDICommand(d);
		break;
	case 0x018103:
		SQLUXBDIDataWrite(d);
		break;
	default:
		debug2("Write to HW register ", addr);
		debug2("at (PC-2) ", (Ptr)pc - (Ptr)theROM - 2);
		/*TRR;*/
		break;
	}
}

uint64_t nanotime;

rw8 ReadHWByte(aw32 addr)
{
	int res = 0;
	struct timespec timer;

	/*printf("read HWreg %x, ",addr-0x18000);*/

	switch (addr) {
	case 0x018000: /* Read from real-time clock */
	case 0x018001:
	case 0x018002:
	case 0x018003:
		return res = ReadRTClock(addr);
	case 0x018020:
		debug("Read from MDV/RS232 status");
		debug2("PC-2=", (Ptr)pc - (Ptr)theROM - 2);
		break;
	case 0x018021:
		/*printf("reading $18021 at pc=%x\n",(Ptr)pc-(Ptr)theROM-2);*/
		res = IntRead();
	case 0x018022: /*debug("Read from MDV track 1");*/
		break;
	case 0x018023: /*debug("Read from MDV track 2");*/
		break;
	case 0x018102:
		res = SQLUXBDIStatus();
		break;
	case 0x018103:
		res = SQLUXBDIDataRead();
		break;
	case 0x01C060:
		/* trigger nanotime update */
		clock_gettime(CLOCK_MONOTONIC, &timer);
		nanotime = timer.tv_sec * 1000000000 + timer.tv_nsec;
		nanotime /= 25;
		nanotime &= 0xFFFFFFFF;
		return (nanotime & 0xFF000000) >> 24;
		break;
	case 0x01C061:
		return (nanotime & 0x00FF0000) >> 16;
		break;
	case 0x01C062:
		return (nanotime & 0x0000FF00) >> 8;
		break;
	case 0x01C063:
		return (nanotime & 0xFF);
		break;
	default:
		debug2("Read from HW register ", addr);
		debug2("at (PC-2) ", (Ptr)pc - (Ptr)theROM - 2);
		break;
	}
	/*printf("result %x \n",res);*/
	return res;
}

rw16 ReadHWWord(aw32 addr)
{
	switch (addr) {
	case 0x018108:
		return SQLUXBDISizeHigh();
		break;
	case 0x01810A:
		return SQLUXBDISizeLow();
		break;
	default:
		return ((w16)ReadHWByte(addr) << 8) | (uw8)ReadHWByte(addr + 1);
	}
}

void WriteHWWord(aw32 addr, aw16 d)
{
	switch (addr) {
	case 0x018104:
		SQLUXBDIAddressHigh(d);
		break;
	case 0x018106:
		SQLUXBDIAddressLow(d);
		break;
	default:
		WriteByte(addr, d >> 8);
		WriteByte(addr + 1, d & 255);
	}
}

aw32 ReadHWLong(aw32 addr)
{
	uint64_t nanotime;
	struct timespec timer;

	switch(addr) {
	case 0x01C060:
		clock_gettime(CLOCK_MONOTONIC, &timer);
		nanotime = timer.tv_sec * 1000000000 + timer.tv_nsec;
		nanotime /= 25;
		nanotime &= 0xFFFFFFFF;
		return nanotime;
		break;
	default:
		return ((w32)ReadWord(addr) << 16) | (uw16)ReadWord(addr + 2);
	}
}
