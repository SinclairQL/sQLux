/*
 * (c) UQLX - see COPYRIGHT
 */


#ifndef IE_XL
// no inlining, global scope for the slow version
#define INLINE
#define vml
//#define STATIC static
#include "QL68000.h"
//#include "memaccess.h"
//#include "mmodes.h"
#endif
 
#ifdef IE_XL
#define IDECL(_iname_)  _iname_ :
#define NEXT goto nextI; 
#else
#define IDECL(_iname_)  void _iname_ (void)
#define NEXT
#endif

#ifndef IE_XL
#define D_ISREG
#endif

IDECL(abcd)
{   
  w8      s,d,r;
  w8      *dx;
  if((code&8)!=0)
    {  
      s=GetFromEA_b_m4();
      d=ModifyAtEA_b(4,(code>>9)&7);
    }
  else
    {    
      dx=(w8*)((Ptr)reg+((code>>7)&28)+RBO);
      d=*dx;
      s=(w8)reg[code&7];
    }
  r=d+s;
  if(xflag) r++;
  if((r&0x0f)>9) r+=6;
  if(xflag=carry=(uw8)r>0x09f) r+=0x60;
  zero=zero && r==0;
  if((code&8)!=0) RewriteEA_b(r);
  else *dx=r;
NEXT;
}

IDECL(add_b_dn)
{ 
  w8 r,s;
  w8 *d;
  s=GetFromEA_b[(code>>3)&7]();
  d=(w8*)((Ptr)reg+((code>>7)&28)+RBO);
  r=*d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x80)==0) && (((s|*d)&0x80)!=0)) || ((s&*d&0x80)!=0);
  overflow=((0x80&s&*d&(r^0x80))!=0)||((0x80&r&(*d^0x80)&(s^0x80))!=0);
  *d=r;
NEXT;
}

IDECL(add_b_dn_dn)
{ 
  w8 r,s;
  w8 *d;
  s=(w8)reg[code&7];
  d=(w8*)((Ptr)reg+((code>>7)&28)+RBO);
  r=*d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x80)==0) && (((s|*d)&0x80)!=0)) || ((s&*d&0x80)!=0);
  overflow=((0x80&s&*d&(r^0x80))!=0)||((0x80&r&(*d^0x80)&(s^0x80))!=0);
  *d=r;
NEXT;
}

IDECL(add_w_dn)
{ 
  w16 r,s;
  w16 *d;
  s=GetFromEA_w[(code>>3)&7]();
  d=(w16*)((Ptr)reg+((code>>7)&28)+RWO);
  r=*d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x8000)==0) && (((s|*d)&0x8000)!=0)) ||
    ((s&*d&0x8000)!=0);

  overflow=((0x8000&s&*d&(r^0x8000))!=0)||((0x8000&r&(*d^0x8000)&(s^0x8000))!=0);
  *d=r;
NEXT;
}

IDECL(add_w_dn_dn)
{ 
  w16 r,s;
  w16 *d;
  s=(w16)reg[code&7];
  d=(w16*)((Ptr)reg+((code>>7)&28)+RWO);
  r=*d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x8000)==0) && (((s|*d)&0x8000)!=0)) ||
    ((s&*d&0x8000)!=0);

  overflow=((0x8000&s&*d&(r^0x8000))!=0)||
    ((0x8000&r&(*d^0x8000)&(s^0x8000))!=0);
  *d=r;
NEXT;
}

IDECL(add_l_dn)
{ 
  w32 r,s;
  w32 *d;
  s=GetFromEA_l[(code>>3)&7]();
  d=((w32*)((Ptr)reg+((code>>7)&28)));
  r=*d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x80000000)==0) && (((s|*d)&0x80000000)!=0)) ||
    ((s&*d&0x80000000)!=0);

  overflow=((0x80000000&s&*d&(r^0x80000000))!=0)||
    ((0x80000000&r&(*d^0x80000000)&(s^0x80000000))!=0);
  *d=r;
NEXT;
}

IDECL(add_l_dn_dn)
{ 
  w32 r,s;
  w32 *d;
  s=reg[code&7];
  d=((w32*)((Ptr)reg+((code>>7)&28)));
  r=*d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x80000000)==0) && (((s|*d)&0x80000000)!=0)) ||
    ((s&*d&0x80000000)!=0);

  overflow=((0x80000000&s&*d&(r^0x80000000))!=0)||
    ((0x80000000&r&(*d^0x80000000)&(s^0x80000000))!=0);
  *d=r;
NEXT;
}

IDECL(add_b_ea)
{ 
  w8 r,s;
  w8 d;
  d=ModifyAtEA_b((code>>3)&7,code&7);
  s=*((w8*)((Ptr)reg+((code>>7)&28)+RBO));
  r=d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x80)==0) && (((s|d)&0x80)!=0)) || ((s&d&0x80)!=0);
  overflow=((0x80&s&d&(r^0x80))!=0)||((0x80&r&(d^0x80)&(s^0x80))!=0);
  RewriteEA_b(r);
NEXT;
}

IDECL(add_w_ea)
{ 
  w16 r,s;
  w16 d;
  D_ISREG;
  d=ModifyAtEA_w((code>>3)&7,code&7);
  s=*((w16*)((Ptr)reg+((code>>7)&28)+RWO));
  r=d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x8000)==0) && (((s|d)&0x8000)!=0)) ||
    ((s&d&0x8000)!=0);

  overflow=((0x8000&s&d&(r^0x8000))!=0)||
    ((0x8000&r&(d^0x8000)&(s^0x8000))!=0)
    ;
  RewriteEA_w(r);
NEXT;
}

IDECL(add_l_ea)
{ 
  w32 r,s;
  w32 d;
  D_ISREG;
  d=ModifyAtEA_l((code>>3)&7,code&7);
  s=*((w32*)((Ptr)reg+((code>>7)&28)));
  r=d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x80000000)==0) && (((s|d)&0x80000000)!=0)) ||
    ((s&d&0x80000000)!=0);

  overflow=((0x80000000&s&d&(r^0x80000000))!=0)||((0x80000000&r&(d^0x80000000)
						   &(s^0x80000000))!=0);
  RewriteEA_l(r);
NEXT;
}

// adda (a7)+,a7 messed up, compiler evalueated assignment in wrong order
IDECL(add_w_an)
{
  //w32 a1,t1,a2;
  w32 t1;
  //a1=aReg[7];
  t1=LongFromWord(GetFromEA_w[(code>>3)&7]());

  *((w32*)((Ptr)aReg+((code>>7)&28)))+= t1; //LongFromWord(GetFromEA_w[(code>>3)&7]());
  //if (0xdedf == code )
  //  printf("a7 before: %x, after %x, temp %x\n",a1,aReg[7],t1);
NEXT;
}


IDECL(add_w_an_dn)
{ 
  *((w32*)((Ptr)aReg+((code>>7)&28)))+=(w32)((w16)reg[code&7]);
NEXT;
}

IDECL(add_l_an)
{ 
  w32 t1;
  t1=GetFromEA_l[(code>>3)&7]();
  *((w32*)((Ptr)aReg+((code>>7)&28)))+= t1; //GetFromEA_l[(code>>3)&7]();
NEXT;
}

IDECL(add_l_an_dn)
{ 
  *((w32*)((Ptr)aReg+((code>>7)&28)))+=reg[code&7];
NEXT;
}

IDECL(addi_b)
{ 
  w8 r,s;
  w8 d;
  s=(w8)RW(pc++);
  d=ModifyAtEA_b((code>>3)&7,code&7);
  r=d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x80)==0) && (((s|d)&0x80)!=0)) || ((s&d&0x80)!=0);
  overflow=((0x80&s&d&(r^0x80))!=0)||((0x80&r&(d^0x80)&(s^0x80))!=0);
  RewriteEA_b(r);
NEXT;
}

