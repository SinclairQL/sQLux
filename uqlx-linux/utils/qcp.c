
/*-------------------------------------------------------------------------.
| program to set uqlx data sizes for QDOS executables from	           |
| Unix. Features automatic recognition of xtc68 data space. If you         |
| give an outfile/path, the infile is copied.			           |
| 								           |
| Install somewhere on your path (i.e. /usr/local/bin)                     |
| 								           |
| qcp [-x data_space] infile [outfile]				           |
| 								           |
| You should give one of -x data or outfile.			           |
| 								           |
| $ qcp -x 1776 /QL/exe/gs #(set gs data size)                             |
| $ qcp	~/develop/qdos/gs262/gs /QL/exe #(copy and set data size for xtc68 |
|                                       # image)                           |
| 								           |
| n.b. symbolic links are very useful to reduce unix file to meet QDOS     |
|      file name restrictions                                              |
| 								           |
| totally not (c) Jonathan Hudson                                          |
`-------------------------------------------------------------------------*/

#define NEED_STPCPY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

#ifdef NOINLINE
# define inline
#endif

#ifdef  __GNUC__
# define PACKED  __attribute__ ((packed))
#else
# define PACKED
#endif

typedef struct
{
    long d_length;              /* file length */
    unsigned char d_access;     /* file access type */
    unsigned char d_type;       /* file type */
    long d_datalen PACKED;      /* data length */
    long d_reserved PACKED;     /* Unused */
    short d_szname;             /* size of name */
    char d_name[36];            /* name area */
    long d_update PACKED;       /* last update */
    short d_version;
    short d_fileno;
    long d_backup;
} QLDIR_t;

static short is_big_endian;

ushort inline swapword (ushort val)
{
    return (is_big_endian) ? val : (ushort) (val << 8) + (val >> 8);
}

ulong inline swaplong (ulong val)
{
    return (is_big_endian) ? val :
	(ulong) (((ulong) swapword (val & 0xFFFF) << 16) |
                    (ulong) swapword (val >> 16));
}

#ifdef NEED_STPCPY
char *stpcpy(char *d, const char *s)
{
    while (*d++ = *s++) /* NULL loop */;
    return d-1;
}
#endif

long CheckXTcc(char * filename)
{
    long len;
    
    static struct 
    {
	union 
	{
	    char xtcc[4];
	    long x;
	} x;
	long dlen;
    } fdat, xtcc =  {{"XTcc"},0};
    int fd;

    len = 0;
    if((fd = open(filename, O_RDONLY, 0666)) > 0)
    {
	lseek(fd, -8, SEEK_END);
	read(fd, &fdat, sizeof(xtcc));
	close(fd);
	if(fdat.x.x == xtcc.x.x)
	{
	    len = swaplong(fdat.dlen);
	}
    }
    return len;
}

void usage(void)
{
    fputs("$ qcp [-x dataspace] infile [outfile]\n", stderr);
    exit(0);
}

int main(int ac, char **av)
{
    long one = 1;
    long dspac = 0;
    int c;
    char *inf = NULL, *ouf = NULL;
    char onam[PATH_MAX], dirnam[PATH_MAX], secret[PATH_MAX];
    
    is_big_endian = 1 - *(char *)&one;

    while((c = getopt(ac, av, "x:")) != EOF)
    {
	switch(c)
	{
	  case 'x':
	    dspac = strtol(optarg, NULL, 0);
	    break;
	  default:
	    usage();
	    break;
	}
    }
    
    for ( ;optind < ac; optind++)
    {
	if(!inf)
	{
	    inf = *(av+optind);
	}
	else if (!ouf)
	{
	    ouf = *(av+optind);
	}
	else
	{
	    usage();
	}
    }

    if(inf)
    {
	char *p,*q;
	int fd;
	QLDIR_t qd;
	short nlen;
	struct stat s;

	if(dspac == 0)
	{ 
	    dspac = CheckXTcc(inf);
	}

	if(dspac)
	{
	    if(ouf)
	    {
		stat(ouf, &s);
		if(S_ISDIR(s.st_mode))
		{
		    p = stpcpy(onam, ouf);
		    if(*(p-1) != '/')
		    {
			*p++ = '/';
		    }
		    if((q = strrchr(inf, '/')))
		    {
			q++;
		    }
		    else
		    {
			q = inf;
		    }
		    strcpy(p, q);
		}
		else
		{
		    strcpy(onam, ouf);
		}
	    }
	    else
	    {
		strcpy(onam, inf);
	    }
	    
	    p = strrchr(onam, '/');
	    if(p)
	    {
		int n;
		
		p++;
		n = p-onam;
		strncpy(secret, onam, n);
		q = p;
		p = secret+n;
	    }
	    else
	    {
		p = secret;
		q = onam;
	    }
	    
	    strcpy(p, ".-UQLX-");
	    nlen = strlen(q);

	    if((fd = open(secret, O_RDWR, 0666)) > -1)
	    {
		while (read (fd, &qd, sizeof(qd)) == sizeof(qd))
		{
		    if(/*qd.d_length &&*/ nlen == swapword(qd.d_szname) &&
		       strncasecmp(qd.d_name, q, nlen) == 0)
		    {
			lseek(fd, -1*sizeof(qd), SEEK_CUR);
			break;
		    }
		}
	    }
	    else
	    {
		fd = open(secret, O_CREAT|O_WRONLY, 0666);
	    }
	    if(fd >= 0)
	    {
		qd.d_type = 1;
		qd.d_access = 0;
		qd.d_length = (stat(onam, &s) == 0) ? swaplong(s.st_size) : 1;
		qd.d_datalen = swaplong(dspac);
		qd.d_szname = swapword(nlen);
		strncpy(qd.d_name, q, nlen);
		write(fd, &qd, sizeof(qd));
		close(fd);
		if(ouf)
		{
		    int fo, n;
		    if((fd = open(inf, O_RDONLY, 0)) >= 0)
		    {
			if((fo = open(onam, O_CREAT|O_WRONLY|O_TRUNC, 0666))
			   >= 0)
			{
			    char buf[1024];
			    while((n = read(fd, buf, sizeof(buf))) > 0)
			    {
				write(fo, buf, n);
			    }
			    close(fo);
			}
			close(fd);
		    }
		}
	    }
	}
	else fputs("Data space required\n", stderr);
    }
    else usage();

    return 0;
}

