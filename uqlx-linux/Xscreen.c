/*
 * (c) UQLX - see COPYRIGHT
 */



/* fancy code to implement bigger screen */



#ifdef XSCREEN
#include "QL68000.h"
#include "QL.h"

#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <sys/mman.h>

#include "xcodes.h"

#include "driver.h"
#include "QDOS.h"

#include "QInstAddr.h"
#include "unix.h"
#include "uqlx_cfg.h"
#include "script.h"
#include "qx_proto.h"

#define min(_a_,_b_) (_a_<_b_ ? _a_ : _b_)
#define max(_a_,_b_) (_a_>_b_ ? _a_ : _b_)


struct SCREENDEF
{
  uw32 scrbase;
  uw32 scrlen;
  uw16 linel;
  uw16 xres;
  uw16 yres;
};


Cond XLookFor(w32 *a,uw32 w,long nMax)
{	
  while(nMax-->0 && RL((*a))!=w) (*a)+=2;
  return nMax>0;
}

static int PtrPatchOk=0;
void XPatchPTRENV()
{
  struct SCREENDEF *scrdef;
  int flag;
  
  scrdef=(Ptr)pc-8000;
  while ( XLookFor((w32*)&scrdef,0x20000,24000))
    {
      if (RL(&(scrdef->scrlen))==0x8000  &&
	  RW(&(scrdef->linel)) ==0x80 &&
	  RW(&(scrdef->xres)) ==0x200 &&
	  RW(&(scrdef->yres)) ==0x100 )
	{
	  PtrPatchOk=1;
	  WL(&(scrdef->scrbase),qlscreen.qm_lo);
	  WL(&(scrdef->scrlen),qlscreen.qm_len);
	  WW(&(scrdef->linel),qlscreen.linel);
	  WW(&(scrdef->xres),qlscreen.xres);
	  WW(&(scrdef->yres),qlscreen.yres);
	  
	  return;
	}
      else
	scrdef=(struct SCREENDEF *)((char*) scrdef+2);
    }
  
  if (!PtrPatchOk)
    printf("WARNING: could not patch Pointer Environment\n");
}


/* oadr is QL format, nadr c-pointer ! */
void scan_patch_chans(w32 oadr)
{
  w32 ca;
  w32 ce,cc;
  
  ce=ReadLong(0x2807c); /* table top */
  ca=ReadLong(0x28078);

  for(;ca<=ce;ca+=4)
    {
      cc=ReadLong(ca);
      if (ReadLong(cc+4) == oadr)
	{
	  WriteWord(cc+0x64,qlscreen.linel);
	  WriteLong(cc+0x32,qlscreen.qm_lo);
	}
    }
}

#if 0
void sdriver_unlink(w32 da)
{
  w32 drp,lp;
  
  lp=0x28044;
  for(drp=ReadLong(lp);drp;lp=drp,drp=ReadLong(drp))
    if (drp==da) break;
  
  if (drp)
    WriteLong(lp,ReadLong(drp));
}
#endif


/* fool ptr_gen */
static int xic=0;
static uw32 cdrv=0;
static uw32 fpdr;
static uw32 orig_open,orig_io,orig_close,orig_cdrv;


#if 0
void XfixCh()
{
  uw32 addr;
  
#if 0
  if (reg[0]!=0)
    return;
  
  addr=ReadLong(0x28078);
  addr=ReadLong(addr+4*((uw16)aReg[0]));
  WriteLong(addr+4,fpdr);

  sdriver_unlink(fpdr);
#endif
}
#endif

