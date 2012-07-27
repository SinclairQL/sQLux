/*
 *		Copyright (c) 1996 jonathan hudson  
 */

/* most of the code has moved to QLserio.c */

#ifdef SERIAL

/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <fcntl.h>
//#include <dirent.h>


#include "QL.h"
//#include "QLfiles.h"
//#include "QFilesPriv.h"
#include "QInstAddr.h"
//#include "QDisk.h"
#include "unix.h"

#include "QSerial.h"
#include "uqlx_cfg.h"

#ifndef NEWSERIAL

#define PRINTF(p1) printf p1

extern int fake_tty_open (char *);
extern void fake_tty_close (void);

typedef struct
{
    char *opts;
    int (*func)(char *, char *, short *, short *);
    short *def;
    int offset;
} seropt_t;

static int fgeneral  (char *name, char *opt, short *def, short *ptr)
{
    char *q;
    short ok = 0;
    
    if((q = strpbrk(name, opt)))
    {
	if((q = strchr(opt, *q)))
	{
	    *ptr = *(def + (q-opt));
	    ok =1;
	}
    }

    if(!ok)
    {
	*ptr = *def;
    }
    return 0;
}

static int fbaud (char *name, char *opt, short *def, short *ptr)
{
    char *p;
    int rate;

    if((p = strchr(name,*opt)))
    {
	p++;
	rate = strtol(p, NULL, 10);
	*(int *)ptr = rate;
    }
    else
    {
	*(int *)ptr =  *def;
    }
    return 0;
}

static int fwrong (char *name, char *opt, short *def, short *ptr)
{
    return (strpbrk(name, opt)) ? -12 : 0;
}

static short pardefs[] = {0,1,2,3,4};
static short shkdefs[] = {-1, 0};
static short prodefs[] = {-1, 0, 1};
#endif
static short defbaud[1] = {9600};
#ifndef NEWSERIAL

static seropt_t sopts[]=
{
    {"OEMS", fgeneral, pardefs, offsetof(serdev_t,parity)},
    {"HI", fgeneral, shkdefs, offsetof(serdev_t,hshake)},
    {"RZC", fgeneral,prodefs, offsetof(serdev_t,xlate)},
    {"B", fbaud, defbaud, offsetof(serdev_t,baud)},
    {"ACDFGJKLNPTUVWXY", fwrong, 0, 0},
    {NULL,NULL}
};




static serdev_t sparams[MAXSERIAL] = {0};


static void  linkin(short unit)
{
    *(reg+1) = 0xE4;
    QLvector(0xc0,200000);
    WriteWord (aReg[0]+0x18, unit+1); /*unit*/
    WriteWord (aReg[0]+0x1a, (sparams+unit)->parity); /*par*/
    WriteWord (aReg[0]+0x1c, (sparams+unit)->hshake); /*txhs*/
    WriteWord (aReg[0]+0x1e, (sparams+unit)->xlate); /*prot*/
}

void SerOpen(void)
{
    short i,n;
    char buf[80];
    int retval = -7;
    char *p;    
    if((long)gPC-(long)theROM-2 != 0xad0l)
    {
	exception=4;
	extraFlag=true;
	nInst2=nInst;
	nInst=0;
	return;
    }

    n = ReadWord(aReg[0]);
    if(n > 3 && n < 80) 
    {
	short unit;
	strncpy(buf, (Ptr)theROM+aReg[0]+2, n);
	*(buf+n) = '\0';
	

	if(strncasecmp (buf, "SER3", 4) == 0)
	{
	    char *pcmd, cmd[80];
	    
	    unit = 2;
	    (sparams+2)->parity = 0; /*par*/
	    (sparams+2)->hshake = -1; /*txhs*/
	    (sparams+2)->xlate  = -1; /*prot*/
	    (sparams+2)->baud  = B0; /*115200;*/ /*faast*/
#ifdef NO_FIONREAD
	    (sparams+2)->bfc_valid=0;
#endif
	    
	    if(n > 4)
	    {
		pcmd = strncpy(cmd, buf+4, n-4);
		*(cmd+n-4) = 0;
	    }
	    else
	    {
		pcmd = NULL;
	    }

	    if(((sparams+2)->fd = fake_tty_open(pcmd)) > 0)
	    {
		linkin(2);
		retval = 0;
	    }
	    else
		retval = -9;
	}
	else   /* serial channels ... */
	{
	    for(p = buf; *p; p++)
	    {
		*p = toupper(*p);
	    }
	    
	    if(memcmp (buf, "SER", 3) == 0)
	    {
		unit = strtol(buf+3, &p, 10);
		if(unit)
		{
		    unit--;
		}
		
		if(unit >= MAXSERIAL)
		{
		    retval = -12;
		}
		else
		{
		    seropt_t *q;
		    retval = 0;
		    
		    for(q = sopts; q->opts && retval == 0; q++)
		    {
			retval = (q->func)(buf+3, q->opts, 
					   q->def, 
					   (short *)((char *)
						     (sparams+unit)+q->offset));
		    }
		}
		
		if(retval == 0)
		{
		    char *portnam;
		    portnam = (unit == 0) ? QMD.ser1 : QMD.ser2;
		    
		    if(tty_open (portnam, (sparams+unit)) > 0)
		    {
#ifdef NO_FIONREAD
		      (sparams+unit)->bfc_valid;
#endif		      
			linkin(unit);
			retval = 0;
		    }
		    else
		    {
			retval = -9;
		    }
		}
	    }		    
	}
    }
    
    *reg = retval;
    rts();
}

