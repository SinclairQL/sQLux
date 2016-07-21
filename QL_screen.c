/*
 * (c) UQLX - see COPYRIGHT
 */


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
#include <X11/extensions/XShm.h>

#include "QL68000.h"

#include "unix.h"

#include "uqlx_cfg.h"
#include "qx_proto.h"

#define vbase qlscreen.qm_lo

extern int do_update;

screen_specs qlscreen = 
{
   128 * 1024, 
   128 * 1024 + 32 * 1024, 
   0x8000, 
   128, 
   256, 
   512
};

extern int screen_drawable;
extern int colors[8];
extern char *xi_buf;
extern int pbytes;
extern int display_mode;

extern int shmflag;
extern Display *display;
extern Window imagewin;
extern GC gc;
extern XImage *image;
extern int planes,plane2;

/* zona del display modificata; 0=nessuna modifica */
w32    displayFrom = 0,displayTo = 0;

void draw_chunk(void);
void (*QL2Pixmap) (w32 , char *, w32 ,w32 );

#define min(a,b)  (a<b ? a : b)

void QClip(int *x1, int *y1, int *w,int *h)
{
  *x1= *x1>=0 ? *x1 : 0;
  *y1= *y1>=0 ? *y1 : 0;
  
  *x1= *x1<=qlscreen.xres-1 ? *x1 : qlscreen.xres-1;
  *y1= *y1<=qlscreen.yres-1 ? *y1 : qlscreen.yres-1;

  if (*x1+*w>qlscreen.xres) *w=qlscreen.xres-*x1;
  if (*y1+*h>qlscreen.yres) *h=qlscreen.yres-*y1;
}

/*
 * experimental code for Linux Palmtops, wristwacthes and similar 
 * devices with very small monochrome displays
 */
void QL2Pixmap2 (w32 st, char *data, w32 from,w32 to)
{
  int r1, r2, ri1, ri2, color, lastcolor,cc;
  int i, k;
  char *r;
  long dist;
  static char mcolors[]={0,0, 0,0, 1,1, 1,1};
  static char invcolors[]={1,1, 1,1, 0,0, 0,0};

  r=st+(Ptr)theROM;

  /*printf("from %d, to %d\n",from,to);*/
   
  from -= qlscreen.qm_lo;
  from /= qlscreen.linel;
  from *= qlscreen.linel;
  from += qlscreen.qm_lo;

  to -= qlscreen.qm_lo;
  to = (to+qlscreen.linel-1)/qlscreen.linel;
  to *= qlscreen.linel; 
  to += qlscreen.qm_lo;

  data+=((from-qlscreen.qm_lo)*4);

  r+=from-qlscreen.qm_lo;

  dist=(to-from)/2;

  /*printf("from %d, to %d, r %d\n",from,to,r);*/

  if (display_mode == 8)
    { 
      for (k=0; k<dist; k++)
	{
	  r1=*(uw8*)r++;
	  r2=*(uw8*)r++;
	  
	  *data=0;
	    for(i=0; i<4; i++)
	      {
		color= (colors[((r1&2)<<1)+((r2&3))+((r1&1)<<3)]);
		*data |= (color<<(2*i));
		*data |= (color<<(2*i+1));
		r1=r1>>2; r2=r2>>2;
	      }
	  data+=8;
	}
    }
  else // mode 4
    {
      for (k=0; k<dist; k++)
	{
	  r1=*(uw8*)r++;
	  r2=*(uw8*)r++;
	  ri1=r1>>(7-i);
	  ri2=r2>>(7-i);
	  /* reset values for every new scanline */
	  if ( ((k*2)%qlscreen.linel) == 0)
	    {
	      lastcolor=((ri1&1)<<2)+((ri2&1)<<1)+((ri1&1)&(ri2&1));
	      cc=0;
	      /*printf("lbreak, k %d, r %d \n",k,r);*/
	    }

	  *data=0;
	  for (i=0; i<8; i++)
	    {
	      ri1=r1>>(7-i);
	      ri2=r2>>(7-i);
	      color=((ri1&1)<<2)+((ri2&1)<<1)+((ri1&1)&(ri2&1));

	      if (color != lastcolor)
		{
		  if (mcolors[color] != mcolors[lastcolor])
		    cc=0;
		  else cc=1;
		}
	      else
		{
		  if ( cc ) cc=0;
		}
#ifndef TEST_MONO
	      /* probably bitmap order-endian..*/
	      *data |= ( cc ? invcolors[color] : mcolors[color]) <<i;
#else
	      *(data+i) = ( cc ? invcolors[color] : mcolors[color]);
#endif
	      lastcolor=color;
	    }
#ifndef TEST_MONO
	  data++;
#else
	  data+=8;
#endif
	}
    }
}



