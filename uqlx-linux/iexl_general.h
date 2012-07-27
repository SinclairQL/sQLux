/*
 * (c) UQLX - see COPYRIGHT
 */
#ifndef IEXL_GENERAL_H
#define UEXL_GENERAL_H

#ifdef USE_VM

#define D_ISREG

#define FIX_INS


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


#if 1
     /****************************************************/
#ifndef QM_BIG_ENDIAN
static int isreg=0;
#endif

rw8 ModifyAtEA_b(ashort mode,ashort r);
rw16 ModifyAtEA_w(ashort mode,ashort r);
rw32 ModifyAtEA_l(ashort mode,ashort r);
#endif

#define RewriteEA_b(_d_)   (*((w8*)dest)=_d_)  

#if 0
static REGP1 inline void RewriteEA_b(aw8 d) 
{
  *((w8*)dest)=d;
}
#endif


#if 1
#ifdef QM_BIG_ENDIAN
#define RewriteEA_w(_d_)    (WW((Ptr)dest,_d_))
#else /* little endian */
#define RewriteEA_w(_d_)  {if (isreg) *((w16*)dest)=_d_; \
                           else   WW((Ptr)dest,_d_);}
#endif
#else
static inline void RewriteEA_w(aw16 d)
{  
#ifndef QM_BIG_ENDIAN
  if (isreg) *((w16*)dest)=d;
  else 
#endif
    WW((Ptr)dest,d);
}
#endif

#if 1
#ifdef QM_BIG_ENDIAN
#define RewriteEA_l(_d_)    (WL((Ptr)dest,_d_))
#else /* little endian */
#define RewriteEA_l(_d_)  {if (isreg) *((w32*)dest)=_d_; \
                           else   WL((Ptr)dest,_d_);}
#endif
#else
static inline void RewriteEA_l(aw32 d)
{    
#ifndef QM_BIG_ENDIAN
      if (isreg) *((w32*)dest)=d;
      else
#endif 
	WL((Ptr)dest,d);

}
#endif


     /****************************************************/

#endif
#endif