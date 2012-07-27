/*
 * (c) UQLX - see COPYRIGHT
 */

#include "QL68000.h"
#include "vm.h"
#include "unix.h"
#include "xcodes.h"

#if defined(VM_SCR) || defined(USE_VM) || defined(ZEROMAP)
#include <sys/mman.h>
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "qx_proto.h"

#ifdef SPARC
#include <sys/ucontext.h>
#include <sys/regset.h>
#endif

 
/*
     catch VM errors that resulted from read/write to QL HW/special memory areas
*/

char *oldscr;
long pagesize;
int pageshift;


extern int main();
extern int strcasecmp();
extern uw32 rtop_hard;
extern int is_patching;
extern int rtc_emu_on;

char *scrModTable;
int sct_size;
int faultaddr;
int vm_ison=0;

uw32 vm_saved_rom_value;
uw32 vm_saved_rom_addr=131072;  /* IMPROTANT */
int vm_saved_nInst=0;

int vmfatalretry=0;

#if defined(USE_VM) || defined(VM_SCR)
volatile int hwRegsEmu=0;
static int rcnt=30;         /* how many HW writes to report */


#ifdef SPARC
static ucontext_t *suap;
void set_ninst(val)
{
  nInst=val;
  suap->uc_mcontext.gregs[REG_G7] =val;
}
#define  SET_nInst(_val_) set_ninst(_val_)
#endif

#ifndef SET_nInst
#define  SET_nInst(_val_)  {nInst=_val_;}
#endif

char segv_emsg[]="Serious problem in segv_generic, exiting without cleanup\n";
char bus_msg[]="Fatal Buserror\n";
/* takes fault-addr and signr */
void segv_generic(long p,int signr)
{
    uw32 i;
    int mtype;
    static int segv_nest;

    segv_nest++;
    if (segv_nest>3)
    {

      write(2,segv_emsg,sizeof(segv_emsg));
      _exit(44);
    }


    if (p<(long)theROM || p>((long)theROM+rtop_hard))
	{
	  printf("unexpected segfault, faultaddr %x, QL-REL %x\n",p,p-(long)theROM);
	  on_fat_int(signr);
	}


    i=(long)p-(long)theROM;
    mtype=RamMap[i>>pageshift];

    //printf("memtype: %d\n",mtype);

    if (QX_SCR==mtype)
      {
	uw32 ix;

	ix=i-qlscreen.qm_lo;
	ix=(w32)(ix)/pagesize;
	scrModTable[ix] = 1;
	/*uqlx_protect(qlscreen.qm_lo+ix*pagesize,pagesize,QX_RAM);*/
	if (MPROTECT((Ptr)theROM+qlscreen.qm_lo+ix*pagesize,pagesize,PROT_READ | PROT_WRITE) <0)
	  perror("fatal failure in SIGSEGV handler - unable to change permission ");
      }
    /* catch all acesses to HW Regs from ROM. Obviously progs outside ROM */
    /* should not access the HW Regs, at least not for writing */
#ifdef USE_VM
    else if (QX_IO==mtype ||
	     is_patching && (QX_NONE==mtype || QX_ROM==mtype) )
      {
	uw32 addr;
	uw16* orig_pc=pc;
	int cnt=0;
	
	if (rtc_emu_on) 
	  {/* try that, if it was something else it will trap again*/
	    set_rtc_emu();
	    goto out;;
	  }

	if ( (long)pc-(long)theROM <=96*1024+10 ) /* within ROM ?*/
	  {
	    while(cnt++<8)
	      if (RW(--pc) == code) break;
	    if ( RW(pc) != code )
	      {
		printf("instruction decoding error in vm.c: cnt %d, orig_pc %x, pc %x code %x \n",cnt,(long)orig_pc-(long)theROM,(long)pc-(long)theROM,code);
		DbgInfo();
		exit(44);
		on_fat_int(signr);
	      }
	    addr=((uw32)((long)pc-(long)theROM));
	    add_patch_data(addr);
	    printf("added patch for addr %x, faultaddr %x\n",addr,i);
	    if (is_patching)
	      {
		printf("restarting emulator with new patch\n");
		restart_emulator();
	      }
	    else 
	      {
		uw16 ocode;
		printf("...not restarting emulator, which may not work\n");
		ocode=RW(((Ptr)theROM+(addr)));
		if (!mprotect((Ptr)theROM+((addr>>pageshift)<<pageshift),pagesize,PROT_READ|PROT_WRITE))
		  {
		    WW(((Ptr)theROM+(addr)),REGEMU_CMD_CODE);
		    uqlx_prestore((addr>>pageshift)<<pageshift,pagesize);
		  }
		add_patch(addr,ocode);
		goto try_hw;
	      }
	  }
	else
	  {
	  try_hw:
#if 0
	    if (rcnt > 0)
	      {
		rcnt --;
		printf("Program attempted %s HW registers %x\n",(hwRegsEmu ? "read" : "write"), addr);
		dbginfo("This can be probably ignored, but here is some debugging info:\n");
	      }
#endif
	    hwRegsEmu=1;
	    vm_saved_nInst=nInst;
	    SET_nInst(0);
	    MPROTECT((Ptr)theROM+0x18000,pagesize,PROT_READ | PROT_WRITE);
	  }
      }
    /* ROM protection and dummy memory emulation, also hit during startup for
       extended screen memory areas */
#endif /* USE_VM */
#if defined(USE_VM) || defined(VM_SCR)
    else if ( QX_ROM==mtype || QX_NONE==mtype)
      {
	/* tricky because there is no way to "ignore" writes in a portable way */
	vm_saved_rom_value=RL((Ptr)theROM+i);
	vm_saved_rom_addr=i;
	vm_saved_nInst=nInst;
	SET_nInst(0);
	DbgInfo();
	printf("disabling ROM protection at %x\n",((i>>pageshift)<<pageshift));
	if (MPROTECT((Ptr)theROM+((i>>pageshift)<<pageshift),pagesize,PROT_READ | PROT_WRITE) <0)
	  perror("fatal failure in SIGSEGV handler - unable to change permission ");
      }
#endif /* USE_VM */
    else
      { 
	printf("VM access error at %x , QL-REL %x, memory type %d\n",p,i,mtype);
	on_fat_int(signr);
      }
    
 out:
    segv_nest--;
}