/* convert image from buffer, from, to */
void QL2Pixmap8 (w32 st, char *data, w32 from,w32 to)
{
  int r1, r2;
  int i, k;
  char *r;
  long dist;


  r=st+(Ptr)theROM;
   
  
  data+=(from-qlscreen.qm_lo)*4;
  r+=from-qlscreen.qm_lo;


  /*if (to>131072+32768)printf("to %d !!!\n",to); */
  dist=(to-from)/2;

  if (display_mode == 8)
    { 
      for (k=0; k<dist; k++)
	{
	  r1=*(uw8*)r++;
	  r2=*(uw8*)r++;
	  
	  for(i=0; i<4; i++)
	    {
	      /* That's a double pixel ! */ 
	      *(data+7-(i*2)) = colors[((r1&2)<<1)+((r2&3))+((r1&1)<<3)];
	      *(data+7-(i*2+1)) = colors[((r1&2)<<1)+((r2&3))+((r1&1)<<3)];
	      r1=r1>>2; r2=r2>>2;
	    }
	  data+=8;
	}
    }
  else  // mode 4
    {
      for (k=0; k<dist; k++)
	{
	  r1=*(uw8*)r++;
	  r2=*(uw8*)r++;
	  
	  for(i=0; i<8; i++)
	    {
	      *(data+7-i) = colors[((r1&1)<<2)+((r2&1)<<1)+((r1&1)&(r2&1))]; 
	      r1=r1>>1; r2=r2>>1;
	    }
	  data+=8;
	}
    }
  /*if (r-(long)theROM>131072+32*1024 || data-8-xi_buf>512*256)printf(".... to %d, buflen %d\n",r-(long)theROM,data-8-xi_buf);*/
}

void QL2Pixmap16 (w32 st, char *data, w32 from,w32 to)
{
  int r1, r2;
  int i, k;
  char *r;
  long dist;


  r=st+(Ptr)theROM;
   
  
  data+=(from-qlscreen.qm_lo)*pbytes*4;
  r+=from-qlscreen.qm_lo;


  /* printf("height %d, width %d\n",height,width); */
  dist=(to-from)/2;
  if (display_mode == 8)
    {  
      for (k=0; k<dist; k++)
	{
	  r1=*(uw8*)r++;
	  r2=*(uw8*)r++;
	  
	  for(i=0; i<4; i++)
	    {
	      unsigned short x;

	      x = colors[((r1&2)<<1)+((r2&3))+((r1&1)<<3)];
	      *(uw16*)((uw16*)data+7-(i*2)) = x; 
	      *(uw16*)((uw16*)data+7-(i*2+1)) = x; 
	      
	      r1=r1>>2; r2=r2>>2;
	    }
	  data+=16;
	}
    }
  else // mode 4
    {
      for (k=0; k<dist; k++)
	{
	  r1=*(uw8*)r++;
	  r2=*(uw8*)r++;
	
      for(i=0; i<8; i++)
	{
	  unsigned short x;
	  x = colors[((r1&1)<<2)+((r2&1)<<1)+((r1&1)&(r2&1))]; 
	  *(uw16*)((uw16*)data+7-i) = x;
	  
	  r1=r1>>1; r2=r2>>1;
	}
      data+=16;
	}
    }
}

