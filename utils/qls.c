/*-------------------------------------------------------------------------.
| program to read uqlx data sizes for QDOS executables from Unix           |
| 								           |
| Install somewhere on your path (i.e. /usr/local/bin)                     |
| 								           |
| qls path                           				           |
| 								           |
| Where path is a directory contains a .-UQLX- file		           |
| 								           |
| totally not (c) Jonathan Hudson                                          |
`-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>

#define NEED_STPCPY

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

void usage(void)
{
    fputs("$ qls path\n", stderr);
    exit(0);
}

#ifdef NEED_STPCPY
char *stpcpy(char *d, const char *s)
{
    while (*d++ = *s++) /* NULL loop */;
    return d-1;
}
#endif


int main(int ac, char **av)
{
    long one = 1;
    char secret[PATH_MAX];
    char *p,*q;
    int fd;
    QLDIR_t qd;
    short nlen;
    struct stat s;
    
    is_big_endian = 1 - *(char *)&one;

    if(*(av+1) && stat(*(av+1), &s) == 0 && (S_ISDIR(s.st_mode)))
    {
	p = stpcpy(secret, *(av+1));
	if(*(p-1) != '/')
	{
	    *p++ = '/';
	}
	q = p;
	
	strcpy(p, ".-UQLX-");
	
	if((fd = open(secret, O_RDONLY, 0)) >= 0)
	{
	    char fnam[PATH_MAX];
	    int n;
	    short len;
	    
	    n = q-secret;
	    strncpy(fnam, secret, n);
	    
	    while (read (fd, &qd, sizeof(qd)) == sizeof(qd))
	    {
		len = swapword(qd.d_szname);
		strncpy(fnam+n, qd.d_name, len);
		*(fnam+n+len) = 0;
		 
		if (qd.d_length==0) continue;
		
		if(stat(fnam, &s) == 0)
		{
		    struct tm * tm;
		    tm = localtime(&s.st_mtime);
		    printf("%-36.*s%9d%8d%4d %02d/%02d/%02d %02d:%02d:%02d\n",
			   len, qd.d_name, s.st_size,
			   swaplong(qd.d_datalen), qd.d_type,
			   tm->tm_mday, tm->tm_mon+1, tm->tm_year,
			   tm->tm_hour, tm->tm_min, tm->tm_sec);
		}
	    }
	}
	close(fd);
    }
    else usage();
    return 0;
}


