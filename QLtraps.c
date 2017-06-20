/*
 * (c) UQLX - see COPYRIGHT
 */


#include "QL68000.h"
#include <string.h>
#include "QL.h"
#include "QDOS.h"

#include "QSerial.h"
#include "QInstAddr.h"
#include "boot.h"
#include "QDOS.h"
#include "qx_proto.h"
#include "QL_screen.h"

/*extern int schedCount;*/
extern int HasPTR;


/* external debugging aid */
/* replaced by gdb's "qldbg" command */
#if 0
void qldbg(void)
{
  exception=32+14;
  extraFlag=true;
  nInst2=nInst;
  nInst=0; 
}
#endif

#if 0
void trap0(void)	/* go supervisor */
{	if(!supervisor)
	{	supervisor=true;
		gUSP=(*sp);
		(*sp)=gSSP;
	}
}
#else
void trap0(void)
{
  exception=32+(code&15);
  extraFlag=true;
  nInst2=nInst;
  nInst=0;
}
#endif

void trap1(void)
{	short op;

	op=(short)(*reg)&0x7f;

	/*	if ((op>3 && op<6) || (op>7 && op<0xc))*/
	  DECR_SC(); /*schedCount--;*/
		
	if(op>=0x11 && op<=0x15)	/* Gestione dell'hardware */
	{	switch(op){
#if 0
		case 0x10:			/* Set or read display mode */
			SetDisplayMode();
			break; 
#endif
		case 0x11:	/* IPC command */
			*reg=0;	/**//* neu, .hpr 21.5.99 */
			if(!IPC_Command()) goto doTrap;
		/*	*reg=0; *//**//* raus, .hpr 21.5.99 */
			break;
		case 0x12:			/* set baud rate */
			*reg=SetBaudRate((short)reg[1])? 0:-15;
			break;  
		case 0x15:			/* adjust clock */
			if(!reg[1]) break;
			qlClock=(uw32)reg[1];
			/*printf("aclk: qlClock set to %ld\n",qlClock);*/
		case 0x13:			/* read clock */
			GetDateTime((uw32*)(reg+1));
			/*reg[1]=(w32)((uw32)reg[1]-qlClock);*/
			*reg=0;
			prep_rtc_emu();
			break;
		case 0x14:			/* set clock */
		  {
		        w32 i;
		    
			GetDateTime(&i);
			qlClock-=i-(uw32)reg[1];
			/*printf("sclk: qlClock set to %ld\n",qlClock);*/
			*reg=0;
			break;
		  }
		}
	}
	else
	{
#if 0
	  if (reg[0]==-26)
	    {
/* =============================== */
/* TRAP #1         D0.L=-26 (-$1A) */
/* =============================== */
	      reg[0]=0;
	      
/* IN: */
/* D1.L    opcode */
 
/* OUT: */
/* D0.L    QDOS error code (QDOS systems return -15, as the trap is not defined) */
 
/* D4-D7/A3-A7     preserved */
 
/* All other registers: depends on the opcode in D1.L. */
 
 
/* Currently there are three opcodes defined: */
	      switch(reg[1])
		{
 
/* D1.L=0  Get extended trap info */
/* ============================== */
/* IN: no input */
/* OUT: */
/* D0.L=0 (-15 in systems without the trap#1 D0=-26) */
/* D1.L=version of this trap (currently version 0) */
/* D2.L=0 (reserved) */
/* All other registers are preserved */
		case 0:
		  reg[1]=0;
		  reg[2]=0;
		  break;
		  
/* This trap is provided for future compatibility. QL software can test for an */
/* emulator using the following opcode (D1.L=1). Version 0 (D1=0) means that */
/* the trap accepts three opcodes in D1.L: 0, 1 and 2. */
 
 
/* D1.L=1  Get emulator info */
/* ========================= */
/* IN: no input */
/* OUT: */
/* D0.L=0 (-15 in systems without the trap#1 D0=-26) */
/* D1.L=EMULATOR */
/* D2.L=VERSION */
/* D3.L=VERSION TYPE */
/* All other registers are preserved */
 
		case 1:
/* EMULATOR long word format: */
 
/* $aabbccdd, where... */
/* dd=emulator type (0=QDOS system (no one at the moment with this trap defined) */
/*                   1=Q-emuLator */
/*                   2=UQLX */
/*                   3=QLAY) */
/* cc=reserved (0) */
/* bb=host platform (0=Win95, 1=DOS, 2=UNIX, 3=Mac) */
/* aa=sub-platform code (for example the UNIX dialect, to be defined if needed) */
 
/* Example: D1.L=$00030001 means 'Q-emuLator running on Mac' */
 
		  reg[1]=0x03020002;
 
/* VERSION long word format: */
 
/* $aabbccdd, where... */
/* aa=version major number */
/* bb=version minor number */
/* cc=version minor sub-number */
/* dd=release */
 
/* The release field is a counter to differentiate releases of the same */
/* version. The release counter is NOT reset when going from alpha to beta or */
/* from beta to a final version. In this way greater version long words always */
/* mean more recent versions. The release counter is reset to 1 for each new */
/* version. */
 
/* Example: D2.L=$01000308 means 'version 1.0.3, eighth release' */
 
		  reg[2]=0x0;
 
/* VERSION TYPE is 0 for ALPHA, 1 for BETA and 2 for FINAL */
 
		  reg[3]=0;
		  break;
		  
 
/* D1.L=2  Get emulator name */
/* ========================= */
/* In: */
/* A1=pointer to buffer */
/* D2.L= buffer len */
/* Out: */
/* the buffer pointed by A1 is filled with a short QL string describing the */
/* emulator type and version (for example 'Q-emuLator 1.0.3'). */
 
/* If the buffer is too small, the string is truncated to the buffer's length */
/* and the function returns the error BUFFER FULL. */

		case 2:
		  strncpy(reg[1]+(Ptr)theROM,release,reg[2]);
		  if (strlen(release)>reg[2])
		      reg[0]=QERR_BF;
		  break;
		default:
		  reg[0]=QERR_BP;
		}
	      return;
	    }
	  else
#endif
	    {
	    doTrap:	exception=33;
	    extraFlag=true;
	    nInst2=nInst;
	    nInst=0;
	    }
	}
}

