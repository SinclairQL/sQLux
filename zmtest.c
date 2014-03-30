#include <unistd.h>
#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

main()
{
  char *p=0;
  long x=(long)mmap(0x0,16*1024*1024,PROT_READ|PROT_WRITE,MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
  if (x != 0)
    {
      perror("couldn´t mmap to zero");
      exit(1);
    }

  memset(0,1,1024*1024);
}
