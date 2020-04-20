
/*
 * (c) UQLX - see COPYRIGHT
 */


#include "QL68000.h"
#include "QL_config.h"
#include "QInstAddr.h"

#include "qx_proto.h"


int tracetrap=0;

void QMExecuteLoop(uw16 *oldPC)  /* fetch and dispatch loop */
{     
  register void           (**tab)(void);

        tab=qlux_table;

	/*printf("enter QME \n");*/

rep:
        while(likely(--nInst>=0 && oldPC!=pc) /* && oldPC!=pc+1 && oldPC!=pc+2 */) 
	  { 
	    /*printf("PC=%x\n",(Ptr)pc-(Ptr)theROM); */
	    tab[code=RW(pc++)&0xffff]();
	  }
	
        if(extraFlag) 
        {       
	  nInst=nInst2;
	  /*
	  if (*reg==0x16)
	  printf("exception processing: exception %d, trap %d, d0=%x\n",exception,exception-32,*reg);
	  tracetrap=1;*/
	  
	  ExceptionProcessing();
	  if(nInst>0) goto rep;
        }
 
	/*printf("QME nInst: %d, oldPC %d, code %x\n",nInst, oldPC,code);*/
	tracetrap=0;
}

void QLchunk(w16 *oldPC,long n)       /* execute n emulated 68K istructions */
{  
  uw16 savePOLLM;
  int save_ninst;
  
  savePOLLM=ReadWord(0x28030);
  WriteWord(0x28030,0);
  
  if((long)pc&1) return;

  /*extraFlag=false;*/
  /*ProcessInterrupts();*/
  
  if(stopped) return;
  if (extraFlag==0) exception=0;

  save_ninst=nInst;
  nInst=n;
  if(extraFlag)
    {      
      nInst2=nInst;

      nInst=0;
    }
  
  QMExecuteLoop(oldPC);

  nInst=save_ninst;
  WriteWord(0x28030,savePOLLM);
}

void QLtrap(int t,int id,int nMax)
{
  /*printf("calling QLtrap #%d, d0=%x d1=%d d2=%d a0=%x a1=%x\n",t,id,reg[1],reg[2],aReg[0],aReg[1]);*/
  
  reg[0]=id;

  exception=32+t;
  extraFlag=true;
  
  QLchunk(pc,nMax);
    
  /*printf("returns : d0=%d d1=%d a0=%x a1=%x\n",reg[0],reg[1],aReg[0],aReg[1]);*/
}

void QLvector(int which, int nMax)
{
  uw32 ea;
  w16 *savedPC=pc;
  /*printf("calling QLvector %x\n",which);*/
  
  ea=ReadWord(which);
  
  WriteLong((*m68k_sp)-=4,(w32)((Ptr)pc-(Ptr)theROM));
  SetPC(ea);

  extraFlag=false;
  exception=0;
  
  QLchunk(savedPC,nMax);
  
}

void QLsubr(uw32 ea, int nMax)
{
  w16 *savedPC=pc;
  
  WriteLong((*m68k_sp)-=4,(w32)((Ptr)pc-(Ptr)theROM));
  SetPC(ea);

  extraFlag=false;
  exception=0;
  
  QLchunk(savedPC,nMax);
}