void devpefio_cmd()
{ 
  int xw,yw,xo,yo;
  int xm,ym;
  uw16 *pbl;
  uw32 saved_regs[16];
  int op;
  
  
  if((long)((Ptr)gPC-(Ptr)theROM)-2 == DEVPEF_IO_ADDR);
  {

/* special action in script mode */
    if (script)
      {
	save_regs(saved_regs);
	op=reg[0];

	/* TK2 screen locking... */
	if (reg[3]<0 && (op==7 || op==5 || op==0x49))
	  goto scr_io;

	/*printf("call io_handle \t\td0=%d\td1=%x\td2=%x\td3=%x\ta1=%x\n",op,reg[1],reg[2],reg[3],aReg[1]);*/
	if (op==4) 
	  {op=2;reg[0]=2;}

	io_handle(script_read,script_write,script_pend,NULL);
	/*printf("ret from io_handle \td0=%d\td1=%x\t\ta1=%x\n",reg[0],reg[1],aReg[1]);*/
	if (op==0 || op==1 || op==2 || op==3 || op==0x48)
	  {
	    rts();
	    return;
	  }
	restore_regs(saved_regs);
      }
    /*printf("call io_handle \t\td0=%d\td1=%x\td2=%x\td3=%x\ta1=%x\n",op,reg[1],reg[2],reg[3],aReg[1]);*/

  scr_io:

    if ((uw8)reg[0]==0xd)
      {
	pbl=(Ptr)theROM+aReg[1];
	xw=RW(pbl++);
	yw=RW(pbl++);
	xo=RW(pbl++);
	yo=RW(pbl);
	
	xm=xw+xo;
	ym=yw+yo;
	
	if ( xm>512 || ym>256 && xm<=qlscreen.xres && ym<=qlscreen.yres )
	  {
	    WriteWord(aReg[0]+0x18,xo/*scr_par[2].i*/);
	    WriteWord(aReg[0]+0x1a,yo/*scr_par[3].i*/);
	    WriteWord(aReg[0]+0x1c,xw/*scr_par[0].i*/);
	    WriteWord(aReg[0]+0x1e,yw/*scr_par[1].i*/);
	    /*printf("sd.wdef %dx%da%dx%d\n",xw,yw,xo,yo);*/

	    reg[0]=0;
	    rts();
	    return;
	  }
      }
    


    code=DEVPEFIO_OCODE;
    table[code]();
    
  }
}
open_arg scr_par[6];
extern struct NAME_PARS con_name,scr_name;

#define GXS  ((Ptr)theROM+UQLX_STR_SCRATCH)
void mangle_args(char *dev)
{
  int xw,yw;
  
#if 0
  printf("opening  %s\t%d x %d a %d x %d _%d\n",dev,scr_par[0].i,scr_par[1].i,scr_par[2].i,scr_par[3].i,scr_par[4].i);
#endif

  xw=scr_par[0].i+scr_par[2].i;
  yw=scr_par[1].i+scr_par[3].i;
  
  if (xw<=512 && yw<=256) return; /* no action needed */
  if (xw>qlscreen.xres || yw>qlscreen.yres)
    return;                       /* same, for other reasons */

  if (scr_par[4].i>=0)
    sprintf(GXS+2,"%s__%d",dev,scr_par[4].i);
  else
    sprintf(GXS+2,"%s",dev);
  
  WW(GXS,strlen(GXS+2));
  aReg[0]=UQLX_STR_SCRATCH;
}


