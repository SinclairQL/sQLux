/*
 * (c) UQLX - see COPYRIGHT
 */



#include "QL68000.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/keysym.h>

/*#include "QLtypes.h"*/

#include "QL_config.h"
#include "QInstAddr.h"
#include "uqlx_cfg.h"
#include "qx_proto.h"

extern Display *display;

int ki_state=0;
int ki_switch=0;
int ql_alt=0;

#define ALT 1
#define SHIFT 4
#define CTRL 2

XComposeStatus compose;


static int maps_per_code;
static KeySym *ktable;

/* many keyboards have brain damaged bindings, KP_CURSOR attempts to get
   something useable out of it */
#define KP_CURSOR
/* SUN has completely braindead bindings for keybooards, try BRAIN_DEAD */

int shiftState,controlState,altState;

/* table of ISO-latin1 -> QL translations*/
struct  
{
  int iso; 
  int ql;
} codetable[] =
  {
#if 0
    {8,194},
#endif
    {13,10},
    {96,159},

    {161,179},
    {162,157},
    {163,96},
    {164,183},
    {165,158},
    {167,182},
    {169,127},
    {171,184},
    {176,186},
    {180,159},
    {181,176},
    {187,185},
    {191,180},
    
    {197,162},
    {196,160},
    {195,161},
    {198,170},
    {199,168},
    {201,163},
    {214,164},
    {213,165},
    {216,166},
    {220,167},
    {223,156},
    {224,141},
    {233,131},
    {227,129},
    {228,128},
    {229,130},
    {230,138},
    {231,136},
    {232,144},
    {234,145},
    {236,148},
    {237,147},
    {238,149},
    {239,146},
    {240,173},
    {241,137},
    {242,151},
    {243,150},
    {244,152},
    {246,132},
    {247,187},
    {248,134},
    {249,154},
    {250,153},
    {251,155},
    {252,135},
    
    {0,0}  
  };


/* very simple method to convert iso to ql chars*/
static unsigned char iso2ql(unsigned c)
{
  int i;
  
  for(i=0; codetable[i].iso!=0; i++)
    if (codetable[i].iso==c)
      return codetable[i].ql;
  return c;
}

struct kstable
{
  KeySym keysym;
  int code;
  int qchar;
} keysymtable1[] = /* this is the table for keysyms that are handled as keycodes*/
  { 
#ifdef XK_KP_Left
    {XK_KP_Left,49,0},
    {XK_KP_Up,50,0},
    {XK_KP_Right,52,0},
    {XK_KP_Down,55,0},
#endif
#if 0 /*(defined(KP_CURSOR) && defined(XK_KP_6))*/
    {XK_KP_4,49,0},
    {XK_KP_8,50,0},
    {XK_KP_6,52,0},
    {XK_KP_2,55,0},
#endif

#ifdef XK_KP_Divide
    {XK_KP_Divide,5,0},
#endif
#ifdef XK_KP_equal
    {XK_KP_equal,37,0},
#endif
#ifdef XK_KP_Mutliply
    {XK_KP_Multiply,-1,42},
#endif
#ifdef XK_KP_Subtract
    {XK_KP_Subtract,21,0},
#endif
#ifdef XK_KP_Add
    {XK_KP_Add,-1,43},
#endif
#ifdef XK_sterling
    {XK_sterling,-1,96},
#endif
    {XK_Left,49,0},
    {XK_Up,50,0},
    {XK_Right,52,0},
    {XK_Down,55,0},

#if (defined(BRAINDEAD) && defined(XK_F34))
    {XK_F30,49,0},
    {XK_F32,52,0},
    {XK_F28,50,0},
    {XK_F34,55,0},
#endif

    {XK_F1,57,0},
    {XK_F2,59,0},
    {XK_F3,60,0},
    {XK_F4,56,0},
    {XK_F5,61,0},

    {XK_Return,48,0},
    {XK_KP_Enter,48,0},
    {XK_space,0x36,0},
    {XK_Tab,19,0},
    {XK_Escape,51,0},
    {XK_Caps_Lock,33,0},
  
    {0,0,0}
  };

/* this is the table for chars that are to be handled by XKeyLookupString
  by default */
