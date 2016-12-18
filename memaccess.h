/*
 * (c) UQLX - see COPYRIGHT
 */


/* define memory access fns */



#define ReadDispByte(_addr_) (*(uw8 *)((Ptr)theROM+((long)_addr_)))
#define ReadDispWord(__addr__) RW((uw16*)((Ptr)theROM+((long)__addr__)))
#define ReadDispLong(_adr_) (RL((Ptr)theROM+((long)_adr_)))

STATIC rw8 REGP1 ReadByte(aw32 addr)
{   
  register bctype c;

  addr&=ADDR_MASK;

  c=RamMap[addr>>RM_SHIFT]&25 &(~(1<<2));
  if(c!=1)
      return ReadHWByte(addr);   

  return *((w8*)theROM+addr);
}

STATIC rw16 REGP1 ReadWord(aw32 addr)
{   
  register bctype c;

#if HOST_ALIGN>1
  if(((char)addr&1)!=0)
    {   
      exception=3;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      badAddress=addr;
      readOrWrite=16;
      return 0;
    }
  addr&=ADDR_MASK_E;
#else
  addr &=ADDR_MASK;
#endif

  c=RamMap[addr>>RM_SHIFT]&25 &(~(1<<2));
  if(c!=1)
        return ((w16)ReadHWWord(addr));
    
  return (w16)RW((w16*)((Ptr)theROM+addr)); /* make sure it is signed */
}

STATIC rw32 REGP1 ReadLong(aw32 addr)
{   
  register bctype c;

#if HOST_ALIGN>1
  if(((char)addr&1)!=0)
    { 
      exception=3;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      badAddress=addr;
      readOrWrite=16;
      return 0;
    }
  addr&=ADDR_MASK_E;
#else
  addr &= ADDR_MASK;
#endif

  c=RamMap[addr>>RM_SHIFT]&25 &(~(1<<2));
  if(c!=1)
      return ((w32)ReadWord(addr)<<16)|(uw16)ReadWord(addr+2);

  return  (w32)RL((Ptr)theROM+addr); /* make sure is is signed */
}

#define ValidateDispByte(_xx) 
#ifdef DEBUG
long watchaddr=0;
#endif

/* move HW access to a separate fn, this makes it practicable to inline */
/* this fn in a few places :) */
STATIC void REGP2 WriteByte(aw32 addr,aw8 d)
{
  register bctype c;

  addr&=ADDR_MASK;

#ifdef DEBUG
  if (addr==watchaddr)dbginfo("write byte %d to watchaddr=%x",d,addr);
#endif
  c=RamMap[addr>>RM_SHIFT]&30;
  if(c!=2)
    { 
#ifndef VM_SCR
      if(c==6)
	{ 
	  vmMarkScreen(addr);
	  *((w8*)theROM+addr)=d;
	}
      else 
	if(c==8)
#endif  /* VM_SCR */
	  WriteHWByte(addr,d);
#ifdef DEBUG
      else dbginfo("WARNING: writing to ROM,addr=%x val=%d\n",addr,d);
#endif
    }
  else *((w8*)theROM+addr)=d;
}

STATIC void REGP2 WriteWord(aw32 addr,aw16 d)
{   
  register bctype c;

#if HOST_ALIGN>1
  if(((char)addr&1)!=0)
    { 
      exception=3;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      badAddress=addr;
      readOrWrite=0;
      return;
    }
  addr&=ADDR_MASK_E;
#else
  addr &= ADDR_MASK;
#endif

#ifdef DEBUG
  if (addr==watchaddr)dbginfo("write word %d to watchaddr=%x",d,addr);
#endif
  c=RamMap[addr>>RM_SHIFT]&14;
  if(c!=2)
    { 
#ifndef VM_SCR
      if(c==6)
	{       
#if defined(MARK_SCREEN)
	  vmMarkScreen(addr);
#endif
	  WW((Ptr)theROM+addr,d);
	}
      else if(c==8)
#endif /* !VM_SCR */
	{ 
	  //WriteByte(addr,d>>8); WriteByte(addr+1,d&255);
	  WriteHWWord(addr,d);
	}
#ifdef DEBUG
      else dbginfo("WARNING: writing to ROM, addr=%x va=%d\n",addr,d);
#endif
    }
  else WW((Ptr)theROM+addr,d);/* *((w16*)((Ptr)theROM+addr))=d; */
}

