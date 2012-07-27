/*
 * (c) UQLX - see COPYRIGHT
 */


#include "QL68000.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>

#include "unix.h"
#include "qx_proto.h"

#ifdef __EMX__
#include <os2emx.h>
#define strncasecmp strncmp /* HACK !!! */
#endif

#ifdef TEST
# define STATICDEVS
#endif
#include "emudisk.h"
#include "uqlx_cfg.h"

QMDATA QMD = {
    qdevs,                  /* ddvlist */
    NULL,                   /* romlist */
    4096,                   /* RAM top */
    "/ql/",                 /* rom dir */
    1,                      /* color   */
    "jsrom",                /* system ROM */
    "",            /* ser1 */
    "",            /* ser2 */
    "",            /* ser3 */
    "",            /* ser4 */
    "lpr",                  /* print cmd */
#if 0
    "/home/rz/.qldir/IMG1", /* hda ide emulation img */                    
    "/home/rz/.qldir/IMG2", /* hda ide emulation img */                    
    "/home/rz/.qldir/IMG3", /* cdrom ide emulation img */
#endif
    1,                      /* CPU hog=true */
    1,                        /* fastStartup=true */
    1,                    /* use real white instead of gray95 */
    0,                    /* ! use different key-read method as default */ 
    1,
    1,             /* do strict locking for disk images */
    0,                    /* no_patch, default off */

    -1,            /* no preferred visualId       */
    -1,            /*         ...  display depth  */
    "",            /*         ... visual class    */

    "632x400",                      /* x size screen */
    "720x512",                      /* xx .. */
    "800x600",
    "F11",              /* Keysym to toggle key-read method */ 
    "F12"            /* use as QDOS ALT key  ...*/
};

static char *strim (char *s)
{
    char *p;
    short n;
    char *c;

    n = strlen(s);
    p = s + n;
    while( *--p <= ' ' && n--)
    {
        *p = '\0';
    }
    c=strchr(s,'#');
    if (c) *c=0;
    return s;
}

void CheckDev (EMUDEV_t *qd, char *d1, char *d2, char *d3 )
{
  /*DIR *dirp;*/
    short ndev = 0;
    short len = 0;
    short idev = -1;
    short lfree = -1;
    short i;
    char tmp[400];
    int err;

    struct stat sbuf;
    
    
    len = strlen(d1);
    if(isdigit(*(d1+len-1)))
    {
	len--;
	ndev = *(d1+len) - '0';
	*(d1+len) = 0;
    }
    else
    {
	ndev = -1;
    }

    for(i = 0; i < MAXDEV; i++)
    {
	if((qd+i)->qname && (strncasecmp(qd[i].qname, d1, len) == 0))
	{
	    idev = i;
	    break;
	}
	else if(qd[i].qname == NULL && lfree == -1)
	{
	    lfree = i;
	}
    }

    if ( idev==-1 && lfree==-1)
      {
	printf("sorry, no more free entries in Directory Device Driver table\n");
	printf("check your ~/.uqlxrc if you really need all this devices\n");
	
	return ;
      }
      
    if(idev != -1 && ndev == 0)
    {
	memset ((qd+idev), '\0', sizeof(EMUDEV_t));
    }
    else
    {
	if(lfree != -1)
	{
	    idev = lfree;
	    (qd+idev)->qname = strdup(d1);
	}
	if(ndev && ndev < 9)
	{
	    if(d2 && *d2)
	    {
		short dlen;
		char *dnam;
		
		if(*d2 == '~')
		{
		  d2++;
		  strncpy(tmp,getenv("HOME"),400);
		  if (*d2) strncat(tmp,d2,400);
		}
		else strncpy(tmp,d2,400);
		
		dlen = strlen(tmp);
		if(dnam = malloc(dlen+16))
		{
		    sprintf(dnam, tmp, getpid());
		    dlen = strlen(dnam);
		    if(*(dnam+dlen-1) != '/')
		    {
			dlen++;
		    }
		    err=stat(dnam,&sbuf);
		    if (V1 && err<0 && strcasecmp((qd+idev)->qname,"ram") )
		      {
			perror("problem, stat failed ");
			printf(" - MountPoint %s for device %s%d_ may not be accessible\n",dnam,(qd+idev)->qname,ndev);

		      }
		    else
		      {
			if (sbuf.st_mode==S_IFDIR)
			  {
			    *(dnam+dlen-1) = '/';
			    *(dnam+dlen) = '\0';	
			  }
			else
			  {
			    /**/
			  }
		      }
	
		    (qd+idev)->mountPoints[ndev-1] = dnam;
		    (qd+idev)->Present[ndev-1] = 1;
		}
	    }
	    else
		(qd+idev)->Present[ndev-1] = 0;

	    if(d3)
	    {
	      int flag_set=0;
	      
	      if ((strstr(d3,"native") != NULL) ||
		  (strstr(d3,"qdos-fs") != NULL))
		flag_set |= (qd+idev)->Where[ndev-1] = 1 ;
	      else if ( strstr(d3,"ide-emu") != NULL )
		flag_set |= (qd+idev)->Where[ndev-1] = 4 ;
	      else if ( strstr(d3,"cd-emu") != NULL )
		flag_set |= (qd+idev)->Where[ndev-1] = 8 ;
	      else if ( strstr(d3,"qdos-like") != NULL )
		flag_set |= (qd+idev)->Where[ndev-1] = 2 ;

	      flag_set |= (qd+idev)->clean[ndev-1] = 
		(strstr(d3,"clean") != NULL);
	      if ( !flag_set )
		printf("WARNING: flag %s in definition of %s%d_ not recognised\n",
		       d3,d1,ndev);
	    }
	}		
    }
}
 
