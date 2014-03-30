/*
 * (c) UQLX - see COPYRIGHT
 */


/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/times.h>
#include <time.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h> 
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <X11/Intrinsic.h>  /* paste etc */

#ifdef VM_SCR
#include <sys/mman.h>
#endif

#include "unix.h"

#include "uqlx_cfg.h"
#include "qx_proto.h"

#ifdef SH_MEM
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

static XShmSegmentInfo shminfo;
static int shm_used;
#endif

XSetWindowAttributes attributes;

XWMHints wmhints;

int cursor_shape=XC_arrow;
extern int ki_state;
static int cs0=XC_arrow;
static int cs1=XC_cross;
Cursor cursor1,cursor0;
extern int invisible;
extern int start_iconic;
extern char *pwindow;

#if 0
static Widget topLevel;
static XtAppContext app_context;
#endif

int screen_drawable=0;

int depth,inside;
long black,white;
extern int scr_width,scr_height;

extern void QLButton(int,int);
extern void QLMovePointer(int,int);

extern int verbose;

#define E_MASK ( KeyPressMask | KeyReleaseMask | ButtonPressMask  \
		 | ButtonReleaseMask | PointerMotionMask | ColormapChangeMask |\
		 EnterWindowMask | LeaveWindowMask | ExposureMask | ResizeRedirectMask |\
		 StructureNotifyMask )

extern void QL2Pixmap2test (w32 , char *, w32 ,w32 );
extern void QL2Pixmap2 (w32 , char *, w32 ,w32 );
extern void QL2Pixmap8 (w32 , char *, w32 ,w32 );
extern void QL2Pixmap16 (w32 , char *, w32 ,w32 );
extern void QL2Pixmap24 (w32 , char *, w32 ,w32 );
extern void QL2Pixmap32 (w32 , char *, w32 ,w32 );

extern void (*QL2Pixmap) (w32 , char *, w32 ,w32 );
     
static int shmmono=1;
static char win_name[128];
static Visual *visual = NULL;

static int screen, nplanes;
static int Max_x, Max_y;

GC gc;
static XGCValues gc_val;

Window imagewin;		/* window ID for image */
XImage *image = NULL;
static char *Image;
Cursor cursor;
Pixmap empty;
int planes;
int pbytes;


char *xi_buf;

#ifdef SH_MEM
static int shmct;
#endif

Display *display;
int kmin,kmax;
int shmflag = 1;
Colormap cmap=0;

void redraw_screen(int ,int ,int ,int );
void ResetMousePtr(void);


void destroy_image (void)
{
  /*printf("Destroy Image\n");*/
  if (image == NULL) return;
  XDestroyImage (image);
#ifdef SH_MEM
  if (shmflag && shm_used)
    {
      XShmDetach (display, &shminfo);
      shmdt (shminfo.shmaddr);
      shmctl (shminfo.shmid, IPC_RMID, 0);
      shm_used = 0;
    }
#endif
  /*printf("End Destroy Image\n");*/
}

void x_reset_state(void)
{
#ifdef DO_GRAB
  if (QMD.do_grab)
    {
      if (ki_state==0)
	XGrabKey(display,AnyKey,AnyModifier,imagewin,False,GrabModeAsync,GrabModeAsync);
      else
	XUngrabKey(display,AnyKey,AnyModifier,imagewin);
    }
#endif
  ResetMousePtr();
}

void x_screen_close (void)
{
  XAutoRepeatOn(display);
  XFlush(display);
  destroy_image();
  XCloseDisplay(display);
}

static void set_window_name (char *name)
{
  XStoreName (display, imagewin, name);
}

