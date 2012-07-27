/*
 * (c) UQLX - see COPYRIGHT
 */



#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef XM
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/MenuButton.h>
#include <Xm/SimpleMenu.h>
#include <Xm/Form.h>
#include <Xm/Box.h>
#include <Xm/Command.h>
#else
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Toggle.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <sys/un.h>


#include "xipc.h"
#include "util.h"

Widget label,button,button_paste,button_clone,button_rom,
  button_keyboard,button_hog,button_redraw;

static sock;

int qmpid=-1;



void ssend(int msgnr, int value)
{
  message msg;
  msg.len=0;
  msg.type=htonl(msgnr);
  msg.body.spar=htonl(value);

  write(sock,&msg,sizeof(msg));
}

/* send request to get initial values */
void sendXreq()
{
  ssend(MX_BREAK,-1);
  ssend(MX_HOG,-1);
  ssend(MX_KEYSW,-1);
}

void Quit()
{
  ssend(MX_EXIT,0);
#if 0 
 message msg;
  int res;

  msg.type=htonl(MX_EXIT);
  msg.len=0;

  res=send(sock,(void*)&msg,sizeof(msg),0);
  if (res<0)
    perror("send command");
#endif
  exit(0);
}


void Clone()
{
  ssend(MX_CLONE,0);
#if 0
  message msg;
  int res;

  msg.type=htonl(MX_CLONE);
  msg.len=0;

  res=send(sock,(void*)&msg,sizeof(msg),0);
  if (res<0)
    perror("send command");
#endif
}
void Redraw()
{
  ssend(MX_REDRAW,0);
#if 0
  message msg;
  int res;

  msg.type=htonl(MX_REDRAW);
  msg.len=0;

  res=send(sock,(void*)&msg,sizeof(msg),0);
  if (res<0)
    perror("send command");
#endif
}

static int rom_toggle=0;;
void ToggleROM()
{
  rom_toggle=rom_toggle ? 0 : 1;
  ssend(MX_BREAK,rom_toggle);
#if 0
  msg.type=htonl(MX_BREAK);
  msg.len=0;
  msg.body.spar=htonl(rom_toggle);

  res=send(sock,(void*)&msg,sizeof(msg),0);
#endif
}
static int key_toggle=0;
void ToggleKeyboard()
{
  key_toggle=key_toggle ? 0 : 1;
  ssend(MX_KEYSW,key_toggle);
}


void paste_requestor_callback(w,client_data,selection,type,value,length,format)
Widget w;
XtPointer client_data;  /* cast to XButtonEvent below */
Atom *selection;
Atom *type;
char * value;
unsigned long *length;
int *format;
{
  message *buf1,*msg;
  int res;

  if (value==NULL || *length==0)
    return;

  if ((*length == 0) && (*value == 0) ) {
    XBell(XtDisplay(w), 100);
    XtWarning("bitmapEdit: no selection or selection timed out:\
try again\n");
  }

  buf1=(message *)malloc(sizeof(message)+*length);
  if (buf1==NULL) return;
  msg=buf1;

  msg->type=htonl(MX_PASTE);
  msg->len=htonl(*length);
  memcpy(msg+1,value,*length);

  res=send(sock,(void*)msg,sizeof(*msg)+*length,0);
  if (res<0)
    {
      perror("send command");
      return;
    }
  /*  send(sock,value,msg.len,0);*/
}

void Paste(Widget w, XtPointer client_data, XButtonEvent *event)
{
  message msg;
  int res;

  XtGetSelectionValue (w, XA_PRIMARY, XA_STRING, paste_requestor_callback,0,0);
}

static int hog_toggle;
void ToggleHog()
{
  hog_toggle=hog_toggle ? 0 : 1;
  ssend(MX_HOG,hog_toggle);
}

static char sock_path[256];
static char sock_dir[256];

init_sock(int pid)
{
  struct sockaddr_un addr;
  
  sock=socket(AF_UNIX, SOCK_STREAM, 0);
  
  sprintf(sock_dir,"/tmp/qm-sockdir-%d",getuid());
  sprintf(sock_path,"%s/qm-%x",sock_dir,qmpid);
  /*printf("init_ipc: sock_path : %s\n",sock_path);*/

  addr.sun_family=AF_UNIX;
  strcpy(addr.sun_path,sock_path);

  if (connect(sock,(struct sockaddr*)&addr, strlen(addr.sun_path)+3))
    {
      perror("Xgui: could not connect");
      printf("sock_path %s\n",sock_path);

      exit(1);
    }

}

