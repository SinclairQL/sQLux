

/*--------------------.
| TTy driver for uqlx |
`--------------------*/

#ifdef SERIAL

/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#if 0 /*ndef __linux__*/
#include <term.h>
#endif
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#if 0
#include <sys/termio.h>
#endif

#include "QSerial.h"
#include "QDOS.h"
#include "QL_cconv.h"
#include "driver.h"
#include "SqluxOptions.hpp"
#include "uqlx_cfg.h"

#ifdef __linux__
#include <sys/ioctl.h>
/* #include <linux/fs.h> */
#include <linux/tty.h>
#include <linux/serial.h>
#endif

/*#define IOTEST*/

struct {
	int baud;
	int code;
} speeds[] = { { 300, B300 },	  { 600, B600 },      { 1200, B1200 },
	       { 2400, B2400 },	  { 4800, B4800 },    { 9600, B9600 },
	       { 19200, B19200 }, { 38400, B38400 },
#ifdef __linux__
	       { 57600, B38400 }, { 115200, B38400 },
#else
#if !(defined(sgi) || defined(SUNOS) || defined(EMX))
	       { 57600, B57600 }, { 115200, B115200 },
#endif
#endif
	       { 0, 0 }

};

#define OPTC 1
#define OPTZ 0

int ser_read(serdev_t *sd, void *buf, int pno)
{
	long count = pno;
	long ci;
	long res;
	char *c;

	ci = 0;
	c = buf;

	/* check for chr$(26) == EOF */
	if (sd->teof) {
		sd->teof = 0;
		return 0;
	}

#ifdef NO_FIONREAD
	if (sd->bfc_valid) {
		*c++ = sd->bfc;
		sd->bfc_valid = 0;
		count--;
		ci = 1;
	}
#endif
	res = readio(sd, c, &count, 0);

	if (res == QERR_EOF)
		sd->teof = 1;

	if (sd->xlate == 3 && count + ci > 0)
		iso2ql_mem(buf, count + ci);

	/*printf("readio result: %d count: %d\n",res,count);*/
	if (!res || count > 0 || ci)
		return count + ci;
	else
		return res;
}

#define CV_BUFF 8192
static char cv_buff[CV_BUFF];

char *cva(int num)
{
	if (num > CV_BUFF)
		return malloc(num);
	else
		return cv_buff;
}
void cvf(char *buff)
{
	if (buff < cv_buff || buff > cv_buff + CV_BUFF)
		free(buff);
}
int ser_write(serdev_t *sd, void *buf, int pno)
{
	long count = pno;
	long res;
	char *conv;
	int xf = 0;

#ifdef NO_FIONREAD
#endif

	if (sd->xlate == 3 && pno > 0) {
		conv = cva(pno);
		memcpy(conv, buf, pno);
		xf++;
		buf = conv;
		ql2iso_mem(buf, pno);
	}

	res = writeio(sd, buf, &count);
	if (xf)
		cvf(conv);

	if (!res || count > 0)
		return count;
	else
		return res;
}

#ifdef NEWSERIAL

open_arg ser_par[6];

extern serdev_t *sparams[MAXSERIAL + 1];

int ser_init(int idx)
{
	int i;

	for (i = 0; i <= MAXSERIAL; i++)
		sparams[i] = NULL;

	return 0;
}

int ser_test(int id, char *name)
{
	return decode_name(name, Drivers[id].namep, ser_par);
}

int ser_open(int id, void **priv)
{
	serdev_t *p;
	const char *portnam;
	int unit = ser_par[0].i;

	if (unit < 1 || unit >= MAXSERIAL)
		return -1;

	*priv = p = malloc(sizeof(serdev_t));

	p->unit = unit;
	p->parity = (long)ser_par[1].i;
	p->hshake = (long)ser_par[2].i;
	p->xlate = (long)ser_par[3].i;
	p->baud = (long)ser_par[5].i; /* par 4 is dummy ! */
#ifdef NO_FIONREAD
	p->bfc_valid = 0;
#endif
	p->killed = 0;
	p->teof = 0;

	switch (unit) {
	case 1:
		portnam = optionString("ser1");
		break;
	case 2:
		portnam = optionString("ser2");
		break;
	case 3:
		portnam = optionString("ser3");
		break;
	case 4:
		portnam = optionString("ser4");
		break;
	}
	sparams[unit] = p;

	if (tty_open(portnam, p) > 0)
		return 0;
	else
		return -1;
}

