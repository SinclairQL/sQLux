/*
 * (c) UQLX - see COPYRIGHT
 */

/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "QL.h"
#include "QLfiles.h"
#include "QFilesPriv.h"
#include "QInstAddr.h"
#include "QDisk.h"
#include "unix.h"

#include "driver.h"
#include "QDOS.h"
#include "qx_proto.h"


#include "xcodes.h"
#include "SDL2screen.h"

/* UQLX basic extensions */


extern int gKeyDown;


#define X_FUN 1
#define X_PROC 2

extern int UQLX_optind;
extern int UQLX_argc;
extern char **UQLX_argv;

typedef int bas_err;
struct BAS_PFENTRY 
{
  char *name;
  int type;              /* X_FUN, X_PROC*/
  bas_err (*command)();
  w32 code_addr;         /* where in QDOS memory is this fn */
  struct BAS_PFENTRY *link;
};

bas_err Kill_UQLX();
bas_err UQLX_Relse();
bas_err UQLX_getenv();
bas_err Fork_UQLX();
bas_err UQLX_getXargc();
bas_err UQLX_getXarg();
bas_err UQLX_getXres();
bas_err UQLX_getYres();
static bas_err MakeDir();


struct BAS_PFENTRY *ext_list=NULL;


void build_entry(w16 **tptr, struct BAS_PFENTRY *p, w16 **iptr)
{
  w16 *t;
  int l;
  

  /*printf("build_entry %s, code at %d\n",p->name,(long)(*iptr)-(long)theROM);*/
  WW(*iptr,BASEXT_CMD_CODE);  /* make the subr and */
  p->code_addr=(w32)((char*)*iptr-(char*)theROM);         /* store the entry addr */
 
  t=*tptr;
  if ((uintptr_t)t&1)
    {
      printf("basic extension problem\n");
      cleanup(3);
    }

  WW(t,(char*)*iptr-(char*)t);
  t++;
  *(char *)t=l=strlen(p->name)&255;
  t=(w16 *)((char*)t+1);
  strncpy((char *)t,p->name,l);
  t=(w16*)((char*)t+l); /* L !!*/
  if ((uintptr_t)t&1) t=(w16*)((char*)t+1);
  
  *tptr=t;
   *iptr=(*iptr)+1;
   /*printf("iptr: %d\n",*iptr);*/
}

void fnext(int type,struct BAS_PFENTRY **list)
{
  struct BAS_PFENTRY *p;
  
  p=*list;
  
  while(p)
    {
      if (type==p->type)
	break;
      p=p->link;
    }
  if (!p)
    {
      printf("basic extension problem\n");
      cleanup(3);
    }

  *list=p;
}


int mangle_count(int totsize,int count)
{
  int r;
  
  if (count*7>=totsize /*|| count==1*/) return count;
  r=(totsize+count+7)/8;
  /*printf("mangle_count: %d, %d -> %d\n",totsize,count,r);*/
  return r;
}


void create_link_table(struct BAS_PFENTRY *list)
{
  int tsize,inst_size,f_cnt,p_cnt;
  struct BAS_PFENTRY *p;
  w32 bext_table;
  w16 *etab;
  w16 *instr;
  int fnccnt,pccnt;
  
  pccnt=fnccnt=f_cnt=p_cnt=0;
  tsize=2+2+4+2; /*count,count, end markers */
  inst_size=0;
  /* somewhat generously counting */
  p=list;
  while(p)
    {        /*    len +.b+allign+addr */
      tsize+= (((strlen(p->name)+1)>>1)<<1)+2;
      inst_size+=2;
      switch(p->type)
	{
	case X_FUN:
	  f_cnt++;
	  fnccnt+=strlen(p->name);
	  break;
	case X_PROC:
	  p_cnt++;
	  pccnt+=strlen(p->name);
	  break;
	default:
	  fprintf(stderr,"wrong basic extension type %d\n",p->type);
	  return;
	}
      p=p->link;
    }
  
  reg[1]=tsize+inst_size+12+100; /* some pad */
  reg[2]=0;
    
  /*printf("totsize %d, tsize %d, inst_size %d,\n reserve %d\n",reg[1],tsize,inst_size,reg[1]);*/
#if 1
  QLtrap(1,0x18,2000000);
  if (reg[0]) 
    {
      fprintf(stderr,"allocation failed, QDOS error %d\n",reg[0]);
      return;
    }
#endif
#if 1
  bext_table=aReg[0];
  etab=(w16 *)((char*)theROM+aReg[0]);
  instr=(w16 *)((char*)etab+(((tsize+6+10)>>1)<<1));
  
  WW(etab++,mangle_count(pccnt,p_cnt));
  p=ext_list;
  while(p_cnt--)
    {
      fnext(X_PROC,&p);
      build_entry(&etab,p,&instr);
      p=p->link;
    }
  WW(etab++,0);  /* end proc marker */
  
  WW(etab++,mangle_count(fnccnt,f_cnt));
  p=ext_list;
  while(f_cnt--)
    {
      fnext(X_FUN,&p);
      build_entry(&etab,p,&instr);
      p=p->link;
    }
  WW(etab++,0);  /* end fun marker */
  WW(etab++,0);  /* another marker to be sure */
  /*printf("Basic Extensions table at %d\n",bext_table);*/
#endif 
#if 1
  aReg[1]=bext_table;
   
  QLvector(0x110,2000000);   /* and BP.INIT */ 
 
#endif
}