static void create_image (int w, int h, int format)
{
  int planes = depth;
  
#ifdef SH_MEM
   if (shmflag)
   {
      image = XShmCreateImage (display, visual, planes, format,
			       NULL, &shminfo, w, h);
      shminfo.shmid = shmget (IPC_PRIVATE, image->bytes_per_line
			      * image->height * ((planes+7)/8), IPC_CREAT | 0777);
      xi_buf = shminfo.shmaddr = image->data = shmat (shminfo.shmid, 0, 0);
      shminfo.readOnly = False;
      XShmAttach (display, &shminfo);
      XSync(display, False);
      shmctl(shminfo.shmid, IPC_RMID, 0);
      shm_used = 1;

#ifdef QM_BIG_ENDIAN
      image -> bitmap_bit_order = MSBFirst;
      image -> byte_order = MSBFirst;
#else
      image -> bitmap_bit_order = LSBFirst;
      image -> byte_order = LSBFirst;
#endif

      XShmPutImage(display,imagewin,gc,image,0,0,0,0,10,10,False);
      XSync(display,False);
   }

   /* XShmPutImage might have failed, setting shmflag=0 ! */
   if (!shmflag)
#endif
   {
      int pbytes;
      if (planes<8) pbytes=1;
      else if (planes<16) pbytes=2;
      else pbytes=4;

      xi_buf= malloc(w*h*pbytes+256);
      
      image = XCreateImage (display, visual, planes, format,
			    0, (char *) xi_buf, w, h, 8, 0);
    }

#ifdef QM_BIG_ENDIAN
   image -> bitmap_bit_order = MSBFirst;
   image -> byte_order = MSBFirst;
#else
   image -> bitmap_bit_order = LSBFirst;
   image -> byte_order = LSBFirst;
#endif
   /*Image = image->data;*/
}

/* Process all events that occured since we last came here...
 */

extern void xql_key(XEvent *,int);

do_xflush()
{
  XFlush(display);
}

void process_events (void)
{
  XEvent e;
  XExposeEvent *x;

  while (XCheckMaskEvent (display, E_MASK, &e))
    {
      switch (e.type)
	{
	case EnterNotify:
	  XAutoRepeatOff(display); /* QL has its autorepeat.. */
	  inside=1;
	  
	  break;
	case LeaveNotify:
	  XAutoRepeatOn(display);
	  inside=0;
	  
	  break;
	case ResizeRequest:
	  /* my way of making it not resizeable... */
	  XResizeWindow(display, imagewin, scr_width, scr_height);
	case KeyPress:
	  xql_key (&e, 1);
	  inside=1;
	  break;
	case KeyRelease:
	  xql_key (&e, 0);
	  break;
#ifdef MOUSE  
	case ButtonPress:
#if 0  /* sometime add cut & paste right here */
	  if (controlState)
	    {
	      switch (((XButtonEvent *)&e)->button)
		{
		case 1: break; // no cut yet
		case 2: break;
		}
	    }
	  else
#endif

	  QLButton(((XButtonEvent *)&e)->button,1);
	  inside=1;
	  break;
#endif
#ifdef MOUSE  
	case ButtonRelease:
	  QLButton(((XButtonEvent *)&e)->button,0);
	  break;
#endif 

	case Expose:
	  /* flush all Expose events .. */
	  screen_drawable=1;
	  /* printf("***Expose event\n"); */
	  scrconv();
	  do{
#if 1
	    x = (XExposeEvent *)&e;
	    redraw_screen(x->x,x->y,x->width,x->height);
#endif
	  } while (XCheckTypedEvent(display,Expose,&e));
	  
	  XFlush(display);
	  break;

	case MapNotify:
	  screen_drawable=1;
	  /*printf("****MapNotify event\n");*/
	  break;
	  
	case UnmapNotify:
	  screen_drawable=0;
	  /*printf("****UnapNotify event\n");*/
	  break;
	  
#ifdef MOUSE 
	case MotionNotify: 
	  if (e.xmotion.window == imagewin)
	    {
	      QLMovePointer(((XMotionEvent *)&e)->x,((XMotionEvent *)&e)->y);
	      /*fprintf (stderr, "Pointer: %d,%d\n", ((XMotionEvent *)&e)->x,((XMotionEvent *)&e)->y); */
	      inside=1;
	    }
	  break;
#endif /* 0 */
	}
    }
}