#if defined(linux) 

#if defined(__i486__)||defined(__arm__)
void buserr_handler(int signr, long dummy /* struct sigcontext_struct sigc*/)
{
  write(2,bus_msg,strlen(bus_msg));
  on_fat_int(signr);
}
#if 0 // moved to vm_linux.c
void segv_handler(int signr, long dummy /*struct sigcontext_struct sigc*/)
{
  struct sigcontext_struct *sc=&dummy/*&sigc*/;
  long i;
#if 0
  printf("faulting address: %d, %x, -theROM  = %d, %x\n",sc->cr2,sc->cr2,
	 (unsigned long)sc->cr2-(unsigned long)theROM, 
	 (unsigned long)sc->cr2-(unsigned long)theROM );
#endif

  /*i=(long)sc->cr2-(long)theROM;*/
  //segv_generic(sc->cr2,signr);
  segv_generic(segv_addr(sc),signr);
}
#endif
#endif /* i486 */
#ifdef m68k

static fatal_error=0;
void buserr_handler(int signr, long dummy/*, struct sigcontext sigc */)
{
  if (fatal_error++==1) exit(45);
  if (fatal_error>1) raise(9);
  write(2,bus_msg,strlen(bus_msg));
  on_fat_int(signr);
}
#if 0
void segv_handler(int signr, int vecnum, long dummy /*struct sigcontext *scp*/)
{
/*  unsigned long ea;*/
/*   int format = (scp->sc_formatvec >> 12) & 0xf; */
/*   Frame *framedata = (struct frame *)(scp + 1); */

/*   vecnum = (vecnum & 0xfff) >> 2; */
  
/*      /*printf("exception vec %d, fmt %d\n",vecnum,format);*/ */
/*   switch (format) */
/*     { */
/*       case 4: ea=framedata->fmt4.effaddr; */
/* 	if ( (framedata->fmt4.pc) & (1<<27)) */
/* 	  ea=PAGEX(PAGEI(ea+4)); */
/* 	break; */
/*       case 7: ea=framedata->fmt7.faddr;  */
/* #if 0 */
/*        printf("segv: ssw=%x, faddr= %x, \n\twb2s=%x, wb2a=%x, wb3s=%x, wb3a=%d\n" */
/*               "MA=%x, RW=%x\n ", */
/* 	      framedata->fmt7.ssw, framedata->fmt7.faddr, */
/* 	      framedata->fmt7.wb2s, framedata->fmt7.wb2a, */
/* 	      framedata->fmt7.wb3s, framedata->fmt7.wb3a, */
/* 	      framedata->fmt7.ssw&MA_040, framedata->fmt7.ssw&RW_040 */
/* 	      ); */
/* #endif */
/* 	 if (framedata->fmt7.ssw&MA_040) { */
/* 	     ea=PAGEX(PAGEI(ea+4)); */
/* 	     /*printf("MA set\n");*/ */
/* 	       } */
/*        break; */
/*       case 0xa: ea=framedata->fmta.daddr; break; */
/*       case 0xb: ea=framedata->fmtb.daddr; break; */
/*     default: */
/*       printf("illegal exception format\n"); */
/*       on_fat_int(signr); */
/*     } */
  segv_generic(segv_addr(&dummy),signr);
}
#endif
#endif /* m68k */
#endif /* linux*/
#endif /*USE_VM || VM_SCR */