struct BAS_PFENTRY *add_link_entry(char *name,bas_err (*command)(),int type)
{
  struct BAS_PFENTRY *p;
  struct BAS_PFENTRY *l;

  l=ext_list;

  p=(void *)malloc(sizeof(struct BAS_PFENTRY ));

  ext_list=p;

  p->name=name;
  p->type=type;
  p->command=command;
  p->link=l;

  return p;
}


void add_bas_fun(char *name,bas_err (*command)())
{
  add_link_entry(name,command,X_FUN);
}
void add_bas_proc(char *name,bas_err (*command)())
{
  add_link_entry(name,command,X_PROC);
}

#ifdef HPR_STYLE
#define BN_KILL_UQLX "KILL_UQLX"
#define BN_GETXENV "GETXENV$"
#define BN_FORK_UQLX "FORK_UQLX"
#define BN_GETXARGC "GETXARGC"
#define BN_GETXARGs "GETXARG$"
#define BN_GETXRES "GETXRES"
#define BN_GETYRES "GETYRES"
#define BN_SCR_XLIM "SCR_XLIM"
#define BN_SCR_YLIM "SCR_YLIM"
#if 0
#define BN_MAKEDIR "MAKEDIR"
#endif
#else
#define BN_KILL_UQLX "Kill_UQLX"
#define BN_GETXENV "getXenv$"
#define BN_FORK_UQLX "Fork_UQLX"
#define BN_GETXARGC "getXargC"
#define BN_GETXARGs "getXarg$"
#define BN_GETXRES "getXres"
#define BN_GETYRES "getYres"
#define BN_SCR_XLIM "SCR_XLIM"
#define BN_SCR_YLIM "SCR_YLIM"
#if 0
#define BN_MAKEDIR "MakeDir"
#endif
#endif

void init_bas_exts()
{
  add_bas_proc(BN_KILL_UQLX,Kill_UQLX);
#if 0
  add_bas_proc(BN_MAKEDIR,MakeDir);
#endif
  add_bas_fun("UQLX_RELEASE$",UQLX_Relse);
  add_bas_fun(BN_GETXENV,UQLX_getenv);
  add_bas_fun(BN_FORK_UQLX,Fork_UQLX);
  add_bas_fun(BN_GETXARGC,UQLX_getXargc);
  add_bas_fun(BN_GETXARGs,UQLX_getXarg);
  add_bas_fun(BN_GETXRES,UQLX_getXres);
  add_bas_fun(BN_GETYRES,UQLX_getYres);
  add_bas_fun(BN_SCR_XLIM,UQLX_getXres);
  add_bas_fun(BN_SCR_YLIM,UQLX_getYres);

  create_link_table(ext_list);
} 

void BASEXTCmd()
{
  w32 where;
  struct BAS_PFENTRY *p=ext_list;
  
  where=(w32)((char*)pc-(char*)theROM-2);
  
  /*printf("BASIC EXTENSION\n");*/
  
  while(p)
    {
      if (where==p->code_addr) break;
      p=p->link;
    }

  if (!p)
    {
      rts();
      printf("problem with basic extension\n");
      return;
    }
  
  reg[0]=(p->command)();
  rts();
}