long colors[16];

XVisualInfo * ChooseVisual(Display *display)
{
  XVisualInfo *visual_list;
  XVisualInfo vTemplate; 
  int vmatch,i,cmax,idepth;
  
  static char *visual_class[]={"StaticGray","GrayScale","StaticColor",
			       "PseudoColor","TrueColor","DirectColor"};

  int vpref_cm[]={TrueColor,StaticColor,DirectColor,PseudoColor,StaticGray,GrayScale,-1};
  int *p;

  XVisualInfo *retval=NULL;
  

  vTemplate.screen=screen;
  /*  vTemplate.depth=8; */
  depth=vTemplate.depth;

  /*XSynchronize(display,1);*/

  visual_list=XGetVisualInfo(display, VisualScreenMask, /* | VisualDepthMask,*/
			     &vTemplate, &vmatch);
#if 0
  if (!vmatch)
    {
      vTemplate.depth=4;
      depth=vTemplate.depth;
      visual_list=XGetVisualInfo(display, VisualScreenMask | VisualDepthMask,
				 &vTemplate, &vmatch);
    }
#endif  
  if (!vmatch)
    {
      printf("Warning: there seems to be no suitable visual\n");
      return NULL; /*XDefaultVisual(display,screen);*/
    }

  cmax=0;


  if (QMD.pref_vid != -1)
    {
      for (i=0;i<vmatch;i++)
	{
	  if (visual_list[i].depth>=4 && QMD.pref_vid == visual_list[i].visualid)
	    {
	      retval=&visual_list[i]; /*.visual;*/
	      goto do_ret;
	    }
	}
      printf("preferred visual %x not available or has insufficient colormap\n",QMD.pref_vid);
      return NULL;
    }

  if (strlen(QMD.pref_class))
    {
      int cls;

      for ( cls=0; cls<6 ;cls++)
	if (!strcasecmp(visual_class[cls],QMD.pref_class)) break;
      if (cls<6)
	{
	  if (QMD.pref_depth == -1)  /* no preference */
	    for (i=0;i<vmatch;i++)
	      {
		if (visual_list[i].depth>=4 && cls == visual_list[i].class)
		  {
		    retval= &visual_list[i]; /*.visual;*/
		    goto do_ret;
		  }
	      }
	  else
	    for (i=0;i<vmatch;i++)
	      {
		if (visual_list[i].depth == QMD.pref_depth && cls == visual_list[i].class)
		  {
		    retval= &visual_list[i]; /*.visual;*/
		    goto do_ret; 
		  }
	      }
	  printf("appears no visual of '%s' class available with suitable colormap\n",visual_class[cls]);
	  return NULL;
	}
      else printf("No Visual Class '%s' found\n",QMD.pref_class);
      return NULL;
    }

  if (QMD.pref_depth == DefaultDepth(display,screen))
    return NULL;  /* default vis is almost always best ...*/
  if (QMD.pref_depth != -1)
    {
      idepth=QMD.pref_depth;
      p=vpref_cm;
      while (*p++ != -1)
	for (i=0;i<vmatch;i++)
	  {
	    if (visual_list[i].depth==idepth && p[-1]==visual_list[i].class)
	      {
		retval= &visual_list[i]; /*.visual;*/
		goto do_ret;
	      }}}
  if (QMD.pref_depth != -1)
    {
      printf("preferred depth %d not available\n",idepth);
      return NULL;
    }
  
  p=vpref_cm;
  while (*p++ != -1)
    for (i=0;i<vmatch;i++)
      {
	if (visual_list[i].colormap_size>8 && p[-1]==visual_list[i].class)
	  {
	    retval= &visual_list[i]; /*.visual;*/
	    goto do_ret;
	  }
      }
  
  /* fallback into default visual .....*/
  /*depth = DefaultDepth (display, screen);*/
  return  NULL; /*XDefaultVisual (display, screen);*/

 do_ret:
  if (V3)printf("using visual %x, class %d\n",retval->visual,retval->class);
  if ( QMD.color && retval->class<2)
    QMD.color=0;
  return retval;
}