#if 0  /* dead code for SPARC */
#if defined(VM_REGS) && defined(GREGS) 
void segv_handler(int signr, siginfo_t info, ucontext_t *uap)
#endif
#endif

#if (defined(USE_VM) || defined(VM_SCR)) && (defined(SPARC) || defined(SGI))
#ifdef SPARC
     /*void segv_handler(int signr, siginfo_t info)*/
void segv_handler(int signr, siginfo_t info, ucontext_t *uap)
#else /* must be sg */
void segv_handler(int signr, siginfo_t *infop)
#endif /* SPARC or sgi */
{
  int i,j;
#ifndef sgi
  siginfo_t *infop=&info; 
#endif
#ifdef SPARC
  suap=uap;
#endif
  
  if (signr!=SIGSEGV)
    {
      printf("wrong signal %d in SIGSEGV handler\n",signr);
      return;      
    }
  segv_generic(infop->si_addr,signr);
}


void buserr_handler(int signr, siginfo_t info)
{
  if(signr!=SIGBUS)
    {
      printf("wrong signal %d in SIGBUS handler\n",signr);
      return;      
    }
  printf("bus error:\n");
  DbgInfo();
  abort();

#ifdef VM_SCR
  switch(info.si_code)
    {
    case BUS_ADRALN: printf("wrong alignment\n");
      break;
    case BUS_ADRERR: printf("nonexistent address\n");
      break;
    default: printf("unknown reason\n");
      break;
    }
  printf("at address %x , QL relative %x\n",info.si_addr,(Ptr)info.si_addr-(Ptr)theROM);
  printf("faddr: %x",info.si_addr);
#endif /* VM_SCR ???*/
}
#endif  /* SPARC,sgi & USE_VM */




#if defined(USE_VM) || defined(VM_SCR)
void prepChangeMem(w32 from, w32 to)
{
  w32 lolx,hilx;
  int i;
  static int bsul=0;
  
  /*printf("prepChangeMem %x to %x\n",from, to);*/

  if(from>to)
    {
      w32 tmp;
      printf("warning: from>to\n");
      tmp=from; from=to; to=tmp;
    }

      if ( (from<qlscreen.qm_hi && from>=qlscreen.qm_lo) ||
           (to<qlscreen.qm_hi && to>=qlscreen.qm_lo))
	{
	  lolx = from <qlscreen.qm_lo ? qlscreen.qm_lo : from;
	  hilx = to >qlscreen.qm_hi ? qlscreen.qm_hi : to;
	  
	  for(i=0;i<sct_size;i++) 
	    if (qlscreen.qm_lo+PAGEX(i)>=lolx &&
		qlscreen.qm_lo+PAGEX(i)<=hilx) scrModTable[i]=1;
	  
	  lolx= PAGEX(PAGEI(lolx));
	  hilx= PAGEX(PAGEI((hilx+pagesize-1)));
	  
	  if (MPROTECT((Ptr)theROM+lolx, (hilx-lolx), PROT_READ | PROT_WRITE) <0)
	    {
	      perror("warning: unable to change VM permission, io to screen may fail ");
	    }
	}
}
#endif

  
  