static void ParseDevs (EMUDEV_t **qd, char *s)
{
    char *p1, *p2;
    short n;
    char *d1,*d2,*d3;

    d1=d2=d3=NULL;
    
    for (p1 = s, n = 0; ; n++)
    {
	short k;
	char c, *p3;
	
	p2 = strchr (p1, ',');
	if (p2)
	{
	    k = (p2 - p1);
	}
	else
	{
	    k = strlen (p1);
	}
	p3 = (p1 + k);
	c = *p3;
	*p3 = 0;
	switch (n)
	{
	  case 0:
	    d1 = p1;
	    break;
	  case 1:
	    d2 = p1;
	    break;
	  case 2:
	    d3 = p1;
	    break;
	}

	if (!(c))
	{
	    break;
	}
	p1 = p2 + 1;
    }
    if(d1 && *d1) 
    {
	CheckDev(*qd, d1, d2, d3 );
    }
}


static void * ParseROM (QMLIST * pn, char *s) 
{
    char *p1, *p2;
    short n;
    ROMITEM *cp;

    if((cp = (ROMITEM *) calloc (sizeof(ROMITEM),1)))
    {
        pn->next = NULL;
        pn->udata = cp;

	for (p1 = s, n = 0; ; n++)
	{
	    short k;
	    char c, *p3;

	    p2 = strchr (p1, ',');
	    if (p2)
	    {
		k = (p2 - p1);
	    }
	    else
	    {
		k = strlen (p1);
	    }
	    p3 = (p1 + k);
	    c = *p3;
	    *p3 = 0;

	    switch (n)
	    {
	      case 0:
		cp->romname = (char *) strdup (p1);
		break;
	      case 1:
		cp->romaddr = strtol (p1, NULL, 0);
		break;
	    }
	    if (!(*p3 = c))
	    {
		break;
	    }
	    p1 = p2 + 1;
	}
    }
    return cp;
}