IDECL(addi_w)
{ 
  w16 r,s;
  w16 d;
  D_ISREG;
  s=(w16)RW(pc++);
  d=ModifyAtEA_w((code>>3)&7,code&7);
  r=d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x8000)==0) && (((s|d)&0x8000)!=0)) ||
    ((s&d&0x8000)!=0);

  overflow=((0x8000&s&d&(r^0x8000))!=0)||
    ((0x8000&r&(d^0x8000)&(s^0x8000))!=0);
  RewriteEA_w(r);
NEXT;
}

IDECL(addi_l)
{ 
  w32 r,s;
  w32 d;
  D_ISREG;
  /*s=*((w32*)pc);*/
  s=RL((Ptr)pc);
  pc+=2;
  d=ModifyAtEA_l((code>>3)&7,code&7);
  r=d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x80000000)==0) && (((s|d)&0x80000000)!=0)) ||
    ((s&d&0x80000000)!=0);

  overflow=((0x80000000&s&d&(r^0x80000000))!=0)||((0x80000000&r&(d^0x80000000)
						   &(s^0x80000000))!=0);
  RewriteEA_l(r);
NEXT;
}

IDECL(addq_b)
{ 
  w8 r,s;
  w8 d;
  d=ModifyAtEA_b((code>>3)&7,code&7);
  s=(code>>9)&7;
  if(s==0) s=8;
  r=d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x80)==0) && ((d&0x80)!=0));
  overflow=(0x80&r&(d^0x80))!=0;
  RewriteEA_b(r);
NEXT;
}

IDECL(addq_w)
{ 
  w16 r,s;
  w16 d;
  D_ISREG;
  d=ModifyAtEA_w((code>>3)&7,code&7);
  s=(code>>9)&7;
  if(s==0) s=8;
  r=d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x8000)==0) && ((d&0x8000)!=0));
  overflow=(0x8000&r&(d^0x8000))!=0;
  RewriteEA_w(r);
NEXT;
}

IDECL(addq_l)
{ 
  w32 r,s;
  w32 d;
  D_ISREG;
  d=ModifyAtEA_l((code>>3)&7,code&7);
  s=(code>>9)&7;
  if(s==0) s=8;
  r=d+s;
  negative=r<0;
  zero=r==0;
  xflag=carry=(((r&0x80000000)==0) && ((d&0x80000000)!=0));
  overflow=(0x80000000&r&(d^0x80000000))!=0;
  RewriteEA_l(r);
NEXT;
}

IDECL(addq_an)
{ 
  register short s;
  s=(code>>9)&7;
  if(s==0) s=8;
  aReg[code&7]+=s;
NEXT;
}

IDECL(addq_4_an)
{ 
  aReg[code&7]+=4;
NEXT;
}

IDECL(addx_b_r)
{ 
  w8      s,r;
  w8      *d;
  d=(w8*)((Ptr)reg+((code>>7)&28)+RBO);
  s=(w8)reg[code&7];
  r=*d+s;
  if(xflag) r++;
  negative=r<0;
  zero=zero && r==0;
  xflag=carry=(((r&0x80)==0) && (((s|*d)&0x80)!=0)) || ((s&*d&0x80)!=0);
  overflow=((0x80&s&*d&(r^0x80))!=0)||((0x80&r&(*d^0x80)&(s^0x80))!=0);
  *d=r;
NEXT;
}

IDECL(addx_w_r)
{ 
  w16     s,r;
  w16     *d;
  d=(w16*)((Ptr)reg+((code>>7)&28)+RWO);
  s=(w16)reg[code&7];
  r=*d+s;
  if(xflag) r++;
  negative=r<0;
  zero=zero && r==0;
  xflag=carry=(((r&0x8000)==0) && (((s|*d)&0x8000)!=0)) ||
    ((s&*d&0x8000)!=0);
	
  overflow=((0x8000&s&*d&(r^0x8000))!=0)||
    ((0x8000&r&(*d^0x8000)&(s^0x8000))!=0);
  *d=r;
NEXT;
}

IDECL(addx_l_r)
{ 
  w32     s,r;
  w32     *d;
  d=(w32*)((Ptr)reg+((code>>7)&28));
  s=reg[code&7];
  r=*d+s;
  if(xflag) r++;
  negative=r<0;
  zero=zero && r==0;
  xflag=carry=(((r&0x80000000)==0) && (((s|*d)&0x80000000)!=0)) ||
    ((s&*d&0x80000000)!=0);

  overflow=((0x80000000&s&*d&(r^0x80000000))!=0)||
    ((0x80000000&r&(*d^0x80000000)&(s^0x80000000))!=0);
  *d=r;
NEXT;
}

IDECL(addx_b_m)
{ 
  w8      s,d,r;
  s=GetFromEA_b_m4();
  d=ModifyAtEA_b(4,(code>>9)&7);
  r=d+s;
  if(xflag) r++;
  negative=r<0;
  zero=zero && r==0;
  xflag=carry=(((r&0x80)==0) && (((s|d)&0x80)!=0)) || ((s&d&0x80)!=0);
  overflow=((0x80&s&d&(r^0x80))!=0)||((0x80&r&(d^0x80)&(s^0x80))!=0);
  RewriteEA_b(r);
NEXT;
}

IDECL(addx_w_m)
{ 
  w16     s,d,r;
  D_ISREG;
  s=GetFromEA_w_m4();
  d=ModifyAtEA_w(4,(code>>9)&7);
  r=d+s;
  if(xflag) r++;
  negative=r<0;
  zero=zero && r==0;
  xflag=carry=(((r&0x8000)==0) && (((s|d)&0x8000)!=0)) ||
    ((s&d&0x8000)!=0);

  overflow=((0x8000&s&d&(r^0x8000))!=0)||((0x8000&r&(d^0x8000)&(s^0x8000))!=0)
    ;
  RewriteEA_w(r);
NEXT;
}

IDECL(addx_l_m)
{ 
  w32     s,d,r;
  D_ISREG;
  s=GetFromEA_l_m4();
  d=ModifyAtEA_l(4,(code>>9)&7);
  r=d+s;
  if(xflag) r++;
  negative=r<0;
  zero=zero && r==0;
  xflag=carry=(((r&0x80000000)==0) && (((s|d)&0x80000000)!=0)) ||
    ((s&d&0x80000000)!=0);

  overflow=((0x80000000&s&d&(r^0x80000000))!=0)||
    ((0x80000000&r&(d^0x80000000) &(s^0x80000000))!=0);
  RewriteEA_l(r);
NEXT;
}

IDECL(and_b_dn)
{ 
  register w8     *d;
  d=(w8*)((Ptr)reg+((code>>7)&28)+RBO);
  *d=*d&GetFromEA_b[(code>>3)&7]();
  negative=*d<0;
  zero=*d==0;
  carry=overflow=false;
NEXT;
}

IDECL(and_w_dn)
{ 
  register w16    *d;
  d=(w16*)((Ptr)reg+((code>>7)&28)+RWO);
  *d=*d&GetFromEA_w[(code>>3)&7]();
  negative=*d<0;
  zero=*d==0;
  carry=overflow=false;
NEXT;
}

IDECL(and_l_dn)
{ 
  register w32    *d;
  d=(w32*)((Ptr)reg+((code>>7)&28));
  *d=*d&GetFromEA_l[(code>>3)&7]();
  negative=*d<0;
  zero=*d==0;
  carry=overflow=false;
NEXT;
}

IDECL(and_l_dn_dn)
{ 
  register w32    *d;
  d=(w32*)((Ptr)reg+((code>>7)&28));
  (*d)&=reg[code&7];
  negative=*d<0;
  zero=*d==0;
  carry=overflow=false;
NEXT;
}

IDECL(and_b_ea)
{ 
  register w8     d;
  d=ModifyAtEA_b((code>>3)&7,code&7);
  d=d&*((w8*)((Ptr)reg+((code>>7)&28)+RBO));
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_b(d);
NEXT;
}

