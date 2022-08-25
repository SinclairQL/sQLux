/*
 * (c) UQLX - see COPYRIGHT
 */


#include "QL68000.h"

#include "QL.h"
#include "xcodes.h"
#include "QDOS.h"
#include "QL_sound.h"
#include "QL_config.h"
#include "QInstAddr.h"
#include "SDL2screen.h"
#include "qmtrap.h"
#include "unixstuff.h"
#include "QL_sound.h"

#define ALT 1
#define SHIFT 4
#define CTRL 2


extern uw32 orig_kbenc;

void rts(void);
void debug2(char*,long);

#define CHAR_BUFF_LEN 50

uw16		asciiChar=0;

int alphaLock, optionKey, gKeyDown, controlKey, shiftKey, altKey;

volatile bool soundOn;
int display_mode=4;

void SetDisplay(w8 d, Cond flag)
{
	if ((d == 0) || (d == 8))
	{
		display_mode = d ? 8 : 4;
	}
	set_rtc_emu();
}

static uw8		IPC_len[16]={0,0,0,0,0,0,0,0,0,1,16,0,0,0,0,0};
static uw8		charBuff[CHAR_BUFF_LEN][2];
static uw16		charAscii[CHAR_BUFF_LEN];
static short	charHead=0;
static short	charTail=0;

static Cond	capsLockStatus=false;
static Cond	pendingCh1Receive=false;
static Cond	pendingCh2Receive=false;
static uw8		IPC_com;
static uw8		IPCW_buff[16];
static short	IPCW_n=0;
static short	IPCW_p=0;
uw8		IPCR_buff[22];
uw16		IPCR_ascii[22];
short	IPCR_n=0;
short	IPCR_p=0;

void ZeroKeyboardBuffer(void)
{	charHead=charTail=0;
	gKeyDown=false;
	asciiChar=0;
}

static uw8 KeyRow(short row)
{
    int mod;

    if(row == 7)
        mod = sdl_shiftstate + (sdl_altstate << 2) + (sdl_controlstate << 1);
    else
        mod=0;

    return sdl_keyrow[row] + mod;
}

void pic_set_irq(int irq, int state)
{
  //printf("warning: pic_set_irq(%d,%d) called\n",irq,state);
}

Cond IPC_Command(void)	/* returns false for commands to handle low-level, true otherwise */
{
	switch(ReadByte(aReg[3])&15){
	case 1:		/* IPC status */
	case 8:		/* read keyboard */
		return false;
	case 9:		/* keyboard direct read */
		reg[1]=KeyRow(ReadByte(aReg[3]+6));
		return true; /*break;*/
	case 10: /* initiate sound generation */
		BeepSound((unsigned char*)memBase+(aReg[3]&0xfffffe));
		return true; /*break;*/
	case 11: /* kill sound */
		KillSound();
		return true; /*break;*/
	case 15:	/* IPC test *//**//* neu, .hpr 21.5.99 */
		reg[1]=~ReadByte(aReg[3]+6);
		return true;	/**//*break;*/
	default:	/* ignore RS232 and other commands */
	  reg[0]=-15;	/**//* neu, .hpr 21.5.99 */
	    return true; 	/**//*break;*/
	}
	return true;
}

void UseIPC(void)		/* ROM patch: executing IPC command */
{
	if((Ptr)gPC-(Ptr)memBase-2==IPC_CMD_ADDR)
	{	if(IPC_Command()) rts();
		else qlux_table[code=0x40e7]();
	}
	else
	{	exception=4;
		extraFlag=true;
		nInst2=nInst;
		nInst=0;
	}
}

void ReadIPC(void)		/* ROM patch: reading from IPC */
{
	if((Ptr)gPC-(Ptr)memBase-2==IPCR_CMD_ADDR)
	{
	  /*printf("ReadIPC\n");*/

	  rts();
	  if(IPCR_n>0)
	    {
	      asciiChar=IPCR_ascii[IPCR_p];
	      /*printf("ReadIPC: asciiChar %d\n",asciiChar);*/

	      reg[1]=IPCR_buff[IPCR_p++];
	      IPCR_n--;
	    }
	  else reg[1]=0;
	}
	else
	  {
	    exception=4;
	    extraFlag=true;
	    nInst2=nInst;
	    nInst=0;
	  }
}

static uw8 qCharLen(void)
{	short n;
	n=charTail-charHead;
	if(n<0) n+=CHAR_BUFF_LEN;
	return (uw8)n;
}