STATIC void REGP2 WriteLong(aw32 addr,aw32 d)
{
  register bctype c;

#if HOST_ALIGN>1
  if(((char)addr&1)!=0)
    { 
      exception=3;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      badAddress=addr;
      readOrWrite=0;
      return;
    }
  addr&=ADDR_MASK_E;
#else
  addr &= ADDR_MASK;
#endif

#ifdef DEBUG
  if (addr==watchaddr)dbginfo("write long %d to watchaddr=%x",d,addr);
#endif
  c=RamMap[addr>>RM_SHIFT]&14;
  if(c!=2)
    { 
#ifndef VM_SCR
      if(c==6)
	{ 
#if defined(MARK_SCREEN)
	  vmMarkScreen(addr);
#endif
	  WL((Ptr)theROM+addr,d);
	}
      else if(c==8)
#endif /* !VM_SCR */
	{ 
	  WriteWord(addr,d>>16);
	  WriteWord(addr+2,(uw16)d);
	}
#ifdef DEBUG
      else {dbginfo("WARNING: writing to ROM, addr=%x, val=%d\n",addr,d);}
#endif
    }
  else  WL((Ptr)theROM+addr,d);           
}

/*############################################################*/
#ifndef QM_BIG_ENDIAN
STATIC int isreg=0;
#endif

STATIC rw8 REGP2 ModifyAtEA_b(ashort mode,ashort r)
{   
  shindex displ;
  w32     addr;
  switch(mode) {
  case 0:
#if 1
    mea_acc=0;
#else
#if !defined(VM_SCR) 
    isDisplay=false;
#endif
    isHW=false;
#endif
    dest=(Ptr)(&reg[r])+RBO;
    return *((w8*)dest);
  case 2:addr=aReg[r];
    break;
  case 3:addr=aReg[r]++;
    if(r==7) (*sp)++;
    break;
  case 4:
    if(r==7) (*sp)--;
    addr=--aReg[r];
    break;
  case 5:addr=aReg[r]+(w16)RW(pc++);
    break;
  case 6:
    displ=(w16)RW(pc++);
    if((displ&2048)!=0) addr=reg[(displ>>12) & 15]+
			  aReg[r]+(w32)((w8)displ);
    else addr=(w32) ((w16)(reg[(displ>>12) & 15]))+
	   aReg[r]+(w32)((w8)displ);
    break;
  case 7:
    switch(r) {
    case 0:addr=(w16)RW(pc++);
      break;
    case 1:addr=RL((w32*)pc);
      pc+=2;
      break;
    default:
      exception=4;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;

      mea_acc=0;

      dest=(Ptr)(&dummy);
      return 0;
    }
    break;
  default:
    exception=4;
    extraFlag=true;
    nInst2=nInst;
    nInst=0;

    mea_acc=0;

    dest=(Ptr)(&dummy);
    return 0;
  }
#if 1
  addr&=ADDR_MASK;
#endif
  switch(RamMap[addr>>RM_SHIFT]) {
  case 1:

    mea_acc=0;

    dest=(Ptr)(&dummy);
    return *((w8*)theROM+addr);
  case 3:

    mea_acc=0;

    dest=(Ptr)theROM+addr;
    return *((w8*)dest);
  case 7:

    mea_acc=0;

    dest=(Ptr)theROM+addr;
    return *((w8*)dest);
  case 23:

    mea_acc=MEA_DISP;

    dest=(Ptr)theROM+addr;
    return *((w8*)dest);
  case 8:

    mea_acc=MEA_HW;

    lastAddr=addr;
    dest=(Ptr)(&dummy);
    return ReadByte(addr);
  }

  mea_acc=0;

  dest=(Ptr)(&dummy);
  return 0;
}

