#ifdef TEST

#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

long x;

void handler(int sig, siginfo_t *sip, struct sigcontext *scp)
{
  if ((long)(sip->si_addr) == (x+123))
    {
      printf("exception reporting with siginfo is ok\n");
      exit(0);
    }

  printf("wrong addr in siginfo struct %d, should be 123\n",sip->si_addr-x);
  exit(1);
}

main()
{
  struct sigaction sx;
  char *p=0;
  
  x=(long)mmap(0x0,17*1024*1024,PROT_READ,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
  
  if (x == -1)
    {
      perror("couldn´t mmap memory to test");
      exit(1);
    }

  sx.sa_handler=handler;
  sigemptyset(&sx.sa_mask);
  sx.sa_flags=SA_SIGINFO;

  sigaction(SIGSEGV,&sx,NULL);
  sigaction(SIGBUS,&sx,NULL);

  *(char*)(x+123)=0x11;
  printf("should not have reached this point\n");
  exit(1);
}

#endif /* TEST */