void DoIPCCommand(uw8 IPC_com)
{	uw8 b;
/*printf("DoIPCCommand\n");*/

	switch(IPC_com){
	case 1:		/* test status */
		b=0;
		if(charHead!=charTail || gKeyDown) b|=1;
		if(soundOn) b|=2;
		if(pendingCh1Receive) b|=16;
		if(pendingCh2Receive) b|=32;
		*IPCR_buff=b;
		IPCR_n=1;
		break;
	case 8:
		b=qCharLen();
		if(b>7) b=7;
		*IPCR_buff=b;
		if(gKeyDown) (*IPCR_buff)|=8;
		IPCR_n=1;
		while(b--)
		{	IPCR_ascii[IPCR_n]=charAscii[charHead];
			IPCR_buff[IPCR_n++]=charBuff[charHead][0];
			IPCR_ascii[IPCR_n]=charAscii[charHead];
			IPCR_buff[IPCR_n++]=charBuff[charHead++][1];
			/*printf("ascii : %c code: %x ascii: %c code: %x\n",
       IPCR_ascii[IPCR_n-2],IPCR_buff[IPCR_n-1],
       IPCR_ascii[IPCR_n-1],IPCR_buff[IPCR_n]); */

			if(charHead>=CHAR_BUFF_LEN) charHead=0;
		}
		break;
	case 9:
		*IPCR_buff=KeyRow(*IPCW_buff);
		IPCR_n=1;
		break;
	default: IPCR_n=0;
	}
	IPCR_p=0;
}

void WriteIPC(void)		/* ROM patch: writing to IPC */
{
  /*printf("write IPC\n");*/

  if((Ptr)gPC-(Ptr)memBase-2==IPCW_CMD_ADDR)
    {
      rts();
      if(IPCW_n--) IPCW_buff[IPCW_p++]=(uw8)(*reg);
      else
	{
	  IPC_com=(short)(*reg)&15;
	  if(IPC_com!=1 && IPC_com!=8) debug2("Bad IPC Command : ",IPC_com);
	  IPCW_n=IPC_len[IPC_com];
	  IPCW_p=0;
	}
      if(!IPCW_n) DoIPCCommand(IPC_com);
    }
  else
    {
      exception=4;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
}

void queueKey(short m,short code,uw16 asciiChar)
{
  charBuff[charTail][1]=code;
  code=0;
  if(m&SHIFT) code|=4;
  if(m&CTRL) code|=2;
  if(m&ALT) code|=1;

    charAscii[charTail]=asciiChar;
    /*printf("queued char: %d\n",charAscii[charTail]);*/
    charBuff[charTail++][0]=code;

    if(charTail>=CHAR_BUFF_LEN) charTail=0;
}

void CheckCapsLock(short m)
{	if(((m&alphaLock)!=0)!=capsLockStatus)
	{	capsLockStatus=!capsLockStatus;
		queueKey(m,33,0);
		if((m&(shiftKey|controlKey|optionKey))!=0) queueKey(0,33,0);	/* per coerenza con il tasto del Mac */
	}
}

void QL_KeyTrans(void)
{	if((Ptr)gPC-(Ptr)memBase-2!=KEYTRANS_CMD_ADDR)
	{	exception=4;
		extraFlag=true;
		nInst2=nInst;
		nInst=0;
		return;
	}
	if(asciiChar)
	{	reg[1]=asciiChar;
		asciiChar=0;
		WriteLong(aReg[7],ReadLong(aReg[7])+4);
		rts();
	}
	else qlux_table[code=KEYTRANS_OCODE]();
}

/* Minerva Keyboard handling */
void KbdCmd(void)   /* do IPC command */
{
  /*printf("calling SX_IPCOM: d0=%d\n",reg[0]);*/

  if (reg[0]==9)
    {
      *((char*)reg+4+RBO)=KeyRow(ReadByte(aReg[3]+3));
      WriteLong(aReg[7],ReadLong(aReg[7])+2);
    }

  /*printf("returning d1=%x\n",reg[1]);*/
  rts();
}

int MButtonDown=0;

void MReadKbd()
{
  int ccode,mod;

  while(qCharLen())
    {
      asciiChar=charAscii[charHead];
      ccode=charBuff[charHead][1];
      mod=charBuff[charHead++][0];

      if (charHead>=CHAR_BUFF_LEN) charHead=0;
      /*printf("char %c %d\t ccode %d\n",asciiChar,asciiChar,ccode);*/

      reg[1]=ccode;
      reg[2]=mod;


      aReg[2]=ReadLong(0x2804c);
      /* printf("calling subr %d\n",((w16)(uw16)ReadWord(0x150))+0x4000);*/
      QLsubr(ReadWord(0x150)+0x4000,2000000);
      /*printf("result: %d\n",reg[0]);*/
    }

  if (gKeyDown)
    {
      aReg[2]=ReadLong(0x2804c);
      reg[5]=-1;/*(gKeyDown!=0)<<4;*/

      QLsubr(ReadWord(0x152)+0x4000,2000000);
    }
}

#if 0
void MEndRep()
{
      aReg[2]=ReadLong(0x2804c);
      reg[5]=0;/*(gKeyDown!=0)<<4;*/

      QLsubr(ReadWord(0x152)+0x4000,2000000);
}
#endif

/* Minerva Keyboard encode routine */
void KBencCmd()
{
  /*printf("KBenc called, char %c\n",asciiChar);*/
  if (asciiChar)
    {
      reg[1]=asciiChar;

      WriteLong(aReg[7],ReadLong(aReg[7])+2);
      rts();
    }
  else   /* fall through into original routine */
    {
      pc = (Ptr)memBase+orig_kbenc;
    }
}

