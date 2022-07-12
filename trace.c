/*
 * (c) UQLX - see COPYRIGHT
 */


#include "QL68000.h"
#include <stdio.h>

#ifdef TRACE

uw16 *tracelo;
uw16 *tracehi;

struct TRT 
{
  uw16 *low;
  uw16 *high;
  char *comment;
} tracetable[]={
#if 1 // ROM
                {0,16384*3,"ROM"},
#endif
#if 0
                {0xf7938,0xf7a38,"Qsave"},
#endif
#if 0
                {0x1fb800,0x1ff14a,"Ptr_gen"},
#endif
#if 0
                {0x36d4,0x3720,"Verify Name"},
                {0x32a2,0x3378,"trap#2"},
		{0x355a,0x36b6,"dirdev open"},
#endif
#if 0
                {0x3ef6,0x405e,"vect $F0"},
		{0x4a0c,0x4a4a,"RI.NEG"},
		{0x497e,0x4a00,"RI.DIV"},
		{0x48de,0x497e,"RI.MULT"},
		{0x3e54,0x3ea8,"CN.ITOD"},
#endif
		/* the end marker, always should be there */
		{0,0,0}};

/* Ptr Backtrace: */

struct TRT *curr=NULL;

struct BTE {
  uw16 *where;       /* absolute address */
  uw16 *to;          /* relative address */
  int what;          /* event type       */
} backtrace[100];

#define BACKTRSIZE 100
struct BTE *btcurr=backtrace;
int btnochng=1;



void TraceInit()
{
  CheckTrace();
}


void AddBackTrace(Ptr p, int type)
{ 
  uw16 *lpc=(uw16*)((Ptr)pc-(Ptr)memBase);
  if (btcurr>backtrace+BACKTRSIZE) btcurr=backtrace;
  
  btcurr->where=p;
  btcurr->to=lpc;
  btcurr->what=type;

  btcurr++;
  btnochng=0;
}


void CheckTrace()
{
  struct TRT *p;
  uw16 *lpc=(uw16*)((Ptr)pc-(Ptr)memBase);


  p=tracetable;
  curr=0;
  
  while((p->low) || (p->high))
    {
      if ( ((p->low <=lpc) && (p->high >=lpc)) ||
	  (p->low >= lpc) && ( curr==NULL || ((p->low) <= (curr->low))))
	curr=p;
      p++;
    }
  if (curr)
    {
      tracelo=(uw16*)((Ptr)(curr->low)+(long)memBase);
      tracehi=(uw16*)((Ptr)(curr->high)+(long)memBase);
    }
  else tracelo=(uw16*)((Ptr)RTOP+(long)memBase); 
}

void DoTrace()
{
  /*printf("entering Dotrace, pcl %x pch %x pc%x\n",tracelo,tracehi,pc);*/
  
  if (pc>tracehi)
    {
      CheckTrace();
      return;
    }
  
  printf("Trace : %s+%x\n",curr->comment,(Ptr)pc-(Ptr)(curr->low)-(long)memBase);
  DbgInfo();
}
 

void BTShowException(int xc)
{    
  short i;
  int p1,p4;
  unsigned char *p3;

  long ixc=xc;

 
  if(ixc==4)
    { 
      p3="Illegal code";
     }
  else
    { 
      p3="";
      if(ixc==2) p3="bus error";
      if(ixc==3) p3="address error";
      if(ixc==5) p3="divide by zero";
      if(ixc==6) p3="CHK instruction";
      if(ixc==7) p3="TRAPV instruction";
      if(ixc==8) p3="privilege violation";
      if(ixc==9) p3="trace xc";
      if(ixc==10) p3="Axxx instruction code";
      if(ixc==11) p3="Fxxx instruction code";
      if(xc>=32 && xc<=32+15) {printf("\tTRAP #%d\t",xc-32);return;}
      if(xc>=24 && xc<=24+7){printf("\tInterrupt #%d\t",xc-24);return;}
         }
  printf("\tException %s \t",p3);
}

void BackTrace(int depth)
{
  struct BTE *p;
  char *evname;
  int what,x=0;
  
  
  printf("BackTrace:\n");
  if (btnochng) {
    printf("\tunchanged\n");
    return;
  }
  btnochng=1;
  p=btcurr;
  
  if (depth>BACKTRSIZE) depth=BACKTRSIZE;
  
   for(;depth--;p--)
    {
      if (p<backtrace) p=backtrace+BACKTRSIZE;
    
      what=p->what;
      if (what>0){
	switch(p->what){
	case RTS : evname="RTS";
	  break;
	case RTE : evname="RTE";
	  break;
	case RTR : evname="RTR";
	  break;
	case JSR : evname="JSR";
	  break;
	case BSR : evname="BSR";
	  break;
	default : evname="unknown";
	}
	printf("\t %s\t",evname);
      }
      else BTShowException(-what);
      
      printf("at PC=%x, new pc=%x\n",(Ptr)(p->where)-(Ptr)memBase,p->to);
    }
}



#endif /* TRACE */
