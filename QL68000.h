#ifndef QL68000_H
#define QL68000_H

/*
 * (c) UQLX - see COPYRIGHT
 */


#ifndef STATIC
#define STATIC
#endif

/* needed for ntoh? functions */
#include <arpa/inet.h>

#if !defined(USE_VM) && !defined(EVM_SCR) && !defined(VM_SCR)
#error USE_VM, VM_SCR or EVM_SCR must be defined
#endif

#ifdef VM_SCR
#error VM_SCR is currently broken
#endif

#undef QM_BIG_ENDIAN

#if defined(sparc) || defined(__sparc)
#define SPARC
#endif

#if defined(__hp9000s800) || defined(hppa)
#define HPPA
#endif

#if defined(ppc) || defined(PPC) || defined(__PPC) || defined(__PPC__)
#ifndef PPC
#define PPC
#endif
#endif

#if defined(ppc) || defined(__PPC) || defined(__PPC__)
#define PPC
#endif

#ifdef PPC
#define QM_BIG_ENDIAN
#define HOST_ALIGN 1
#endif

#if defined(mips)
#define MIPS
#endif

#if defined(__alpha__)
#define ALPHA
#define HOST_ALIGN 1
#endif

#if defined(i386) || defined(i486) || defined(i586) || defined(i686) || defined(__i386__) ||defined(__i586__) || defined(__i686__)
#ifndef __i486__
#define __i486__
#endif
#endif

#if defined(m68000) || defined(_m68k_) || defined(mc68000) \
    || defined(__mc68000)
#define m68k
#endif

#ifdef USE_BUILTIN_EXPECT
#define likely(exp)   __builtin_expect((exp),1)
#define unlikely(exp) __builtin_expect((exp),0)
#else
#define likely(exp) (exp)
#define unlikely(exp) (exp)
#endif

/* 
 * QLtypes.h has been "inlined" here 'cause some typedefs are 
 * CPU dependent  now 
 */

#define true 1
#define false 0

typedef signed char w8;
typedef signed short w16;
typedef signed int w32;
typedef unsigned char uw8;
typedef unsigned short uw16;
typedef unsigned int uw32;

/* should probably apply for some other RISC chips as well */
#if defined(SPARC) || defined(HPPA)
typedef int Cond;  /* load/store CPUs deal badly with char arithmetic */
#else
typedef unsigned char Cond;
#endif

/* use the wide type because otherwise gcc will promote *every*
 * arg and return value */
#if 1
typedef int ashort;
typedef signed int aw8,aw16,aw32;
typedef signed int rw8,rw16,rw32;
typedef unsigned int ruw8,ruw16,ruw32;
typedef unsigned int gshort;
typedef unsigned int rCond;
#else
typedef short ashort;
typedef w8 aw8,rw8;
typedef w16 aw16,rw16;
typedef w32 aw32,rw32;
typedef unsigned short gshort;
typedef unsigned char rCond;
#endif

/* shindex is for quants that would fit into 16 bits but are used as index */
/* somewhere, most CPUs dont like 16 bit indexes  */
//#if defined(__i486__) || defined(SPARC) || defined(HPPA)
/* probably easier to tell the one which does */
#ifndef __mc68000
typedef int shindex;
#else
typedef short shindex;
#endif

/* must hold 8bit, used for comparison */
#ifdef m68k
typedef char bctype;
#else
typedef int bctype;  
#endif

#if 0 /*def SPARC*/
#define SMALLCACHE
#endif

typedef void* Ptr;     /* non ANSI, but convenient... */

struct qFloat
{
   uw16    exp;
   uw32    mant;
};

/* end QLtypes.h */
#define FIX_INS

extern void (**table)(void);

typedef int OSErr;
extern char *release;
extern char *uqlx_version;

#ifndef NULL
#define NULL (void *)0
#endif

extern int gKeyDown, shiftKey, controlKey, optionKey, alphaLock, altKey;

#ifdef m68k
#define G_reg
#endif

#ifndef G_reg 
extern w32              reg[16];
#endif
extern w32              usp,ssp;