IDECL(and_w_ea)
{ 
  register w16    d;
  D_ISREG;
  d=ModifyAtEA_w((code>>3)&7,code&7);
  d=d&*((w16*)((Ptr)reg+((code>>7)&28)+RWO));
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_w(d);
NEXT;
}

IDECL(and_l_ea)
{ 
  register w32    d;
  D_ISREG;
  d=ModifyAtEA_l((code>>3)&7,code&7);
  d=d&*((w32*)((Ptr)reg+((code>>7)&28)));
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_l(d);
NEXT;
}

IDECL(andi_b)
{ 
  register w8     d;
  d=(w8)RW(pc++);
  d=d&ModifyAtEA_b((code>>3)&7,code&7);
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_b(d);
NEXT;
}

IDECL(andi_w)
{ 
  register w16    d;
  D_ISREG;
  d=(w16)RW(pc++);
  d=d&ModifyAtEA_w((code>>3)&7,code&7);
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_w(d);
NEXT;
}

IDECL(andi_l)
{ 
  register w32 d;
  D_ISREG;
  d=RL((Ptr)pc); /* d=*((w32*)pc); */
  pc+=2;
  d=d&ModifyAtEA_l((code>>3)&7,code&7);
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_l(d);
NEXT;
}

IDECL(andi_to_ccr)
{ 
  register uw16 d;
  d=RW(pc++);
  carry=carry&&((d&1)!=0);
  overflow=overflow&&((d&2)!=0);
  zero=zero&&((d&4)!=0);
  negative=negative&&((d&8)!=0);
  xflag=xflag&&((d&16)!=0);
NEXT;
}

IDECL(andi_to_sr)
{ 
  register w16 d;
  d=(w16)RW(pc++);
  if(supervisor)
    { 
      d&=GetSR();
      PutSR(d);
    }
  else
    { 
      exception=8;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
NEXT;
}

IDECL(asl_m)
{ 
  register w16 d;
  D_ISREG;
  d=ModifyAtEA_w((code>>3)&7,code&7);
  xflag=carry=(d&0x8000)!=0;
  d<<=1;
  negative=d<0;
  zero=d==0;
  overflow=carry!=negative;
  RewriteEA_w(d);
NEXT;
}

IDECL(asr_m)
{ 
  register w16 d;
  D_ISREG;
  d=ModifyAtEA_w((code>>3)&7,code&7);
  xflag=carry=(d&1)!=0;
  negative=d<0;
  d>>=1;  /* attenzione: deve essere un signed shift !! */
  zero=d==0;
  overflow=false;
  RewriteEA_w(d);
NEXT;
}

IDECL(bcc_l)
{ 
  if(ConditionTrue[(code>>8)&15]())
    SetPC((Ptr)pc-(Ptr)theROM+(w16)RW(pc));
  else pc++;
NEXT;
}

IDECL(beq_l)
{ 
  if(zero) SetPC((Ptr)pc-(Ptr)theROM+(w16)RW(pc));
  else pc++;
NEXT;
}

IDECL(bne_l)
{ 
  if(!zero) SetPC((Ptr)pc-(Ptr)theROM+(w16)RW(pc));
  else pc++;
NEXT;
}

IDECL(bcc_bad)
{ 
  if(ConditionTrue[(code>>8)&15]())
    { 
      SetPC((Ptr)pc-(Ptr)theROM+(w8)code); /* cause address error */
    }
NEXT;
}

IDECL(bcc_s)
{ 
  if(ConditionTrue[(code>>8)&15]()) 
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(beq_s)
{ 
  if(zero) 
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(bne_s)
{ 
  if(!zero) 
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(bcs_s)
{ 
  if(carry) 
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(bccc_s)
{ 
  if(!carry) 
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(bpl_s)
{ 
  if(!negative) 
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(bmi_s)
{ 
  if(negative) 
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(bge_s)
{ 
  if((negative&&overflow)||(!(negative||overflow)))
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif      
NEXT;
}

IDECL(blt_s)
{ 
  if((negative&&(!overflow))||((!negative)&&overflow))
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(bgt_s)
{ 
  if((!zero)&&((negative&&overflow)||(!(negative||overflow))))
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(ble_s)
{ 
  if(zero||(negative&&(!overflow))||((!negative)&&overflow))
#ifdef DEBUG
    SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(bra_l)
{ 
  SetPC((Ptr)pc-(Ptr)theROM+(w16)RW(pc));
NEXT;
}

IDECL(bra_s)
{
#ifdef DEBUG
  SetPC((Ptr)pc-(Ptr)theROM+(w8)code);
#else
  pc=(uw16*)((Ptr)pc+(w8)code);
#ifdef TRACE
  CheckTrace();
#endif
#endif
NEXT;
}

IDECL(bchg_d)
{ 
  w32 mask=1;
  w8  d;
  short EAmode;
  short bit;
  EAmode=(code>>3)&7;
  bit=reg[code>>9];
  if(EAmode!=0)
    { 
      mask<<=(short)reg[code>>9]&7;
      d=ModifyAtEA_b(EAmode,code&7);
      zero=(d&(w8)mask)==0;
      RewriteEA_b(d^(w8)mask);
    }
  else
    { 
      mask<<=(short)reg[code>>9]&31;
      zero=(reg[code&7]&mask)==0;
      reg[code&7]^=mask;
    }
NEXT;
}

IDECL(bchg_s)
{ 
  w32 mask=1;
  w8  d;
  short EAmode;
  short bit;
  EAmode=(code>>3)&7;
  bit=RW(pc++);
  if(EAmode!=0)
    { 
      mask<<=bit&7;
      d=ModifyAtEA_b(EAmode,code&7);
      zero=(d&(w8)mask)==0;
      RewriteEA_b(d^(w8)mask);
    }
  else
    { 
      mask<<=bit&31;
      zero=(reg[code&7]&mask)==0;
      reg[code&7]^=mask;
    }
NEXT;
}

IDECL(bclr_d)
{ 
  w32 mask=1;
  w8  d;
  short EAmode;
  short bit;
  EAmode=(code>>3)&7;
  bit=reg[code>>9];
  if(EAmode!=0)
    { 
      mask<<=(short)reg[code>>9]&7;
      d=ModifyAtEA_b(EAmode,code&7);
      zero=(d&(w8)mask)==0;
      if(!zero) d^=(w8)mask;
      RewriteEA_b(d);
    }
  else
    { 
      mask<<=(short)reg[code>>9]&31;
      zero=(reg[code&7]&mask)==0;
      if(!zero) reg[code&7]^=mask;
    }
NEXT;
}

IDECL(bclr_s)
{ 
  w32 mask=1;
  w8  d;
  short EAmode;
  short bit;
  EAmode=(code>>3)&7;
  bit=RW(pc++);
  if(EAmode!=0)
    { 
      mask<<=bit&7;
      d=ModifyAtEA_b(EAmode,code&7);
      zero=(d&(w8)mask)==0;
      if(!zero) d^=(w8)mask;
      RewriteEA_b(d);
    }
  else
    { 
      mask<<=bit&31;
      zero=(reg[code&7]&mask)==0;
      if(!zero) reg[code&7]^=mask;
    }
NEXT;
}

IDECL(bsr)
{ 
  w16 displ;
  w32 oldPC;

  oldPC=(w32)pc-(w32)theROM;
  if((displ=(w16)(((w8)(code&255))))==0)
    { 
      displ=(w16)RW(pc);
      oldPC+=2;
    }
  WriteLong((*m68k_sp)-=4,oldPC);
#ifdef BACKTRACE
  SetPCB((Ptr)pc-(Ptr)theROM+displ,BSR);
#else
  SetPC((Ptr)pc-(Ptr)theROM+displ);
#endif
NEXT;
}

IDECL(bset_d)
{ 
  w32 mask=1;
  w8  d;
  short EAmode;
  short bit;
  EAmode=(code>>3)&7;
  bit=reg[code>>9];
  if(EAmode!=0)
    { 
      mask<<=(short)reg[code>>9]&7;
      d=ModifyAtEA_b(EAmode,code&7);
      zero=(d&(w8)mask)==0;
      if(zero) d|=(w8)mask;
      RewriteEA_b(d);
    }
  else
    { 
      mask<<=(short)reg[code>>9]&31;
      zero=(reg[code&7]&mask)==0;
      if(zero) reg[code&7]|=mask;
    }
NEXT;
}

IDECL(bset_s)
{ 
  w32 mask=1;
  w8  d;
  short EAmode;
  short bit;
  EAmode=(code>>3)&7;
  bit=RW(pc++);
  if(EAmode!=0)
    { 
      mask<<=bit&7;
      d=ModifyAtEA_b(EAmode,code&7);
      zero=(d&(w8)mask)==0;
      if(zero) d|=(w8)mask;
      RewriteEA_b(d);
    }
  else
    { 
      mask<<=bit&31;
      zero=(reg[code&7]&mask)==0;
      if(zero) reg[code&7]|=mask;
    }
NEXT;
}

IDECL(btst_d)
{ 
  w32 mask=1;
  short EAmode;
  short bit;
  EAmode=(code>>3)&7;
  bit=reg[code>>9];
  if(EAmode!=0)
    { 
      mask<<=(short)reg[code>>9]&7;
      zero=(GetFromEA_b[EAmode]()&mask)==0;
    }
  else
    { 
      mask<<=(short)reg[code>>9]&31;
      zero=(reg[code&7]&mask)==0;
    }
NEXT;
}

IDECL(btst_s)
{ 
  w32 mask=1;
  short EAmode;
  short bit;
  EAmode=(code>>3)&7;
  bit=RW(pc++);
  if(EAmode)
    { 
      mask<<=bit&7;
      zero=(GetFromEA_b[EAmode]()&mask)==0;
    }
  else
    { 
      mask<<=bit&31;
      zero=(reg[code&7]&mask)==0;
    }
NEXT;
}

IDECL(chk)
{ 
  w16 *d;
  w16 ea;
  d=(w16*)(((Ptr)reg) + ((code>>7)&0x1c) +RWO);
  ea=GetFromEA_w[(code>>3)&7]();
  if(*d<0)
    { 
      negative=true;
      exception=6;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
  else
    { 
      if(*d>ea)
	{ 
	  negative=false;
	  exception=6;
	  extraFlag=true;
	  nInst2=nInst;
	  nInst=0;
	}
    }
NEXT;
}

IDECL(clr_b)
{ 
  ARCALL(PutToEA_b,(code>>3)&7,code&7,0);
  /*((void (*)(short,w8)REGP2)PutToEA_b[(code>>3)&7])(code&7,0);*/
  /*PUT_TOEA_B((code>>3)&7,code&7,0);*/
  negative=overflow=carry=false;
  zero=true;
NEXT;
}

IDECL(clr_w)
{ 
  ARCALL(PutToEA_w,(code>>3)&7,code&7,0);
  /*PUT_TOEA_W((code>>3)&7,code&7,0);*/
  negative=overflow=carry=false;
  zero=true;
NEXT;
}

IDECL(clr_l)
{ 
  ARCALL(PutToEA_l,(code>>3)&7,code&7,0);
    /*PUT_TOEA_L((code>>3)&7,code&7,0);*/
  negative=overflow=carry=false;
  zero=true;
NEXT;
}

IDECL(cmp_b)
{ 
  w8 r,s,d;
  s=GetFromEA_b[(code>>3)&7]();
  d=*((w8*)((Ptr)reg+((code>>7)&28)+RBO));
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80)==0) && (((s|r)&0x80)!=0)) || ((s&r&0x80)!=0);
  overflow=((0x80&(s^0x80)&d&(r^0x80))!=0)||((0x80&r&(d^0x80)&s)!=0);
NEXT;
}

IDECL(cmp_b_dan)
{ 
  w8 r,s,d;
  s=ReadByte(aReg[code&7]+(w16)RW(pc++));
  d=*((w8*)((Ptr)reg+((code>>7)&28)+RBO));
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80)==0) && (((s|r)&0x80)!=0)) || ((s&r&0x80)!=0);
  overflow=((0x80&(s^0x80)&d&(r^0x80))!=0)||((0x80&r&(d^0x80)&s)!=0);
NEXT;
}

IDECL(cmp_b_dn)
{ 
  w8 r,s,d;
  s=(w8)reg[code&7];
  d=*((w8*)((Ptr)reg+((code>>7)&28)+RBO));
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80)==0) && (((s|r)&0x80)!=0)) || ((s&r&0x80)!=0);
  overflow=((0x80&(s^0x80)&d&(r^0x80))!=0)||((0x80&r&(d^0x80)&s)!=0);
NEXT;
}

IDECL(cmp_w)
{ 
  w16 r,s,d;
  s=GetFromEA_w[(code>>3)&7]();
  d=*((w16*)((Ptr)reg+((code>>7)&28)+RWO));
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x8000)==0) && (((s|r)&0x8000)!=0)) || ((s&r&0x8000)!=0);

  overflow=((0x8000&(s^0x8000)&d&(r^0x8000))!=0)||
    ((0x8000&r&(d^0x8000)&s)!=0)
    ;
NEXT;
}

