/*
 * (c) UQLX - see COPYRIGHT
 */

#ifdef linux

#include <signal.h>

// hack, QL68000.h needed for machine defines
#include "QL68000.h" 

#include "vm_linux.h"

extern void segv_generic(long p,int signr);

#if  defined(VM_SCR) || defined(USE_VM)
#ifdef __i486__
void segv_handler(int signr, siginfo_t *info, void *c)
{
   segv_generic(info->si_addr, signr);
}
#endif /* i486 */

#ifdef __arm__

void segv_handler(int signr, siginfo_t *info, void *c)
{
   segv_generic(info->si_addr, signr);
}
#endif

#ifdef m68k
void segv_handler(int signr, int vecnum, struct sigcontext *scp)
{
 unsigned long ea;
  int format = (scp->sc_formatvec >> 12) & 0xf;
  Frame *framedata = (struct frame *)(scp + 1);

  vecnum = (vecnum & 0xfff) >> 2;
  
     /*printf("exception vec %d, fmt %d\n",vecnum,format);*/
  switch (format)
    {
      case 4: ea=framedata->fmt4.effaddr;
	if ( (framedata->fmt4.pc) & (1<<27))
	  ea=PAGEX(PAGEI(ea+4));
	break;
      case 7: ea=framedata->fmt7.faddr; 
#if 0
       printf("segv: ssw=%x, faddr= %x, \n\twb2s=%x, wb2a=%x, wb3s=%x, wb3a=%d\n"
              "MA=%x, RW=%x\n ",
	      framedata->fmt7.ssw, framedata->fmt7.faddr,
	      framedata->fmt7.wb2s, framedata->fmt7.wb2a,
	      framedata->fmt7.wb3s, framedata->fmt7.wb3a,
	      framedata->fmt7.ssw&MA_040, framedata->fmt7.ssw&RW_040
	      );
#endif
	 if (framedata->fmt7.ssw&MA_040) {
	     ea=PAGEX(PAGEI(ea+4));
	     /*printf("MA set\n");*/
	       }
       break;
      case 0xa: ea=framedata->fmta.daddr; break;
      case 0xb: ea=framedata->fmtb.daddr; break;
    default:
      printf("illegal exception format\n");
      on_fat_int(signr);
    }
  segv_generic(ea,signr);
}

#endif /* m68k */
#endif /* VM_SCR || USE_VM */
#endif /* linux */