struct kstable keysymtable2[]={
  {XK_5,56+2,0},
  {XK_4,56+6,0},
  {XK_7,56+7,0},
  {XK_bracketleft,40,0},
  {XK_z,41,0},
  {XK_period,42,0},
  {XK_c,43,0},
  {XK_b,44,0},
  {XK_quoteleft,45,0},
  {XK_m,46,0},
  {XK_asciitilde,47,0},
  {XK_bar,32,0},{XK_k,34,0},{XK_s,35,0},{XK_f,36,0},{XK_equal,37,0},
  {XK_g,38,0},{XK_semicolon,39,0},
  {XK_l,24,0},{XK_3,25,0},{XK_h,26,0},{XK_1,27,0},{XK_a,28,0},
  {XK_p,29,0},{XK_d,30,0},{XK_j,31,0},
  {XK_9,16,0},{XK_w,17,0},{XK_i,18,0},{XK_r,20,0},{XK_minus,21,0},
  {XK_y,22,0},{XK_o,23,0},
  {XK_8,8,0},{XK_2,9,0},{XK_6,10,0},{XK_q,11,0},{XK_e,12,0},{XK_0,13,0},
  {XK_t,14,0},{XK_u,15,0},
  {XK_x,3,0},{XK_v,4,0},{XK_slash,5,0},{XK_n,6,0},{XK_comma,7,0},

  {XK_Next,-1,220},
  {XK_Prior,-1,212},
  {XK_Home,-1,193},
  {XK_End,-1,201},

#ifdef XK_KP_Next
  {XK_KP_Next,-1,220},
  {XK_KP_Prior,-1,212},
  {XK_KP_Home,-1,193},
  {XK_KP_End,-1,201},
#endif 

#ifdef XK_KP_6
  {XK_KP_5,56+2,0},
  {XK_KP_4,56+6,0},
  {XK_KP_7,56+7,0},
  {XK_KP_9,16,0},
  {XK_KP_8,8,0},
  {XK_KP_2,9,0},
  {XK_KP_6,10,0},
  {XK_KP_1,27,0},
  {XK_KP_3,25,0},
  {XK_KP_0,13,0},
#endif


  {0,0,0}
};



int keycode(KeySym sym, int * di,char *qchar)
{
  int i;
  
  *di=1;*qchar=0;
  
  for(i=0; keysymtable1[i].keysym!=0; i++)
    if (sym==keysymtable1[i].keysym)
      {
	*qchar=keysymtable1[i].qchar;
	return keysymtable1[i].code;
      }

  *di=0;
  for(i=0; keysymtable2[i].keysym!=0; i++)
    if (sym==keysymtable2[i].keysym)
      {
	*qchar=keysymtable1[i].qchar;
	return keysymtable2[i].code;
      }
  

  return -1;
}

int keycode_from_Xkeycode(int xcode, char * qchar)
{
  int i,ix;

  for (i=0;i<maps_per_code;i++)
    ktable[i]=XKeycodeToKeysym(display,xcode,i);

  *qchar=0;
  for(i=0; keysymtable1[i].keysym!=0; i++)
    for(ix=0;ix<maps_per_code;ix++)
      {
	/*printf("1: %d Keysym is %d,table entry %d\n",i,XKeycodeToKeysym(display,xcode,ix),keysymtable1[i].keysym);*/
	if (ktable[ix]==keysymtable1[i].keysym)
	  {
	    *qchar=keysymtable1[i].qchar;
	    return keysymtable1[i].code;
	  }
      }
  
  for(i=0; keysymtable2[i].keysym!=0; i++)
    for(ix=0;ix<maps_per_code;ix++)
      {
	/*printf("2: %d Keysym is %d, table entry %d\n",i,XKeycodeToKeysym(display,xcode,ix),keysymtable2[i].keysym);*/
	if (ktable[ix]==keysymtable2[i].keysym)
	  {
	    *qchar=keysymtable2[i].qchar;
	    return keysymtable2[i].code;
	  }
      }
  return -1;
}




#if 0
void KeyInit()
{
  /* rebind some keys */

  KeySym modlist[3];
  int strlen,listlen;
  
  strlen=1;
  
  modlist[0]=XK_Control;
  XrebindKeySym(display,XK_Space,modlist,1,"BREAK",1);
  
}
#endif