IDECL(cmp_w_dn)
{ 
  w16 r,s,d;
  s=(w16)reg[code&7];
  d=*((w16*)((Ptr)reg+((code>>7)&28)+RWO));
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x8000)==0) && (((s|r)&0x8000)!=0)) || ((s&r&0x8000)!=0);

  overflow=((0x8000&(s^0x8000)&d&(r^0x8000))!=0)||((0x8000&r&(d^0x8000)&s)!=0)
    ;
NEXT;
}

IDECL(cmp_l)
{ 
  w32 r,s,d;
  s=GetFromEA_l[(code>>3)&7]();
  d=*((w32*)((Ptr)reg+((code>>7)&28))); /* d=reg[(code>>9)&7] */
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80000000)==0) && (((s|r)&0x80000000)!=0)) ||
    ((s&r&0x80000000)!=0);

  overflow=((0x80000000&(s^0x80000000)&d&(r^0x80000000))!=0)||
    ((0x80000000&r&(d^0x80000000)&s)!=0);
NEXT;
}

IDECL(cmp_l_dn)
{ 
  w32 r,s,d;
  s=reg[code&7];
  d=*((w32*)((Ptr)reg+((code>>7)&28))); /* d=reg[(code>>9)&7] */
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80000000)==0) && (((s|r)&0x80000000)!=0)) ||
    ((s&r&0x80000000)!=0);

  overflow=((0x80000000&(s^0x80000000)&d&(r^0x80000000))!=0)||
    ((0x80000000&r&(d^0x80000000)&s)!=0);
NEXT;
}

IDECL(cmpa_w)
{ 
  w32 r,s,d;
  s=LongFromWord(GetFromEA_w[(code>>3)&7]());
  d=*((w32*)((Ptr)aReg+((code>>7)&28)));
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80000000)==0) && (((s|r)&0x80000000)!=0)) ||
    ((s&r&0x80000000)!=0);

  overflow=((0x80000000&(s^0x80000000)&d&(r^0x80000000))!=0) || 
    ((0x80000000&r&(d^0x80000000)&s)!=0);
NEXT;
}

IDECL(cmpa_l)
{ 
  w32 r,s,d;
  s=GetFromEA_l[(code>>3)&7]();
  d=*((w32*)((Ptr)aReg+((code>>7)&28)));
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80000000)==0) && (((s|r)&0x80000000)!=0)) ||
    ((s&r&0x80000000)!=0);

  overflow=((0x80000000&(s^0x80000000)&d&(r^0x80000000))!=0)||
    ((0x80000000&r&(d^0x80000000)&s)!=0);
NEXT;
}

