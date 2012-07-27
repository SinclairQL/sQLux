#ifndef VM_H
#define VM_H

/*
 * (c) UQLX - see COPYRIGHT
 */





#ifdef SPARC
#include <sys/ucontext.h>
#include <sys/regset.h>
#endif

#if  defined(VM_SCR) || defined(USE_VM)
#include <signal.h>

typedef struct STATE {
  uw16 code;
  void *pc;
  uw32 save_aReg[8];
 }x_state;

x_state orig_state,res_state;



#if 0 /*defined(USE_VM) && defined(GREGS)*/
extern void segv_handler(int , siginfo_t  , ucontext_t *);
#else
#if defined(SPARC)
/*extern void segv_handler(int , siginfo_t );*/
extern void segv_handler(int , siginfo_t  , ucontext_t *);
#endif
#ifdef SPARC
extern void buserr_handler(int , siginfo_t );
#else
#ifdef linux
#ifdef __arm__
void segv_handler(int, siginfo_t*, void*);
void buserr_handler(int, long);
#endif
#ifdef __i486__
void segv_handler(int , long /*struct sigcontext_struct*/ );
void buserr_handler(int , long /*struct sigcontext_struct*/ );
#endif /* i486 */
#ifdef m68k
void buserr_handler(int , long /*struct sigcontext*/ );
void segv_handler(int , int , long /*struct sigcontext*  */);
#endif /*m68k*/
#else
extern void segv_handler(int , siginfo_t *, void*);
extern void buserr_handler(int , siginfo_t );
#endif
#endif
#endif
extern void vm_on(void);
extern void vm_off(void);
extern void vm_emureg(void);

#else 
#if  defined(USE_VM) 

typedef struct STATE {
  uw16 code;
  void *pc;
  uw32 save_aReg[8];
 }x_state;

x_state orig_state,res_state;


extern long pagesize;
//extern char *scrModTable;
//extern int sct_size;
extern char * oldscr;


extern void buserr_handler(int  );
extern void segv_handler(int );


extern void vm_on(void);
extern void vm_off(void);
#endif
#endif

#endif /* VM_H */