void allocColors(Colormap map)
{
#if 1
  static char *Cname[]={"black", "#0000FF","#FF0000","#FF00FF","#00FF00","#00FFFF","#FFFF00","#FFFFFF",  
              "#BFBFBF","#00BFFF","#FF00BF","#BF00FF","#BFFF00","#00FFBF","#FFBF00","#DFDFDF" ,"#FFFFFF"};
  static char *Gname[]={"black","gray15","gray30","gray45","gray60","gray75","gray90","gray95",
              "gray5" ,"gray10","gray25","gray45","gray55","gray70","gray85","gray95"};
#else
  static char *Cname[]={"Black","Blue","Red","Magenta","Green","Cyan","Yellow","gray95" };
  static char *Gname[]={"Black","gray15","gray30","gray45","gray60","gray75","gray90","gray95"};
#endif
  char **name;
  
  XColor exact_def;
  /*int default_depth=DefaultDepth(display,screen); */
  int res,i=5;
  int i1=0;
  int lim;

#if 0  
  XVisualInfo visual_info;
  static char *visual_class[]={"StaticGray","GrayScale","StaticColor",
			       "PseudoColor","TrueColor","DirectColor"};
#endif

  planes=depth;

  if (depth<4)  {
    printf("sorry, support fo monochrome display is experimental\n");
    colors[0]=colors[1]=black;
    colors[2]=colors[3]=white;
    colors[4]=colors[5]=black;
    colors[6]=colors[7]=white;
  }  

#if 0
  while(!XMatchVisualInfo(display,screen,depth,i--,&visual_info));
  if (V3) printf("using Visual %s\n",visual_class[i++]);
#endif

  if ( /*i>1 && */ QMD.color) name=Cname;
  else name=Gname;

  if (depth<8)
    {
      colors[0]=colors[1]=BlackPixel(display,screen);
      colors[14]=colors[15]=WhitePixel(display,screen);
      i1=1;
      lim=15;
    }
  else
    {
      i1=0;
      lim=16-QMD.fwhite;
    }
  
  
  for(i=i1; i<lim; i++)
    {
      res=XParseColor(display,map,name[i],&exact_def);
      /*printf("XParseColor returns: %d\n",res);*/
      res=XAllocColor(display,map,&exact_def);
      /*printf("XAllocColor returns: %d\n",res);*/
      colors[i]=exact_def.pixel;/*colors[2*(i+1)]=colors[2*(i+1)+1]=exact_def.pixel;*/
      /*printf("pixval : %d\n",exact_def.pixel);*/
    }

  if (QMD.fwhite || QMD.color==0)
    colors[14]=colors[15]=WhitePixel(display,screen);

  /*  for(i=0; i<8; i++)printf("Color %d= %d\n",i,colors[i]);*/
}

int xbreak=0;
extern int QLdone;
int myXIOerr(Display* d)
{
  if (V2)printf("X connection closed\n");
  /*cleanup(0);*/
  QLdone=1;
  xbreak=1;
  cleanup(0);
}
int myXerr(Display* d, XErrorEvent* ev)
{
  if (shmflag) shmflag=0;
  else if (V3)
    {
      printf("X error : probably non-local display, trying to ignore\n"); 
    }
}

void setwmhints()
{
  wmhints.initial_state= start_iconic ? IconicState : NormalState;
  
  wmhints.flags=StateHint;
}


