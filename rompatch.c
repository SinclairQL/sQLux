/*
 * (c) UQLX - see COPYRIGHT
 */

/* read list of patches from patchfile and apply */

#include "QL68000.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include <fcntl.h> 
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


#include "xcodes.h"  
#include "QL_config.h" 
#include "QInstAddr.h"
#include "unix.h"
#include "qx_proto.h"

#if defined(USE_VM) || defined(VM_SCR)

static uw32 rom_csum[4];
FILE *pdb_f;

#define PTABLE_SIZE 800
static struct {
  uw32 addr;
  uw16 code;
} patchtable[PTABLE_SIZE];
static int patchentries=0; /* how many are used */

extern char **argv;

void getRomCS(uw32 start, int len, uw32 *csum)
{
  uw32 *p;
  char buf[5];
  
  for (p=(uw32*)((char*)theROM+start); (char*)p<(char*)theROM+len+start;)
    {
      csum[0] ^= *p++;
      csum[1] ^= *p++;
      csum[2] ^= *p++;
      csum[3] ^= *p++;
    }
  if (V3)printf("ROM checksum 0x%08x,0x%08x,0x%08x,0x%08x\n",csum[0],csum[1],csum[2],csum[3]);
}

static char p[PATH_MAX];
FILE *open_patch_dbase()
{
  char d[PATH_MAX];
  char *p1;
  FILE *f;

  if (strlen(getenv("HOME"))+23>PATH_MAX)
    {
      printf("PATH_MAX too low\n");
      exit(1);
    }
  p1 = (char*)stpcpy(p, getenv("HOME"));
  if(*(p1-1) != '/')
    {
      *p1++ = '/';
    }
  strcpy(p1, ".uqlxpatch");
  strncpy(d,p,PATH_MAX);    /* dir name */
  mkdir(d,0755);
  sprintf(d,"/rom_%x_%x_%x_%x",rom_csum[0],rom_csum[1],rom_csum[2],rom_csum[3]);
  strncat(p,d,PATH_MAX);

  f=fopen(p,"a+");
  if (!f)
    {
      printf("FATAL: could not access patch database %s\n",p);
      exit(1);
    }
  fseek(f,0,SEEK_SET);
  return f;
}

/* Patch Database has this Format:
# all numbers are hexnums with 0x prefix, addresses relative in QL space
#entry header:
ROM hexnum32,hexnum32,hexnum32,hexnum32  # entry header with csum
hexnum32,hexnum16                           # addr, unpatched value
# repeated as needed
#end of entry
# next entry or end of file
 */

void add_patch_data(uw32 addr)
{
  char buf[128];

  sprintf(buf,"  0x%08x,0x%04x\n",addr,RW((Ptr)theROM+addr));
  write(fileno(pdb_f),buf,strlen(buf));
  /*fflush(pdb_f);*/
}

static int foundrom=0;
/* read in patch data (addr,code) terminated by EOF or start of new entry */
static void getpdata(FILE *f,int ismain)
{
  char buf[256];
  char *p=buf;
  uw32 addr,code;

  foundrom=1;
  while (!feof(f))
    {
      if (!fgets(buf,256,f)) return;
      if (strlen(buf)<8) continue;
      while (strchr(" \t\n",*p)) p++;
      if (*p=='#') continue;
      if (strstr(buf,"ROM")) return;
      if ( 2!=sscanf(p," %10x,%6x",&addr,&code))
	{
	  printf("problem parsing patch database, line '%s'",buf);
	  continue;
	}
      if (addr>(48+32)*1024)
	  printf("patch address %x out of bounds\n",addr);

      patchtable[patchentries].addr=addr;
      patchtable[patchentries++].code=code;
      if (patchentries >= PTABLE_SIZE)
	{
	  printf("PTABLE_SIZE to low, adjust in rompatch.c\n");
	  exit(1);
	}
    }
}
      

/* look entry header, verify checksum */
static int readentrydb(FILE *f)
{
  char buf[256];
  char *p=buf;

  uw32 lcsum[4];

  while (!feof(f))
    {
      if (!fgets(buf,256,f)) return 0;
      p=buf;
      while (strchr(" \t\n",*p)) p++;
      if (*p=='#' || *p==0 ) continue;
      if (!strncasecmp("ROM",p,3))
	{
	  p += 3;
	  sscanf(p," %10x,%10x,%10x,%10x",&lcsum[0],&lcsum[1],&lcsum[2],&lcsum[3]);
	  if ( rom_csum[0]==lcsum[0] && rom_csum[1]==lcsum[1] &&
	       rom_csum[2]==lcsum[2] && rom_csum[3]==lcsum[3] )
	    getpdata(f,1);
	  else continue;
	  }
    }
}
static void read_dbase(FILE *f)
{
  while(!feof(f))
    readentrydb(f);
}

/* given corrected addr, return original 'code' */
uw16 find_ocode(uw32 addr)
{
  int i;

  for (i=0; i< patchentries; i++)
    {
      if (patchtable[i].addr==addr)
	{
	  /*printf("addr=%x, code=%x\n",patchtable[i].addr,patchtable[i].code);*/
	  return patchtable[i].code; 
	}
    }
  printf("illegal RegEmuCmd() at %x command\n",addr);
}


void add_patch(uw32 addr,uw16 ocode)
{
  if (patchentries+1<PTABLE_SIZE)
    {
      patchtable[patchentries].addr=addr;
      patchtable[patchentries++].code=ocode;
    }
  else
    {
      printf("PTABLE_SIZE too small\n");
      exit(1);
    }
}

void instrumentCode()
{
  FILE *f;
  int i;

  getRomCS(0,(48+32)*1024,rom_csum);    

  f=open_patch_dbase();
  read_dbase(f);
  pdb_f=f;             /* may be used in segv handler */
  for(i=0; i<patchentries; i++)
    {
      if (RW((Ptr)theROM+patchtable[i].addr)==(patchtable[i].code))
	{
	  /*printf("adding patch at %x, orig-code %x\n",patchtable[i].addr,patchtable[i].code);*/
	  WW(((Ptr)theROM+(patchtable[i].addr)),REGEMU_CMD_CODE);
	}
      else
	{
	  printf("patch database mismatch at %x, purging pathcdatabase - needs to restart\n",
		 patchtable[i].addr);
	  unlink(p);
	  exit(1);
	}
    }
  fseek(f,SEEK_END,0); /* data may get appended */
  if ( !foundrom )  /* new entry needs o be created */
    {
      char buf[PATH_MAX];
      int error;

      fprintf(f,"\nROM 0x%08x,0x%08x,0x%08x,0x%08x\n" ,rom_csum[0],rom_csum[1],rom_csum[2],rom_csum[3]);
      fflush(f);
      sprintf(buf,"%s -p -s Kill_UQLX",argv[0]);
      while (error=system(buf));
      {
	if ((error==-1) || (error==-127))
	  {
	    perror("problem executing child");
	    printf("**** could not execute %s\n",buf);
	    exit(127);
	  }
	if (error==44)
	  {
	    printf("buggy compiler or another desaster encountered\n");
	    printf("can't proceed\n");
	    exit(3);
	  }
	if (error)
	  printf("child %s returned unexpectedly %d, retrying\n",buf,error);
	printf("*************************************************\n");
      }
      printf("\nWARNING: ROM was not yet in database, created new entry. Before\n"
	     "         using the emulator with this ROM, make sure to TEST\n"
	     "         all applications\n\n");
      restart_emulator();
    }
}
#else  /* just a dummy version */
void instrumentCode()
{
}
#endif
