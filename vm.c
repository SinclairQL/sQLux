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

extern int main();
extern int strcasecmp();
extern int is_patching;
extern int rtc_emu_on;

char *scrModTable;
int faultaddr;
int vm_ison=0;

uw32 vm_saved_rom_value;
uw32 vm_saved_rom_addr=131072;  /* IMPROTANT */
int vm_saved_nInst=0;

int vmfatalretry=0;

void vm_on(void)
{
  vm_ison=1;
}

void vm_off(void)
{
  vm_ison=0;
}