void trap2(void)
{
  DECR_SC();/*schedCount--;*/

//  if (reg[0] == 1 )
//    printf("trap2: %s\n",aReg[0]+(char*)theROM+2);
  exception=34;
  extraFlag=true;
  nInst2=nInst;
  nInst=0;
}



void trap3(void)
{
    DECR_SC();/*schedCount--;*/

    if (!HasPTR && *reg==0x70)
        QLPatchPTRENV();

    exception=35;
    extraFlag=true;
    nInst2=nInst;
    nInst=0;
}

extern int script_read_enable;

#ifdef AUTO_BOOT
void btrap3(void)
{
  DECR_SC();
  
  if (((w8)reg[0])==1)
    {
      /*printf("btrap3: a0=%x\n",aReg[0]);*/
      *((char*)reg+4+RBO)= BOOT_SELECT;
      reg[0]=0;
      qlux_table[code]=trap3;
      script_read_enable=1;
    }
  else trap3();
}
#endif
#if 0
void trap4(void)
{	w32 a;

	a=ReadLong(ReadLong(0x28064))+0x16;
	WriteByte(a,ReadByte(a)|0x80);
}
#endif

#if 1
void FastStartup(void)
{	
  if((Ptr)gPC-(Ptr)theROM-2!=RL(&theROM[1]))
    {
      exception=4;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      return;
    }

  memset((Ptr)theROM+131072l,0,RTOP-131072l);

  while(RL((w32*)gPC)!=0x28000l) gPC++;
  gPC-=4;
  aReg[5]=RTOP;
}

#endif
