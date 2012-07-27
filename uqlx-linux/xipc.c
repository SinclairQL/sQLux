/*
 * (c) UQLX - see COPYRIGHT
 */


/* remote control and message passing */

/*#include "QLtypes.h"*/
#include "QL68000.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <fcntl.h>
#include <stdio.h>


#include <sys/un.h>
#include <netinet/in.h>

//#include <X11/Xos.h>
//#include <X11/Xatom.h>
//#include <X11/Xlib.h> 
//#include <X11/Xutil.h>
//#include <X11/cursorfont.h>


#include "util.h"
#include "xipc.h"
#include "qx_proto.h"

struct paste_buf *paste_bl=NULL;

typedef struct CONNECTION 
{
  int sd;
  struct CONNECTION *next;
}connection; 

static int sock;
static  connection *sc=NULL;
static char sock_path[256];
static char sock_dir[256];

void insert_keyQ(int len,char *text)
{
  struct paste_buf *buf,*p;

  buf=(void *)malloc(len+sizeof(struct paste_buf));
  if (buf==NULL) return;

  memcpy(buf->text,text,len);
  /*buf->text[len]=0;*/
  buf->p=(buf->text);
  buf->size=len; /*strlen(buf->text);*/
  buf->next=NULL;

  if (paste_bl)
    {
      /*printf("link in paste block %x, ll start %x\n",buf,paste_bl);*/
      for(p=paste_bl; /*p!=NULL &&*/ p->next!=NULL ; p=p->next);
      p->next=buf;
    }
  else
    paste_bl=buf;
}



void do_paste_keyboard(int len, int fd)
{
  struct paste_buf *buf,*p;
  message *buf1;
  int res;

  buf1=(message *)malloc(len+sizeof(message));
  if (buf1==NULL) return;
  res=recv(fd,buf1,len+sizeof (message),0);
  if (res<1) 
    { 
      free(buf1);
      return;
    }

  buf=(struct paste_buf *)malloc(len+sizeof(struct paste_buf));
  if (buf==NULL) return;
  memcpy(buf->text,buf1+1,len);
  free(buf1);
  
  /*buf->text[len]=0;*/
  buf->p=(buf->text);
  buf->size=res; /*strlen(buf->text);*/
  buf->next=NULL;

  if (paste_bl)
    {
      /*printf("link in paste block %x, ll start %x\n",buf,paste_bl);*/
      for(p=paste_bl; /*p!=NULL &&*/ p->next!=NULL ; p=p->next);
      p->next=buf;
    }
  else
    paste_bl=buf;
}


int init_ipc()
{
  struct sockaddr_un addr;
  int res;
  struct stat sb;
  int xx=1;

  /*printf("initing IPC - ");*/
  sock=socket(AF_UNIX, SOCK_STREAM, 0);
#if 0
  res=fchmod(sock,0700);
  if (res<0)
    perror("fchmod");
#endif
  fcntl(sock,F_SETFL,O_NONBLOCK | fcntl(sock,F_GETFL));

  sprintf(sock_dir,"/tmp/qm-sockdir-%d",getuid());
  sprintf(sock_path,"%s/qm-%x",sock_dir,getpid());
  /*printf("init_ipc: sock_path ; %s, getpid()==%x\n",sock_path,getpid());*/
  addr.sun_family=AF_UNIX;
  strcpy(addr.sun_path,sock_path);

  res=stat(sock_dir,&sb);
  if (res<0 && errno==ENOENT)
    res=mkdir(sock_dir,0700);
  else 
    { 
      if ( res==0 && !(sb.st_mode & S_IFDIR) )
	{
	  printf("xipc.c: can't init IPC: %s exists and is not dir\n",sock_dir);
	  return -1;
	}
    }
  res=chmod(sock_dir,0700);
  if (res<0)
    {
      perror("could not init IPC, chmod error");
      return -1;
    }

#if 0
  res=setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&xx,sizeof xx);
  if (res<0)
    perror("could not setsockopt");
#endif

  if (bind(sock,(struct sockaddr*)&addr, sizeof (struct sockaddr_un)/*strlen(addr.sun_path)+3*/)<0)
    perror("init_ipc : could not bind socket");
  res=chmod(sock_path,0700);
  if (res<0)
    perror("fchmod");
  listen(sock,1);
}

int process_fork()
{
  int res,pid;
  connection *cn;
  message msg;

  pid=do_fork();
  /*printf("fork result %d\n",pid);*/
  if (pid<0)
    return;

  /* close all connections */
  for (cn=sc;cn;cn=cn->next)
    {
      msg.type=MX_EXIT;
      msg.len=0;
      
      res=send(cn->sd,(void*)&msg,sizeof(msg),0);
      /*printf("sent MX_EXIT to GUI: res %d\n",res);*/
      close(cn->sd);
    }
  
  if (pid==0)  /* new process needs another socket to listen */
    {
      close(sock);
      init_ipc();
    }
  
  InitDialog();
  return pid;
}


void reply(int fd, int msgnr, int value)
{
  message msg;
  msg.len=0;
  msg.type=htonl(msgnr);
  msg.body.spar=htonl(value);

  write(fd,&msg,sizeof(msg));
}

void check_connection(int fd)
{
  int res,unread;
  message msg;
  char *buff;

  while(check_pend(fd,SLC_READ))
    {
      /*printf("checking connection %d\n",fd);*/
      
      res=recv(fd,(void *)&msg,sizeof(msg),MSG_PEEK);
      unread=0;

      if (res<MSG_MINSIZE && res!=0)
	{
	  perror("IPC message problem");
	  return;
	}
      if (res==0) return;

      switch(ntohl(msg.type))
	{
	case MX_EXIT : 
	  oncc(0); return;
	case MX_CLONE :
	  if (process_fork())
	    unread=1;
	  else unread=0;
	  break;
	case MX_BREAK :
	  reply(fd,MX_BREAK,allow_rom_break(ntohl(msg.body.spar)));
	  break;
	case MX_KEYSW :
	  reply(fd,MX_KEYSW,Xsim(ntohl(msg.body.spar)));
	  break;
	case MX_HOG :
	  reply(fd,MX_HOG,toggle_hog(ntohl(msg.body.spar)));
	  break;
	case MX_DEVINFO : break;
	case MX_PASTE :
	    do_paste_keyboard(ntohl(msg.len),fd);
	    unread=1;
	  break;
	case MX_REDRAW:
	  conv_chunk(qlscreen.qm_lo,qlscreen.qm_hi);
	  redraw_screen(0,0,qlscreen.xres,qlscreen.yres);
	  break;
	default: printf("unknown message\n");
	}  
      if (!unread)
      res=recv(fd,(void *)&msg,sizeof(msg),0);  /* remove the message */
    }
}


void process_ipc()
{
  int res;
  int fd;
  connection *cn;

  fd=accept(sock,NULL,0); /* non-blocking !! */
  if (fd>1)
    {
      cn=(connection*) malloc(sizeof(cn));
      cn->sd=fd;
      cn->next=sc;
      sc=cn;
      /*printf("accepted new connection\n");*/
    }
    
  for(cn=sc;cn; cn=cn->next)
    check_connection(cn->sd);

}

void cleanup_ipc()
{
  message msg;
  connection *cn;

  /*printf("closing connections\n");*/
  for(cn=sc;cn; cn=cn->next)
    {
      if(check_pend(cn->sd,SLC_WRITE))
	{
	  msg.type=MX_EXIT;
	  msg.len=0;
	  send(sock,(void*)&msg,sizeof(message),0);
	}
      close(cn->sd);
    }
  close(sock);
  unlink(sock_path);
}

