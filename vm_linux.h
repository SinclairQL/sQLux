/*
 * (c) UQLX - see COPYRIGHT
 */

#ifndef VM_LINUX_H
#define VM_LINUX_H

#include <signal.h>


#if  defined(VM_SCR) || defined(USE_VM)

#if defined(linux) && defined(m68k)
typedef union {
    struct {
      unsigned long  iaddr;	/* instruction address */
    } fmt2;
    struct {
      unsigned long  effaddr;	/* effective address */
    } fmt3;
    struct {
      unsigned long  effaddr;	/* effective address */
      unsigned long  pc;	/* fslw or pc of faulted instr */
    } fmt4;
    struct {
      unsigned long  effaddr;	/* effective address */
      unsigned short ssw;	/* special status word */
      unsigned short wb3s;	/* write back 3 status */
      unsigned short wb2s;	/* write back 2 status */
      unsigned short wb1s;	/* write back 1 status */
      unsigned long  faddr;	/* fault address */
      unsigned long  wb3a;	/* write back 3 address */
      unsigned long  wb3d;	/* write back 3 data */
      unsigned long  wb2a;	/* write back 2 address */
      unsigned long  wb2d;	/* write back 2 data */
      unsigned long  wb1a;	/* write back 1 address */
      unsigned long  wb1dpd0;	/* write back 1 data/push data 0*/
      unsigned long  pd1;	/* push data 1*/
      unsigned long  pd2;	/* push data 2*/
      unsigned long  pd3;	/* push data 3*/
    } fmt7;
    struct {
      unsigned long  iaddr;	/* instruction address */
      unsigned short int1[4];	/* internal registers */
    } fmt9;
    struct {
      unsigned short int1;
      unsigned short ssw;	/* special status word */
      unsigned short isc;	/* instruction stage c */
      unsigned short isb;	/* instruction stage b */
      unsigned long  daddr;	/* data cycle fault address */
      unsigned short int2[2];
      unsigned long  dobuf;	/* data cycle output buffer */
      unsigned short int3[2];
    } fmta;
    struct {
      unsigned short int1;
      unsigned short ssw;	/* special status word */
      unsigned short isc;	/* instruction stage c */
      unsigned short isb;	/* instruction stage b */
      unsigned long  daddr;	/* data cycle fault address */
      unsigned short int2[2];
      unsigned long  dobuf;	/* data cycle output buffer */
      unsigned short int3[4];
      unsigned long  baddr;	/* stage B address */
      unsigned short int4[2];
      unsigned long  dibuf;	/* data cycle input buffer */
      unsigned short int5[3];
      unsigned	   ver : 4;	/* stack frame version # */
      unsigned	   int6:12;
      unsigned short int7[18];
    } fmtb;
} Frame;


#ifndef _ASM_M68k_SIGCONTEXT_H
#define _ASM_M68k_SIGCONTEXT_H

struct sigcontext {
	unsigned long  sc_mask; 	/* old sigmask */
	unsigned long  sc_usp;		/* old user stack pointer */
	unsigned long  sc_d0;
	unsigned long  sc_d1;
	unsigned long  sc_a0;
	unsigned long  sc_a1;
	unsigned short sc_sr;
	unsigned long  sc_pc;
	unsigned short sc_formatvec;
	unsigned long  sc_fpregs[2*3];  /* room for two fp registers */
	unsigned long  sc_fpcntl[3];
	unsigned char  sc_fpstate[216];
};

#endif

/* bits for 68040 special status word */
#define CP_040	(0x8000)
#define CU_040	(0x4000)
#define CT_040	(0x2000)
#define CM_040	(0x1000)
#define MA_040	(0x0800)
#define ATC_040 (0x0400)
#define LK_040	(0x0200)
#define RW_040	(0x0100)
#define SIZ_040 (0x0060)
#define TT_040	(0x0018)
#define TM_040	(0x0007)