IDECL(cmpa_l_an)
{ 
  w32 r,s,d;
  s=aReg[code&7];
  d=*((w32*)((Ptr)aReg+((code>>7)&28)));
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80000000)==0) && (((s|r)&0x80000000)!=0)) ||
    ((s&r&0x80000000)!=0);

  overflow=((0x80000000&(s^0x80000000)&d&(r^0x80000000))!=0)||
    ((0x80000000&r&(d^0x80000000)&s)!=0);
NEXT;
}

IDECL(cmpi_b)
{ 
  w8 r,s,d;
  s=(w8)RW(pc++);
  d=GetFromEA_b[(code>>3)&7]();
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80)==0) && (((s|r)&0x80)!=0)) || ((s&r&0x80)!=0);
  overflow=((0x80&(s^0x80)&d&(r^0x80))!=0)||((0x80&r&(d^0x80)&s)!=0);
NEXT;
}

IDECL(cmpi_w)
{ 
  w16 r,s,d;
  s=(w16)RW(pc++);
  d=GetFromEA_w[(code>>3)&7]();
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x8000)==0) && (((s|r)&0x8000)!=0)) || ((s&r&0x8000)!=0);

  overflow=((0x8000&(s^0x8000)&d&(r^0x8000))!=0)||((0x8000&r&(d^0x8000)&s)!=0)
    ;
NEXT;
}

IDECL(cmpi_l)
{ 
  w32 r,s,d;
  /* s=*((w32*)pc); */
  s=RL((Ptr)pc);

  pc+=2;
  d=GetFromEA_l[(code>>3)&7]();
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80000000)==0) && (((s|r)&0x80000000)!=0)) ||
    ((s&r&0x80000000)!=0);

  overflow=((0x80000000&(s^0x80000000)&d&(r^0x80000000))!=0)||
    ((0x80000000&r&(d^0x80000000)&s)!=0);
NEXT;
}

IDECL(cmpm_b)
{ 
  w8 r,s,d;
  /*printf("call cmpm\n");*/

  s=GetFromEA_b_m3();
  /*code>>=9; */      /* una delle uniche modifiche di 'code' !! !!!*/
  d=GetFromEA_rb_m3((code>>9)&7);

  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80)==0) && (((s|r)&0x80)!=0)) || ((s&r&0x80)!=0);
  overflow=((0x80&(s^0x80)&d&(r^0x80))!=0)||((0x80&r&(d^0x80)&s)!=0);
NEXT;
}

IDECL(cmpm_w)
{ 
  w16 r,s,d;
  /*printf("call cmpm_w\n");*/
  s=GetFromEA_w_m3();
  /*code>>=9;*/       /* una delle uniche modifiche di 'code' !! !!!*/
  d=GetFromEA_rw_m3((code>>9)&7);
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x8000)==0) && (((s|r)&0x8000)!=0)) || ((s&r&0x8000)!=0);

  overflow=((0x8000&(s^0x8000)&d&(r^0x8000))!=0)||
    ((0x8000&r&(d^0x8000)&s)!=0)
    ;
NEXT;
}

IDECL(cmpm_l)
{ 
  w32 r,s,d;
  /*printf("call cmpm_l\n");*/
  s=GetFromEA_l_m3();
  /*code>>=9;*/       /* una delle uniche modifiche di 'code' !! !!!*/
  d=GetFromEA_rl_m3((code>>9)&7);
  r=d-s;
  negative=r<0;
  zero=r==0;
  carry=(((d&0x80000000)==0) && (((s|r)&0x80000000)!=0)) ||
    ((s&r&0x80000000)!=0);

  overflow=((0x80000000&(s^0x80000000)&d&(r^0x80000000))!=0)||
    ((0x80000000&r&(d^0x80000000)&s)!=0);
NEXT;
}


IDECL(dbcc)
{ 
  if(ConditionTrue[(code>>8)&15]()) pc++;
  else
    { 
      if(((*((uw16*)((Ptr)(&(reg[code&7]))+RWO)))--)==0) pc++;
      else
	{    
#ifdef DEBUG
	  SetPC((Ptr)pc-(Ptr)theROM+(w16)RW(pc));
#else
	  pc=(uw16*)((Ptr)pc+(w16)RW(pc));
#ifdef TRACE
	  CheckTrace();
#endif
#endif

	  if((long)pc&1)
	    { 
	      exception=3;
	      extraFlag=true;
	      nInst2=nInst;
	      nInst=0;
	      readOrWrite=16;
	      badAddress=(Ptr)pc-(Ptr)theROM;
	      badCodeAddress=true;
	    }
	}
    }
NEXT;
}

IDECL(dbf)
{  
  if(((*((uw16*)((Ptr)(&(reg[code&7]))+RWO)))--)==0) pc++;
  else
    {      
#ifdef DEBUG
      SetPC((Ptr)pc-(Ptr)theROM+(w16)RW(pc));
#else
      pc=(uw16*)((Ptr)pc+(w16)RW(pc));
#ifdef TRACE
      CheckTrace();
#endif
#endif
      if((long)pc&1)
	{ 
	  exception=3;
	  extraFlag=true;
	  nInst2=nInst;
	  nInst=0;
	  readOrWrite=16;
	  badAddress=(Ptr)pc-(Ptr)theROM;
	  badCodeAddress=true;
	}
    }
NEXT;
}