#ifdef SPARC
#ifndef BROKEN_GREGS
register uw16  *pc asm("g5");
register gshort  code asm("g6");
register int   nInst asm("g7");
#define GREGS
#define MANYREGS
#endif
#endif

#ifdef m68k
// most m68k users will have my patched gcc
register w32     *reg asm("%a5");
register uw16    *pc asm("%a4");
register gshort  code asm("%d6");
register int     nInst asm("%d7");
#define GREGS
/*#define ASSGN_CODE(val) ((uw16)code=(uw16)val)*/
/* code is fixed to d6, MSW never used - set only lower 16 bit */
#define ASSGN_CODE(val) ({asm("move.w  %0,%%d6 \n" :: "m" (val) : "d6"); code;})
#endif

#ifdef __i486__
// compiler gets internal errors
#if 0
//register w32     *reg asm("ebp");
//register uw16    *pc asm("esi");
register uw16    *pc asm("ebx");
//register gshort  code asm("ebx");
register gshort  code asm("ebp");
register int     nInst asm("edi");
#define GREGS
#else
#define ASSGN_486() ({gshort tmp;                    \
                          asm("movzwl %1,%0 \n"          \
			      "rolw   $8,%0 \n"          \
                               : "=r" (tmp) : "m" (*pc++) : "cc"); \
                          code=tmp; })
#endif
#endif

#ifdef HPPA
#ifndef BROKEN_GREGS
register uw16    *pc asm("r5");
register gshort  code asm("r6");
register int     nInst asm("r7");
#define GREGS
#define MANYREGS
#endif
#endif

#ifdef MIPS
#ifndef BROKEN_GREGS
register uw16    *pc asm("$23");
register gshort  code asm("$22");
register int     nInst asm("$21");
#define GREGS
#define MANYREGS
#endif
#endif

#if defined(PPC)
#ifndef BROKEN_GREGS
register uw16    *pc asm("r14");
register gshort  code asm("r15");
register int     nInst asm("r16");
#define GREGS
#define MANYREGS
#endif
#endif

#ifdef ALPHA
#define HUGE_POINTER
#define MANYREGS
#endif

#ifndef GREGS
extern uw16 *pc;
extern gshort code;
extern int nInst;    /* dangerous - it is 'volatile' to some extent */
#endif

#ifdef DARWIN
// apple sux // really?? ;)
#define ASSGN_CODE(val) (code = (unsigned)(unsigned short)val)
#endif

#ifndef ASSGN_CODE
#define ASSGN_CODE(val) (code = val & 0xffff)
#endif

#define gPC  pc
#define gUSP usp
#define gSSP ssp

/* define the maximum amount of QL addressable memory, will wrap */
#define ADDR_MASK    0xffffff
#define ADDR_MASK_E  0xfffffe

#ifdef TRACE
extern uw16 *tracelo; 
extern uw16 *tracehi;
extern void CheckTrace(void);
extern void TraceInit(void);
extern void DoTrace(void);
#endif

extern int   nInst2;
extern Cond             trace,supervisor,xflag,negative,zero,overflow,carry;
extern char             iMask;
extern Cond             stopped;
extern volatile char    pendingInterrupt;

/* reduce register pressure */
#define NEW_AREG

#ifdef NEW_AREG
#define   aReg  (reg+8)
#define   sp    (aReg+7)
extern w32      *g_reg;
#else
extern w32              *sp;
extern w32              *aReg;
#endif

#define SETREG16(_ra_,_val_) ({w16 *dn; dn=(w16*)(RWO+(char*)&_ra_); *dn=_val_;})

#ifdef ZEROMAP
#define theROM          ((w32*)0)
#else
extern w32              *theROM;
#endif

//extern w32              *ramTop;
extern w32              RTOP;
extern short            exception;
extern w32              badAddress;
extern w16              readOrWrite;
extern w32              dummy;
extern Ptr              dest;
#if 1
#define MEA_DISP 1
#define MEA_HW 2
extern Cond mea_acc;
#else  
#ifndef VM_SCR
extern Cond             isDisplay;
#endif
extern Cond             isHW;
#endif
extern w32              lastAddr;
extern volatile Cond    extraFlag;
extern volatile w8      intReg;
extern volatile w8      theInt;