STATIC rw16 REGP2 ModifyAtEA_w(ashort mode,ashort r)
{  
  /*w16*/ shindex displ;
  w32     addr;

#ifndef QM_BIG_ENDIAN
  isreg=0;
#endif

  switch(mode) {
  case 0:
// mea_acc is not set *if* little endian?????
#ifdef QM_BIG_ENDIAN
#if 1
    mea_acc=0;
#else
#if !defined(VM_SCR) 
    isDisplay=false;
#endif
    isHW=false;
#endif
#else
    isreg=1;
#endif
    dest=(Ptr)(&reg[r])+RWO;
    return *((w16*)dest);
  case 1:
#ifdef QM_BIG_ENDIAN
#if 1
    mea_acc=0;
#else
#if !defined(VM_SCR) 
    isDisplay=false;
#endif
    isHW=false;
#endif
#else
    isreg=1;
#endif
    dest=(Ptr)(&aReg[r])+RWO;
    return *((w16*)dest);
  case 2:addr=aReg[r];
    break;
  case 3:addr=aReg[r];
    aReg[r]+=2;
    break;
  case 4:addr=(aReg[r]-=2);
    break;
  case 5:addr=aReg[r]+(w16)RW(pc++);
    break;
  case 6:
    displ=(w16)RW(pc++);
    if((displ&2048)!=0) addr=reg[(displ>>12) & 15]+
			  aReg[r]+(w32)((w8)displ);
    else addr=(w32)((w16)(reg[(displ>>12) & 15]))+
	   aReg[r]+(w32)((w8)displ);
    break;
  case 7:
    switch(r) {
    case 0:addr=(w16)RW(pc++);
      break;
    case 1:addr=RL((w32*)pc);
      pc+=2;
      break;
    default:
      exception=4;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
#if 1
      mea_acc=0;
#else
      isHW=false;
#if !defined(VM_SCR) 
      isDisplay=false;
#endif
#endif
      dest=(Ptr)(&dummy);
      return 0;
    }
    break;
  }
#if HOST_ALIGN>1
  if(((short)addr&1)!=0)
    { 
      exception=3;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      badAddress=addr;
      readOrWrite=16;
#if 1
      mea_acc=0;
#else
      isHW=false;
#if !defined(VM_SCR) 
      isDisplay=false;
#endif
#endif
      dest=(Ptr)(&dummy);
      return 0;
    }
  addr&=ADDR_MASK_E;
#else
  addr &=ADDR_MASK;
#endif /* HOST_ALIGN>1 */

  switch(RamMap[addr>>RM_SHIFT]) {
  case 1:   /* ROM */
#if 1
    mea_acc=0;
#else
    isHW=false;
#if !defined(VM_SCR) 
    isDisplay=false;
#endif
#endif
    dest=(Ptr)(&dummy);
    return (w16)RW((w16*)((Ptr)theROM+addr));
  case 3:
#if 1
    mea_acc=0;
#else
    isHW=false;
#if !defined(VM_SCR) 
    isDisplay=false;
#endif
#endif
    dest=(Ptr)theROM+addr;
    return (w16)RW((w16*)dest);

  case 7:   /* screen access */
  case 23:
#if 1
    mea_acc=MEA_DISP;
#else
    isHW=false;
#if !defined(VM_SCR) 
    isDisplay=true;
#endif
#endif
    dest=(Ptr)theROM+addr;
    return (w16)RW((w16*)dest);

#if 0  /* delete that ...*/
  case 23:
    isHW=false;
#if !defined(VM_SCR)
    isDisplay=true;
#endif
    dest=(Ptr)theROM+addr;
    return (w16)RW(dest);
#endif

  case 8:
#if 1
    mea_acc=MEA_HW;
#else
    isHW=true;
#if !defined(VM_SCR) 
    isDisplay=false;
#endif
#endif
    lastAddr=addr;
    dest=(Ptr)(&dummy);
    return ReadWord(addr);
  }
#if 1
  mea_acc=0;
#else
  isHW=false;
#if !defined(VM_SCR) 
  isDisplay=false;
#endif
#endif
  dest=(Ptr)(&dummy);
  return 0;
}

