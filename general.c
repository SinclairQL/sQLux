/*
 * (c) UQLX - see COPYRIGHT
 */

// memory access and addressing modes for slow emulation engine
// hw access for slow and fast versions

// want globally visible definitions and little inlining
#define vml
#define STATIC
#define INLINE

#include <string.h>
#include "QL68000.h"
#include "sqlux_bdi.h"
#include "dummies.h"
#include "unixstuff.h"
#include <stdbool.h>
#include <signal.h>
#include <time.h>

#include "sqlux_debug.h"
#include "QL_screen.h"
#include "SDL2screen.h"
#include "mdv.h"
#include "ipc.h"
#include "zx8301.h"

extern int display_mode;
extern volatile bool is_display_blank;

volatile bool is_display_blank = false; // Display active by default (bit 1 set to 0)
void debug(char *);
void debug2(char *, long);

extern void vmMarkScreen(uw32 /*addr*/);
extern int speed;                              // unixstuff.c
extern uint64_t ql_cycles;                     // iexl_general.c

#ifdef DEBUG
int trace_rts = 0;
#endif

inline void REGP1 WriteHWByte(aw32 addr, aw8 d);
inline rw8 REGP1 ReadHWByte(aw32 addr);
rw16 ReadHWWord(aw32 addr);
void WriteHWWord(aw32 addr, aw16 d);

#include "memaccess.h"
#include "mmodes.h"
#include "emulator_options.h"

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

/*
 * HW_TRACE (debugging aid): hardware activity per second of emulated time,
 * printed on the console: interrupts accepted by level, interrupts cleared
 * in $18021 by source, accesses to the ZX8302 registers and IPC traffic.
 */
unsigned hw_trace_irq_taken[8];
static int hw_trace = -1;
static unsigned hwt_frames, hwt_ack[5], hwt_r18020, hwt_w18002, hwt_w18003,
		hwt_w18022;
static uint64_t hwt_cycles0, hwt_wait0;
static uint64_t hwt_late_sum, hwt_late_max;
static unsigned hwt_late_n;

/* Cycles by which the frame ran past its end before the frame interrupt */
void hw_trace_vsync_late(uint64_t late)
{
	hwt_late_sum += late;
	if (late > hwt_late_max)
		hwt_late_max = late;
	hwt_late_n++;
}

/* HW_TRACE >= 3: cycles spent in supervisor mode, by 4K page of the PC,
 * sampled every 16 instructions */
#define HWT_PAGES 4096
static uint64_t hwt_prof_last, hwt_user, hwt_super, hwt_page[HWT_PAGES];

void hw_trace_sample(void)
{
	uint64_t d;

	if (hw_trace < 3)
		return;
	d = ql_cycles - hwt_prof_last;
	hwt_prof_last = ql_cycles;
	if (supervisor) {
		uint32_t a = (uint32_t)((char *)pc - (char *)memBase);
		hwt_super += d;
		hwt_page[(a >> 12) % HWT_PAGES] += d;
	} else {
		hwt_user += d;
	}
}

static void hw_trace_profile(void)
{
	uint64_t total = hwt_user + hwt_super;
	int i, k;

	if (!total)
		return;
	printf("HW: supervisor %.1f %% (%llu cycles/frame) | top:",
	       100.0 * (double)hwt_super / (double)total,
	       (unsigned long long)(hwt_super / hwt_frames));
	for (k = 0; k < 6; k++) {
		int best = -1;
		for (i = 0; i < HWT_PAGES; i++)
			if (hwt_page[i] && (best < 0 || hwt_page[i] > hwt_page[best]))
				best = i;
		if (best < 0)
			break;
		printf(" $%05X %.1f%%", best << 12,
		       100.0 * (double)hwt_page[best] / (double)total);
		hwt_page[best] = 0;
	}
	printf("\n");
	memset(hwt_page, 0, sizeof(hwt_page));
	hwt_user = hwt_super = 0;
}
extern uint64_t zx8301_wait_total;       /* zx8301.c */
extern volatile bool is_display_blank;