extern bctype           *RamMap;
extern char             dispScreen;
extern Cond             dispMode;
extern Cond             badCodeAddress;

/* RamMap now works with almost any pagesize.. */
#if 1
/* slower but more flexible than other case, speed irelevant in USE_VM case */
#define RM_SHIFT pageshift
#else
#ifdef SPARC
#define RM_SHIFT        13
#define RM_PAGESIZE    8192
#else
#ifndef RM_PAGESIZE
#define RM_SHIFT        12
#define RM_PAGESIZE     4096
#endif
#endif
#endif
extern int MPROTECT(void *,long, int);

extern int schedCount;
#define INCR_SC() {schedCount++;}
#define DECR_SC() {if (schedCount>0) schedCount--;}

//#ifdef USE_VM
//extern sigjmp_buf instr_retry_catch;
//#endif

extern w32  displayFrom;
extern w32  displayTo;

extern int isMinerva;

#ifndef vml
#define vml static
#endif

/* pass args for selected fns through registers */
#if defined(__i486__) && (!defined(NO_REGP))
#define REGP1 __attribute__ ((regparm(3)))
#define REGP2 __attribute__ ((regparm(3)))
#ifndef NO_AREGP  /* sometimes reason actually slower ??*/
#if __GNUC__>2 || ( __GNUC__==2 && __GNUC_MINOR__>=8) 
/*#define USE_AREGP*/  /* requires explicit setting */
#endif
#endif
#else
#define REGP1
#define REGP2
#endif

/* same, but for vectored functions gets rather ugly and dangerous,
 * known to break 2.7.2.1, 2.7.2.3 and posibly all versions earlier 2.8.1 */
#ifdef USE_AREGP
/* WARN: exactly matching prototypes assumed */
#define AREGP __attribute__ ((regparm(3)))
#define ARCALL(_farray_,_index_,_args_...)  \
                  ({ typeof(_farray_[0]) AREGP _fx_ = _farray_[_index_]; \
                     (*_fx_)(_args_);})
#else
#define AREGP
#define ARCALL(_farray_,_index_,_args_...)  \
                  ( _farray_[_index_](_args_) )
#endif

vml rw8 GetFromEA_rb_m3(ashort );
vml rw16 GetFromEA_rw_m3(ashort );
vml rw32 GetFromEA_rl_m3(ashort );


extern rw32      (*GetEA[8])(ashort) /*AREGP*/;      /**/
extern rw8       (*GetFromEA_b[8])(void);
extern rw16      (*GetFromEA_w[8])(void);
extern rw32      (*GetFromEA_l[8])(void);
extern void (*PutToEA_b[8])(ashort,aw8)  /*AREGP*/;  /**/
extern void (*PutToEA_w[8])(ashort,aw16) /*AREGP*/; /**/
extern void (*PutToEA_l[8])(ashort,aw32) /*AREGP*/; /**/
extern Cond (*ConditionTrue[16])(void) ;

rw8 ReadHWByte(aw32 )  REGP1;
void WriteHWByte(aw32, aw8) REGP1;
STATIC rw8 ReadByte(aw32 ) REGP1;
STATIC rw16 ReadWord(aw32 ) REGP1;
STATIC rw32 ReadLong(aw32 ) REGP1;
STATIC void WriteByte(aw32 ,aw8 ) REGP2;
STATIC void WriteWord(aw32 ,aw16 ) REGP2;
STATIC void WriteLong(aw32 ,aw32 ) REGP2;