void queueCode(int m,int c)  
{
  /*printf("queueCode %d\n",c);*/
  queueKey(m,c,0);gKeyDown=1;
}

void queueChar(int alt,int c) { if (alt) queueKey(0,0,0xff+(c<<8)); else queueKey(0,0,c);gKeyDown=1;}

#define MODSTATE() (altState|controlState*2|shiftState*4)

unsigned int keyrow[]={0,0,0,0,0,0,0,0};
extern int gKeyDown;


void KeyrowChg(int code,int press)
{
  int j;
  
  /* printf("KeyrowChg code=%d, press=%d\n",code,press);*/

   if (code>-1)
    {
      j= 1<<(code%8);
      /*printf("Keyrow: %x\t",keyrow[7-code/8]);*/

      if (press) keyrow[7-code/8]=keyrow[7-code/8]|j;
      else keyrow[7-code/8]=keyrow[7-code/8]&~j;

      /* printf(" %x\n",keyrow[7-code/8]);*/
    }
}

int AltMask;

inline int isxchar(int c)
{
  if (c>=0 && c<256) return 1;
  else return 0;
}
 
void init_keymap()
{
  int kpk,i,kmin,kmax;
  XModifierKeymap *k;
  KeySym key,*ksyms;
  int kcode;
  
  ki_state=QMD.xkey_on;
  ki_switch=XStringToKeysym(QMD.xkey_switch);
  ql_alt=XStringToKeysym(QMD.xkey_alt);
#if 0
  printf("xkey_switch: %s %x\n",QMD.xkey_switch,ki_switch);
  printf("xkey_alt: %s %x\n",QMD.xkey_alt,ql_alt);
#endif
  kcode=XKeysymToKeycode(display,ki_switch);
  if (ki_switch != XKeycodeToKeysym(display,kcode,0))
    {
      printf("Warning: XKEY_SWITCH is configured to a Keysym not accessible\n");
      printf("\t without modifiers, this may cause problems\n");
    }

  kcode=XKeysymToKeycode(display,ql_alt);
  if (ql_alt != XKeycodeToKeysym(display,kcode,0))
    {
      printf("Warning: XKEY_ALT is configured to a Keysym not accessible\n");
      printf("\t without modifiers, this may cause problems\n");
    }
  

  k=XGetModifierMapping(display);
  
  for(i=0;i<8*((*k).max_keypermod);i++)
    {
      key=XKeycodeToKeysym(display,(*k).modifiermap[i],0);
      if (key==XK_Alt_L || key==XK_Alt_R)
	AltMask=1<<(i/8);
    }

  /*printf("ShiftMask %d, ControlMask %d, AltMask %d\n",ShiftMask,ControlMask,AltMask);*/

  XFreeModifiermap(k);
  /*maps_per_code=display->keysyms_per_keycode; */
  XDisplayKeycodes(display, &kmin, &kmax);
  ksyms = XGetKeyboardMapping(display, kmin, (kmax-kmin+1), &maps_per_code);
  XFree(ksyms);
  /* printf("k_P_c %d\n",maps_per_code);*/
  ktable=(KeySym *)malloc(maps_per_code*(sizeof(KeySym)));
}

int Xsim(int val)
{
  if (val<0) return ki_state;
  ki_state=val;
  x_reset_state();
  return ki_state;
}

int caps_emu=0;
int caps_down=0;