static void  ParseList (void * p, char *s,  void * (*func)(void *, char *))
{
    QMLIST *pn, **ph;
    
    ph = (QMLIST **) p;
    if((pn = (QMLIST *) calloc (sizeof(QMLIST) ,1)))
    {
	if((func)(pn, s) != NULL)
	{
            if(*ph == NULL)
            {
                *ph = pn;
            }
            else
            {
                QMLIST *pz,*pl;
                for(pl = *ph; pl; pl = pl->next)
                {
                    pz = pl;
                }
                pz->next = pn;
            }
        }
	else
	{
	    free(pn);
	}
    }
}

static void  pString (char *p, char *s,...)
{
    va_list va;
    int n;
    
    if(*s)
    {
        va_start(va, s);
        n = va_arg(va, int);
        strncpy(p, s, n);
	*(p+n) = 0;
    }
}

static void  pInt4 (long *p, char *s,...)
{
    *p  = strtol(s,NULL,0);
}

static void  pInt2 (short *p, char *s,...)
{
    *p  = (short) strtol(s,NULL,0);
}

static PARSELIST pl[] = {  
{"DEVICE", (PVFV) ParseDevs, offsetof(QMDATA, qdev) },
{"ROMIM",  (PVFV) ParseList, offsetof(QMDATA, romlist), (uxt)ParseROM},
{"PRINT",  (PVFV) pString, offsetof(QMDATA, prtcmd), 63},
#if 0
{"HDA",    (PVFV) pString, offsetof(QMDATA, hda), 511},
{"HDB",    (PVFV) pString, offsetof(QMDATA, hdb), 511},
{"CDROM",    (PVFV) pString, offsetof(QMDATA, cdrom), 511},
#endif
{"SER1",   (PVFV) pString, offsetof(QMDATA, ser1), 63},
{"SER2",   (PVFV) pString, offsetof(QMDATA, ser2), 63},
{"SER3",   (PVFV) pString, offsetof(QMDATA, ser2), 63},
{"SER4",   (PVFV) pString, offsetof(QMDATA, ser2), 63},
{"ROMDIR", (PVFV) pString, offsetof(QMDATA, romdir),  127},
{"SYSROM", (PVFV) pString, offsetof(QMDATA, sysrom),  63},
{"RAMTOP", (PVFV) pInt4, offsetof(QMDATA, ramtop) },
{"COLOUR", (PVFV) pInt2, offsetof(QMDATA, color) },
{"CPU_HOG",(PVFV) pInt2, offsetof(QMDATA, cpu_hog) },
{"REAL_WHITE",(PVFV) pInt2, offsetof(QMDATA, fwhite) },
{"FAST_START",(PVFV) pInt2, offsetof(QMDATA, fastStartup) },
{"XKEY_ON",(PVFV) pInt2, offsetof(QMDATA, xkey_on) },
{"STRICT_LOCK",(PVFV) pInt2, offsetof(QMDATA, strict_lock) },
{"NO_PATCH",(PVFV) pInt2, offsetof(QMDATA, no_patch) },
{"DO_GRAB",(PVFV) pInt2, offsetof(QMDATA, do_grab) }, 

{"XVID",(PVFV) pInt2, offsetof(QMDATA, pref_vid) },
{"XDEPTH",(PVFV) pInt2, offsetof(QMDATA, pref_depth) },
{"XVIS_CLASS",(PVFV) pString, offsetof(QMDATA, pref_class), 63},

{"SIZE_1",   (PVFV) pString, offsetof(QMDATA, size_x), 127},
{"SIZE_2",   (PVFV) pString, offsetof(QMDATA, size_xx), 127},
{"SIZE_3",   (PVFV) pString, offsetof(QMDATA, size_xxx), 127},
{"XKEY_SWITCH",   (PVFV) pString, offsetof(QMDATA, xkey_switch), 127},
{"XKEY_ALT",  (PVFV) pString, offsetof(QMDATA, xkey_alt), 127},
{NULL, NULL},
};