void devpefo_cmd()
{
  int res;
  uw32 sA0;
#if defined(USE_VM) || defined(VM_SCR)
  Ptr base_addr;
#endif
  
  sA0=aReg[0];
  
#if 1
  res=decode_name((Ptr)theROM+((aReg[0])&ADDR_MASK_E),&scr_name,&scr_par);
  if (res==1)
    mangle_args("SCR_");
  else
    {
      res=decode_name((Ptr)theROM+((aReg[0])&ADDR_MASK_E),&con_name,&scr_par);
      if (res==1)
	mangle_args("CON_");
    }
#endif

  /* we must patch ROM, but that is mprotect'ed !*/
#if defined(USE_VM) || defined(VM_SCR)
  base_addr= (Ptr)theROM+((orig_open>>pageshift)<<pageshift);
  MPROTECT(base_addr,pagesize,PROT_READ|PROT_WRITE);
#endif
  WW((Ptr)theROM+orig_open,DEVPEFO_OCODE);
  QLsubr(orig_open/*XS_GETOPEN((Ptr)theROM+(aReg[3]+0x18))*/,200000000);
  WW((Ptr)theROM+orig_open,DEVPEFO_CMD_CODE);
#if defined(USE_VM) || defined(VM_SCR)
  MPROTECT(base_addr,pagesize,PROT_READ);
#endif

  if ((w16)reg[0]<0)
    {
      aReg[0]=sA0;

      rts();
      return;
    }

  /* set real bounds */
  WriteWord(aReg[0]+0x18,scr_par[2].i);
  WriteWord(aReg[0]+0x1a,scr_par[3].i);
  WriteWord(aReg[0]+0x1c,scr_par[0].i);
  WriteWord(aReg[0]+0x1e,scr_par[1].i);
  /* set physical screen: */

  WriteWord(aReg[0]+0x64,qlscreen.linel);
  WriteLong(aReg[0]+0x32,qlscreen.qm_lo);
  rts();
}




#if 0
int init_con(char * ddef)
{
  uw32 ca;

  if (dr1)
    dr2=ddef-(long)theROM;
  else dr1=ddef-(long)theROM;
  
  
  if (!xic)
    {
      /* get ch#0 */
      ca=ReadLong(0x28078);
      ca=ReadLong(ca);
      /* driver addr */
      ca=ReadLong(ca+4);
      cdrv=ca;
    }
  
  orig_io=ReadLong(cdrv+4);
  orig_open=ReadLong(cdrv+8);
  orig_close=ReadLong(cdrv+12);
  orig_cdrv=cdrv;
  
  printf("original console driver: %x\n",cdrv);
  
  xic++;
  
  return 0;
}
#endif

/* problems that could not be handled by device init */
int init_xscreen()
{
  uw32 ca;
  int j;
  
  /* get ch#0 */
  ca=ReadLong(0x28078);
  ca=ReadLong(ca);
  /* driver addr */
  ca=ReadLong(ca+4);
  cdrv=ca;

  orig_io=ReadLong(cdrv+4);
  orig_open=ReadLong(cdrv+8);
  orig_close=ReadLong(cdrv+12);
  orig_cdrv=cdrv;

  DEVPEFIO_OCODE=ReadWord(orig_io);
  WW((Ptr)theROM+orig_io,DEVPEF_CMD_CODE);
  
  table[DEVPEF_CMD_CODE]=devpefio_cmd;

  scan_patch_chans(orig_cdrv);

  DEVPEFO_OCODE=ReadWord(orig_open);
  WW((Ptr)theROM+orig_open,DEVPEFO_CMD_CODE);
  table[DEVPEFO_CMD_CODE]=devpefo_cmd;
  
  /*if (qlscreen.qm_lo>131072)*/
  uqlx_protect(qlscreen.qm_lo,qlscreen.qm_len,QX_SCR);

#if 0 /*defined(VM_SCR) || defined(USE_VM)*/
  vm_on();  
#endif
}


#if 0
int scr_init(int id,void *def)
{
  char *p=def;
  int res;
  w32 savedRegs[14*4];
  
  /* useless and dangerous without Minerva */
  if (!isMinerva)
    return -1;
  

  BlockMoveData(reg,savedRegs,14*sizeof(w32));

  /* find QDOS con/scr driver, store addr and unlink it */
  /* set new handler with wdef for PE, bit in $34(a6) */

  printf("call scr_init, id %id, driver defblock %d\n",id,def);

  res=init_con(def);

  if (res<0)
    goto sci_exit;
  
  fpdr=(uw32)((Ptr)def-(Ptr)theROM);
  /* set up screen info block for Pointer Environment */
  /* set new handler with wdef for PE, bit in $34(a6) */

#if 0
  p=p+0x5c-0x18;
  printf("screen definition block at %x\n",(Ptr)p-(Ptr)theROM);
  if(qlscreen.qm_len>0x8000)
    WL(p,RTOP);
  else
    WL(p,qlscreen.qm_lo);

  WL(p+4,qlscreen.qm_len);
  WW(p+8,qlscreen.linel);
  WW(p+10,qlscreen.xres);
  WW(p+12,qlscreen.yres);
  WriteByte(0x28034,1&ReadByte(0x28034));
#endif

 sci_exit:
  BlockMoveData(savedRegs,reg,14*sizeof(w32));
  return res;
}
#endif