void xql_key(XEvent *e, int press)
{ 
  KeySym keysym,keysym2;
  /*XComposeStatus compose;*/
  int c,code,code2,j,di;
  char qchar;
   
  int count,prefer_qtrans,force_qtrans;

  unsigned char str[40],str1[40];

  if (press==0) gKeyDown=0;  

#if 0  /* I wish I remember'd the reason for this hack ... */
  if (gKeyDown==0 && isMinerva ) 
    WriteWord(0x2808a,0);   /* Minerva HACK */
#endif

  /* get the keysym and eventual ISO-8859-1 binding */

  count=XLookupString((XKeyEvent *)e,str,10,&keysym,NULL/*&compose*/);
  str[count]=0; 

#if 0
  printf("Keycode %d, state %d\n",((XKeyEvent *)e)->keycode,((XKeyEvent *)e)->state);
    printf("Key %s, string %s, sym %s count %d\n",(press ? "pressed" : "released"),
    str, XKeysymToString(keysym),
    count); 
#endif
  

 /* display->keysyms_per_keycode*/  

  if (press)
    DECR_SC(); /*schedCount--;*/
  
  /* change input method ? */
  if (press && (keysym==ki_switch || keysym==268828432))
    {
      /* ki_state=~ki_state;
      x_reset_state();*/
      Xsim(!ki_state);
      return;
    }
  

  if (press && count==0 && isxchar(keysym) &&isalpha(keysym))
    return;  /* this happens when compose is used on XSun */


  /* attempt to handle differences in CapsLock handling */
#if 0
  if (keysym==XK_Caps_Lock)
    printf("CapsLock, caps_down=%d, caps_emu=%d, press=%d\n",caps_down,caps_emu,press);
#endif

  if (keysym==XK_Caps_Lock)
    caps_down=press;

  if (caps_down && keysym != XK_Caps_Lock)
    {
      caps_emu=1;
    }

  if (caps_emu && keysym==XK_Caps_Lock && press==0)
    {
      /*printf("emulated CapsLock release\n");*/
      press=1;
      caps_down=0;
    }

  /* keyrow state for these is controlled elsewhere... */  
  switch(keysym)     /* test modifier keys */
    {
    case XK_Control_L:
    case XK_Control_R:
      controlState=press;
      return;
      
    case XK_Shift_L:
    case XK_Shift_R:
      shiftState=press;
      return;
      
    case XK_Alt_L:
    case XK_Alt_R:
      altState=press;
      return;

      /* handle compose/input method */
#ifdef XK_Multi_key
    case XK_Multi_key: return;
#endif
    }
  if (keysym == ql_alt)
    {
      altState=press;
      return;
    }

#ifdef DO_NMI
  if (altState && controlState && keysym==XK_7)
    {	
      printf("**NMI**\n");
      pendingInterrupt=7;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      return;
    }
#endif

#if 1
  /* now test whether the keycode is fully interpreted in X */
  prefer_qtrans=0;force_qtrans=0;
  
  if (press & (~ki_state))
  {
    int ax1,ax2,ax3;
    int ostate=((XKeyEvent *)e)->state;
    int lcount;

    if (altState || shiftState ||controlState)
      for(ax1=0;ax1<=altState;ax1++)
	for(ax2=0;ax2<=shiftState;ax2++)
	  for(ax3=0;ax3<=controlState;ax3++)
	    {
	      ((XKeyEvent *)e)->state=(ostate&(~(AltMask|ControlMask|ShiftMask))) |
		(ax1*AltMask)|(ax2*ShiftMask)|(ax3*ControlMask);
	      lcount=XLookupString((XKeyEvent *)e,str1,10,&keysym2,&compose);
	      str1[lcount]=0;
	      if(!strcmp(str,str1) && (ostate!= ((XKeyEvent *)e)->state))
		{
		  prefer_qtrans=1;
		  goto exit_xmaptest;
		}
	    }
    ((XKeyEvent *)e)->state=ostate;
  }
  
 exit_xmaptest:
  /*if(press)printf("prefer_qtrans is %d\n",prefer_qtrans);*/
#endif

  /* if keysym indicates an upcase ASCII character, let QDOS handle it */
  if ( isxchar(keysym) && isascii(keysym) && isalpha(keysym) && isupper(keysym) && (((((XKeyEvent *)e)->state)&LockMask)  || shiftState))
    keysym=tolower(keysym);

  code=keycode(keysym,&di,&qchar);
  code2=keycode(keysym2,&di,&qchar);
  
  if (prefer_qtrans && code2>-1)
    code=code2,force_qtrans=1;

  if (code<0 && count<1 && !qchar)
    {
      code=keycode_from_Xkeycode(((XKeyEvent*)e)->keycode,&qchar);
    }
  

#if 0
  if (code>-1)
    {
      j= 1<<(code%8);
      if (press) keyrow[7-code/8]=keyrow[7-code/8]|j;
      else keyrow[7-code/8]=keyrow[7-code/8]&~j;
    }
#endif
  
  if (code>-1)
    KeyrowChg(code,press);
  else
    if (qchar && (count<1 || di) && (~ (ki_state && count>0)))
      {
	if (press)
	  queueChar(0,qchar);
	return;
      }


  if (press)
    
      switch(keysym)
	{	
#if 0
	case XK_Return:
	case XK_KP_Enter:
	  queueCode(MODSTATE(),48);
	  return;

	case XK_space:
	  queueCode(MODSTATE(),0x36);
	  return;
#endif 


	case XK_F6 : 
	  c=232+2;
	  queueChar(altState,c);
	  return;
	
	case XK_F7 :
	  c=236+2;
	  queueChar(altState,c);
	  return;

	case XK_F8 :
	  c=240+2;
	  queueChar(altState,c);
	  return;

	case XK_F9 :
	  c=244+2;
	  queueChar(altState,c);
	  return;

	case XK_F10 :
	  c=248+2;
	  queueChar(altState,c);
	  return;

	case XK_BackSpace:
	  queueChar(0,194);
	  return;
	
	case XK_Delete:
	  queueChar(0,202);
	  return;


	case XK_Home:
#if (defined(KP_CURSOR) && defined(XK_KP_Home))
	case XK_KP_Home:
#endif
#if (defined(KP_CURSOR) && defined(XK_KP_6))
	  /*case XK_KP_7:*/
#endif
#if (defined(BRAINDEAD) && defined(XK_F27))
	case XK_F27:
#endif
	  queueChar(0,193);
	  return;
	  
	case XK_End:
#if (defined(KP_CURSOR) && defined(XK_KP_End))
	case XK_KP_End:
#endif
#if (defined(KP_CURSOR) && defined(XK_KP_6))
	  /*case XK_KP_1:*/
#endif
#if (defined(BRAINDEAD) && defined(XK_F33))
	case XK_F33:
#endif
	  queueChar(0,201);
	  return;

#if 0
	case XK_Left:
	case XK_KP_Left:
	  c=192+altState+2*controlState+4*shiftState;
	  queueChar(0,c);
	  return;
	
	case XK_Right:
	case XK_KP_Right:
	  c=200+altState+2*controlState+4*shiftState;
	  queueChar(0,c);
	  return;
#endif 
	  
	case XK_Prior:
#if (defined(KP_CURSOR) && defined(XK_KP_Prior))
	case XK_KP_Prior:
#endif
#if (defined(KP_CURSOR) && defined(XK_KP_6))
	  /*case XK_KP_9:*/
#endif
#if (defined(BRAINDEAD) && defined(XK_F29))
	case XK_F29:
#endif
	  queueChar(0,212);
	  return;
	  
	case XK_Next:
#if (defined(KP_CURSOR) && defined(XK_KP_Next))
	case XK_KP_Next:
#endif
#if (defined(KP_CURSOR) && defined(XK_KP_6))
	  /*case XK_KP_3:*/
#endif
#if (defined(BRAINDEAD) && defined(XK_F35))
	case XK_F35:
#endif
	  queueChar(0,220);
	  return;

#if 0	
	case XK_Up:
	case XK_KP_Up:
	  c=208+altState+2*controlState+4*shiftState;
	  queueChar(0,c);
	  return;
	
	case XK_Down:
	case XK_KP_Down:
	  c=216+altState+2*controlState+4*shiftState;
	  queueChar(0,c);
	  return;
#endif	
	
	}

  
      if(press)
	{
	  if(code>-1 && (force_qtrans || (di /*|| isMinerva*/))){
	  hdirect:	  
	    queueCode(MODSTATE(),code);

	    return;
	  }
	  if (count>0)
	    {
	      c=str[0];
	      if (ki_state) goto hchar;
#if 0 
	      if (code>-1 && isxchar(keysym) && isalpha(keysym) && 
		  (shiftState&controlState) && (code>-1)) 
		goto hdirect;
	      if (code>-1 && (isxchar(keysym) || 
			      (!isalpha(keysym) && controlState && (code>-1))))
		goto hdirect;
#else
	      if (code>-1 && !shiftState )
		goto hdirect;

	      if (code>-1 && isxchar(keysym) && isalpha(keysym))
		goto hdirect;
#endif
	      
	    hchar:
	      queueChar(altState,iso2ql(c));
	      return; 
	    }
	  if (qchar)
	    {
	      queueChar(0,qchar);
	      return;
	    }
	  if (code>-1) goto hdirect;
	}
      
}
