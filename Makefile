# (c) UQLX - see COPYRIGHT
#
BUILDFLAGS := -DLINUX -DUSE_IPC -DQVFS -DMOUSE -DUSE_IOSZ -DDO_GRAB -DSERIAL \
	-DNEWSERIAL -DNEWPTY  -DFASTLOOP  -DSH_MEM -DIPDEV  -DXSCREEN -DSOUND \
	-DUX_WAIT -DHAS_STPCPY -DEVM_SCR -D_GNU_SOURCE -D_XOPEN_SOURCE

DEBUG = -ggdb

OPTFLAGS = -O2

CFLAGS = $(BUILDFLAGS) $(DEBUG) $(OPTFLAGS)

SRC := Init.c general.c instructions_ao.c instructions_pz.c   \
	QLtraps.c QL_hardware.c QL_config.c dummies.c vm.c \
	xqlkey.c qmtrap.c uxfile.c QL_serial.c pty.c \
	QL_files.c QL_driver.c QDisk.c trace.c version.c QLserio.c \
	QL_screen.c QL_poll.c xcodes.c QL_boot.c QL_basext.c \
	QL_cconv.c iexl_general.c QVFS.c \
	Xscreen.c QLip.c util.c xc68.c xipc.c script.c \
	QL_sound.c mach_exception.c siginfo.c \
	vl.c ide.c block.c unixstuff.c xqlmouse.c \
	x.c xlmain.c uqlx_cfg.c SDL2screen.c

OBJ := $(SRC:.c=.o)

DOCS := COPYRIGHT CONTRIBUTING \
	docs/socket.texi docs/uqlx.texi docs/ql.html docs/qxlwin.html

EXE_NAME = qlux

XLIBS := -lXaw -lXmu -lXt -lX11 -lXext -lSDL2


all : $(EXE_NAME)

.PHONY : depend
depend : $(SRC)
	$(CC) $(CFLAGS) -MM $^ > ./Makefile.depend;

-include Makefile.depend

.c.o:
	$(CC) $(CFLAGS) -c $<

$(EXE_NAME) : $(OBJ)
	$(CC) $(LIBS) -o $(EXE_NAME) $(OBJ) $(XLIBS)

.PHONY : clean
clean :
	rm $(OBJ)

:PHONY : install
install: qm