void x_screen_open (int frk)
{
  XClassHint xch; 
  unsigned long valuemask;
  int i;
  char *ip;
  
#ifdef SH_MEM
  int shm_major, shm_minor;
  Bool shm_pixmaps;
#endif

  XSizeHints h;
  XColor ccls[2];

#if 0
  topLevel = XtVaAppInitialize (&app_context,   /* Application context */
				"Xql",   /* Application class */
				NULL, 0,
				NULL, NULL,
				NULL,   /* for missing app-defaults file */
				NULL);  /* terminate varargs list */
#endif

  scr_width = qlscreen.xres;
  scr_height = qlscreen.yres;


  /*strcpy (win_name, "QL");*/
  sprintf(win_name,"QL - %s, %dK",QMD.sysrom,RTOP/1024);

  if ((display = XOpenDisplay (NULL)) == NULL)
    {
      perror(NULL);
      fprintf (stderr, "Could not open display.\n");
      exit (1);
    }
 
  if (!frk)
    init_keymap();
  
  /* set X error handler */
  XSetErrorHandler(myXerr);
  XSetIOErrorHandler(myXIOerr);
#ifdef SH_MEM 
  if (shmflag)
    {
      if (!XShmQueryVersion (display, &shm_major, &shm_minor, &shm_pixmaps))
	{
	  if (V3) fprintf(stderr,
			     "MIT Shared Memory Extension not supported.\n");
	  shmflag = 0;
	}
      else
	{
	  int foo;
	  if(V3) fprintf (stderr,
			  "Using MIT Shared Memory Extension %d.%d," \
			  " %s shared pixmaps.\n", shm_major, shm_minor,
			  (shm_pixmaps ? "with" : "without"));
	  foo = XShmPixmapFormat (display);
#if 0
	  verbose && fprintf (stderr,
			      "XShm Pixmap format: %s\n", ((foo == XYBitmap ? 
							    "XYBitmap" : (foo == XYPixmap ? "XYPixmap" : "ZPixmap"))));
#endif
	}
    }
#endif

  screen = XDefaultScreen (display);

#if 1 /* XVIS */
  {	
    XVisualInfo *v=NULL;

    /* take defaults if user doesn't care */
    if (QMD.pref_vid != -1 || strlen(QMD.pref_class) || QMD.pref_depth != -1)
      v=ChooseVisual(display);

    if (v)
      {
	visual=v->visual;
	depth=v->depth;
      }
    else
      {
	int i=5;
	XVisualInfo vi;
	char *visual_class[]={"StaticGray","GrayScale","StaticColor",
			      "PseudoColor","TrueColor","DirectColor"};

	depth = DefaultDepth (display, screen);
	visual = XDefaultVisual (display, screen);
	if (V3) printf("using default visual %x\n",visual);
	/* find out about color etc.. */
	while(!XMatchVisualInfo(display,screen,depth,i--,&vi));
	/*if (verbose) printf("using Visual %s\n",visual_class[i++]);*/
	if (i<2 && QMD.color)
	  QMD.color=0;
      }
    nplanes=depth;
  }
#else

  depth = DefaultDepth (display, screen);
  visual = XDefaultVisual (display, screen);
  nplanes = XDisplayPlanes (display, screen);
#endif


  h.width = scr_width;
  h.height = scr_height; 
  h.x = 500;
  h.y = 0;

  h.flags = PSize; /*| PPosition; /*  ?????? */
  black=BlackPixel (display, screen);
  white = WhitePixel (display, screen);
  
  attributes.backing_store=/*start_iconic ? WhenMapped : */ Always;
  attributes.border_pixel=BlackPixel(display,screen);
  attributes.background_pixel=BlackPixel(display,screen);
  valuemask= 
#ifdef BACKING_STORE
    /* use backing store only for small screen size */
    ((qlscreen.xres==512)&&CWBackingStore) |
#endif
    CWBorderPixel | CWBackPixel |CWColormap; 
 
  if (visual==XDefaultVisual(display, screen) && depth>4)
    cmap=DefaultColormap(display,screen);
  else
    cmap=XCreateColormap(display, RootWindow (display, screen), visual,AllocNone);

  attributes.colormap=cmap;
  
  imagewin = XCreateWindow (display, RootWindow (display, screen),
			    h.x, h.y, h.width, h.height, 0, depth,
			    InputOutput, visual, valuemask, &attributes);
  XSetWindowColormap(display,imagewin,cmap);
  
  setwmhints();
  XSetWMHints(display, imagewin, &wmhints);

  xch.res_name = "uqlx";
  xch.res_class = "Uqlx";      
  XSetClassHint(display, imagewin, &xch);
  
  
  XSelectInput (display, imagewin, StructureNotifyMask|E_MASK);
  XSetStandardProperties (display, imagewin, win_name, win_name, None,
			  NULL, 0, &h);
  
  if(pwindow)
    {
      Window w;
      w = strtoul(pwindow, NULL, 16);
      XReparentWindow(display, imagewin, w, 0, 0);
    }
  

  XMapWindow (display, imagewin);

  screen_drawable=0;
  
  gc_val.background = black;
  gc_val.foreground = white;
  gc = XCreateGC (display, imagewin, GCForeground | GCBackground, &gc_val);
#ifdef SH_MEM
  shmct = XShmGetEventBase (display) + ShmCompletion;
#endif
  XSetForeground (display, gc, white);
  XSetBackground (display, gc, black);

  if(!frk)
    allocColors(cmap);

  XClearWindow (display, imagewin);


  XSelectInput (display, imagewin, E_MASK);

  create_image (scr_width,scr_height,ZPixmap);

  pbytes=(image->bits_per_pixel+7)/8;
  
  if (nplanes<=2) 
    {
      QL2Pixmap=QL2Pixmap2;
    }

  if ( nplanes < 4) QL2Pixmap=QL2Pixmap2;
#ifdef TEST_MONO
  if (2 < nplanes && nplanes <=8) QL2Pixmap=QL2Pixmap2;

#else
  if (2 < nplanes && nplanes <=8) QL2Pixmap=QL2Pixmap8;
#endif
  
  if (8 < nplanes && nplanes <= 16) QL2Pixmap=QL2Pixmap16;

  if (16 < image->bits_per_pixel && image->bits_per_pixel <= 24)
     QL2Pixmap=QL2Pixmap24;
  
  if (24 < image->bits_per_pixel) QL2Pixmap=QL2Pixmap32;

  if (nplanes>32)
    {
      printf("sorry, unsupported screen depth %d\n",nplanes);
      QL2Pixmap=QL2Pixmap32;
    }


  /* define XCursors used */
  ccls[0].pixel=0;
  ccls[1].pixel=0;
  empty =  XCreatePixmapFromBitmapData (display, imagewin, calloc(32,1),
					16, 16,0,0,1);

  cursor0= XCreateFontCursor(display,cs0);
  cursor1= XCreateFontCursor(display,cs1);
  cursor = XCreatePixmapCursor (display,empty,empty, &ccls[0],&ccls[1], 0,0);

  /* Set Passive Grabs for every key that could be snatched away by the WM */

  x_reset_state();
  
#if 0 /* done by x_reset_state */
  XGrabKey(display,AnyKey,AnyModifier,imagewin,False,GrabModeAsync,GrabModeAsync);
#endif

}


void InvisibleMousePtr()
{
  XDefineCursor(display,imagewin,cursor);
}


void ResetMousePtr()
{
#if 0
  XUndefineCursor(display,imagewin);
#else
   if (invisible==0)
   {
      if (ki_state)
         XDefineCursor(display,imagewin,cursor1);
      else
         XDefineCursor(display,imagewin,cursor0);
   }
   else 
      InvisibleMousePtr();
#endif
}