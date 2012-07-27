#ifdef ENABLE_IDE

/*
 * QEMU System Emulator
 * 
 * Copyright (c) 2003-2004 Fabrice Bellard
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/poll.h>
#include <errno.h>
#include <sys/wait.h>
//#include <pty.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
//#include <linux/if.h>
//#include <linux/if_tun.h>


#include "vl.h"

void *ide_state;

#define hw_error printf

#define MAX_IOPORTS 65536
void *ioport_opaque[MAX_IOPORTS];
IOPortReadFunc *ioport_read_table[3][MAX_IOPORTS];
IOPortWriteFunc *ioport_write_table[3][MAX_IOPORTS];
BlockDriverState *bs_table[MAX_DISKS], *fd_table[MAX_FD];

/* size is the word size in byte */
int register_ioport_write(int start, int length, int size, 
                          IOPortWriteFunc *func, void *opaque)
{
    int i, bsize;


    // printf("register_ioport_write: start %x, len %x, size %x\n", start, length, size);

    ide_state=opaque;

    if (size == 1) {
        bsize = 0;
    } else if (size == 2) {
        bsize = 1;
    } else if (size == 4) {
        bsize = 2;
    } else {
        hw_error("register_ioport_write: invalid size");
        return -1;
    }
    for(i = start; i < start + length; i += size) {
        ioport_write_table[bsize][i] = func;
        if (ioport_opaque[i] != NULL && ioport_opaque[i] != opaque)
            hw_error("register_ioport_read: invalid opaque");
        ioport_opaque[i] = opaque;
    }
    return 0;
}

/* size is the word size in byte */
int register_ioport_read(int start, int length, int size, 
                         IOPortReadFunc *func, void *opaque)
{
    int i, bsize;

    // printf("register_ioport_read: start %x, len %x, size %x\n", start, length, size);
    if (size == 1) {
        bsize = 0;
    } else if (size == 2) {
        bsize = 1;
    } else if (size == 4) {
        bsize = 2;
    } else {
        hw_error("register_ioport_read: invalid size");
        return -1;
    }
    for(i = start; i < start + length; i += size) {
        ioport_read_table[bsize][i] = func;
        if (ioport_opaque[i] != NULL && ioport_opaque[i] != opaque)
            hw_error("register_ioport_read: invalid opaque");
        ioport_opaque[i] = opaque;
    }
    return 0;
}

void pstrcpy(char *buf, int buf_size, const char *str)
{
    int c;
    char *q = buf;

    if (buf_size <= 0)
        return;

    for(;;) {
        c = *str++;
        if (c == 0 || q >= buf + buf_size - 1)
            break;
        *q++ = c;
    }
    *q = '\0';
}

void *get_mmap_addr(unsigned long size)
{
    return NULL; //(void *)addr;
}

void qemu_free(void *p)
{
  free(p);
}

#if 0

    /* we always create the cdrom drive, even if no disk is there */
    if (has_cdrom) {
        bs_table[2] = bdrv_new("cdrom");
        bdrv_set_type_hint(bs_table[2], BDRV_TYPE_CDROM);
    }

    /* open the virtual block devices */
    for(i = 0; i < MAX_DISKS; i++) {
        if (hd_filename[i]) {
            if (!bs_table[i]) {
                char buf[64];
                snprintf(buf, sizeof(buf), "hd%c", i + 'a');
                bs_table[i] = bdrv_new(buf);
            }
            if (bdrv_open(bs_table[i], hd_filename[i], snapshot) < 0) {
                fprintf(stderr, "qemu: could not open hard disk image '%s\n",
                        hd_filename[i]);
                exit(1);
            }
            if (i == 0 && cyls != 0) 
                bdrv_set_geometry_hint(bs_table[i], cyls, heads, secs);
        }
    }

#endif
#endif // ENABLE_IDE