static void hw_trace_frame(void)
{
	unsigned bits = 0, notes = 0, ipc_ms = 0, host_ms = 0;

	if (hw_trace < 0) {
		hw_trace = emulatorOptionInt("hw_trace");
		ipc_lle_set_trace(hw_trace);
	}
	if (!hw_trace)
		return;
	if (++hwt_frames < (unsigned)zx8301_hz())
		return;
	if (ipc_lle_active())
		ipc_lle_trace(&bits, &notes, &ipc_ms, &host_ms);
	{
		uint64_t cyc = ql_cycles - hwt_cycles0;
		uint64_t wait = zx8301_wait_total - hwt_wait0;

		printf("HW: frames %u | IRQ L2 %u L5 %u L7 %u | ack frame %u gap %u "
		       "interface %u transmit %u external %u | r18020 %u w18002 %u "
		       "w18003 %u w18022 %u | IPC bits %u notes %u time %u ms "
		       "(host %u ms) | CPU %llu cycles, contention %.1f %%%s | "
		       "VSYNC late avg %llu max %llu cycles\n",
		       hwt_frames, hw_trace_irq_taken[2], hw_trace_irq_taken[5],
		       hw_trace_irq_taken[7], hwt_ack[3], hwt_ack[0], hwt_ack[1],
		       hwt_ack[2], hwt_ack[4], hwt_r18020, hwt_w18002, hwt_w18003,
		       hwt_w18022, bits, notes, ipc_ms, host_ms,
		       (unsigned long long)cyc,
		       cyc ? 100.0 * (double)wait / (double)cyc : 0.0,
		       is_display_blank ? " (display off)" : "",
		       (unsigned long long)(hwt_late_n ? hwt_late_sum / hwt_late_n : 0),
		       (unsigned long long)hwt_late_max);
		hwt_late_sum = hwt_late_max = 0;
		hwt_late_n = 0;
		hwt_cycles0 = ql_cycles;
		hwt_wait0 = zx8301_wait_total;
	}
	if (hw_trace >= 3)
		hw_trace_profile();
	fflush(stdout);
	hwt_frames = hwt_r18020 = hwt_w18002 = hwt_w18003 = hwt_w18022 = 0;
	memset(hwt_ack, 0, sizeof(hwt_ack));
	memset(hw_trace_irq_taken, 0, sizeof(hw_trace_irq_taken));
}

/*
 * Interrupt request lines of the 68008, as in zx8302.v of the MiSTer QL core.
 * IPL0 and IPL2 are tied together; the IPC drives them through P2.2, and
 * IPL1 through P2.3. The ZX8302 also pulls IPL1 low while any interrupt is
 * pending in $18021, so the level is 2 for as long as a pending bit has not
 * been cleared (a level, not an event), 5 or 7 when the IPC asks for them.
 * Level 7 (non maskable) is edge triggered: it is taken once per assertion.
 */
static int ipc_ipl_lines = 3;           /* bit 0 = P2.2, bit 1 = P2.3; 1 = inactive */
static int nmi_taken = 0;

void ql_update_ipl(void)
{
	int ipl0 = ipc_ipl_lines & 1;
	int ipl1 = ((ipc_ipl_lines >> 1) & 1) && !(theInt & 0x1f);
	int level = (ipl0 ? 0 : 5) + (ipl1 ? 0 : 2);

	if (level != 7)
		nmi_taken = 0;
	else if (nmi_taken)
		level = 0;
	pendingInterrupt = level;

	/* taken after the current instruction if the mask allows it */
	if (level == 7 || level > iMask) {
		extraFlag = true;
		if (nInst > 0) {
			nInst2 = nInst;
			nInst = 0;
		}
	}
}