#ifdef SMALLCACHE
#define inl_ReadByte ReadByte
#define inl_ReadWord ReadWord
#define inl_ReadLong ReadLong
#define inl_WriteByte WriteByte
#define inl_WriteWord WriteWord
#define inl_WriteLong WriteLong
#else
/* inline versions of those */
STATIC rw8 inline inl_ReadByte(aw32 ) REGP1;
STATIC rw16 inline inl_ReadWord(aw32 ) REGP1;
STATIC rw32 inline inl_ReadLong(aw32 ) REGP1;
STATIC void inline inl_WriteByte(aw32 ,aw8 ) REGP2;
STATIC void inline inl_WriteWord(aw32 ,aw16 ) REGP2;
STATIC void inline inl_WriteLong(aw32 ,aw32 ) REGP2;
#endif

rw16 GetSR(void);
void PutSR(aw16 ) REGP1;
rw16 BusErrorCode(aw16 ) REGP1;
void SetPC(w32 )  REGP1;
void SetPCX(int )  REGP1;

vml rw8      GetFromEA_b_m3(void);
vml rw8      GetFromEA_b_m4(void);
vml rw16     GetFromEA_w_m3(void);
vml rw16     GetFromEA_w_m4(void);
vml rw32     GetFromEA_l_m3(void);
vml rw32     GetFromEA_l_m4(void);
STATIC rw8 ModifyAtEA_b(ashort ,ashort )  REGP2;
STATIC rw16 ModifyAtEA_w(ashort ,ashort ) REGP2;
STATIC rw32 ModifyAtEA_l(ashort ,ashort ) REGP2;
STATIC void RewriteEA_b(aw8 )  REGP1;
STATIC void RewriteEA_w(aw16 ) REGP1;
STATIC void RewriteEA_l(aw32 ) REGP1;

void FrameInt(void);
void WriteInt(aw8) REGP1; 

void ExceptionIn(char) REGP1;
void ExceptionOut(void);
void UpdateNowRegisters(void);

Cond IPC_Command(void);
void WriteMdvControl(aw8) REGP1;

#define LongFromByte(__d__) ((w32)((w8)(__d__)))
#define LongFromWord(__d__) ((w32)((w16)(__d__)))
#define WordFromByte(__d__) ((w16)((w8)(__d__)))

#define nil (void*)0
/*int Error;*/
#define gError Error


extern long pagesize;
extern int pageshift;
extern char *scrModTable;
extern int sct_size;
extern char * oldscr;


#ifdef NO_PSHIFT
#define PAGEI(_x_)  ((int)(_x_)/pagesize)
#define PAGEX(_x_)  ((int)(_x_)*pagesize)
#else
#define PAGEI(_x_)  ((int)(_x_)>>pageshift)
#define PAGEX(_x_)  ((int)(_x_)<<pageshift)
#endif
#include "cond.h"
#include "trace.h"
#include "iexl.h"
#include "QDOS.h"

/* QL memory types */
#define QX_NONE      0
#define QX_ROM       1
#define QX_QXM       1
#define QX_RAM       3
#define QX_SCR       7
#define QX_IO        8

#ifdef SPARC
#define HOST_ALIGN 4
#define QM_BIG_ENDIAN
#endif

#ifdef sgi
#define HOST_ALIGN 4
#define QM_BIG_ENDIAN
#endif

#if defined(HPPA) 
#define QM_BIG_ENDIAN
#define HOST_ALIGN 4
#endif

#if defined(m68k) || defined(m68000) || defined(_m68k_) || defined(mc68000)
#define QM_BIG_ENDIAN
#define HOST_ALIGN 1   /* ignore 68000 */
#endif

#ifdef PPC
#define QM_BIG_ENDIAN
#define HOST_ALIGN 1
#endif

#define WB(_addr_,_val_)(*(uw8*)(_addr_)=(_val_))
#define RB(_addr_) (*(uw8 *)(_addr_))




#ifdef QM_BIG_ENDIAN


static inline ruw16 q2hw(uw16 val)
{
  return val;
}
static inline ruw32 q2hl(uw32 val)
{
  return val;
}

static inline ruw16 h2qw(uw16 v)
{
  return v;
}
static inline ruw32 h2ql(uw32 v)
{
  return v;
}

