/*
 * (c) UQLX - see COPYRIGHT
 */



#include "QL68000.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "uxfile.h"

#define DUMMY(name,par)  void name(par){/*printf("warning: calling dummy function: %s \n",__FUNCTION__);*/}

size_t x_read(int fildes, void *buf, size_t byt)
{
  int res;

  do
    res=read(fildes,buf,byt);
  while(res<=0 && byt>0 && eretry());

  return res;
}

void ValidateDispByte(w32 addr){}

void debug(char *msg)
{
#ifdef DEBUG
printf("%s\n",msg);
#endif
}
void debug2(char *msg, long n)
{
#ifdef DEBUG
 printf("%s\t%x\n",msg,n);
#endif
}

/*#define DEBUG_IPC*/
void debugIPC(char *msg, long n)
{
#ifdef DEBUG_IPC
 printf("%s\t%x\n",msg,n);
 /*DbgInfo();*/
#endif
}




DUMMY(HFlushFile,void);
DUMMY(HQRead,void)
DUMMY(HRewriteHeader,void)
DUMMY(HKillFileTail,void)
DUMMY(HGetFileHeader,void)
DUMMY(HQWrite,void)

DUMMY(InstallSerial,void)
DUMMY(NoteAlert,void)
DUMMY(DiskEject,void)

DUMMY(CautionAlert,void)
DUMMY(MemError,void)
DUMMY(GetEOF,void)
DUMMY(StopAlert,void)

void ErrorAlert(int x){}
void CustomErrorAlert(char *x){}




#ifndef HAS_STPCPY
char *stpcpy(char *s1, const char *s2)
{
  strcpy(s1,s2);
  return s1+strlen(s1);
}
#endif

#define min(_a_,_b_)  (_a_<_b_ ? _a_ : _b_)


#ifdef NEEDS_STRNCPY
char *strncpy(char *dest, const char *src, size_t n)
{
  int slen=1+strlen(src);
  memcpy(dest,src,min(n,slen));
}
#endif


#ifndef SERIAL
Cond SetBaudRate(short dummy){}
#endif


void * NewPtr(long need)
{
  return (void *)malloc(need);
}


int Random()
{
#ifdef __WIN32__
  return rand();
#else
  return random();
#endif
}

#ifdef  NEED_STRNCASECMP
int strncasecmp(char *s1,char *s2,int len)
{
  while (*s1 || *s2 && len--)
    {
      if (tolower(*s1++)==tolower(*s2++)) continue;
      else return 1; /* don't care which is really greater */
    }
  if (tolower(*s1)==tolower(*s2)) return 0;    /* TRUE */
}
#endif


void BlockMoveData(void *source, void *dest,long len)
{
#ifndef NO_MEMMOVE
  memmove(dest,source,len);
#else
  bcopy(source,dest,len);
#endif
}