#if 0
int scr_test(int id, char *name)
{
  int res;

  /*printf("called scr_test %s\n",name+2);*/
  
  res=decode_name(name,Drivers[id].namep,&scr_par);
  if (res<=0) return res;
  
  if (scr_par[4].i!=-1)  /* has it trailing '_buf' ?*/
    return -1;

  return res;
}

int con_test(int id, char *name)
{
  /*printf("called con_test %s\n",name+2);*/

  return decode_name(name,Drivers[id].namep,&scr_par);
}


int scr_open(int idx, void **priv)
{
  /* call orig driver with params fitting 512x256 */
  /* set real size */
  /* set scrbase,sd_linel etc*/
  printf("opening  %d x %d a %d x %d _%d\n",scr_par[0].i,scr_par[1].i,scr_par[2].i,scr_par[3].i,scr_par[4].i);
#if 0
  (*sp)-=4;
  WriteLong(*sp,XS_GETOPEN((Ptr)theROM+(aReg[3]+0x18)));
#else
  /* set new name */
  QLsubr(orig_open/*XS_GETOPEN((Ptr)theROM+(aReg[3]+0x18))*/,200000000);
  if (*reg)
    goto sco_err;
  /* set real bounds */
  WriteWord(aReg[0]+0x18,scr_par[2].i);
  WriteWord(aReg[0]+0x1a,scr_par[3].i);
  WriteWord(aReg[0]+0x1c,scr_par[0].i);
  WriteWord(aReg[0]+0x1e,scr_par[1].i);
  /* set physical screen: */
  WriteWord(aReg[0]+0x64,qlscreen.linel);
  WriteLong(aReg[0]+0x32,qlscreen.qm_lo);
 sco_err:
#endif

  return 1;  /* special device */
}

void scr_close(int idx, void *priv)
{
  /* pass control to original handler */
    /* fool DRvIO - calling un-rts */
#if 1
  (*sp)-=4;
  aReg[3]=orig_cdrv;
  WriteLong(*sp,orig_close);
#else
  QLsubr(orig_close,20000000);
#endif
}

void scr_io(int idx, void *priv)
{
  /* functionality moved to devpefio_cmd*/
  printf("calling scr io ??"); 
#if 1
  /* redefine WINDOW, else */
  /* pass control to original handler */
    /* fool DRvIO - calling un-rts */
  (*sp)-=4;
  aReg[3]=orig_cdrv;
  WriteLong(aReg[0]+4,orig_cdrv);
  WriteLong(*sp,orig_io);
#endif
}
#endif /* 0 */

/***************************************************************/
/* various related stuff */

static void pps_usg(char *m)
{
  printf("Bad geometry: %s. Please use 'nXm' where n=x size, m=y size\n");
  
  qlscreen.xres=512;
  qlscreen.yres=256;
}

void parse_screen(char * geometry)
{
  char *p,*pp;
  long i;
  
  qlscreen.xres=512;
  qlscreen.yres=256;

  i=strtol(geometry,&p,10);
  if (p==geometry)
    {
      pps_usg(geometry);
      return;
    }
  qlscreen.xres=max(i,512);
  if (! (*p=='x' || *p=='X'))
    {
      pps_usg(geometry);
      return;
    }
  else p++;
  i=strtol(p,&pp,10);
  if (p==pp)
    {
      pps_usg(geometry);
      return;
    }
  qlscreen.yres=max(i,256);
}

#endif /* XSCREEN */