void vmMarkScreen(uw32 addr)
{
  uw32 i=addr;
  if (i>=qlscreen.qm_lo && i<=qlscreen.qm_hi)
    {
      i=PAGEI((w32)(i-qlscreen.qm_lo));
      if (scrModTable[i]) return;
  
      scrModTable[i]=1;

#if defined(USE_VM) || defined(VM_SCR)
      if (MPROTECT((Ptr)theROM+qlscreen.qm_lo+i*pagesize,pagesize,PROT_READ | PROT_WRITE) <0)
	{
	  perror("fatal failure in SIGSEGV handler - unable to change permission ");
	  printf("pagesize %d, QL address %d, screen page number %d\n",pagesize,131072+i*pagesize,i);
	  abort();
	}
#endif
#if 1
      uqlx_protect(PAGEX(PAGEI(addr)),pagesize,QX_RAM);
#else
      i = (i>>RM_SHIFT);
      RamMap[i]=QX_RAM;
#endif
    }
}


#if  defined(USE_VM) || defined(VM_SCR) || defined(ZEROMAP)
void vm_init(void)
{
  int res;
  int scratch;
  int i;
  
#if 0
  printf("addr of:\t main \t%x\n\t\t strcasecmp \t%x\n\t\t read \t%x\n",main,strcasecmp,read);
#endif

#ifndef sgi
  res=sysconf(_SC_MEMORY_PROTECTION); 
  if (!res){
    printf("sorry, there seems to be no VM support\n recompile without USE_VM\n ");
    exit(0);
  }
#endif
  

#ifdef ZEROMAP
  i=(long)mmap(0x0,16*1024*1024+16*pagesize,PROT_READ|PROT_WRITE,MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
  if (i!=0)
    {
      perror("couldn't mmap QL memory at addr 0x0");
      printf("try removing '-DZEROMAP' from .config\n");
      exit(1);
    }
#else
  scratch=(int) malloc(16*1024*1024);   /* alloc security gap */
  theROM=(void*)malloc(/*RTOP*/ 16*1024*1024+16*pagesize);
  if (!theROM)
    {
      printf("could not allocate space for QDOS memory!");
      abort();
    }
  free((void*)scratch);


  i=((long)theROM/pagesize);
  theROM=(w32 *)((i+4)*pagesize);

#endif

}
#endif

void vm_setscreen()
{   
  int i,xsize;
  
  xsize=PAGEX(PAGEI(qlscreen.qm_len+pagesize-1));

  sct_size=PAGEI(xsize);
  
  scrModTable=(void*)malloc(sct_size+1);
  for (i=0 ; i<sct_size ; i++) scrModTable[i]=1;  /* hack, force init display */
 
  oldscr=(void*)malloc(xsize);
  memset(oldscr,255,xsize);    /* force first screen draw !*/
}


void vm_on(void)
{
  int res;
  int i,x=0;
  
#if 0
  printf("turn write protection on\n");
#endif
  

  vm_ison=1;
  
#if 1 /*def USE_VM */
  uqlx_prestore(0,96*1024);
  uqlx_prestore(0x18000,pagesize); /* currently not needed */
#else
  if(MPROTECT((Ptr)theROM,96*1024,PROT_READ)<0) /* ROM */
    perror("sorry, could not enable ROM protection");

  if(MPROTECT((Ptr)theROM+0x18000,pagesize,PROT_NONE)<0) /* HWREGS */
    perror("sorry, could not enable HW register protection");
#endif



#if 1 /*defined(USE_VM) || defined(VM_SCR)*/
  uqlx_prestore(qlscreen.qm_lo,qlscreen.qm_len);
#else
  for(i=0; i<sct_size; i++) 
    if(!scrModTable[i])
      if(MPROTECT((char*)theROM+qlscreen.qm_lo+PAGEX(i),pagesize,PROT_READ)<0)  
	perror("sorry, could not enable screen write protection");
#endif
}

void vm_off(void)
{
#if  defined(USE_VM) || defined(VM_SCR)
  if (MPROTECT(theROM,131072+32768,PROT_READ | PROT_WRITE)<0)
    perror("could not remove memory protection");

  if (qlscreen.qm_lo!=131072)
    if (MPROTECT((Ptr)theROM+qlscreen.qm_lo,qlscreen.qm_len,PROT_READ | PROT_WRITE)<0)
      perror("could not remove memory protection");
#endif  
  vm_ison=0;
}




#if defined(USE_VM) || defined(VM_SCR)
int MPROTECT(void *p,long s, int how)
{
#if 0
  printf("mprotect: %x, %x, %x\n",(Ptr)p-(Ptr)theROM,s,how);
#endif
  return mprotect(p,s,how);
}
#endif


