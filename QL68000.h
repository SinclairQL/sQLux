#include <SDL_endian.h>
#include <stdint.h>

#ifndef QL68000_H
#define QL68000_H

/*
 * (c) UQLX - see COPYRIGHT
 */


#ifndef STATIC
#define STATIC
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

typedef int8_t w8;
typedef int16_t w16;
typedef int32_t w32;
typedef uint8_t uw8;
typedef uint16_t uw16;
typedef uint32_t uw32;

typedef unsigned char Cond;
typedef Cond Boolean;
typedef void* Ptr;

/* use the wide type because otherwise gcc will promote *every*
 * arg and return value */
typedef w16 ashort;
typedef w8 aw8,rw8,ruw8;
typedef w16 aw16,rw16,ruw16;
typedef w32 aw32,rw32,ruw32;
typedef uw16 gshort;
typedef unsigned char rCond;

typedef int shindex;

typedef int bctype;

typedef void* Ptr;     /* non ANSI, but convenient... */

struct qFloat
{
   uw16    exp;
   uw32    mant;
};

/* end QLtypes.h */
#define FIX_INS

extern void (**qlux_table)(void);

typedef int OSErr;

#ifndef NULL
#define NULL (void *)0
#endif

extern int gKeyDown, shiftKey, controlKey, optionKey, alphaLock, altKey;

extern w32              reg[16];
extern w32              usp,ssp;

extern uw16 *pc;
extern gshort code;
extern int nInst;    /* dangerous - it is 'volatile' to some extent */

#if defined(__x86_64__) || defined(__aarch64__)
#define HUGE_POINTER
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

#define   aReg  (reg+8)
#define   m68k_sp    (aReg+7)

#define SETREG16(_ra_,_val_) ({w16 *dn; dn=(w16*)(RWO+(char*)&_ra_); *dn=_val_;})

#ifdef ZEROMAP
#define memBase          ((w32*)0)
#else
extern w32              *memBase;
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

extern char             dispScreen;
extern Cond             dispMode;
extern Cond             badCodeAddress;

#define RM_SHIFT pageshift

extern int MPROTECT(void *,long, int);

extern int schedCount;
#define INCR_SC() {schedCount++;}
#define DECR_SC() {if (schedCount>0) schedCount--;}

extern w32  displayFrom;
extern w32  displayTo;

extern int isMinerva;

#ifndef vml
#define vml static
#endif

// FIXME: Remove these
#define REGP1
#define REGP2

#define AREGP
#define ARCALL(_farray_,_index_,_args_...)  \
                  ( _farray_[_index_](_args_) )

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

rw16 GetSR(void);
void PutSR(aw16 ) REGP1;
rw16 BusErrorCode(aw16 ) REGP1;
void SetPC(w32 )  REGP1;
void SetPCX(int )  REGP1;

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

#define WB(_addr_,_val_)(*(uw8*)(_addr_)=(_val_))
#define RB(_addr_) (*(uw8 *)(_addr_))

static inline ruw16 q2hw(uw16 val)
{
  return SDL_SwapBE16(val);
}
static inline ruw32 q2hl(uw32 val)
{
	return SDL_SwapBE32(val);
}
static inline ruw16 h2qw(uw16 v)
{
	return SDL_SwapBE16(v);
}
static inline ruw32 h2ql(uw32 v)
{
	return SDL_SwapBE32(v);
}

static inline ruw16 _rw_(uw16 *s)
{
	return SDL_SwapBE16(*s);
}
static inline ruw32 _rl_(uw32 *s)
{
	return SDL_SwapBE32(*s);
}

static inline void _ww_(uw16 *d, uw16 v)
{
	*d = SDL_SwapBE16(v);
}

static inline void _wl_(uw32 *d, uw32 v)
{
	*d = SDL_SwapBE32(v);
}

#ifndef RW
#define RW(_r_a) _rw_((void *)(_r_a))
#endif
#ifndef RL
#define RL(_r_al) _rl_((void *)(_r_al))
#endif
#ifndef WW
#define WW(_r_a, _r_v) _ww_((void *)(_r_a), (_r_v))
#endif
#ifndef WL
#define WL(_r_al, _r_vl) _wl_((void *)(_r_al), (_r_vl))
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
	uint32_t val1 = ((uintptr_t)_val_) >> 32;
	uint32_t val2 = ((uintptr_t)_val_) & 0xFFFFFFFF;

	WL(_addr_,val1);
	WL(_addr_+4,val2);
}
static inline void *GET_POINTER(w32* _addr_)
{
	uintptr_t val=((((uintptr_t)RL(_addr_))<<32 ) | (uint32_t)RL(_addr_+4));

	return (void *)val;
}
#else
#define SET_POINTER(_addr_,_val_)  (WL(_addr_,(w32)_val_))
#define GET_POINTER(_addr_)  ((void *)RL(_addr_))
#endif




extern void QLtrap(int ,int ,int );
extern void QLvector(int , int );

extern int script;
extern Cond doTrace;

#endif