static XtAppContext app_context;
void check_status(XtPointer p, XtIntervalId *id)
{
  int res;
  message msg;

  XtAppAddTimeOut(app_context, 20, check_status, NULL);

  while(check_pend(sock,SLC_READ))
    {
      res=recv(sock,(void *)&msg,sizeof(msg),0);
      if (res==0) return;
#if 1
      if (res<sizeof(msg))
	{
	  printf("recv res : %d\n",res);
	  perror("Xgui: IPC message problem");
	  return;
	}
#endif
      /*printf("received msg %d, par %d\n",msg.type,msg.body.spar);*/
      switch(msg.type)
	{
	case MX_EXIT: exit(0); break;
	case MX_BREAK:
	  rom_toggle=ntohl(msg.body.spar);
	  XtVaSetValues (button_rom, XtNstate,rom_toggle  , NULL);
	  break;
	case MX_HOG:
	  hog_toggle=ntohl(msg.body.spar);
	  XtVaSetValues (button_hog, XtNstate,hog_toggle  , NULL);
	  break;
	case MX_KEYSW:
	  key_toggle=ntohl(msg.body.spar);
	  XtVaSetValues (button_keyboard, XtNstate,key_toggle  , NULL);
	  break;
	}
    }
}

Widget  bitMap;

static Widget topLevel, form0, form, form1,form2;

static XImage *image = NULL;
static GC gc;
static Pixmap pixmap;
static char *buf;
static int colors[8];
static int black,white;
static Cursor cursor;
static Pixmap empty;



int main (int ac, char **av)
{

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

    if (ac>1)
      {
	qmpid=atol(av[1]);
	init_sock(qmpid);
      }

    topLevel = XtVaAppInitialize (&app_context,   /* Application context */
				  "Xql",   /* Application class */
				  NULL, 0,
				  &ac, av,
				  NULL,   /* for missing app-defaults file */
				  NULL);  /* terminate varargs list */

    form = XtVaCreateManagedWidget("form", 
				 formWidgetClass, topLevel, 
				 XtNjustify,   XtJustifyCenter,
				 XtNtop, XawChainTop,
				 NULL, 0);

    form0 = XtVaCreateManagedWidget("form0", 
				    formWidgetClass, form, 
				    XtNjustify,   XtJustifyCenter,
				    XtNtop, XawChainTop,
				    XtNborderWidth, 0,
				    XtNleft, XawChainLeft,
				    XtNright, XawChainRight,
				    NULL);


    label = XtCreateManagedWidget("UQLX", 
				  labelWidgetClass, form0, 
				  Lbldef, XtNumber(Lbldef));


    form1= XtVaCreateManagedWidget("form1", 
				   formWidgetClass, form, 
				   XtNfromVert, form0,
				   XtNright, XawChainLeft,
				   NULL);
    
#if 1
    form2= XtVaCreateManagedWidget("form2", 
				   formWidgetClass, form, 
				   XtNfromVert, form1,
				   /*XtNfromHoriz, form1, */
				   XtNright, XawChainRight,
				 NULL);
#endif


    button = XtVaCreateManagedWidget("Salir", 
				   commandWidgetClass, form0,
				   XtNfromHoriz, label,
				   XtNright,XawChainRight, 
				   NULL);
    
    XtAddCallback(button,  XtNcallback, Quit, NULL);


    button_paste = XtCreateManagedWidget("Pegar", 
				     commandWidgetClass, form1, 
				      NULL, 0);


    XtAddCallback(button_paste,  XtNcallback, Paste, NULL);


    button_clone = XtVaCreateManagedWidget("Clonar UQLX", 
				     commandWidgetClass, form1,
				      XtNfromHoriz, button_paste,
				      NULL);

    XtAddCallback(button_clone,  XtNcallback, Clone, NULL);



    button_rom = XtVaCreateManagedWidget("ROM breakpoints", 
					 toggleWidgetClass, form1, 
					 XtNfromVert, button_paste,
				      NULL);

    XtAddCallback(button_rom,  XtNcallback, ToggleROM, NULL);

    button_redraw = XtVaCreateManagedWidget("Redibujar", 
					    commandWidgetClass, form1, 
					    XtNfromHoriz, button_rom,
					    XtNfromVert, button_clone,
				      NULL);

    XtAddCallback(button_redraw,  XtNcallback, Redraw, NULL);


#if 1
    button_keyboard = XtVaCreateManagedWidget("XKeyLookup", 
					      toggleWidgetClass, form2, 
					      /*XtNfromVert, button_clone,*/
					      /*XtNfromHoriz, button_rom,*/
				      NULL);

    XtAddCallback(button_keyboard,  XtNcallback, ToggleKeyboard, NULL);

    button_hog = XtVaCreateManagedWidget("cpu hog", 
					 toggleWidgetClass, form2, 
					 XtNfromHoriz, button_keyboard,
				      NULL);

    XtAddCallback(button_hog,  XtNcallback, ToggleHog, NULL);

#endif




    XtAppAddTimeOut(app_context, 20, check_status, NULL);



    XtRealizeWidget (topLevel);

    sendXreq();  /* ask UQLX for parameter settings */

    XtAppMainLoop (app_context);
    return 0;
}