/* this handles the obscure and broken case of 3 bytes/pixel,
 * old style 24 bit servers are now served using QL2Pixmap32
 */

#ifdef QM_BIG_ENDIAN
#define SSI 1
#else
#define SSI 0
#endif

static inline void setpix24(char *base, int index, int col)
{
  int i;
  char *src, *dest;
  dest=base+3*index;
  src=((char *)(&col))+SSI;
  for (i=0;i<3;i++) *dest++ = *src++;
}
void QL2Pixmap24 (w32 st, char *data, w32 from,w32 to)
{
  int r1, r2;
  int i, k;
  char *r;
  long dist;


  r=st+(Ptr)theROM;
   
  /* in theory we would need expensive line alignment
   * calculation, luckilly UQLX requires screen width
   * evenly divisible by 4 */
  
  data+=(from-qlscreen.qm_lo)*pbytes*4;
  r+=from-qlscreen.qm_lo;


  /* printf("height %d, width %d\n",height,width); */
  /*if (to>131072+32768)printf("to %d !!!\n",to); */

  dist=(to-from)/2;

  if (display_mode == 8)
    {
      for (k=0; k<dist; k++)
	{
	  r1=*(uw8*)r++;
	  r2=*(uw8*)r++;
	  
	  for(i=0; i<4; i++)
	    {
	      uint32_t x;
	      
	      x = colors[((r1&2)<<1)+((r2&3))+((r1&1)<<3)];
	      setpix24(data,7-(2*i),x);
	      setpix24(data,7-(2*i+1),x);
	      r1=r1>>2; r2=r2>>2;
	    }
	  data+=pbytes*8;
	}
    }
  else // mode 4
    {
      for (k=0; k<dist; k++)
	{
	  r1=*(uw8*)r++;
	  r2=*(uw8*)r++;
	  
	  for(i=0; i<8; i++)
	    {
	      uint32_t x;
	      
	      x = colors[((r1&1)<<2)+((r2&1)<<1)+((r1&1)&(r2&1))]; 
	      setpix24(data,7-i,x);
	      r1=r1>>1; r2=r2>>1;
	    }
	  data+=pbytes*8;
	}
    }
  /*if (r-(long)theROM>131072+32768 || data-8*pbytes-xi_buf>512*256*4)printf(".... to %d, buflen %d from %d\n",r-(long)theROM,data-8*pbytes-xi_buf,from);*/
}

void QL2Pixmap32 (w32 st, char *data, w32 from,w32 to)
{
  int r1, r2;
  int i, k;
  char *r;
  long dist;


  r=st+(Ptr)theROM;
    
  data+=(from-qlscreen.qm_lo)*pbytes*4/*plane2*/;
  r+=from-qlscreen.qm_lo;

  /* printf("height %d, width %d\n",height,width); */
  dist=(to-from)/2;

  if (display_mode == 8)
    {
      for (k=0; k<dist; k++)
	{
	r1=*(uw8*)r++;
	r2=*(uw8*)r++;
	
	for(i=0; i<4; i++)
	  {
	    uint32_t x;
	    x = colors[((r1&2)<<1)+((r2&3))+((r1&1)<<3)];
	    *(uint32_t *)((uint32_t *)data+7-(2*i)) = x;
	    *(uint32_t *)((uint32_t *)data+7-(2*i+1)) = x;
	    
            r1=r1>>2; r2=r2>>2;
	  }
	data+=pbytes*8;
	}
    }
  else // mode 4
    {
      for (k=0; k<dist; k++)
	{
	  r1=*(uw8*)r++;
	  r2=*(uw8*)r++;
	
	  for(i=0; i<8; i++)
	    {
	      uint32_t x;
	      
	      x = colors[((r1&1)<<2)+((r2&1)<<1)+((r1&1)&(r2&1))]; 
	      *(uint32_t *)((uint32_t *)data+7-i) = x;	      
	      r1=r1>>1; r2=r2>>1;
	    }
	  data+=pbytes*8;
	}
    }
}


/*#define TEST_REDRAW*/

