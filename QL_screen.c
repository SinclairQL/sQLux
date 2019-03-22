/*
 * (c) UQLX - see COPYRIGHT
 */
#include <stdio.h>
#include "QL68000.h"
#include "QL_config.h"
#include "QL_screen.h"

screen_specs qlscreen = 
{
   128 * 1024, 
   128 * 1024 + 32 * 1024, 
   0x8000, 
   128, 
   256, 
   512,
   2
};

struct SCREENDEF
{
  uw32 scrbase;
  uw32 scrlen;
  uw16 linel;
  uw16 xres;
  uw16 yres;
};

static int PtrPatchOk=0;
void QLPatchPTRENV(void)
{
    w32 scrdef_search;
    //struct SCREENDEF *scrdef;
    int flag;

    scrdef_search=((Ptr)pc-8000) - (void *)theROM;

    while ( LookFor(&scrdef_search,0x20000,24000)) {
        struct SCREENDEF *scrdef = (void *)theROM + scrdef_search;

        if (RL(&(scrdef->scrlen))==0x8000 &&
	        RW(&(scrdef->linel)) ==0x80 &&
	        RW(&(scrdef->xres)) ==0x200 &&
	        RW(&(scrdef->yres)) ==0x100 ) {
	        PtrPatchOk=1;
	        WL(&(scrdef->scrbase),qlscreen.qm_lo);
	        WL(&(scrdef->scrlen),qlscreen.qm_len);
	        WW(&(scrdef->linel),qlscreen.linel);
	        WW(&(scrdef->xres),qlscreen.xres);
	        WW(&(scrdef->yres),qlscreen.yres);

	        return;
	    } else {
	        scrdef_search = (void *)scrdef_search + 2;
        }
    }

    if (!PtrPatchOk)
        printf("WARNING: could not patch Pointer Environment\n");
    else
        printf("Patched Pointer Environment with screen size\n");
}