IDECL(divs)
{ 
  w32 *d;
  w32 r;
  w16 s;
  d=(w32*)((Ptr)reg+((code>>7)&28));
  s=GetFromEA_w[(code>>3)&7]();
  if(s!=0)
    { 
      r=*d/s;
      if(r<-32768 || r>32767) overflow=true;
      else
	{ 
	  zero=r==0;
	  negative=r<0;
	  overflow=carry=false;
	  *((w16*)((Ptr)d+UW_RWO))=*d-r*s;
	  *((w16*)((Ptr)d+RWO))=(w16)r;
	}
    }
  else
    { 
      exception=5;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
NEXT;
}

IDECL(divu)
{ 
  uw32 *d;
  uw32 r;
  uw16 s;
  d=(uw32 *)((Ptr)reg+((code>>7)&28));
  s=GetFromEA_w[(code>>3)&7]();
  if(s!=0)
    { 
      r=*d/s;
      if(r>65535) overflow=true;
      else
	{ 
	  zero=r==0;
	  negative=(w32)r<0;
	  overflow=carry=false;
	  *((uw16*)((Ptr)d+UW_RWO))=*d-r*s;
	  *((uw16*)((Ptr)d+RWO))=(uw16)r;
	}
    }
  else
    { 
      exception=5;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
NEXT;
}

IDECL(eor_b)
{ 
  register w8     d;
  d=ModifyAtEA_b((code>>3)&7,code&7);
  d=d^*((w8*)((Ptr)reg+((code>>7)&28)+RBO));
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_b(d);
NEXT;
}

IDECL(eor_w)
{ 
  register w16    d;
  D_ISREG;
  d=ModifyAtEA_w((code>>3)&7,code&7);
  d=d^*((w16*)((Ptr)reg+((code>>7)&28)+RWO));
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_w(d);
NEXT;
}

IDECL(eor_l)
{ 
  register w32    d;
  D_ISREG;
  d=ModifyAtEA_l((code>>3)&7,code&7);
  d=d^*((w32*)((Ptr)reg+((code>>7)&28)));
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_l(d);
NEXT;
}

IDECL(eori_b)
{ 
  register w8     d;
  d=(w8)RW(pc++);
  d=d^ModifyAtEA_b((code>>3)&7,code&7);
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_b(d);
NEXT;
}

IDECL(eori_w)
{ 
  register w16    d;
  D_ISREG;
  d=(w16)RW(pc++);
  d=d^ModifyAtEA_w((code>>3)&7,code&7);
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_w(d);
NEXT;
}

IDECL(eori_l)
{ 
  register w32    d;
  D_ISREG;
  d=RL((w32*)pc);
  pc+=2;
  d=d^ModifyAtEA_l((code>>3)&7,code&7);
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_l(d);
NEXT;
}

IDECL(eori_to_ccr)
{ 
  register uw16 d;
  d=RW(pc++);
  if((d&1)!=0) carry=!carry;
  if((d&2)!=0) overflow=!overflow;
  if((d&4)!=0) zero=!zero;
  if((d&8)!=0) negative=!negative;
  if((d&16)!=0) xflag=!xflag;
NEXT;
}

IDECL(eori_to_sr)
{ 
  register w16 d;
  d=(w16)RW(pc++);
  if(supervisor)
    { 
      PutSR(GetSR()^d);
    }
  else
    { 
      exception=8;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
NEXT;
}

IDECL(exg_d)
{ 
  w32 t;
  short r1,r2;
  r1=code&7;
  r2=(code>>9)&7;
  t=reg[r1];
  reg[r1]=reg[r2];
  reg[r2]=t;
NEXT;
}

IDECL(exg_a)
{ 
  w32 t;
  short r1,r2;
  r1=code&7;
  r2=(code>>9)&7;
  t=aReg[r1];
  aReg[r1]=aReg[r2];
  aReg[r2]=t;
NEXT;
}

IDECL(exg_ad)
{ 
  w32 t;
  short r1,r2;
  r1=code&7;
  r2=(code>>9)&7;
  t=aReg[r1];
  aReg[r1]=reg[r2];
  reg[r2]=t;
NEXT;
}

IDECL(ext_w)
{ 
  register w16 *dn;
  dn=(w16*)(RWO+(Ptr)(&reg[code&7]));
       
  *dn=WordFromByte((w8)(*dn));
  zero=*dn==0;
  negative=*dn<0;
  overflow=carry=false;
NEXT;
}

IDECL(ext_l)
{ 
  register w32 *dn;
  dn=&reg[code&7];
  *dn=(w32)((w16)(*dn));
  zero=*dn==0;
  negative=*dn<0;
  overflow=carry=false;
NEXT;
}

IDECL(illegal)
{ 
  exception=4;
  extraFlag=true;
  nInst2=nInst;
  nInst=0;
  pc--; /* ???? sembra che al QL piaccia cos=EC! Altrimenti i
	   breakpoint non funzionano */
NEXT;
}

IDECL(jmp)
{ 
  SetPC(ARCALL(GetEA,(code>>3)&7,(code&7)));
  /*SetPC(GET_EA((code>>3)&7,code&7));*/
NEXT;
}

IDECL(jsr)
{ 
  w32 ea;

  ea=ARCALL(GetEA,(code>>3)&7,(code&7));
  /* ea=GET_EA((code>>3)&7,(code&7));*/
  WriteLong((*m68k_sp)-=4,(w32)((Ptr)pc-(Ptr)theROM));
#ifdef BACKTRACE
  SetPCB(ea,JSR);
#else
  SetPC(ea);
#endif
NEXT;
}

IDECL(jsr_displ)
{ 
  register w32 ea;
  ea=(w32)pc-(w32)theROM;
  WriteLong((*m68k_sp)-=4,ea+2);
  SetPC(ea+(w16)RW(pc));
NEXT;
}

IDECL(lea)
{ 
  *((w32*)((Ptr)aReg+((code>>7)&28)))=ARCALL(GetEA,(code>>3)&7,(code&7));
NEXT;
}

IDECL(link)
{ 
  register w32 *r;
  r=&(aReg[code&7]);
  WriteLong((*m68k_sp)-=4,*r);
  *r=(*m68k_sp);
  (*m68k_sp)+=(w16)RW(pc++);
NEXT;
}

IDECL(lsl_m)
{ 
  register uw16   d;
  D_ISREG;
  d=(uw16)ModifyAtEA_w((code>>3)&7,code&7);
  carry=xflag=(d&0x8000)!=0;
  d<<=1;
  negative=(d&0x8000)!=0;
  zero=d==0;
  overflow=false;
  RewriteEA_w((w16)d);
NEXT;
}

IDECL(lsr_m)
{ 
  register uw16   d;
  D_ISREG;
  d=(uw16)ModifyAtEA_w((code>>3)&7,code&7);
  carry=xflag=(d&1)!=0;
  d>>=1;
  zero=d==0;
  negative=overflow=false;
  RewriteEA_w((w16)d);
NEXT;
}

IDECL(move_b)
{ 
  register w8 d;
  d=GetFromEA_b[(code>>3)&7]();
  ARCALL(PutToEA_b,(code>>6)&7,(code>>9)&7,d);
  /*PUT_TOEA_B((code>>6)&7,(code>>9)&7,d);*/
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_b_from_dn)
{ 
  register w8 d;
  d=*((w8*)(&reg[code&7])+RBO);
  ARCALL(PutToEA_b,(code>>6)&7,(code>>9)&7,d);
  /*PUT_TOEA_B((code>>6)&7,(code>>9)&7,d);*/
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_b_to_dn)
{ 
  register w8 d;
  d=*((w8*)reg+((code>>7)&28)+RBO)=GetFromEA_b[(code>>3)&7]();
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_b_reg)
{ 
  register w8 d;
  d=*((w8*)reg+((code>>7)&28)+RBO)=*((w8*)(&reg[code&7])+RBO);
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_w)
{ 
  register w16 d;
  d=GetFromEA_w[(code>>3)&7]();
  ARCALL(PutToEA_w,(code>>6)&7,(code>>9)&7,d);
  /*PUT_TOEA_W((code>>6)&7,(code>>9)&7,d);*/
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_w_from_dn)
{ 
  register w16 d;
  d=(w16)reg[code&7];
  ARCALL(PutToEA_w,(code>>6)&7,(code>>9)&7,d);
  /*PUT_TOEA_W((code>>6)&7,(code>>9)&7,d);*/
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_w_to_dn)
{ 
  register w16 d;
  d=GetFromEA_w[(code>>3)&7]();
  *((w16*)((Ptr)reg+RWO+((code>>7)&28)))=d;
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_w_reg)
{ 
  register w16 d;
  d=*((w16*)((Ptr)reg+((code>>7)&28)+RWO))=*(w16*)((Ptr)(&reg[code&7])+RWO);
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_l) 
{ 
  register w32 d;
  d=GetFromEA_l[((code>>3)&7)]();
  ARCALL(PutToEA_l,((code>>6)&7),((code>>9)&7),d);
  /*PUT_TOEA_L(((code>>6)&7),((code>>9)&7),d);*/
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_l_from_dn)
{ 
  register w32 d;
  d=reg[code&7];
  ARCALL(PutToEA_l,(code>>6)&7,(code>>9)&7,d);
  /*PUT_TOEA_L((code>>6)&7,(code>>9)&7,d);*/
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_l_to_dn)
{ 
  register w32 d;
  d=GetFromEA_l[(code>>3)&7]();
  *((w32*)((Ptr)reg+((code>>7)&28)))=d;
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}
IDECL(move_l_reg)
{ 
  register w32 d;
  d=*((w32*)((Ptr)reg+((code>>7)&28)))=reg[code&7];
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(move_to_ccr)
{ 
  register w16 x;
  x=GetFromEA_w[(code>>3)&7]();
  carry=(x&1)!=0;
  overflow=(x&2)!=0;
  zero=(x&4)!=0;
  negative=(x&8)!=0;
  xflag=(x&16)!=0;
NEXT;
}

IDECL(move_to_sr)
{ 
  register w16 x;
  x=GetFromEA_w[(code>>3)&7]();
  if(supervisor) PutSR(x);
  else
    { 
      exception=8;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
NEXT;
}

IDECL(move_from_sr)
{ 
  ARCALL(PutToEA_w,(code>>3)&7,code&7,GetSR());
  /*PUT_TOEA_W((code>>3)&7,code&7,GetSR());*/
NEXT;
}

IDECL(move_to_usp)
{ 
  if(supervisor) usp=aReg[code&7];
  else
    { 
      exception=8;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
NEXT;
}

IDECL(move_from_usp)
{ 
  if(supervisor) aReg[code&7]=usp;
  else
    { 
      exception=8;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
NEXT;
}

IDECL(movea_w)
{
  *((w32*)((Ptr)aReg+((code>>7)&28)))=LongFromWord(GetFromEA_w[(code>>3)&7]());
NEXT;
}

IDECL(movea_l)
{ 
  *((w32*)((Ptr)aReg+((code>>7)&28)))=GetFromEA_l[(code>>3)&7]();
NEXT;
}

IDECL(movea_l_an)
{ 
  *((w32*)((Ptr)aReg+((code>>7)&28)))=aReg[code&7];
NEXT;
}

IDECL(movem_save_w)
{ 
  register uw16 mask;
  register w32 ea;
  register short i;
  w8      eaMode;

  mask=RW(pc++);
  eaMode=((w8)code>>3)&7;
  if(eaMode==4)   /* predecrement mode */
    { 
      ea=aReg[eaMode=(code&7)];
      if((ea&1)!=0) WriteWord(ea,0); /* bad address */
      else
	{ 
	  for(i=15;mask!=0;mask>>=1,i--)
	    { 
	      if((mask&1)!=0) WriteWord(ea-=2,(w16)reg[i]);
	    }
	  aReg[eaMode]=ea;
	}
    }
  else
    { 
      ea=ARCALL(GetEA,eaMode,(code&7));
      if((ea&1)!=0) WriteWord(ea,0); /* bad address */
      else
	{ 
	  for(i=0;mask!=0;mask>>=1,i++)
	    { 
	      if((mask&1)!=0)
		{ 
		  WriteWord(ea,(w16)reg[i]);
		  ea+=2;
		}
	    }
	}
    }
NEXT;
}

IDECL(movem_save_l)
{ 
  register uw16 mask;
  register w32 ea;
  register short i;
  w8      eaMode;

  mask=RW(pc++);
  eaMode=((w8)code>>3)&7;
  if(eaMode==4)   /* predecrement mode */
    { 
      ea=aReg[eaMode=(code&7)];
      if((ea&1)!=0) WriteLong(ea,0); /* bad address */
      else
	{ 
	  for(i=15;mask!=0;mask>>=1,i--)
	    { 
	      if((mask&1)!=0) WriteLong(ea-=4,reg[i]);
	    }
	  aReg[eaMode]=ea;
	}
    }
  else
    { 
      ea=ARCALL(GetEA,eaMode,(code&7));
      if((ea&1)!=0) WriteLong(ea,0); /* bad address */
      else
	{ 
	  for(i=0;mask!=0;mask>>=1,i++)
	    { 
	      if((mask&1)!=0)
		{ 
		  WriteLong(ea,reg[i]);
		  ea+=4;
		}
	    }
	}
    }
NEXT;
}

IDECL(movem_load_w)
{ 
  register uw16 mask;
  register w32 ea;
  register short i;
  w8      eaMode,eaReg;

  mask=RW(pc++);
  eaMode=((w8)code>>3)&7;
  eaReg=code&7;
  ea=(eaMode==3)? aReg[eaReg]:ARCALL(GetEA,eaMode,(eaReg));
  if((ea&1)!=0) ReadWord(ea); /* bad address */
  else
    { 
      for(i=0;mask!=0;mask>>=1,i++)
	{ 
	  if((mask&1)!=0)
	    { 
	      reg[i]=LongFromWord(ReadWord(ea));
	      ea+=2;
	    }
	}
      if(eaMode==3) aReg[eaReg]=ea;
    }
NEXT;
}

IDECL(movem_load_l)
{ 
  register uw16 mask;
  register w32 ea;
  register short i;
  w8      eaMode,eaReg;

  mask=RW(pc++);
  eaMode=((w8)code>>3)&7;
  eaReg=code&7;
  ea=(eaMode==3)? aReg[eaReg]:ARCALL(GetEA,eaMode,(eaReg));
  if((ea&1)!=0) ReadLong(ea); /* bad address */
  else
    { 
      for(i=0;mask!=0;mask>>=1,i++)
	{ 
	  if((mask&1)!=0)
	    { 
	      reg[i]=ReadLong(ea);
	      ea+=4;
	    }
	}
      if(eaMode==3) aReg[eaReg]=ea;
    }
NEXT;
}

IDECL(movep_w_mr)   /* word from memory */
{ 
  register w32 ea;
  register w8* dn;

#if 0
  printf("movep\n"); 
  DbgInfo(); 
  /*BackTrace(10);*/
#endif
  
  
  ea=aReg[code&7]+(w32)((w16)RW(pc++));
#ifdef QM_BIG_ENDIAN
  dn=(w8 *)reg+((code>>7)&28)+RWO;
  *dn++=ReadByte(ea);
  ea+=2;
  *dn=ReadByte(ea);
#else

  dn=(w8 *)reg+((code>>7)&28)+MSB_W;
  *dn--=ReadByte(ea);
  ea+=2;
  *dn=ReadByte(ea);

#endif
NEXT;
}

IDECL(movep_l_mr)   /* long from memory */
{ 
  register w32 ea;
  register w8* dn;

  /*  printf("movep\n"); */

  ea=aReg[code&7]+(w32)((w16)RW(pc++));
#ifdef QM_BIG_ENDIAN
  dn=(w8 *)reg+((code>>7)&28);
  *dn++=ReadByte(ea);
  ea+=2;
  *dn++=ReadByte(ea);
  ea+=2;
  *dn++=ReadByte(ea);
  ea+=2;
  *dn=ReadByte(ea);
#else
  dn=(w8 *)reg+((code>>7)&28)+MSB_L;
  *dn--=ReadByte(ea);
  ea+=2;
  *dn--=ReadByte(ea);
  ea+=2;
  *dn--=ReadByte(ea);
  ea+=2;
  *dn=ReadByte(ea);
#endif
NEXT;
}

IDECL(movep_w_rm)   /* word to memory */
{ 
  register w32 ea;
  register w8* dn;

  /*    printf("movep\n"); 
   DbgInfo(); */
   /*BackTrace(10);*/
   

  ea=aReg[code&7]+(w32)((w16)RW(pc++));
#ifdef QM_BIG_ENDIAN
  dn=(w8 *)reg+((code>>7)&28)+RWO;
  WriteByte(ea,*dn++);
  ea+=2;
  WriteByte(ea,*dn);
#else

  dn=(w8 *)reg+((code>>7)&28)+MSB_W;
  WriteByte(ea,*dn--);
  ea+=2;
  WriteByte(ea,*dn);

#endif
NEXT;
}

IDECL(movep_l_rm)   /* long to memory */
{ 
  register w32 ea;
  register w8* dn;

  /* printf("movep\n"); */

  ea=aReg[code&7]+(w32)((w16)RW(pc++));
#ifdef QM_BIG_ENDIAN
  dn=(w8 *)reg+((code>>7)&28);
  WriteByte(ea,*dn++);
  ea+=2;
  WriteByte(ea,*dn++);
  ea+=2;
  WriteByte(ea,*dn++);
  ea+=2;
  WriteByte(ea,*dn);
#else
  dn=(w8 *)reg+((code>>7)&28)+MSB_L;
  WriteByte(ea,*dn--);
  ea+=2;
  WriteByte(ea,*dn--);
  ea+=2;
  WriteByte(ea,*dn--);
  ea+=2;
  WriteByte(ea,*dn);
#endif
NEXT;
}

IDECL(moveq)
{ 
  register w32 d;
  d=LongFromByte((w8)code);
  *((w32*)((Ptr)reg+((code>>7)&28)))=d;
  negative=d<0;
  zero=d==0;
  overflow=carry=false;
NEXT;
}

IDECL(muls)
{ 
  register w32 *d;
  d=(w32*)((Ptr)reg+((code>>7)&28));
  *d=*((w16*)((Ptr)d+RWO))*(w32)GetFromEA_w[(code>>3)&7]();
  zero=*d==0;
  negative=*d<0;
  overflow=carry=false;
NEXT;
}

IDECL(mulu)
{ 
  register uw32 *d;
  d=(uw32*)((Ptr)reg+((code>>7)&28));
  *d=*((uw16*)((Ptr)d+RWO))*(uw32)((uw16)GetFromEA_w[(code>>3)&7]());
  zero=*d==0;
  negative=(w32)(*d)<0;
  overflow=carry=false;
NEXT;
}

IDECL(nbcd)
{ 
  w8      d,r;
  w8      d2,r2;
  d=ModifyAtEA_b((code>>3)&7,code&7);
  d2=((d&0x0f)>9? 9:(d&0x0f));
  d>>=8;
  if(d>9) d2+=90; else d2+=d*10;
  carry=d2!=0;
  r2=100-d2;
  if(xflag) r2--;
  xflag=carry;
  zero=zero && r2==0;
  r=(r2%10)+((r2/10)<<4);
  RewriteEA_b(r);
NEXT;
}

IDECL(neg_b)
{ 
  w8 r,d;
  d=ModifyAtEA_b((code>>3)&7,code&7);
  r=-d;
  negative=r<0;
  zero=r==0;
  xflag=carry=((d|r)&0x80)!=0;
  overflow=(0x80&r&d)!=0;
  RewriteEA_b(r);
NEXT;
}

IDECL(neg_w)
{ 
  w16 r,d;
  D_ISREG;
  d=ModifyAtEA_w((code>>3)&7,code&7);
  r=-d;
  negative=r<0;
  zero=r==0;
  xflag=carry=((d|r)&0x8000)!=0;
  overflow=(0x8000&r&d)!=0;
  RewriteEA_w(r);
NEXT;
}

IDECL(neg_l)
{ 
  w32 r,d;
  D_ISREG;
  d=ModifyAtEA_l((code>>3)&7,code&7);
  r=-d;
  negative=r<0;
  zero=r==0;
  xflag=carry=((d|r)&0x80000000)!=0;
  overflow=(0x80000000&r&d)!=0;
  RewriteEA_l(r);
NEXT;
}

IDECL(negx_b)
{ 
  w8 r,d;
  d=ModifyAtEA_b((code>>3)&7,code&7);
  r=-d;
  if(xflag) r--;
  negative=r<0;
  zero=zero && r==0;
  xflag=carry=((d|r)&0x80)!=0;
  overflow=(0x80&r&d)!=0;
  RewriteEA_b(r);
NEXT;
}

IDECL(negx_w)
{ 
  w16 r,d;
  D_ISREG;
  d=ModifyAtEA_w((code>>3)&7,code&7);
  r=-d;
  if(xflag) r--;
  negative=r<0;
  zero=zero && r==0;
  xflag=carry=((d|r)&0x8000)!=0;
  overflow=(0x8000&r&d)!=0;
  RewriteEA_w(r);
NEXT;
}

IDECL(negx_l)
{ 
  w32 r,d;
  D_ISREG;
  d=ModifyAtEA_l((code>>3)&7,code&7);
  r=-d;
  if(xflag) r--;
  negative=r<0;
  zero=zero && r==0;
  xflag=carry=((d|r)&0x80000000)!=0;
  overflow=(0x80000000&r&d)!=0;
  RewriteEA_l(r);
NEXT;
}

IDECL(nop)
{
NEXT;
}

IDECL(not_b)
{ 
  register w8 d;
  d=ModifyAtEA_b((code>>3)&7,code&7)^0xff;
  zero=d==0;
  negative=d<0;
  overflow=carry=false;
  RewriteEA_b(d);
NEXT;
}

IDECL(not_w)
{ 
  register w16 d;
  D_ISREG;
  d=ModifyAtEA_w((code>>3)&7,code&7)^0xffff;
  zero=d==0;
  negative=d<0;
  overflow=carry=false;
  RewriteEA_w(d);
NEXT;
}

IDECL(not_l)
{ 
  register w32 d;
  D_ISREG;
  d=ModifyAtEA_l((code>>3)&7,code&7)^0xffffffff;
  zero=d==0;
  negative=d<0;
  overflow=carry=false;
  RewriteEA_l(d);
NEXT;
}

IDECL(or_b_dn)
{ 
  register w8     *d;
  d=(w8*)((Ptr)reg+((code>>7)&28)+RBO);
  *d=*d|GetFromEA_b[(code>>3)&7]();
  negative=*d<0;
  zero=*d==0;
  carry=overflow=false;
NEXT;
}

IDECL(or_w_dn)
{ 
  register w16    *d;
  d=(w16*)((Ptr)reg+((code>>7)&28)+RWO);
  *d=*d|GetFromEA_w[(code>>3)&7]();
  negative=*d<0;
  zero=*d==0;
  carry=overflow=false;
NEXT;
}

IDECL(or_l_dn)
{ 
  register w32    *d;
  d=(w32*)((Ptr)reg+((code>>7)&28));
  *d=*d|GetFromEA_l[(code>>3)&7]();
  negative=*d<0;
  zero=*d==0;
  carry=overflow=false;
NEXT;
}

IDECL(or_b_ea)
{ 
  register w8     d;
  d=ModifyAtEA_b((code>>3)&7,code&7);
  d=d|*((w8*)((Ptr)reg+((code>>7)&28)+RBO));
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_b(d);
NEXT;
}

IDECL(or_w_ea)
{ 
  register w16    d;
  D_ISREG;
  d=ModifyAtEA_w((code>>3)&7,code&7);
  d=d|*((w16*)((Ptr)reg+((code>>7)&28)+RWO));
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_w(d);
NEXT;
}

IDECL(or_l_ea)
{ 
  register w32    d;
  D_ISREG;
  d=ModifyAtEA_l((code>>3)&7,code&7);
  d=d|*((w32*)((Ptr)reg+((code>>7)&28)));
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_l(d);
NEXT;
}

IDECL(ori_b)
{ 
  register w8     d;
  d=(w8)RW(pc++);
  d=d|ModifyAtEA_b((code>>3)&7,code&7);
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_b(d);
NEXT;
}

IDECL(ori_w)
{ 
  register w16    d;
  D_ISREG;
  d=(w16)RW(pc++);
  d=d|ModifyAtEA_w((code>>3)&7,code&7);
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_w(d);
NEXT;
}

IDECL(ori_l)
{ 
  register w32    d;
  D_ISREG;
  d=RL((w32*)pc);
  pc+=2;
  d=d|ModifyAtEA_l((code>>3)&7,code&7);
  negative=d<0;
  zero=d==0;
  carry=overflow=false;
  RewriteEA_l(d);
NEXT;
}

IDECL(ori_to_ccr)
{ 
  register w16 d;
  d=(w16)RW(pc++);
  carry=carry||((d&1)!=0);
  overflow=overflow||((d&2)!=0);
  zero=zero||((d&4)!=0);
  negative=negative||((d&8)!=0);
  xflag=xflag||((d&16)!=0);
NEXT;
}

IDECL(ori_to_sr)
{ 
  register w16 d;
  d=(w16)RW(pc++);
  if(supervisor)
    { 
      PutSR(GetSR()|d);
    }
  else
    { 
      exception=8;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
NEXT;
}



IDECL(code1111)
{
#ifdef IE_XL
  qlux_table[code]();
#else   
  exception=11;
  extraFlag=true;
  nInst2=nInst;
  nInst=0;
#endif

NEXT;
}


IDECL(InvalidCode)
{ 
  exception=4;
  extraFlag=true;
  nInst2=nInst;
  nInst=0;
  pc--; /* ???? sembra che al QL piaccia cos=EC! Altrimenti i
	   breakpoint non funzionano */
NEXT;
}

IDECL(code1010)
{   
#ifdef IE_XL
  qlux_table[code]();
#else 
  exception=10;
  extraFlag=true;
  nInst2=nInst;
  nInst=0;
#endif

NEXT;
}