/*************************************************************/
/* argument access functions etc. */

typedef struct 
{
  uw16 len;      /* len is in HOST byteorder.. */
  char str[0];
} qstr;

/* use BV.CHRIX to allocate BASIC stack, returns *stacktop or 0 */
w32 bas_resstack(w32 size)
{
  w32 r,s;
  
  s=((size+1)>>1)<<1;
  reg[1]=s;
  
  QLvector(0x11a,200000);
  if (reg[0]<-1) return 0;
  r=ReadLong(aReg[6]+0x58)-s;
  WriteLong(aReg[6]+0x58,r);
  aReg[1]=r;

  /*printf("SB arithmetic stack: %d\n",r+aReg[6]);*/

  return r+aReg[6];
}

void bas_deallocstack(uw32 size)
{
  int r;
  
  if (size&1)
    {
      printf("deallocing %d bytes of SB stack ?!?!!\n",size);
      size&=(uw32)-1;
    }
  
  r=ReadLong(aReg[6]+0x58)+size;
  WriteLong(aReg[6]+0x58,r);
  aReg[1]=r;
}


int bas_argcount()
{
  return (aReg[5]-aReg[3])/8; 
}

void free_qstr(void *p)
{
  free(p);
}

static unsigned int bas_sa5;
void sch1arg()
{
  bas_sa5=aReg[5];
  aReg[5]=aReg[3]+8;
}
void resarg()
{
  aReg[5]=bas_sa5;
}

// Basic name table:
// byte(0): use
//       0 unset, 1 expression, 2 variable, 3 dim variable, 4 Procedure
//       5 Function, 6 repeat name, 7 for variable, 8 res proc, 9 res fn
// byte(1): type
//       0: use 3 substr, use 4 SB Proc, use 8 res Proc, 9 res Fn 
//       1 string, 2 fp, 3 integer
//       bit 7:   '#'
//       bit 4-6:
//         000: no separator
//         001: ','
//         010: ';'
//         011: '\'
//         100: '!'
//         101: 'TO'
// word(2): pointer to name in name list
//    or:-1 expression, substr
// long(4): line of definition (SB Proc,Fn) in MSW
//    or:   mc code addr (res proc/fn)



static inline int argused()
{
  return (ReadByte(aReg[3]+aReg[6])!=0);
}
static inline int argtype()
{
  return ReadByte(aReg[3]+aReg[6]+1);
}
static inline int parname()
{
  return ReadWord(aReg[3]+aReg[6]+2);
}

qstr *bas_getstr();

/* *L3C8E< l1d24 ca_name */
/* ca_vnam   */
/*  moveq #0,d1 */
/*  move.w 2(a6,a3.l),d1 index */
/*  blt.s ca_nuls ? */
/*  lsl.l #3,d1 */
/*  add.l $18(a6),d1 bv_nbas */
/* *L3C9C< l1f4c */
/* ca_vinam   */
/*  moveq #0,d3 */
/*  move.w 2(a6,d1.l),d3 */
/*  add.l $20(a6),d3 bv_nlbas */
/*  bsr.l ri_vinam > */
/* *L3CAA< ca_nuls ca_name */
/* ca_rtok   */
/*  moveq #0,d0 */
/* *L3CAC< ca_name */
/* ca_ret   */
/*  rts */


qstr *bas_getstrpar()
{
  int l;
  qstr *p;
  char *bp;

  if (argused())
    return bas_getstr();
  l=parname();
  if (l == -1)
    return (qstr*)0;
  bp = (char*) (l*8+ReadLong(aReg[6]+0x18));
  printf("bp %p\n",bp);
  bp = ReadWord(aReg[6]+bp+2)+ReadLong(aReg[6]+0x20);
  printf("%p ,%d\n%s\n",bp,l,(char*)bp+1+aReg[6]);
  return p;
}

qstr *bas_getstr()
{
  int l;
  qstr *p;
  
  sch1arg();
  QLvector(0x116,2000000);
  resarg();
  
  if (reg[0]<0 || reg[3]!=1) return 0;

  /*else return (char*)theROM+aReg[1]+aReg[6]; /* dealloc!!! */

  l=ReadWord(aReg[1]+aReg[6]);
  p=(qstr*)malloc(l+1);
  p->len=l;
  memcpy(p->str,(char *)theROM+2+aReg[1]+aReg[6],l);
  p->str[l]=0;

  return p;
}

