/*
 * (c) UQLX - see COPYRIGHT
 */


/* define memory access fns */
#include "QL68000.h"
#include "memaccess.h"
#include "general.h"
#include "QL_screen.h"
#include "SDL2screen.h"
#include "qsound.h"

static int is_hw(uint32_t addr)
{
	if ((addr >= QL_INTERNAL_IO_BASE) &&
        	(addr < (QL_INTERNAL_IO_BASE + QL_INTERNAL_IO_SIZE))) {
		return 1;
	}

	return 0;
}

// Check if address belongs to QSound space ($C0000 - $C3FFF)
static inline int is_qsound(uint32_t addr)
{
	uint32_t a = addr & 0xFFFF;
	return ((a & 0xF000) == 0x2000 || (a & 0xF000) == 0x3000) && ((addr & 0xFF0000) == 0x0C0000);
}

rw8 ReadByte(aw32 addr)
{
	addr &= ADDR_MASK;

	// QSound register read hook
	if (is_qsound(addr)) {
		return qsound_read_byte(addr);
	}

	// Internal hardware (ZX8301 / ZX8302 IPC)
	if (is_hw(addr)) {
		return ReadHWByte(addr);
	}

	// Expansion ROM space ($C0000 - $CFFFF)
	if (addr >= 0x000C0000 && addr < 0x00100000) {
		return *((w8 *)memBase + addr);
	}

	// Outside mapped RAM
	if ((addr >= RTOP) && (addr >= qlscreen.qm_hi))
		return 0;

	// Standard RAM and system ROM ($000000)
	return *((w8 *)memBase + addr);
}

rw16 ReadWord(aw32 addr)
{
	addr &= ADDR_MASK;

	// QSound register read hook
	if (is_qsound(addr)) {
		return ((rw16)qsound_read_byte(addr) << 8) | qsound_read_byte(addr + 1);
	}

	// Internal hardware (ZX8301 / ZX8302 IPC)
	if (is_hw(addr)) {
		return ((w16)ReadHWWord(addr));
	}

	// Expansion ROM space ($C0000 - $CFFFF)
	if (addr >= 0x000C0000 && addr < 0x00100000) {
		return (w16)RW((w16 *)((Ptr)memBase + addr));
	}

	// Outside mapped RAM
	if ((addr >= RTOP) && (addr >= qlscreen.qm_hi))
		return 0;

	return (w16)RW((w16 *)((Ptr)memBase + addr)); /* make sure it is signed */
}

rw32 ReadLong(aw32 addr)
{
	addr &= ADDR_MASK;

	if (is_qsound(addr)) {
		return ((rw32)ReadWord(addr) << 16) | (uint16_t)ReadWord(addr + 2);
	}

	if (is_hw(addr)) {
		return ((w32)ReadHWLong(addr));
	}

	if (addr >= 0x000C0000 && addr < 0x00100000) {
		return (w32)RL((Ptr)memBase + addr);
	}

	if ((addr >= RTOP) && (addr >= qlscreen.qm_hi))
		return 0;

	return (w32)RL((Ptr)memBase + addr); /* make sure it is signed */
}

void WriteByte(aw32 addr, aw8 d)
{
	addr &= ADDR_MASK;

	if (is_qsound(addr)) {
		qsound_write_byte(addr, d);
		return;
	}

	if (addr == 0x00018063) {
		WriteHWByte(addr, d);
		*((w8 *)memBase + addr) = d;
		return;
	}

	if ((addr >= RTOP) && (addr >= qlscreen.qm_hi))
		return;

	if (is_hw(addr)) {
		WriteHWByte(addr, d);
	} else if (addr >= QL_SCREEN_BASE) {
		*((w8 *)memBase + addr) = d;
	}
}

void WriteWord(aw32 addr,aw16 d)
{
	addr &= ADDR_MASK;

	if (is_qsound(addr)) {
		qsound_write_byte(addr, (uint8_t)(d >> 8));
		qsound_write_byte(addr + 1, (uint8_t)d);
		return;
	}

	if ((addr >= RTOP) && (addr >= qlscreen.qm_hi))
		return;

	if (is_hw(addr)) {
		WriteHWWord(addr, d);
	} else if (addr >= QL_SCREEN_BASE) {
		WW((Ptr)memBase + addr, d);
	}
}

void WriteLong(aw32 addr,aw32 d)
{
	addr &= ADDR_MASK;

	if (is_qsound(addr)) {
		WriteWord(addr, (uint16_t)(d >> 16));
		WriteWord(addr + 2, (uint16_t)d);
		return;
	}

	if ((addr >= RTOP) && (addr >=qlscreen.qm_hi))
		return;

	if (is_hw(addr)) {
		WriteHWWord(addr, d >> 16);
		WriteHWWord(addr + 2, d);
	} else if (addr >= QL_SCREEN_BASE) {
		WL((Ptr)memBase + addr, d);
	}
}

/*############################################################*/
int isreg=0;

