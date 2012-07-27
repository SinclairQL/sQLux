/*
 * (c) UQLX - see COPYRIGHT
 */


#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

/*-------------------------------------------------------------------.
| Hack QLTypes.h to not typedef Boolean if the following is defined, |
| otherwise gcc gets upset :-)					     |
`-------------------------------------------------------------------*/

#define BOOLEAN_ALREADY_DEFINED
/*#include "QLtypes.h"*/
#include "QL68000.h"
#include "unix.h"
#include "uqlx_cfg.h"

#define vbase 0x20000

int inside;
Widget  bitMap;

static Widget topLevel, form;
static XtAppContext app_context;
XImage *image = NULL;
GC gc;
static Pixmap pixmap;
char *xi_buf;
int colors[8];
static int black,white;
static Cursor cursor;
static Pixmap empty;

int shmflag=0;
Display *display;
Window imagewin;
int planes,plane2;
int pbytes;

extern void QL2Pixmap8 (w32 , char *, w32 ,w32 );
extern void QL2Pixmap16 (w32 , char *, w32 ,w32 );
extern void QL2Pixmap24 (w32 , char *, w32 ,w32 );
extern void QL2Pixmap32 (w32 , char *, w32 ,w32 );

extern void (*QL2Pixmap) (w32 , char *, w32 ,w32 );


extern int verbose;
static void allocColours(Widget w)
{
    Colormap map;

    static char *Cname[]=
    {"Blue","Red","Magenta","Green","Cyan","Yellow","white" };
    static char *Gname[]=
    {"gray15","gray30","gray45","gray60","gray75","gray90","gray95"};
    char **name;
    int screen = XDefaultScreen (XtDisplay(w));  
    XColor exact_def;
    int default_depth=DefaultDepth(XtDisplay(w), screen);
    int res,i=5;

    XVisualInfo visual_info;
    
    static char *visual_class[]={"StaticGray","GrayScale","StaticColor",
			       "PseudoColor","TrueColor","DirectColor"};

    map=DefaultColormap(XtDisplay(w), screen);
    XSetWindowColormap(XtDisplay(w), XtWindow(w), map);

    if (default_depth<4)  {
	printf("sorry, not much support fo monochrome display\n");
	colors[0]=colors[1]=black;
	colors[2]=colors[3]=white;
	colors[4]=colors[5]=black;
	colors[6]=colors[7]=white;
  }  

    while(!XMatchVisualInfo(XtDisplay(w), screen, 
			    default_depth, i--, &visual_info));
  
    if (verbose)
	printf("using Visual %s\n",visual_class[i++]);
    
    if (i>1 && QMD.color)
	name=Cname;
    else
	name=Gname;
#define MAXCOLORS 7
 
  if (verbose) printf("Default Visual %s\n",visual_class[DefaultVisual(XtDisplay(w),screen)->class]); 

  colors[0]=colors[1]=BlackPixel(XtDisplay(w),screen);
  colors[7]=colors[6]=WhitePixel(XtDisplay(w),screen);  

  for(i=0; i<MAXCOLORS; i++)
    {
      res=XParseColor(XtDisplay(w),map,name[i],&exact_def);
      /*    printf("XParseColor returns: %d\n",res);*/
      res=XAllocColor(XtDisplay(w),map,&exact_def);
      /*    printf("XAllocColor returns: %d\n",res);*/
      colors[(i+1)]=exact_def.pixel;/*colors[2*(i+1)]=colors[2*(i+1)+1]=exact_def.pixel;*/
      /*    printf("pixval : %d\n",exact_def.pixel);*/
    }

  if (QMD.fwhite)
      colors[7]=WhitePixel(XtDisplay(w),screen); 

  /*  for(i=0; i<8; i++)printf("Color %d= %d\n",i,colors[i]);*/

}

