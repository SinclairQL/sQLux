/*
 * (c) UQLX - see COPYRIGHT
 */


#ifdef USE_VM

#define D_ISREG

/* define a few FAST inlines */

#define ReadByte(_addr_)    ({\
  w32 __x_addr=(_addr_);\
  __x_addr&=ADDR_MASK;\
  FIX_INS\
  *((w8*)theROM+__x_addr);\
})
#define inl_ReadByte ReadByte

#if HOST_ALIGN>1
#define ReadWord(_addr_)   ({\
  w32 __x_addr=(_addr_);\
  if(((char)__x_addr&1)!=0) \
    {   \
      exception=3; \
      extraFlag=true; \
      nInst2=nInst; \
      nInst=0; \
      badAddress=__x_addr; \
      readOrWrite=16;\
      return; \
    } \
  __x_addr&=ADDR_MASK_E; \
  FIX_INS\
  RW((w16*)((Ptr)theROM+__x_addr));\
})
#else
#define ReadWord(_addr_)   ({\
  w32 __x_addr=(_addr_);\
  __x_addr&=ADDR_MASK;\
  FIX_INS\
  RW((w16*)((Ptr)theROM+__x_addr));\
})
#endif

#if HOST_ALIGN>1
#define ReadLong(_addr_)   ({\
  w32 __x_addr=(_addr_);\
  if(((char)__x_addr&1)!=0) \
    {   \
      exception=3; \
      extraFlag=true; \
      nInst2=nInst; \
      nInst=0; \
      badAddress=__x_addr; \
      readOrWrite=16;\
      return; \
    } \
  __x_addr&=ADDR_MASK_E; \
  FIX_INS\
  RL((Ptr)theROM+__x_addr);\
})
#else
#define ReadLong(_addr_)   ({\
  w32 __x_addr=(_addr_);\
  __x_addr&=ADDR_MASK;\
  FIX_INS\
  RL((Ptr)theROM+__x_addr);\
})
#endif

#define inl_ReadLong ReadLong
#define inl_ReadWord ReadWord

#define WriteByte(_addr_,_d_)      ({\
  w32 __x_addr=(_addr_);\
  w8 __x_d=(_d_);\
  __x_addr&=ADDR_MASK;\
  FIX_INS\
  *((w8*)theROM+__x_addr)=__x_d;\
})
#define inl_WriteByte WriteByte

#if HOST_ALIGN>1
#define WriteWord(_addr_,_d_)        ({\
  w32 __x_addr=(_addr_);\
  w16 __x_d=(_d_);\
  if(((char)__x_addr&1)!=0) \
    {   \
      exception=3; \
      extraFlag=true; \
      nInst2=nInst; \
      nInst=0; \
      badAddress=__x_addr; \
      readOrWrite=16;\
      return ; \
    } \
  __x_addr&=ADDR_MASK_E; \
  FIX_INS\
  WW((Ptr)theROM+__x_addr,__x_d);\
})
#else
#define WriteWord(_addr_,_d_)        ({\
  w32 __x_addr=(_addr_);\
  w16 __x_d=(_d_);\
  __x_addr&=ADDR_MASK;\
  FIX_INS\
  WW((Ptr)theROM+__x_addr,__x_d);\
})
#endif

#define inl_WriteWord WriteWord

#if HOST_ALIGN>1
#define WriteLong(_addr_,_d_)        ({\
  w32 __x_addr=(_addr_);\
  w32 __x_d=(_d_);\
  if(((char)__x_addr&1)!=0) \
    {   \
      exception=3; \
      extraFlag=true; \
      nInst2=nInst; \
      nInst=0; \
      badAddress=__x_addr; \
      readOrWrite=16;\
      return ; \
    } \
  __x_addr&=ADDR_MASK_E; \
  FIX_INS\
  WL((Ptr)theROM+__x_addr,__x_d);\
})
#else
#define WriteLong(_addr_,_d_)        ({\
  w32 __x_addr=(_addr_);\
  w32 __x_d=(_d_);\
  __x_addr&=ADDR_MASK;\
  FIX_INS\
  WL((Ptr)theROM+__x_addr,__x_d);\
})
#endif

#define inl_WriteLong WriteLong

     /****************************************************/

#ifndef QM_BIG_ENDIAN
static int isreg=0;
#endif


static rw8 REGP2 ModifyAtEA_b(ashort mode,ashort r)
{   
  shindex displ;
  w32     addr;
  switch(mode) {
  case 0:
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
      dest=(Ptr)(&dummy);
      return 0;
    }
    break;
  default:
    exception=4;
    extraFlag=true;
    nInst2=nInst;
    nInst=0;
    dest=(Ptr)(&dummy);
    return 0;
  }

  addr&=ADDR_MASK;

  dest=(Ptr)theROM+addr;
  return *((w8*)dest);
}

static rw16 REGP2 ModifyAtEA_w(ashort mode,ashort r)
{  
  shindex displ;
  w32     addr;
#ifndef QM_BIG_ENDIAN
  isreg=0;
#endif
  switch(mode) {
  case 0:
    dest=(Ptr)(&reg[r])+RWO;
#ifndef QM_BIG_ENDIAN
    isreg=1;
#endif
    return *((w16*)dest);
  case 1:
#ifndef QM_BIG_ENDIAN
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
      dest=(Ptr)(&dummy);
      return 0;
    }
  addr&=ADDR_MASK_E;
#else
  addr&=ADDR_MASK;
#endif  

  dest=(Ptr)theROM+addr;
  return RW((w16*)dest);
}


static REGP2 rw32 ModifyAtEA_l(ashort mode,ashort r)
{   
  shindex displ;
  w32     addr;

#ifndef QM_BIG_ENDIAN
  isreg=0;
#endif
  switch(mode) {
  case 0:
#ifndef QM_BIG_ENDIAN
    isreg=1;
#endif
    dest=(Ptr)(&reg[r]);
    return *((w32*)dest);
  case 1:
#ifndef QM_BIG_ENDIAN
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
      dest=(Ptr)(&dummy);
      return 0;
    }
  addr&=ADDR_MASK_E;
#else
  addr&=ADDR_MASK;
#endif
  dest=(Ptr)theROM+addr;
  return RL((w32*)dest);
}


#define RewriteEA_b(_d_)   (*((w8*)dest)=_d_)  

#ifdef QM_BIG_ENDIAN
#define RewriteEA_w(_d_)    (WW((Ptr)dest,_d_))
#else /* little endian */
#define RewriteEA_w(_d_)  {if (isreg) *((w16*)dest)=_d_; \
                           else   WW((Ptr)dest,_d_);}
#endif


#ifdef QM_BIG_ENDIAN
#define RewriteEA_l(_d_)    (WL((Ptr)dest,_d_))
#else /* little endian */
#define RewriteEA_l(_d_)  {if (isreg) *((w32*)dest)=_d_; \
                           else   WL((Ptr)dest,_d_);}
#endif

#endif