#if 0 /* turns out that macros are till faster than inline */
static inline void  _ww_(uw16 *addr, w16 d)
{
  *(uw16*)addr=d;
}
static inline uw16 _rl_(uw16 *addr)
{
  return *addr;
}
#else
#define WW(_addr_,_val_) (*(uw16*)(_addr_)=(_val_))
#define RW(_addr_) (*(uw16*)(_addr_))
#endif


#if (HOST_ALIGN>2)
static inline void _wl_(w32 *addr,w32 d)
{
  if ((long)addr&2)
    {
      *(uw16*)addr=d>>16;
      *(((uw16*)addr)+1)=d&0xffff;
    }
  else
    *addr=d;
}
static inline uw32 _rl_(w32 *addr)
{
  if ((long)addr&2)
    return (w32) (((*(uw16*)addr)<<16)|((*(uw16*)(2+(long)(addr)))));
  else
    return *addr;
}
#else /* HOST_ALIGN <=2 */
#define WL(_addr_,_val_) (*(uw32*)(_addr_)=(_val_))
#define RL(_addr_) (*(uw32*)(_addr_))
#endif

#else
/* little endian stuff comes here */

static inline ruw16  q2hw(uw16 val)
{
  return ((val&0xff)<<8)|((val>>8)&0xff);
}
static inline ruw32 q2hl(uw32 val)
{
  return ((val&0xff)<<24)|((val&0xff00)<<8)|((val>>8)&0xff00)|((val>>24)&0xff);
}
static inline ruw16 h2qw(uw16 v)
{
  return ((v&0xff)<<8)|((v>>8)&0xff);
}
static inline ruw32 h2ql(uw32 v)
{
  return ((v&0xff)<<24)|((v&0xff00)<<8)|((v>>8)&0xff00)|((v>>24)&0xff);
}

#if defined(__i486__) 

#define HOST_ALIGN 1

#define NEW_I486_MACROS

/* old macros had problems with egcs and suboptimal declarations*/
#ifdef NEW_I486_MACROS

static inline ruw16 _rw_(uw16 *s)
{	register uw16 x=*s;
	asm ( "rolw $8,%0\n" : "=c" (x) : "0" (x) : "cc");
	return x;
}
static inline ruw32 _rl_ (uw32 *a)
{
    uw32 retval;

    asm ("bswap %0" : "=r" (retval) : "0" (*a) : "cc");
    return retval;
}

static inline void _wl_ (uw32 *a, uw32 v)
{
   asm("bswapl %0" : "=r" (v) : "0" (v) : "cc");  /* bswap or bswapl !??!*/
   *a = v;
}
static inline void _ww_ (uw16 *a, uw16 v)
{
#ifdef X86_PPRO_OPT
    __asm__ ("bswapl %0" : "=&r" (v) : "0" (v << 16) : "cc");
#else
    __asm__ ("rolw $8,%0" : "=r" (v) : "0" (v) : "cc");
#endif
    *a = v;
}
#else
static inline ruw16 _rw_(uw16 *s)
{	register uw16 x=*s;
	asm ( "rolw $8,%0\n" : "=c" (x) : "0" (x));
	return x;
}
static inline ruw32 _rl_(uw32 *s)
{	register uw32 x=*s;
	asm ( "bswap %0\n" : "=S" (x) : "0" (x));
	return x;
}
static inline void _ww_(uw16 *d, uw16 v)
{	asm ( 	"xchgb %%ch,%%cl\n\t"
		"movw %%cx,(%%esi)\n"   
        : 
        : "c" (v), "S" (d)
        : "cx", "memory", "cc");
}
static inline void _wl_(uw32 *d, uw32 v)
{	asm (	"bswap %0\t\n"
		"movl %0,(%1)\n"   
	: 
	: "S" (v), "D" (d) 
	: "memory", "cc"
        );
}
#endif

#else /* not i486 */