STATIC rw32 REGP2 ModifyAtEA_l(ashort mode,ashort r)
{   
  /*w16*/ shindex displ;
  w32     addr;

#ifndef QM_BIG_ENDIAN
  isreg=0;
#endif

  switch(mode) {
  case 0:
#ifdef QM_BIG_ENDIAN
#if 1
    mea_acc=0;
#else
#if !defined(VM_SCR) 
    isDisplay=false;
#endif
    isHW=false;
#endif
#else
    isreg=1;
#endif
    dest=(Ptr)(&reg[r]);
    return *((w32*)dest);
  case 1:
#ifdef QM_BIG_ENDIAN
#if 1
    mea_acc=0;
#else
#if !defined(VM_SCR) 
    isDisplay=false;
#endif
    isHW=false;
#endif
#else
    isreg=1;
#endif
    dest=(Ptr)(&aReg[r]);
    return *((w32*)dest);
  case 2:addr=aReg[r];
    break;
  case 3:addr=aReg[r];
    aReg[r]+=4;
    break;
  case 4:addr=(aReg[r]-=4);
    break;
  case 5:addr=aReg[r]+(w16)RW(pc++);
    break;
  case 6:
    displ=(w16)RW(pc++);
    if((displ&2048)!=0) addr=reg[(displ>>12) & 15]+
			  aReg[r]+(w32)((w8)displ);
    else addr=(w32)((w16)(reg[(displ>>12) & 15]))+
	   aReg[r]+(w32)((w8)displ);
    break;
  case 7:
    switch(r) {
    case 0:addr=(w16)RW(pc++);
      break;
    case 1:addr=RL((w32*)pc);
      pc+=2;
      break;
    default:
      exception=4;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
#if 1
      mea_acc=0;
#else
      isHW=false;
#if !defined(VM_SCR)
      isDisplay=false;
#endif
#endif
      dest=(Ptr)(&dummy);
      return 0;
    }
    break;
  }
#if HOST_ALIGN>1
  if(((short)addr&1)!=0)
    { 
      exception=3;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      badAddress=addr;
      readOrWrite=16;
#if 1
      mea_acc=0;
#else
      isHW=false;
#if !defined(VM_SCR) 
      isDisplay=false;
#endif
#endif
      dest=(Ptr)(&dummy);
      return 0;
    }

  addr&=ADDR_MASK_E;
#else
  addr &= ADDR_MASK;
#endif /* HOST_ALIGN>1*/

  switch(RamMap[addr>>RM_SHIFT]) {
  case 1:
#if 1
    mea_acc=0;
#else
    isHW=false;
#if !defined(VM_SCR)
    isDisplay=false;
#endif
#endif
    dest=(Ptr)(&dummy);
    return (w32)RL((w32*)((Ptr)theROM+addr));
  case 3:
#if 1
    mea_acc=0;
#else
    isHW=false;
#if !defined(VM_SCR)
    isDisplay=false;
#endif
#endif
    dest=(Ptr)theROM+addr;
    return (w32)RL((w32*)dest);
  case 7:
  case 23:
#if 1
    mea_acc=MEA_DISP;
#else
    isHW=false;
#if !defined(VM_SCR) 
    isDisplay=true;
#endif
#endif
    dest=(Ptr)theROM+addr;
    return (w32)RL((w32*)dest);

#if 0  /* delete that */
  case 23:isHW=false;
#if !defined(VM_SCR) 
    isDisplay=true;
#endif
    dest=(Ptr)theROM+addr;
    /*tmp=ReadDispLong(addr);*/
    return (w32)RL(dest);
#endif

  case 8:
#if 1
    mea_acc=MEA_HW;
#else
    isHW=true;
#if !defined(VM_SCR) 
    isDisplay=false;
#endif
#endif
    lastAddr=addr;
    dest=(Ptr)(&dummy);
    return ReadLong(addr);
  }
#if 1
  mea_acc=0;
#else
  isHW=false;
#if !defined(VM_SCR) 
  isDisplay=false;
#endif
#endif
  dest=(Ptr)(&dummy);
  return 0;
}