static void emuinit(Widget w, int wid, int hgt)
{
    XColor ccls[2];
    static XGCValues values;
    int screen;

    screen = XDefaultScreen (XtDisplay(w));
    planes = DefaultDepth (XtDisplay(w), screen);
    plane2 = planes/2;

    black=BlackPixel (XtDisplay(w), screen);
    white = WhitePixel (XtDisplay(w), screen);

    allocColours(w);

    if (planes<=16) pbytes=(planes+7)/8;
    else pbytes=4;
    
    xi_buf = XtCalloc (1, wid * hgt *  pbytes);

 
    image = XCreateImage (XtDisplay(w),
			  DefaultVisualOfScreen(XtScreen(w)),
			  planes, ZPixmap, 0,
			  xi_buf, wid, hgt, 8, 0);    

#ifdef QM_BIG_ENDIAN
    image -> bitmap_bit_order = MSBFirst;
    image -> byte_order = MSBFirst;
#else
    image -> bitmap_bit_order = LSBFirst;
    image -> byte_order = LSBFirst;
#endif

 
    switch(planes)
      {
      case 8: QL2Pixmap=QL2Pixmap8;
	break;
      case 16: QL2Pixmap=QL2Pixmap16;
	break;
      case 24: QL2Pixmap=QL2Pixmap24;
	break;
      case 32: QL2Pixmap=QL2Pixmap32;
	break;
      default: printf("sorry, unsupported screen depth %d\n",planes);
	QL2Pixmap=QL2Pixmap8;
	break;
      }

#if 0
    XInitImage (image);
#endif


    values.foreground = white;
    values.background = black;
    
    pixmap = XCreatePixmap (XtDisplay(w), 
			    XtWindow(w), wid, hgt, planes);
    gc = XCreateGC (XtDisplay(w), pixmap,
		    GCForeground | GCBackground, &values);
    ccls[0].pixel=0;
    ccls[1].pixel=0;

    empty =  XCreatePixmapFromBitmapData (XtDisplay(w), XtWindow(w), 
					  XtCalloc(32,1), 16, 16,0,0,1);
    cursor = XCreatePixmapCursor (XtDisplay(w), empty, empty, 
				  &ccls[0], &ccls[1], 0,0);

    display=XtDisplay(w);
    imagewin=XtWindow(w);
}

/* reconnecting Xtk widgets appears too tricky :-( */
void x_screen_open(int frk)
{
  fprintf(stderr,"sorry, forking works only with the Xlib version\n");
  fprintf(stderr,"compile as 'make noaw'\n");
  exit(-3);
}


void ResetMousePtr(void)
{
    XUndefineCursor(XtDisplay(bitMap), XtWindow(bitMap));
}
void InvisibleMousePtr(void)
{
    XDefineCursor(XtDisplay(bitMap), XtWindow(bitMap), cursor);
}

extern int rx1,rx2,ry1,ry2,finishflag;  
extern void xql_key(XEvent *,int);

#ifndef min
# define min(a,b)  (a<b ? a : b)
#endif



#define E_MASK (KeyPressMask | KeyReleaseMask | ButtonPressMask | \
		ButtonReleaseMask | PointerMotionMask | ColormapChangeMask | \
		EnterWindowMask | LeaveWindowMask | ExposureMask | \
		ResizeRedirectMask)

void event_handler (Widget w, XtPointer unused, XEvent *e, Boolean *cont)
{
  XExposeEvent *x;

  switch (e->type)
  {
    case EnterNotify:
      XAutoRepeatOff(XtDisplay(w)); /* QL has its autorepeat.. */
      inside = 1;
      break;
    case LeaveNotify:
      XAutoRepeatOn(XtDisplay(w));
      inside = 0;
      break;
    case ResizeRequest:
      /* move way of making it not resizeable... */
      break;
    case KeyPress:
      xql_key (e, 1);
      inside = 1;
      break;
    case KeyRelease:
      xql_key (e, 0);
      break;
#ifdef MOUSE   /*mouse not yet supported */
    case ButtonPress:
      QLButton(((XButtonEvent *)e)->button,1);
      inside = 1;
      break;
#endif
#ifdef MOUSE  /* no mouse yet */
    case ButtonRelease:
      QLButton(((XButtonEvent *)e)->button,0);
      break;
#endif 
    case Expose:
      x = (XExposeEvent *)e;
      redraw_screen(/*w,*/ x->x,x->y,x->width,x->height);	  
      break;
      
#ifdef MOUSE 
    case MotionNotify: 
      if (e->xmotion.window == XtWindow(w))
      {
	  inside = 1;
	  QLMovePointer(((XMotionEvent *)e)->x,((XMotionEvent *)e)->y);
#if 0
	  fprintf (stderr, "Pointer: %d,%d\n", 
		   ((XMotionEvent *)e)->x,((XMotionEvent *)e)->y);
#endif
      }

      break;
#endif 0
  }
}