/* The IPC changed its interrupt lines (P2.2 and P2.3) */
void ql_set_ipc_ipl(int lines)
{
	ipc_ipl_lines = lines & 3;
	ql_update_ipl();
}

/* The CPU has just accepted an interrupt of this level */
void ql_irq_accepted(int level)
{
	if (level == 7)
		nmi_taken = 1;
	ql_update_ipl();
}

void FrameInt(void)
{
	hw_trace_frame();
	/* the frame interrupt is latched on the rising edge of VSYNC */
	if ((theInt & 8) == 0) {
		theInt |= 8;
		*((uw8 *)memBase + 0x280a0l) = 16;
		ql_update_ipl();
	}

	// Real vertical sync (50 Hz PAL / 60 Hz NTSC): from here the beam
	// position is counted in emulated cycles. At SPEED = 1 a frame lasts
	// 7.5 MHz / hz cycles; at unlimited speed (speed = 0) the screen is
	// copied at once.
	extern void QLSDLFrameStart(uint64_t now, uint64_t frame_len);
	uint64_t frame_cycles = speed ? zx8301_frame_budget(speed) : 0;
	zx8301_frame(ql_cycles, speed);
	ipc_lle_frame(frame_cycles, zx8301_frame_cycles());
	QLSDLFrameStart(ql_cycles, frame_cycles);
}

void ql_trigger_gap_interrupt(void) {
    theInt |= 0x01;        // Bit 0: ZX8302 GAP interrupt pending
    ql_update_ipl();
}

void WriteInt(uint8_t d)
{
	int i;

	for (i = 0; i < 5; i++)
		if (d & (1 << i))
			hwt_ack[i]++;

	// remove the mask bits
	d &= 0x1F;

	// clear interrupts: the interrupt line drops when nothing is pending
	theInt &= ~d;
	ql_update_ipl();
}

uint8_t IntRead(void)
{
	return theInt;
}

static int ipc_wait = 1;
static int ipc_rcvd = 1;
static int ipc_previous = 0x10;
static int ipc_return;
static int ipc_count = 0;
static int ipc_return;
static uint32_t ipc_read;

void ipc_exec(int command)
{
	DEBUG_PRINT("IPC previous %x cmd: %x\n", ipc_previous, command);

	switch(command) {
	case 0x01:
		ipc_return = 0;
		ipc_count = 8;
		break;
	case 0x08:
		ipc_return = 0x1039;
		ipc_count = 16;
	case 0x0d:
		ipc_wait = 1;
		break;
	case 0x10: // Dummy
		break;
	default:
		ipc_return = 0;
		ipc_count = 4;
		break;
	}

	ipc_previous = command;

}

void ipc_write(uint8_t d)
{
	int command;

	DEBUG_PRINT("ipc_write %x\n", d);
	if (ipc_wait) {
		if ((d & 0x0c) == 0x0c) {
			ipc_rcvd <<= 1;
			if (d == 0x0c) {
				ipc_rcvd |= 0;
			} else {
				ipc_rcvd |= 1;
			}
			DEBUG_PRINT("ipc_rcvd %x\n", ipc_rcvd);
			if (ipc_rcvd & 0x10) {
				command = ipc_rcvd & 0x0f;
				ipc_rcvd = 1;
				ipc_wait = 0;
				ipc_exec(command);
			}
		}
	} else {
		DEBUG_PRINT("result read %x\n", d);
		ipc_read = 0;
		ipc_count--;

		if (ipc_return & (1 << ipc_count)) {
			ipc_read |= 0x80;
		}
		ipc_read <<= 8;
		ipc_read |= 0xa50000;

		if (ipc_count == 0) {
			ipc_wait = 1;
		}
	}
}