int ser_pend(serdev_t *p)
{
#ifdef NO_FIONREAD
	long count;
	int res;

	if (p->bfc_valid)
		return 0;

	count = 1;
	res = readio(p, &(p->bfc), &count, 0);
	/*printf("pend: readio result: %d, count %d\n",res,count);*/
	if (!res && count == 1) {
		p->bfc_valid = 1;
		return 0;
	}
	if (res == -10)
		return QERR_EF;
	else
		return 0;
#else
	return pendingio(p->fd);
#endif
}
void ser_io(int id, serdev_t *priv)
{
	io_handle(ser_read, ser_write, ser_pend, priv);
}
int ser_close(int id, serdev_t *priv)
{
	tty_close(priv->fd);
	sparams[priv->unit] = NULL;

	return 0;
}

#endif /* NEWSERIAL */

static unsigned getspeed(int baud, unsigned code)
{
	unsigned n = -1;

	for (n = 0; speeds[n].baud; ++n) {
		if (baud) {
			if (speeds[n].baud == baud)
				return speeds[n].code;
		} else if (code) {
			if (speeds[n].code == code)
				return speeds[n].baud;
		}
	}
	return -1;
}

void QLsetmode(serdev_t *sd)
{
	struct termios tty;
	if (tcgetattr(sd->fd, &tty) < 0)
		perror("problems with tcgetattr?! :");
	tty.c_iflag = 0;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 0;
	tty.c_cflag |= (CLOCAL | CREAD);

	if (sd->parity == 0)
		tty.c_cflag &= ~PARENB;
	else {
		tty.c_cflag |= (PARENB | ((sd->parity == 1) ? PARODD : 0));
	}

#if !defined(sgi) && !defined(EMX)
	if (sd->hshake)
		tty.c_cflag |= CRTSCTS;
	else
		tty.c_cflag &= ~CRTSCTS;
#endif

	if (tcsetattr(sd->fd, TCSADRAIN, &tty) < 0)
		perror("sorry, problems with tcsetattr() :");
}

void tty_baud(serdev_t *sd)
{
	unsigned bspeed;
	struct termios tty;

#ifdef __linux__
	struct serial_struct ser_io;
	ioctl(sd->fd, TIOCGSERIAL, &ser_io);
#endif

	(void)tcgetattr(sd->fd, &tty);
	if ((bspeed = getspeed(sd->baud, 0))) {
		cfsetospeed(&tty, bspeed);
		cfsetispeed(&tty, bspeed);
	}
#ifdef __linux__
	switch (sd->baud) {
	case 38400:
		ser_io.flags &= ~ASYNC_SPD_MASK;
		break;
	case 57600:
		ser_io.flags &= ~ASYNC_SPD_MASK;
		ser_io.flags |= ASYNC_SPD_HI;
		break;
	case 115200:
		ser_io.flags &= ~ASYNC_SPD_MASK;
		ser_io.flags |= ASYNC_SPD_VHI;
		break;
	}
#endif
	(void)tcsetattr(sd->fd, TCSADRAIN, &tty);
#ifdef __linux__
	ioctl(sd->fd, TIOCSSERIAL, &ser_io);
#endif
}

int tty_open(const char *dev, serdev_t *sd)
{
	if ((sd->fd = open(dev, O_RDWR | O_NONBLOCK)) > 0) {
#ifndef NO_FIONREAD
		fcntl(sd->fd, F_SETFL, fcntl(sd->fd, F_GETFL) & ~O_NONBLOCK);
#endif
		QLsetmode(sd);
		if (sd->baud)
			tty_baud(sd);
	}
	return sd->fd;
}

void tty_close(int f)
{
	long mstat;
#ifndef EMX
	ioctl(f, TIOCMGET, &mstat);
	mstat &= ~(TIOCM_DTR | TIOCM_RTS);
	ioctl(f, TIOCMSET, &mstat);
#endif
	close(f);
}

#ifndef FIONREAD
#define FIONREAD FIORDCHK
#endif

