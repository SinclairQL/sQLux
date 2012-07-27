/*
 * (c) UQLX - see COPYRIGHT
 */


#include "QL68000.h" 

#include "vm_linux.h"

extern void segv_generic(long p,int signr);

#if  defined(VM_SCR) || defined(USE_VM)


void segv_handler(int signr, struct sigcontext_struct sigc)
{
  struct sigcontext_struct *sc=&sigc;
  long i;
#if 0
  printf("faulting address: %d, %x, -theROM  = %d, %x\n",sc->cr2,sc->cr2,
	 (unsigned long)sc->cr2-(unsigned long)theROM, 
	 (unsigned long)sc->cr2-(unsigned long)theROM );
#endif

  /*i=(long)sc->cr2-(long)theROM;*/
  segv_generic(sc->cr2,signr);
}


/// MPROTECT for WinCE
#   define PROTECT(addr, len) \
    	  if (mprotect((caddr_t)(addr), (size_t)(len), \
    	      	       PROT_READ | OPT_PROT_EXEC) < 0) { \
    	    ABORT("mprotect failed"); \
    	  }
#   define UNPROTECT(addr, len) \
    	  if (mprotect((caddr_t)(addr), (size_t)(len), \
    	  	       PROT_WRITE | PROT_READ | OPT_PROT_EXEC ) < 0) { \
    	    ABORT("un-mprotect failed"); \
    	  }
    	  
///

    static DWORD protect_junk;
#   define PROTECT(addr, len) \
	  if (!VirtualProtect((addr), (len), PAGE_EXECUTE_READ, \
	  		      &protect_junk)) { \
	    DWORD last_error = GetLastError(); \
	    GC_printf1("Last error code: %lx\n", last_error); \
	    ABORT("VirtualProtect failed"); \
	  }
#   define UNPROTECT(addr, len) \
	  if (!VirtualProtect((addr), (len), PAGE_EXECUTE_READWRITE, \
	  		      &protect_junk)) { \
	    ABORT("un-VirtualProtect failed"); \
	  }
///	  

/// 
# if defined(MSWIN32) || defined(MSWINCE)
long WINAPI GC_write_fault_handler(struct _EXCEPTION_POINTERS *exc_info)
#   define SIG_OK (exc_info -> ExceptionRecord -> ExceptionCode == \
			STATUS_ACCESS_VIOLATION)
#   define CODE_OK (exc_info -> ExceptionRecord -> ExceptionInformation[0] == 1)
{
  register unsigned i;			/* Write fault */
  char * addr = (char *) (exc_info -> ExceptionRecord
			  -> ExceptionInformation[1]);
  segv_generic(addr);
# endif


#endif  //defined(VM_SCR) || defined(USE_VM)