extern int rx1,rx2,ry1,ry2,finishflag;  

void conv_chunk(w32 from, w32 to)
{
  int x1,x2,y1,y2,width,height;
  
  
  if (from<qlscreen.qm_lo) from=qlscreen.qm_lo;
  if (to>qlscreen.qm_hi) to=qlscreen.qm_hi;
  
  /* old code had wrong assumption that linel==2**n :( */
#if 1
  x1=(((from-qlscreen.qm_lo)%(qlscreen.linel)))*4;
  x2=min((((to-qlscreen.qm_lo)%(qlscreen.linel)))*4+4,qlscreen.xres-1);

  y1=(from-qlscreen.qm_lo)/qlscreen.linel;
  y2=(to-qlscreen.qm_lo)/qlscreen.linel;
#else
  x1=(from&(qlscreen.linel-1))*4;
  x2=min((to&(qlscreen.linel-1))*4+4,qlscreen.xres-1);

  y1=(from-qlscreen.qm_lo)/qlscreen.linel;
  y2=(to-qlscreen.qm_lo)/qlscreen.linel;
#endif

#ifdef TEST_REDRAW
  printf("conv_chunk %d %d ",from-qlscreen.qm_lo,to-qlscreen.qm_lo);
#endif

  if(y1!=y2) { x1=0; x2=qlscreen.xres-1; }
 
  if (x1>rx1) x1=rx1;
  if (x2<rx2) x2=rx2;
  if (y1>ry1) y1=ry1;
  if (y2<ry2) y2=ry2;
#if 0  
  width=min((x2-x1+4),qlscreen.xres);
  height=min((y2-y1+1),qlscreen.yres);
#endif
  
  (*QL2Pixmap)(vbase,xi_buf, from, to);

  displayFrom=displayTo=0;

  rx1=x1; rx2=x2;
  ry1=y1; ry2=y2;

#ifdef TEST_REDRAW
  printf("  %d,%d x %d,%d\n",rx1,rx2,ry1,ry2);
#endif
}


void do_scrconv()
{
  int i,ii,xfrom,xto; 
  long *ascr,*oscr;

  char *nxpage;
  
  xfrom=xto=i=0;

next_page:
  while (!scrModTable[i] && i<sct_size)
    {
      // scrModTable[i]=0;
      i++;
    }
  if (i>=sct_size) goto x_exit;
  
  ascr=(long *)((char*)theROM+qlscreen.qm_lo+PAGEX(i));
  //printf("%d,\t%d\n",oscr,PAGEX(i));
  
  oscr=(long*)((char*)oldscr+PAGEX(i));
  nxpage=(char*)oldscr+PAGEX(i+1)+2*sizeof(*oscr);

  if (nxpage>(char*)oldscr+qlscreen.qm_len)
    nxpage=(char*)oldscr+qlscreen.qm_len;
  
  /*scrModTable[i]=0;*/

next_change:
  while( *ascr++ == *oscr++ && 
	 ((char *)oscr <= nxpage) /*(char*)oldscr+32768+sizeof(*oscr))*/  );

  // -- stopped by crossing page boundary? continue with next page
  if ((char *)oscr > nxpage /*&& *(ascr-1)==*(oscr-1)*/ ) 
    {
      scrModTable[i]=0;
      goto next_page;
    }

  //printf("i1 %d\t\t",i);
  
  i= PAGEI((char*)(oscr-2)-oldscr);
  nxpage=(char*)oldscr+PAGEX(i+1)+2*sizeof(*oscr);
  if (nxpage>(char*)oldscr+qlscreen.qm_len)
    nxpage=(char*)oldscr+qlscreen.qm_len;
  

  //printf("i2 %d\n",i);
  // -- point to start of diff area again
  ascr--;
  oscr--;
  
  xfrom=(w32)((char*)ascr-(char*)theROM);
  ii=i;
  
  while( (char *)oscr<(char*)oldscr+qlscreen.qm_len+2*sizeof(long) &&
	 *ascr++!=*oscr++ )  
    *(oscr-1)=*(ascr-1);    /* actualise cmp/shadow buffer */

  /*if ((char *)oscr >= oldscr+(i+1)*pagesize+2*sizeof(*oscr)) /*goto next_page;*/

  i=(int)PAGEI((char*)oscr-oldscr);
  //printf("page: %d, state %d\n",i,scrModTable[i]);
  
  xto=(w32)((char*)ascr-(char*)theROM);
  
  if(xfrom<qlscreen.qm_hi)
    conv_chunk(xfrom,xto);

  if (i<sct_size && ((char*)oscr-oldscr)<qlscreen.qm_len){
    if (i==ii)
      goto next_change;
    else goto next_page;
  }

x_exit:
  for(i=0;i<sct_size;i++) scrModTable[i]=0;

  //uqlx_prestore(qlscreen.qm_lo,qlscreen.qm_hi-qlscreen.qm_lo/*,QX_SCR*/);
#if 1 /* hack, above did loose some updates */
  // must not change protecion before memory test has completed
  // otherwise would result in erroneous sys.rtop 
  if (do_update)
    uqlx_protect(qlscreen.qm_lo,qlscreen.qm_len,QX_SCR);
#endif

  if (finishflag)
    draw_chunk();

  //printf("exiting scrconv\n");
}