FILE *lopen(const char *s, const char *mode)
{
    FILE *fp;
#ifndef __EMX__
    char fnam[PATH_MAX];
#else
    char fnam[CCHMAXPATH];
#endif
    
    if(*s == '~')
    {
        char *p = fnam;
	strcpy(p, getenv("HOME"));
	strcat(p, s + 1);
	s = p;
    }
    
    fp = fopen(s, mode);
    if(fp == NULL)
    {
	char pname[512];
	char *pf;
	
	if((pf = getenv("HOME")))
	{
	    short n;
	    
	    strcpy(pname, pf);
	    n = strlen(pname);
	    if(*(pname + n-1) != '/' 
#ifdef __NT__
                && *(pname + n-1) != '\\'
#endif
                )
	    {
		*(pname + n) = '/';
		*(pname + n+1) = '\0';
	    }
	    strcat(pname, s);
	    fp = fopen(pname, mode);
	}
    }

    return fp;
}

int QMParams (void)
{
    FILE *fp;
    char *pf;
    int rv = 0,iil;
    QMDATA *p;
    
    if((pf = getenv("UQLX_CFG")) == NULL)
    {
        pf = QMFILE;
    }

    p = &QMD;

    if(!(fp = lopen(pf, "r"))) 
      {
	if ( !strcmp(pf,QMFILE) )
	  {
	    char buf[PATH_MAX*2+1];
	    char *loc;

	    loc=qm_findx("do_install");

	    strncpy(buf,loc,PATH_MAX+1);
	    qaddpath(buf,"do_install ",PATH_MAX+1);
	    strncat(buf,IMPL,PATH_MAX+1);
	    printf("configuration file %s not found, trying to create\n",pf);
	    system(buf);
	    fp = lopen(pf, "r");
	  }
      }

    if(fp) 
    {
        char buff[128];
        char *ptr;
        ptr = (char *)p;

        while(fgets(buff, 128, fp) == buff)
        {
            char *s;
            PARSELIST *ppl;

            strim(buff);

            for(ppl = pl; ppl->id; ppl++)
            {  
                if(strncasecmp(buff, ppl->id, strlen(ppl->id)) == 0)
                {
                    int l;

                    if((s = strchr(buff, '=')))
                    {
                        s++;
                        l = strspn(s, " \t");
                    }

                    if(s)
                    {
                        (ppl->func)((ptr + ppl->offset), s + l, ppl->mx.mfun);
                        break;
                    }
                }
            }
        }
        fclose(fp);
    }
    else printf("Warning: could not find %s\n",pf);
}

#ifdef TEST
void ShowROMS (QMLIST *p, char *title)
{
    fputs (title, stdout);
    for(;p;p=p->next)
    {
        ROMITEM *c = p->udata;
        printf("%s %lx\n", c->romname, c->romaddr);
    } 
}


int main(int ac, char **av)
{
    short i;
    static QMDATA ff = {0};    

    QMParams(&ff);

    printf ("Top             : %ld\n", ff.ramtop);
    printf ("ROM Dir         : %s\n",  ff.romdir);
    printf ("Colour          : %d\n", (long) ff.color);
    printf ("SYSrom          : %s\n", ff.sysrom);
    printf ("ser1            : %s\n", ff.ser1);
    printf ("ser2            : %s\n", ff.ser2);
    printf ("prt             : %s\n", ff.prtcmd);

    ShowROMS(ff.romlist, "ROMS\n");
    for(i = 0; i < MAXDEV; i++)
    {
	if((qdevs+i)->qname)
	{
	    short j;
	    
	    for(j = 0; j < 8; j++)
	    {
		if ((qdevs+i)->Present[j])
		    printf("%s%d_ %d %d %s\n",
			   (qdevs+i)->qname, j+1,
			   (qdevs+i)->Where[j],
			   (qdevs+i)->clean[j],
			   (qdevs+i)->mountPoints[j]);
	    }
	}
    }
}
#endif