rw8 ModifyAtEA_b(ashort mode,ashort r)
{
	shindex displ;
	w32 addr;

	isreg = 0;

	switch (mode)
	{
	case 0:
		isreg = 1;
		mea_acc = 0;
		lastAddr = 0;
		dest = (Ptr)(&reg[r]) + RBO;
		return *((w8 *)dest);
	case 2:
		addr = aReg[r];
		break;
	case 3:
		addr = aReg[r]++;
		if (r == 7)
			(*m68k_sp)++;
		break;
	case 4:
		if (r == 7)
			(*m68k_sp)--;
		addr = --aReg[r];
		break;
	case 5:
		addr = aReg[r] + (w16)RW(pc++);
		break;
	case 6:
		displ = (w16)RW(pc++);
		if ((displ & 2048) != 0)
			addr = reg[(displ >> 12) & 15] +
			       aReg[r] + (w32)((w8)displ);
		else
			addr = (w32)((w16)(reg[(displ >> 12) & 15])) +
			       aReg[r] + (w32)((w8)displ);
		break;
	case 7:
		switch (r)
		{
		case 0:
			addr = (w16)RW(pc++);
			break;
		case 1:
			addr = RL((w32 *)pc);
			pc += 2;
			break;
		default:
			exception = 4;
			extraFlag = true;
			nInst2 = nInst;
			nInst = 0;

			mea_acc = 0;
			lastAddr = 0;
			dest = (Ptr)(&dummy);
			return 0;
		}
		break;
	default:
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;

		mea_acc = 0;
		lastAddr = 0;
		dest = (Ptr)(&dummy);
		return 0;
	}

	addr &= ADDR_MASK;

	lastAddr = addr;
	dest = (Ptr)memBase + addr;
	return ReadByte(addr);
}

rw16 ModifyAtEA_w(ashort mode,ashort r)
{
	/*w16*/
	shindex displ;
	w32 addr = 0;

	isreg = 0;

	switch (mode)
	{
	case 0:
		isreg = 1;
		dest = (Ptr)(&reg[r]) + RWO;
		return *((w16 *)dest);
	case 1:
		isreg = 1;
		dest = (Ptr)(&aReg[r]) + RWO;
		return *((w16 *)dest);
	case 2:
		addr = aReg[r];
		break;
	case 3:
		addr = aReg[r];
		aReg[r] += 2;
		break;
	case 4:
		addr = (aReg[r] -= 2);
		break;
	case 5:
		addr = aReg[r] + (w16)RW(pc++);
		break;
	case 6:
		displ = (w16)RW(pc++);
		if ((displ & 2048) != 0)
			addr = reg[(displ >> 12) & 15] +
			       aReg[r] + (w32)((w8)displ);
		else
			addr = (w32)((w16)(reg[(displ >> 12) & 15])) +
			       aReg[r] + (w32)((w8)displ);
		break;
	case 7:
		switch (r)
		{
		case 0:
			addr = (w16)RW(pc++);
			break;
		case 1:
			addr = RL((w32 *)pc);
			pc += 2;
			break;
		default:
			exception = 4;
			extraFlag = true;
			nInst2 = nInst;
			nInst = 0;
			mea_acc = 0;
			dest = (Ptr)(&dummy);
			return 0;
		}
		break;
	}
	addr &= ADDR_MASK;

	lastAddr = addr;
	dest = (Ptr)memBase + addr;
	return ReadWord(addr);
}

rw32 ModifyAtEA_l(ashort mode, ashort r)
{
	/*w16*/
	shindex displ;
	w32 addr = 0;

	isreg = 0;

	switch (mode)
	{
	case 0:
		isreg = 1;
		dest = (Ptr)(&reg[r]);
		return *((w32 *)dest);
	case 1:
		isreg = 1;
		dest = (Ptr)(&aReg[r]);
		return *((w32 *)dest);
	case 2:
		addr = aReg[r];
		break;
	case 3:
		addr = aReg[r];
		aReg[r] += 4;
		break;
	case 4:
		addr = (aReg[r] -= 4);
		break;
	case 5:
		addr = aReg[r] + (w16)RW(pc++);
		break;
	case 6:
		displ = (w16)RW(pc++);
		if ((displ & 2048) != 0)
			addr = reg[(displ >> 12) & 15] +
			       aReg[r] + (w32)((w8)displ);
		else
			addr = (w32)((w16)(reg[(displ >> 12) & 15])) +
			       aReg[r] + (w32)((w8)displ);
		break;
	case 7:
		switch (r)
		{
		case 0:
			addr = (w16)RW(pc++);
			break;
		case 1:
			addr = RL((w32 *)pc);
			pc += 2;
			break;
		default:
			exception = 4;
			extraFlag = true;
			nInst2 = nInst;
			nInst = 0;
			mea_acc = 0;

			dest = (Ptr)(&dummy);
			return 0;
		}
		break;
	}

	addr &= ADDR_MASK;

	lastAddr = addr;
	dest = (Ptr)memBase + addr;
	return ReadLong(addr);
}

void RewriteEA_b(aw8 d)
{
	if (isreg)
		*((w8*)dest)=d;
	else {
		WriteByte(lastAddr, d);
	}
}

void RewriteEA_w(aw16 d)
{
	if (isreg) {
		*((w16*)dest)=d;
	} else {
		WriteWord(lastAddr, d);
	}
}

void RewriteEA_l(aw32 d)
{
	if (isreg) {
		*((w32*)dest)=d;
	} else {
		WriteLong(lastAddr, d);
	}
}