void WriteHWByte(aw32 addr, aw8 d)
{
	/*printf("write HWreg at %x val=%x\n",addr-0x18000,d);*/

	switch (addr) {
	case 0x018063: /* Display control */
        display_mode = (d & 8) ? 8 : 4;
        qlscreen.qm_lo = (d & 0x80) ? 0x00028000 : 0x00020000;
        qlscreen.qm_hi = qlscreen.qm_lo + qlscreen.qm_len;
				// Bit 1 a 1 = Disabled display (Blank)
				// Bit 1 a 0 = Enabled display
		is_display_blank = (d & 0x02) ? true : false;
		SetDisplay(d, true);
		break; // add for security
	case 0x018000:
	case 0x018001:
		/* ignore write to real-time clock registers */
		break;
	case 0x018002:
		hwt_w18002++;
		/* bits 0-2 select the baud rate, which also sets BAUDx4 (IPC T1) */
		if (ipc_lle_active())
			ipc_lle_set_baud(d);
		if (d != 16) {
			debug2("Write to transmit control >", d);
			debug2("at pc-2 ", (Ptr)pc - (Ptr)memBase - 2);
			/*TRR;*/
		}
		break;
	case 0x018003:
		DEBUG_PRINT("Write to IPC link > %d\n", d);
		DEBUG_PRINT("at (PC-2) %8.8x\n", (Ptr)pc - (Ptr)memBase - 2);
		hwt_w18003++;
		if (ipc_lle_active())
			ipc_lle_write_comdata(d);
		else
			ipc_write(d);
		break;
	case 0x018020:
		mdv_sync();
		mdv_write_control(d);
		break;
	case 0x018021:
		mdv_sync();
		mdv_write_int(d);
		WriteInt(d);
		break;
	case 0x018022:
		hwt_w18022++;
		debug2("Write to MDV/RS232 data >", d); /*TRR;*/
		break;
	case 0x018023:
		/* ignore write to no reg */
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
		debug2("at (PC-2) ", (Ptr)pc - (Ptr)memBase - 2);
		/*TRR;*/
		break;
	}
}

uint64_t nanotime;

rw8 ReadHWByte(aw32 addr)
{
	int res = 0;
	struct timespec timer;
	uint8_t ret_byte;

	/*printf("read HWreg %x, ",addr-0x18000);*/

	switch (addr) {
	case 0x018000: /* Read from real-time clock */
	case 0x018001:
	case 0x018002:
	case 0x018003:
		return res = ReadRTClock(addr);

	case 0x018020: /* Read MDV / IPC status */
		hwt_r18020++;
		if (ipc_lle_active()) {
			mdv_sync();
			return (mdv_read_status() & 0x3f) | ipc_lle_status();
		}
		if (ipc_read) {
			ret_byte = ipc_read & 0xff;
			ipc_read >>= 8;
			if (ipc_read == 0xa5) {
				ipc_read = 0;
			}
			return ret_byte;
		}

		mdv_sync();
		return mdv_read_status();

	case 0x018021: /* Interrupt status */
		mdv_sync();
		return mdv_read_int() | IntRead();

	case 0x018022: /* MDV track 1 data */
	case 0x018023: /* MDV track 2 data */
		mdv_sync();
		return mdv_read_data();
	case 0x018102:
		res = SQLUXBDIStatus();
		break;
	case 0x018103:
		res = SQLUXBDIDataRead();
		break;
#ifndef WINXP_COMPAT
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
#endif
	default:
		debug2("Read from HW register ", addr);
		debug2("at (PC-2) ", (Ptr)pc - (Ptr)memBase - 2);
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
#ifndef WINXP_COMPAT
	case 0x01C060:
		clock_gettime(CLOCK_MONOTONIC, &timer);
		nanotime = timer.tv_sec * 1000000000 + timer.tv_nsec;
		nanotime /= 25;
		nanotime &= 0xFFFFFFFF;
		return nanotime;
		break;
#endif
	default:
		return ((w32)ReadWord(addr) << 16) | (uw16)ReadWord(addr + 2);
	}
}