static inline ruw16 _rw_(uw16 *s)
{
#if SWAP_LOAD_IN_MEMORY
	uw16* x = *(s);
	return ((x&0xff)<<8)|((x>>8)&0xff);
#else
	return ntohs(*s);
   //w8 *ss = (w8 *)s;
	//uw8 *su = (uw8 *)s;
	//return ((*(ss))<<8)|(*(su+1));
#endif
}
static inline ruw32 _rl_(uw32 *s)
{
#if SWAP_LOAD_IN_MEMORY
	uw32 x = *(s);
	return ((x&0xff)<<24)|((x&0xff00)<<8)|((x>>8)&0xff00)|((x>>24)&0xff);
#else
	return ntohl(*s);
   //w8 *ss = (w8 *)s;
	//uw8 *su = (uw8 *)s;
	//return ((*(ss))<<24)|((*(su+1))<<16)
	//		|((*(su+2))<<8)|((*(su+3)));
#endif
}


static inline void _ww_(uw16 *d, uw16 v)
{
#if SWAP_STORE_IN_MEMORY
	*d=((v&0xff)<<8)|((v>>8)&0xff);
#else
	*d = htons(v);
   //w8 *ds = (w8 *)d;
	//uw8 *du = (uw8 *)d;
	//*ds=v>>8;
	//*(du+1)=v&0xff;
#endif
}

static inline void _wl_(uw32 *d, uw32 v)
{
#if SWAP_STORE_IN_MEMORY
	*d=((v&0xff)<<24)|((v&0xff00)<<8)|((v>>8)&0xff00)|((v>>24)&0xff);
#else
	//w8 *ds = (w8 *)d;
	//uw8 *du = (uw8 *)d;
	//*ds= v>>24;
	//*(du+1)=(v>>16)&0xff;
	//*(du+2)=(v>>8)&0xff;
	//*(du+3)=v&0xff;
   *d = htonl(v);
#endif
}

#endif 



#endif /* QM_BIG_ENDIAN */

#ifndef RW
#define RW(_r_a)         _rw_((void *)(_r_a))
#endif
#ifndef RL
#define RL(_r_al)        _rl_((void *)(_r_al))
#endif
#ifndef WW
#define WW(_r_a,_r_v)    _ww_((void *)(_r_a), (_r_v))
#endif
#ifndef WL
#define WL(_r_al,_r_vl)  _wl_((void *)(_r_al),(_r_vl))
#endif

#define dbginfo(format,args...) {printf(format, ## args);\
                                 DbgInfo();}

		     
#ifdef QM_BIG_ENDIAN
#define RWO 2
#define UW_RWO 0
#define RBO 3
#define MSB_W  2
#define MSB_L  0
#else
#define RWO 0
#define UW_RWO 2
#define RBO 0
#define MSB_W  1
#define MSB_L  3
#endif

#ifdef HUGE_POINTER
void static inline SET_POINTER(w32*_addr_,void *_val_) 
{ 
  WL(_addr_,(((long)_val_)>>32)&0xffffffff);
  WL(_addr_+4,((long)_val_)&0xffffffff);
}
static inline void *GET_POINTER(w32* _addr_)  
{
  return ((void *) ((((long)RL(_addr_))<<32 )| (long)RL(_addr_+4)));
}
#else
#define SET_POINTER(_addr_,_val_)  (WL(_addr_,(w32)_val_)) 
#define GET_POINTER(_addr_)  ((void *)RL(_addr_))
#endif




extern void QLtrap(int ,int ,int );
extern void QLvector(int , int );


#include "QL_screen.h"

#ifdef QM_BIG_ENDIAN
#define big_endian 1
#else
#define big_endian 0
#endif

extern int script;
extern Cond doTrace;

#if defined(USE_VM) && !defined(VM_SCR)
#define MARK_SCREEN
#endif
#if defined(EVM_SCR)
#define MARK_SCREEN
#endif

#if defined(USE_VM) || defined(VM_SCR) 
void prepChangeMem(w32, w32);
#else
#define prepChangeMem
#endif

extern int verbose;
#define V1 (verbose>0)
#define V2 (verbose>1)
#define V3 (verbose>2)

/*#define inline*/

#include "misdefs.h"

#endif