void SerClose(void)
{
    short unit;
    
    if((long)gPC-(long)theROM-2 != 0xb8el)
    {
	exception=4;
	extraFlag=true;
	nInst2=nInst;
	nInst=0;
	return;
    }

    if((unit = ReadWord (aReg[0]+0x18)) > 0)
    {
	unit--;
	if(unit == 2)
	{
	    fake_tty_close();
	}
	else
	{
	    tty_close((sparams+unit)->fd);
	}
	(sparams+unit)->fd = -1;
	QLvector(0xc2,200000);
    }	
    
    rts();
}

#define IO_PEND   0
#define IO_FBYTE  1
#define IO_FLINE  2
#define IO_FSTRG  3
#define IO_EDLIN  4
#define IO_SBYTE  5
#define IO_SSTRG  7

#define FS_LOAD   0x48
#define FS_SAVE   0x49

void SerIO(void)
{
    short act;
    short unit;

    if((long)gPC-(long)theROM-2 != 0xbb6l)
    {
	exception=4;
	extraFlag=true;
	nInst2=nInst;
	nInst=0;
	return;
    }

    act = *reg;
    *reg = -6;
    
#ifdef IOTEST
    printf("call SerIO \t\td0=%d\td1=%x\td2=%x\td3=%x\ta1=%x\n",act,reg[1],reg[2],reg[3],aReg[1]);    
#endif

    if((unit = ReadWord (aReg[0]+0x18)) > 0)
    {
	unit--;
	if((sparams+unit)->fd != -1)
	{
	    unsigned char c;
	    long count;
	    short termc = 0;

	    switch(act)
	    {
	      case IO_PEND:
		*reg = pendingio((sparams+unit)->fd);
		break;
	      case IO_FBYTE:
		count = 1;
		*reg = readio(sparams+unit, &c, &count, 0);
		reg[1] = c;
		break;
	      case IO_FLINE:
		termc = 10;
	      case FS_LOAD:
	      case IO_FSTRG:
		count = (uw16)reg[2];
		*reg = readio(sparams+unit, (Ptr)theROM+aReg[1], &count, termc);
		aReg[1] += count;
		reg[1]  = count;
		break;
	      case IO_SBYTE:
		c = reg[1];
		count = 1;
		*reg = writeio(sparams+unit, &c, &count);
		break; 
	      case IO_SSTRG:
	      case FS_SAVE:
		count = (uw16)reg[2];
		*reg = writeio(sparams+unit, (Ptr)theROM+aReg[1], &count);
		aReg[1] += count;
		reg[1]  = count;
		break;
	      default:
		*reg = -15;
		break;
	    }
	}
    }
#ifdef IOTEST
printf("ret from SerIO \t\td0=%d\td1=%x\ta1=%x\n",reg[0],reg[1],aReg[1]);
#endif
  		
    rts();
}

#endif /* NEWSERIAL*/

#ifdef NEWSERIAL
serdev_t *sparams[MAXSERIAL+1];


Cond SetBaudRate(short rate)
{
    short i;
    
    for(i = 1; i < MAXSERIAL+1; i++)
    {
	defbaud[0] = rate;
	if (sparams[i])
	  {
	    (sparams[i])->baud = rate;
	    if((sparams[i])->fd > 0)
	      {
		tty_baud(sparams[i]);
	      }
	  }
    }
    return 1;
}

#else

Boolean SetBaudRate(short rate)
{
    short i;
    
    for(i = 0; i < MAXSERIAL; i++)
    {
	defbaud[0] = rate;
	(sparams+i)->baud = rate;
	if((sparams+i)->fd > 0)
	{
	    tty_baud(sparams+i);
	}
    }
    return 1;
}
#endif


#endif 