int  bas_retstr(int len,char *str)
{
  w32 p;

  p=bas_resstack(len+2);
  WriteWord(p,len);
  memcpy((char *)theROM+p+2,str,len);

  reg[4]=1;
  return 0;
}


int bas_getln(int * val)
{
  sch1arg();
  QLvector(0x118,20000000);
  resarg();
  
  if( ((uw16) reg[3]!=1) || reg[0]) return -1;
  
  *val=ReadLong(aReg[1]+aReg[6]);
  bas_deallocstack(4);

  return 0;
}

int bas_retint(int i)
{
  w32 p;
  
  p=bas_resstack(2);
  WriteWord(p,i);
  reg[4]=3;
  
  return 0;
}



/*************************************************************/
/**/
/* the extensions itself */
/*************************************************************/
bas_err Kill_UQLX()
{
  int rx;  

  /* do NOT signal ERR.BP*/

  if (bas_getln(&rx)<0)
    rx=0;

  printf("\nexiting UQLX: Kill_UQLX %d\n\n",rx);
  cleanup(rx);
  return 0;
}

bas_err UQLX_Relse()
{

  if (bas_argcount()!=0)
    return QERR_BP;
  
  return bas_retstr(strlen(release),release);
  
}

bas_err UQLX_getenv()
{
  qstr *p;
  char *name;
  char *c;

  if (bas_argcount()!=1) return QERR_BP;
  
  p=bas_getstr();
  if (p==0) return QERR_BP;

  /*name=malloc(p->len+1);
    memcpy(name,p->str,p->len);
    name[p->len]=0;*/
  
  c=getenv(p->str);
  free_qstr(p);
  
  if (c) return bas_retstr(strlen(c),c);
  else return bas_retstr(0,(char*)theROM);
  
}

int do_fork()
{
    int pid,i;

    /* We must close the screen while forking
     * and re-open it when we are finished
     * otherwise we die horrible death
     */
    QLSDLExit();

    pid=fork();
    if (pid < 0) {
        perror("sorry, could not fork");
        QLSDLScreen();
    }

    /* We are in the child */
    if (pid == 0) {
        QLSDLScreen();
        fork_files();
    } else { /* We are in the parent */
      QLSDLScreen();
    }

    /* resetting the state of the keyboard seems the best */
    gKeyDown=0;
    for (i=0;i<8;i++) sdl_keyrow[i]=0;

    return pid;
}


bas_err Fork_UQLX()
{
  int pid;
  
#ifndef HPR_STYLE
  if (bas_argcount()!=0) return QERR_BP;
#endif

  pid=do_fork();

  return bas_retint(pid);
}

bas_err UQLX_getXres()
{
#ifndef HPR_STYLE
  if (bas_argcount()!=0) return QERR_BP;
#endif

  return bas_retint(qlscreen.xres);
}
bas_err UQLX_getYres()
{
#ifndef HPR_STYLE
  if (bas_argcount()!=0) return QERR_BP;
#endif

  return bas_retint(qlscreen.yres);
}

bas_err UQLX_getXargc()
{
#ifndef HPR_STYLE
  if (bas_argcount()!=0) return QERR_BP;
#endif
  
  return bas_retint(UQLX_argc-UQLX_optind+1);
}


bas_err UQLX_getXarg()
{
  uw32 n;
  char *r;
  
#ifdef HPR_STYLE
  if (bas_argcount()<1) return QERR_BP;
#else
  if (bas_argcount()!=1) return QERR_BP;
#endif
  
  if (bas_getln(&n)<0)
    return QERR_BP;
  
  if (/*n>UQLX_argc-1 || */ n<0  || ((n>0) && (n+UQLX_optind>UQLX_argc)))
    return bas_retstr(0,NULL);

  if (n==0) r=UQLX_argv[n];
  else r=UQLX_argv[n+UQLX_optind-1];
  
  return bas_retstr(strlen(r),r);
}

static bas_err MakeDir()
{
  qstr *p;

  if (bas_argcount()!=1) return QERR_BP;
  p=bas_getstrpar(); 
  // saveregs
  // trap#4
  // trap#2,createfile
  // trap#4
  // trap#3,77
  // restoreregs
  
  return bas_retint(UQLX_argc-UQLX_optind+1);
}
