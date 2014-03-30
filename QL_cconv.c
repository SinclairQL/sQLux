/*
 * (c) UQLX - see COPYRIGHT
 */



#include "QL68000.h"

#include <stdio.h>
#include <string.h>
#include "QL_config.h"
#include "QInstAddr.h"
#include "qx_proto.h"



/* converts buffer using QDOS tra table, dest must have sufficient length! */
/* TRA table structure, from PiQ:
   basis 0000 $4AFB Konventionszahl
     0002 liste1-basis Ofs zu den Einzelzeichen   : liste1-* oder 0 ?!!
     0004 liste2-basis Ofs zu den Zeichengruppen  : liste2-* !!
   liste1 0006 zeichen.00 Code für CHR$(0)
     0007 zeichen.01 Code für CHR$(1)
   ... bis CHR$(255)
   liste2 0000 anzahl.gruppen  Byte, ggf. Null
     0001 zeichen.x0 erste zu ersetzende Nummer
     0002 ausgabe.x0.1 bis zu drei Zeichen, die
   stattdessen gesendet werden;
   auf Viererposten mit Null aufgefüllt.
     0005 zeichen.x1 nächste zu ersetzende Nummer,
   wie oben
   ... etc. ...
     xxxx zeichen.nn bis alle mit Null markierten Zeichen der ersten
   Anordnung definiert sind.
*/


int tra_conv(char *dest, char *src,int len)
{
  uw32 tratab;
  uw32 tab1,tab2,tab2cnt,tab2data;
  char *pd,*ps,c;

  if (len==0) return 0;

  tratab=ReadLong(0x28146);

  if (tratab==0)
    {     
      memcpy(dest,src,len);
      return len;
    }
  
  if ( ReadWord(tratab) != 0x4afb)
    {
      printf("illegal TRA table %x!\n",tratab);
      memcpy(dest,src,len);
      return len;
    }

  if (ReadWord(tratab+2)==0) /* minerva specialitaet ? */
    {
      memcpy(dest,src,len);
      return len;
    }

  tab1=ReadWord(tratab+2)+tratab+2;
  tab2=ReadWord(tratab+4)+tratab+4;
  tab2cnt=ReadByte(tab2);
  tab2data=tab2+1;
  
  for (pd=dest,ps=src; len-->0; ps++)
    {
      if ( (c=ReadByte(tab1+*ps)) ) *pd++=c;
      else /* the complicated case .. */
	{
	  int i=0;
	  while(i++<tab2cnt)
	    {
	      if ( c==ReadByte(tab2data+4*i))
		{
		  *pd++=ReadByte(tab2data+4*i+1);
		  if (ReadByte(tab2data+4*i+2))
		    {
		      *pd++=ReadByte(tab2data+4*i+2);
		      if (ReadByte(tab2data+4*i+3))
			  *pd++=ReadByte(tab2data+4*i+3);
		    }
		}
	    } 
	  if (i==tab2cnt) 
	    {	    
	      printf("no translation for char %d\n",c);
	      *pd++=c;
	    }
	}
    }
  return pd-dest;
}



/* table of ISO-latin1 -> QL translations*/
static struct  
{
  int iso; 
  int ql;
} codetable[] =
  {
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
static unsigned char x__iso2ql(unsigned c)
{
  int i;
  
  for(i=0; codetable[i].iso!=0; i++)
    if (codetable[i].iso==c)
      return codetable[i].ql;
  return c;
}
static unsigned char x__ql2iso(unsigned c)
{
  int i;
  
  for(i=0; codetable[i].iso!=0; i++)
    if (codetable[i].ql==c)
      return codetable[i].iso;
  return c;
}



unsigned char i2q[256];
unsigned char q2i[256];


void init_iso()
{
  int i;
  
  for(i=0;i<256;i++) i2q[i]=x__iso2ql(i);
  for(i=0;i<256;i++) q2i[i]=x__ql2iso(i);
}


int iso2ql(int c)
{
  return i2q[c];
}
int ql2iso(int c)
{
  return q2i[c];
}

int iso2ql_mem(unsigned char * buf,int len)
{
  while(len-->0)
    buf[len]=i2q[buf[len]];
}
int ql2iso_mem(unsigned char * buf,int len)
{
  while(len-->0)
    buf[len]=q2i[buf[len]];
}