void FlushDisplay()
{
  finishflag=1;
  if (screen_drawable)
    do_scrconv();
  finishflag=0;
}


void scrconv()
{
  finishflag=1;
  if (screen_drawable)
    do_scrconv();
  finishflag=0;
}


void draw_chunk()
{ 
  int width,height;
  
#if 0
  printf("draw_chunk %d %d %d %d\n",rx1,ry1,rx2,ry2);
#endif

  if (rx1>rx2 || ry1>ry2) return;

  width=rx2-rx1+1; height=ry2-ry1+1;
  
  QClip(&rx1,&ry1,&width,&height);

  if (! screen_drawable) return;
#ifdef TEST_REDRAW
  printf("...drawing %d,%d    %d,%d\n",rx1,ry1,width,height);
#endif
  
#ifdef SH_MEM
  if (shmflag) 
    XShmPutImage(display,imagewin,gc,image,
		 rx1,ry1,            /* src x,y */
		 rx1,ry1,             /* dest x,y */
		 width, height,     /* width,heigth */
		 False);
  else 
#endif
    XPutImage(display, imagewin, gc, image,
		 rx1, ry1,        
		 rx1, ry1,        
		 width, height ); 

  rx1=qlscreen.xres;rx2=0;ry1=qlscreen.yres;ry2=0;
  /*displayFrom=displayTo=0;*/

  XFlush (display);
  /*process_events();*/

}


void redraw_screen( int x1,int y1,int width,int height)
{
  int x2,y2;

#if 0
 printf("redraw screen: %d,%d %d,%d\n",x1,y1,x1+width,y1+height); 
#endif
  x2=x1+width;
  y2=y1+height;

  if (x1>rx1) x1=rx1;
  if (x2<rx2) x2=rx2;
  if (y1>ry1) y1=ry1;
  if (y2<ry2) y2=ry2; 

#if 0
 printf(".... combined to: %d,%d %d,%d\n",x1,y1,x2,y2); 
#endif

 QClip(&x1,&y1,&width,&height);

#if 0
 printf("... clipped to: %d,%d %d,%d\n",x1,y1,width,height); 
#endif

 
#ifdef SH_MEM
   if (shmflag)
      XShmPutImage(display,imagewin,gc,image,
                   x1,y1,            /* src x,y */
		   x1,y1,             /* dest x,y */
		   width,height,     /* width,heigth */
		   False);
   else
#endif
      XPutImage(display, imagewin, gc, image,
	      x1, y1,        
	      x1, y1,        
	      width, height ); 
  
      rx1=qlscreen.xres;rx2=0;ry1=qlscreen.yres;ry2=0; /* nothing to do*/

  /*XFlush (display); */ /* done in x.c:process_events*/

}