extern void QLInit(int, char **);

static int self;
extern void signalTimer();


#if 0
void ontsignal(XtPointer p, XtIntervalId *id)
{
  signalTimer();
  XtAppAddTimeOut(app_context, 20, ontsignal, NULL);
}
#endif

extern  Boolean CPUWork(XtPointer);
 
void Quit(Widget w, XtPointer unused, XtPointer notused)
{
  CleanRAMDev("RAM");
  exit(0);
}


int main (int ac, char **av)
{
    Widget label,button;
    static Arg Lbldef [] =
    {
	{XtNjustify,   XtJustifyCenter},
	{XtNinternalHeight, 0},
	{XtNborderWidth, 0},
	{XtNleft, XawChainLeft},
	{XtNright, XawChainRight},
	{XtNtop, XawChainTop},
	{XtNbottom, XawChainTop}
    };

#ifdef USE_VM
    vmtest();
#endif

    topLevel = XtVaAppInitialize (&app_context,   /* Application context */
				  "Xqlaw",   /* Application class */
				  NULL, 0,
				  &ac, av,
				  NULL,   /* for missing app-defaults file */
				  NULL);  /* terminate varargs list */

    form = XtCreateWidget("form", 
			  formWidgetClass, topLevel, 
			  NULL, 0);

#if 0
#include "buttons.c"
#else
    button = XtCreateWidget("Exit", 
				   commandWidgetClass, form, 
				   NULL, 0);
    XtAddCallback(button,  XtNcallback, Quit, NULL);

    label = XtCreateWidget("Label", 
				  labelWidgetClass, form, 
				  Lbldef, XtNumber(Lbldef));
#endif
    
    bitMap  = XtCreateManagedWidget ("Bitmap", simpleWidgetClass,
				     form, NULL, 0);

    QLInit(ac, av);

 


    {
	Dimension xl,xh;
	String st;
	char *p;
	int n;
	void *alist[2];

	XtVaGetValues (label, XtNlabel, &st, NULL);	

	for(n = 0, p = st; *p,p = strchr(p, '%'); n++,p++)
	{
	    if(*(p+1) == 's')
	    {
		alist[n] = QMD.sysrom;
	    }
	    else if(*(p+1) == 'd')
	    {
		alist[n] = (void *)(RTOP/1024);
	    }
	}
	if(n)
	{
	    char buf[80];
	    sprintf(buf, st, alist[0],alist[1]);
	    XtVaSetValues(label, XtNlabel, buf, NULL);
	}

	XtVaSetValues (label, XtNfromHoriz,  button, NULL);    
	XtVaSetValues (bitMap, XtNfromVert,  button, NULL);    
	XtVaGetValues(button, XtNwidth, &xl, XtNheight, &xh, NULL);    
	xl = qlscreen.xres-8-xl;
	XtVaSetValues(label, XtNwidth, xl, NULL);    
    }
    XtManageChild(form);
    XtManageChild(button);
    XtManageChild(label);
    
    XtVaSetValues (bitMap, XtNwidth, qlscreen.xres, XtNheight, qlscreen.yres, NULL);
    XtAddEventHandler(bitMap, E_MASK, False, event_handler, NULL);
    /*XtAppAddTimeOut(app_context, 20, ontsignal, NULL);*/
    XtRealizeWidget (topLevel);

    emuinit(bitMap, qlscreen.xres, qlscreen.yres);

   
    XtSetKeyboardFocus(topLevel, bitMap);

    XtAppAddWorkProc(app_context, CPUWork, NULL);

    XtAppMainLoop (app_context);

    return 0;
}