STATIC void REGP1 RewriteEA_b(aw8 d) 
{   
#if 1
  *((w8*)dest)=d;
  if (!mea_acc) return;
  else
#ifdef EVM_SCR 
    if (mea_acc==MEA_DISP) vmMarkScreen((char*)dest-(char*)theROM);
    else
#endif
      WriteByte(lastAddr,d);
#else
  if(isHW) WriteByte(lastAddr,d);
  else
    { 
#if defined(EVM_SCR)
      if(isDisplay)
	vmMarkScreen((char*)dest-(char*)theROM);
#endif
      *((w8*)dest)=d;
    }
#endif
}

/* ufast version for iexl_general.c */
#if 1  /* split to very fast macro and slow fcall*/
STATIC void REGP1 rwb_acc(w8 d)
{
#ifdef EVM_SCR 
  if (mea_acc==MEA_DISP) vmMarkScreen((char*)dest-(char*)theROM);
  else
#endif
    WriteByte(lastAddr,d);
}
/* for the rest of the file the function gets redefined by a macro */
#define RewriteEA_b(_d_)   do{\
        *((w8*)dest)=_d_;     \
        if (mea_acc) rwb_acc(_d_);  } while(0)
#endif


STATIC void REGP1 RewriteEA_w(aw16 d) 
{  
#ifndef QM_BIG_ENDIAN
      if (isreg) 
	*((w16*)dest)=d;
      else
	{
	  WW((Ptr)dest,d);
#else 
	  WW((Ptr)dest,d);
#endif /* QM_BIG_ENDIAN */
	  if (!mea_acc) return;
#if defined(EVM_SCR)
	  if(mea_acc==MEA_DISP)
	    vmMarkScreen((char*)dest-(char*)theROM);
	  else
#endif /* EVM_SCR */
	    WriteWord(lastAddr,d);
#ifndef QM_BIG_ENDIAN
	}
#endif
}

#if 1
STATIC void REGP1 rww_acc(w16 d) 
{
#if defined(EVM_SCR)
	  if(mea_acc==MEA_DISP)
	    vmMarkScreen((char*)dest-(char*)theROM);
	  else
#endif /* EVM_SCR */
	    WriteWord(lastAddr,d);
}
#ifdef QM_BIG_ENDIAN
#define RewriteEA_w(_d_)       do{    \
	  WW((Ptr)dest,_d_);          \
          if (mea_acc) rww_acc(_d_); } while(0)
#else
#define RewriteEA_w(_d_)       do{    \
          if (isreg) *((w16*)dest)=_d_;       \
          else {                              \
            	  WW((Ptr)dest,_d_);          \
                  if (mea_acc) rww_acc(_d_); }} while(0)
#endif
#endif     
   

STATIC void REGP1 RewriteEA_l(aw32 d)
{
#ifndef QM_BIG_ENDIAN
      if (isreg) *((w32*)dest)=d;
      else 
	{
	  WL((Ptr)dest,d);
#else
	  WL((Ptr)dest,d);
#endif /* QM_BIG_ENDIAN */
	  if (!mea_acc) return;
#if defined(EVM_SCR)
	  if(mea_acc==MEA_DISP)
	    vmMarkScreen((char*)dest-(char*)theROM);
	  else
#endif /* EVM_SCR */
	    WriteLong(lastAddr,d);
#ifndef QM_BIG_ENDIAN
	}
#endif
}

#if 1
STATIC void REGP1 rwl_acc(w32 d) 
{
#if defined(EVM_SCR)
	  if(mea_acc==MEA_DISP)
	    vmMarkScreen((char*)dest-(char*)theROM);
	  else
#endif /* EVM_SCR */
	    WriteLong(lastAddr,d);
}
#ifdef QM_BIG_ENDIAN
#define RewriteEA_l(_d_)       do{    \
	  WL((Ptr)dest,_d_);          \
          if (mea_acc) rwl_acc(_d_); } while(0)
#else
#define RewriteEA_l(_d_)       do{    \
          if (isreg) *((w32*)dest)=_d_;       \
          else {                              \
            	  WL((Ptr)dest,_d_);          \
                  if (mea_acc) rwl_acc(_d_); }} while(0)
#endif
#endif     
