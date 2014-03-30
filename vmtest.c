/*
 * UQLX - test for bug that may crash 68040 based Linux
 */

#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


char *membase;
int pagesize=4096;
int pageshift=12;
int x_taken=0;

int sighandler(int sig)
{
   mprotect(membase,pagesize,PROT_READ|PROT_WRITE);
   x_taken=1;
}

main()
{
  long l;

   signal(SIGSEGV,sighandler);
   l=(long)mmap(NULL,pagesize,PROT_READ,MAP_PRIVATE | MAP_ANON,-1,0);
  if (l==-1)
     {
       perror("mmap/malloc");
       abort();
     }
  membase=(char*)l;
    *(long*)(membase+sizeof(long))=123456789;
  if (*(long*)(membase+sizeof(long)) != 123456789 )
    {
      fprintf(stderr,"writeback failed !\n");
      exit(1);
    }
  if (!x_taken)
    {
      fprintf(stderr,"exception not taken !\n");
      exit(1);
    }
  fprintf(stderr,"vmtest Ok\n");
  exit(0);
}