/* bits for 68040 write back status word */
#define WBV_040   (0x80)
#define WBSIZ_040 (0x60)
#define WBBYT_040 (0x20)
#define WBWRD_040 (0x40)
#define WBLNG_040 (0x00)
#define WBTT_040  (0x18)
#define WBTM_040  (0x07)

/* bus access size codes */
#define BA_SIZE_BYTE    (0x20)
#define BA_SIZE_WORD    (0x40)
#define BA_SIZE_LONG    (0x00)
#define BA_SIZE_LINE    (0x60)

/* bus access transfer type codes */
#define BA_TT_MOVE16    (0x08)

/* bits for 68040 MMU status register (mmusr) */
#define MMU_B_040   (0x0800)
#define MMU_G_040   (0x0400)
#define MMU_S_040   (0x0080)
#define MMU_CM_040  (0x0060)
#define MMU_M_040   (0x0010)
#define MMU_WP_040  (0x0004)
#define MMU_T_040   (0x0002)
#define MMU_R_040   (0x0001)

/* bits in the 68060 fault status long word (FSLW) */
#define	MMU060_MA	(0x08000000)	/* misaligned */
#define	MMU060_LK	(0x02000000)	/* locked transfer */
#define	MMU060_RW	(0x01800000)	/* read/write */
# define MMU060_RW_W	(0x00800000)	/* write */
# define MMU060_RW_R	(0x01000000)	/* read */
# define MMU060_RW_RMW	(0x01800000)	/* read/modify/write */
# define MMU060_W		(0x00800000)	/* general write, includes rmw */
#define	MMU060_SIZ	(0x00600000)	/* transfer size */
#define	MMU060_TT	(0x00180000)	/* transfer type (TT) bits */
#define	MMU060_TM	(0x00070000)	/* transfer modifier (TM) bits */
#define	MMU060_IO	(0x00008000)	/* instruction or operand */
#define	MMU060_PBE	(0x00004000)	/* push buffer bus error */
#define	MMU060_SBE	(0x00002000)	/* store buffer bus error */
#define	MMU060_PTA	(0x00001000)	/* pointer A fault */
#define	MMU060_PTB	(0x00000800)	/* pointer B fault */
#define	MMU060_IL	(0x00000400)	/* double indirect descr fault */
#define	MMU060_PF	(0x00000200)	/* page fault (invalid descr) */
#define	MMU060_SP	(0x00000100)	/* supervisor protection */
#define	MMU060_WP	(0x00000080)	/* write protection */
#define	MMU060_TWE	(0x00000040)	/* bus error on table search */
#define	MMU060_RE	(0x00000020)	/* bus error on read */
#define	MMU060_WE	(0x00000010)	/* bus error on write */
#define	MMU060_TTR	(0x00000008)	/* error caused by TTR translation */
#define	MMU060_BPE	(0x00000004)	/* branch prediction error */
#define	MMU060_SEE	(0x00000001)	/* software emulated error */

/* cases of missing or invalid descriptors */
#define MMU060_DESC_ERR	(MMU060_TWE | MMU060_PTA | MMU060_PTB | \
						 MMU060_IL  | MMU060_PF)
/* bits that indicate real errors */
#define MMU060_ERR_BITS	(MMU060_PBE | MMU060_SBE | MMU060_DESC_ERR | \
						 MMU060_SP  | MMU060_WP  | MMU060_RE | \
						 MMU060_WE)

#endif /* linux & m68k */

#endif

#if  defined(USE_VM) 

typedef struct STATE {
  uw16 code;
  void *pc;
  uw32 save_aReg[8];
 }x_state;

x_state orig_state,res_state;


extern long pagesize;
extern char *scrModTable;
extern int sct_size;
extern char * oldscr;


extern void buserr_handler(int  );
//extern void segv_handler(int );
extern void segv_handler(int, siginfo_t*, void*);

extern void vm_on(void);
extern void vm_off(void);
#endif

#endif