int pendingio(int f)
{
	int res, nn;

#ifndef NO_FIONREAD
	if (ioctl(f, FIONREAD, &nn) == 0)
		res = (nn) ? 0 : -1;
	else
		res = -6;
	return res;
#else
	return QERR_EF; /* should not get there !! */
#endif
}

#ifdef USE_IOSZ
#define IOSZ 32
#endif

int writeio(serdev_t *sd, char *buf, long *pno)
{
	int no = *pno;
	int nw;
	int res;
	short done = 0;
	int nr = (sd->xlate == -1) ? no : 1;
	int sum = 0;
	int sts;

#ifdef IOTEST
	printf("call writeio: sd %d, bufp %x, count %d\n", sd, buf, *pno);
#endif

#ifdef USE_IOSZ
	no = nw = *pno;
#endif
	if (no == 0)
		nr = 0;

	while (!done) {
#ifdef USE_IOSZ
		if (nw > IOSZ)
			nr = IOSZ;
		else
			nr = nw;
#endif
		if (sd->xlate == OPTC && *buf == 10)
			*buf = 13;

		res = write(sd->fd, buf, nr);
		if (res <= 0) {
			if (res < 0) {
				if (errno == EINTR || errno == EAGAIN)
					continue;

				done = 1;
				sts = -6;
			} else {
				done = 1;
				sts = 0;
			}
		} else {
#ifdef USE_IOSZ
			nw -= res;
#endif
			sum += res;
			if (sd->xlate == OPTZ && *buf == 26) {
				done = 1;
				res = 0;
			}
			if (sum == no) {
				done = 1;
				sts = 0;
			}
#if 0
	  if (res != nr)
	    {
	      done = 1;
	      sts = -1;
	    }
#endif
			buf += res;
		}
	}
#if 0
    if (res == 0)
	sts = -10;
    else if (res < 0)
    {
	if (errno == EINTR)
	    sts = -1;
	else
	    sts = -6;
    }
    else
	sts = 0;
#endif

#ifdef IOTEST
	printf("exit writeio: count %d, err %d\n", sum, sts);
#endif
	*pno = sum;
	return sts;
}

int readio(serdev_t *sd, char *buf, long *pno, short tc)
{
	int no = *pno;
	int res;
	short done = 0;
	int nr;
	int sum = 0;
	int nn;
	int sts;
	char *xx = buf;

#ifdef IOTEST
	printf("call readio: sd %d, bufp %x, count %d\n", sd, buf, *pno);
#endif

	if (*pno <= 0) {
		done = 1;
		sts = 0;
	}

#ifndef NO_FIONREAD
	res = ioctl(sd->fd, FIONREAD, &nn);
#if 0
    if (res<0) perror("ioctl() FIONREAD problem:");
#endif
#else
	nn = no;
#endif

#ifndef NO_FIONREAD
	if (res) {
		sts = -6;
	} else
#endif
	{
		if (nn < no)
			no = nn;

		if (no > 0) {
			nr = (sd->xlate != -1 || tc) ? 1 : no;
			while (!done) {
				res = read(sd->fd, buf, nr);
				if (res <= 0) {
					done = 1;
					if (res < 0) {
						switch (errno) {
						case EINTR:
						case EAGAIN:
							if (!sd->killed)
								sts = QERR_NC;
							else
								sts = 0;
							break;
						default:
							sts = QERR_TE;
							break;
						}
#if 0
			if (errno == EINTR)
			    sts = -1;
			else
			    sts = -6;
#endif
					}
				} else {
					if (sd->xlate == OPTC && *buf == 13) {
						*buf = 10;
					}

					if (sd->xlate == OPTZ && *buf == 26) {
						done = 1;
						sts = -10;
					}

					if (tc && *buf == tc) {
						sts = 0;
						done = 1;
					}

					buf += res;
					sum += res;
					if (sum == no) {
						done = 1;
						sts = 0;
					}
					if (res != nr) {
						done = 1;
						sts = -1;
					}
				}
			}
		} else
			sts = -1;
	}
#ifdef IOTEST
	printf("exit readio: count %d, err %d char %d\n", sum, sts, *buf);
#endif
	*pno = sum;
	return sts;
}

#ifdef NEWSERIAL

#endif

#endif /* SERIAL */
