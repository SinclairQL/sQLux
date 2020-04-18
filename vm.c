/*
 * (c) UQLX - see COPYRIGHT
 */

#include "QL68000.h"
#include "unix.h"
#include "xcodes.h"

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "qx_proto.h"

extern int main();
extern int strcasecmp();
extern uw32 rtop_hard;
extern int is_patching;
extern int rtc_emu_on;

char *scrModTable;
int faultaddr;
int vm_ison=0;

uw32 vm_saved_rom_value;
uw32 vm_saved_rom_addr=131072;  /* IMPROTANT */
int vm_saved_nInst=0;

int vmfatalretry=0;

void vmMarkScreen(uw32 addr)
{
  uw32 i=addr;
  if (i>=qlscreen.qm_lo && i<=qlscreen.qm_hi)
    {
      i=PAGEI((w32)(i-qlscreen.qm_lo));
      if (scrModTable[i]) return;
  
      scrModTable[i]=1;

      uqlx_protect(PAGEX(PAGEI(addr)),pagesize,QX_RAM);
    }
}

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
  vm_ison=1;
}

void vm_off(void)
{
  vm_ison=0;
}

