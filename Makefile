# (c) UQLX - see COPYRIGHT
#
# UQLX Makefile
#  Targets:
#	all: build uqlx according to ~/.uqlx_cprefs
#	xaw: 		athena widget interface version, override prefs
#       noaw:	  	without xaw interface, override prefs

# Compile Preferences File:
#  ~/.uqlx_cprefs
#
#  you may set this variables:
#    USE_XAW= yes			# or no
#    BUILD_SPECIALFLAGS= -Dxx 		# need some strange define flags ?
#    LOCAL_LIBPATHS= -L /amd/hx/lib/X11/
#    LOCAL_INCLUDES= -I /usr/include/
    LOCAL_OPTFLAGS= -Wall -g
#    LOCAL_DEFINES=-DEVM_SCR
#    LOCAL_LINKFLAGS=


# the whole Makefile is utterly GNU Make dependent

IMPLPATH := $(shell pwd)
RELSE := $(shell date '+%m/%d/%y  %H:%M:%S')
NRELSE := $(shell date '+%d.%m.%y-%H.%M')
PWD := $(shell pwd)

BUILDFLAGS := -DLINUX -DUSE_IPC -DQVFS -DMOUSE -DUSE_IOSZ -DDO_GRAB -DSERIAL \
	-DNEWSERIAL -DNEWPTY  -DFASTLOOP  -DSH_MEM -DIPDEV  -DXSCREEN -DSOUND \
	-DUX_WAIT -DHAS_STPCPY -DEVM_SCR

# Library and Include paths:
# most configurationss don't complain about nonexistent directories in the search path,  IRIX is the exception ..
ifeq ($(XOS),IRIX)
else
INCLUDES := -I /usr/openwin/include/ -I /usr/include/X11R6/ -I /usr/include/X11R5/ \
	-I /usr/X11/include/ -I/usr/X11R6/include
LIBS := -L/usr/X11/lib/ -L/usr/openwin+/lib/ -L/usr/openwin/lib/ \
	-L/usr/X11R6/lib/ -L/usr/X11R5/lib/
endif

# append local definitions and preferences
BUILDFLAGS += $(BUILD_SPECIALFLAGS)
BUILDFLAGS += $(LOCAL_DEFINES)
INCLUDES += $(LOCAL_INCLUDES)
LIBS += $(LOCAL_LIBPATHS)

#SUN includes:
#INCLUDES= -I /usr/openwin/include/
# HPUX includes:
#INCLUDES = -I /usr/include/X11R5/

#SUN libs
#LIBS = #-L /usr/openwin/lib/  #-L /opt/SUNWspro/SC4.0/lib/
#HP libs
# LIBS = -L /usr/lib/X11R5/

# X11R6 libs (Linux)
# LIBS = -L  /usr/X11R6/lib

PROFFLAGS= #-pg -a


#OPTFLAGS=  $(PROFFLAGS) -O3 $(DEBUG)  -finline-functions $(LOCAL_OPTFLAGS) -fomit-frame-pointer
OPTFLAGS=  $(PROFFLAGS) -O2 $(DEBUG)  $(LOCAL_OPTFLAGS) -fomit-frame-pointer 

DEBUGFLAGS=  #-DTRACE #-DBACKTRACE  #-DDEBUG -DVTIME

LINKFLAGS= $(LOCAL_LINKFLAGS)


WFLAGS= # -Wimplicit -Wno-multichar

ENX :=  '-DIMPL="$(IMPLPATH)/"' 
ENVFLAGS := $(ENX)




XTSRCS := xql.c xtmain.c unixstuff.c xqlmouse.c
XLSRCS := x.c xlmain.c unixstuff.c xqlmouse.c
XWSRCS := x.c xql.c xtmain.c xlmain.c unixstuff.c xqlmouse.c

XTOBJS := xql.o xtmain.o xqlmouse_aw.o unixstuff_aw.o
XLOBJS := x.o xlmain.o xqlmouse.o unixstuff.o



HEADERS := QL.h  QLtypes.h  QDisk.h QL68000.h cond.h iexl.h QSerial.h  unix.h \
	QFilesPriv.h  QL_config.h  QSound.h   QInstAddr.h  QLfiles.h boot.h \
	QDOS.h trace.h uqlx_cfg.h emudisk.h driver.h vm.h \
	misdefs.h xcodes.h QL_screen.h \
	iexl_general.h memaccess.h mmodes.h iexl_ug.h QVFS.h \
        QL_sound.h \
	QLip.h iptraps.h util.h xc68.h xipc.h script.h qx_proto.h vl.h \
        cow.h


OBJ := Init.o general.o instructions_ao.o instructions_pz.o  \
	QLtraps.o QL_hardware.o QL_config.o dummies.o vm.o \
	xqlkey.o qmtrap.o uxfile.o QL_serial.o pty.o uqlx_cfg.o\
	QL_files.o QL_driver.o QDisk.o trace.o version.o QLserio.o \
	QL_screen.o QL_poll.o xcodes.o QL_boot.o QL_basext.o \
	QL_cconv.o  iexl_general.o QVFS.o QL_sound.o\
	Xscreen.o QLip.o util.o xc68.o xipc.o script.o \
	mach_exception.o siginfo.o vl.o ide.o block.o

SRC := Init.c general.c instructions_ao.c instructions_pz.c   \
	QLtraps.c QL_hardware.c QL_config.c dummies.c vm.c \
	xqlkey.c qmtrap.c uxfile.c QL_serial.c pty.c uqlx_cfg.c \
	QL_files.c QL_driver.c QDisk.c trace.c version.c QLserio.c \
	QL_screen.c QL_poll.c xcodes.c QL_boot.c QL_basext.c \
	QL_cconv.c iexl_general.c QVFS.c \
	Xscreen.c QLip.c util.c xc68.c xipc.c script.c \
	QL_sound.c vm_win.c mach_exception.c siginfo.c \
	vl.c ide.c block.c


DOCS := COPYRIGHT CONTRIBUTING \
	docs/socket.texi docs/uqlx.texi docs/ql.html docs/qxlwin.html

ROMS := romdir/js_rom romdir/min.189

CONFIGFILES := Makefile Xql Xqlaw 

AUX := .gdbinit BOOT.test bench1 bench2 xx.c Xgui.c \
	guesscpu_linux config MK.all do_install browse_manual vmtest.c \
	zmtest.c uqlx.bat  dtest.c
UTILS := utils/README utils/xheader utils/Makefile utils/qcp.c utils/qls.c \
	utils/qxl_fschk.zip utils/tracesplit


## conditionalise !!!!!!
ifneq (,$(findstring DXAW,$(XAW_FLAG)))
 EXE_NAME= qm-aw
else
 EXE_NAME= qm
endif

# link in required libraries - and nothing more
ifneq (,$(findstring DXAW,$(XAW_FLAG)))
    XLIBS := -lXaw -lXmu -lXt -lX11 -lXext
else
  ifneq (,$(findstring DSH_MEM,$(BUILDFLAGS)))
    XLIBS :=  -lX11 -lXext
  else
    XLIBS := -lX11
  endif
endif


ifneq (,$(findstring DXAW,$(XAW_FLAG)))
   WINSRCS := $(XTSRCS)
   WINOBJS := $(XTOBJS)
else
   WINSRCS := $(XLSRCS)
   WINOBJS := $(XLOBJS)
endif



ifneq (,$(findstring $(DEBUG_ON),$(DEBUG_FILES)))
  DO_DEBUG=true
else 
  DO_DEBUG=false
#ifneq (,$(strip $(DEBUG_ON) ))
#  NODEBUG=true
#endif
endif


# export ALL variables (lazy typing)
export

all: printarch qm docs

noaw:
	$(MAKE) all USE_XAW="no"

xaw:  
	$(MAKE) all USE_XAW="yes"	

##exe:	$(EXE_NAME) qm

.PHONY: printarch
printarch:
	@echo ""
ifeq (,$(UNKNOWN_ARCH))
	@echo "*** Making UQLX for $(XOS) ***" 
ifneq (,$(findstring ERROR,$(DEF_CPU)))
	@echo "couldn't set CPU flags, guesscpu_* returned $(DEF_CPU)"
else
	@echo "$(CC) compiling with $(DEF_CPU) flag"
endif
ifneq (,$(REDEF_CC))
	@echo "Warning: CC redefined to $(CC)"
endif
else
	@echo "*** Wanrning: unknown architecture $(XOS) ***"
endif
	@echo

.PHONY: print_usage
print_usage:
	@echo "Please do 'make config' first"

xqlmouse.o:	xqlmouse.c
	$(CC) $(PROFFLAGS)  -c -O3 $(ENVFLAGS) $(INCLUDES) $(OPTFLAGS) $(BUILDFLAGS) $(WFLAGS) $(XAW_FLAG) $<

xqlmouse_aw.o:	xqlmouse.c
	$(CC) $(PROFFLAGS)  -c -O3 -o xqlmouse_aw.o $(ENVFLAGS) $(INCLUDES) $(OPTFLAGS) $(BUILDFLAGS) $(WFLAGS) $(XAW_FLAG) $<




unixstuff.o:	unixstuff.c
	$(CC) $(PROFFLAGS)  -c  $(DEBUGFLAGS) $(BUILDFLAGS) $(INCLUDES) $(OPTFLAGS)  $(ENVFLAGS) $(WFLAGS) $(XAW_FLAG) $<

unixstuff_aw.o:	unixstuff.c
	$(CC) $(PROFFLAGS)  -c -o unixstuff_aw.o $(DEBUGFLAGS) $(BUILDFLAGS) $(INCLUDES) $(OPTFLAGS) $(ENVFLAGS) $(WFLAGS) $(XAW_FLAG) $<


# place exotic optimisation flags here:
XOPTS=-fcprop-registers -fcrossjumping -fgcse -fgcse-after-reload -foptimize-register-move 

general.o:	general.c instructions_ao.c instructions_pz.c
	$(CC) $(PROFFLAGS)  -c  $(XOPTS) -fexpensive-optimizations -fschedule-insns2  \
	-frerun-cse-after-loop \
		$(OPTFLAGS) $(DEBUGFLAGS) $(BUILDFLAGS) $(INCLUDES) $(WFLAGS) \
	 $<  

iexl_general.o:	iexl_general.c instructions_ao.c instructions_pz.c
	$(CC) $(PROFFLAGS)  -c  $(XOPTS) -fexpensive-optimizations  -fschedule-insns2  \
	-frerun-cse-after-loop \
		$(OPTFLAGS) $(DEBUGFLAGS) $(BUILDFLAGS) $(INCLUDES) $(WFLAGS) \
	 $<  



.c.o:
  ifdef DEBUG_ON
    ifneq (,$(findstring true,$(DO_DEBUG)))
	$(CC) $(PROFFLAGS)  -c  -g $(INCLUDES) $(DEBUGFLAGS) $(BUILDFLAGS) $(ENVFLAGS)  $(WFLAGS) $(XAW_FLAG) $<
    else
	$(CC) $(PROFFLAGS)  -c  $(INCLUDES) $(DEBUGFLAGS) $(OPTFLAGS) $(BUILDFLAGS) $(ENVFLAGS)  $(WFLAGS) $(XAW_FLAG) $<
    endif
  else
   ifneq (,$(strip $(DEBUG_FILES) ))
	$(MAKE) $@ DEBUG_ON=$< DEBUG_TARGET=$@
   else
	$(CC) $(PROFFLAGS)  -c  $(INCLUDES) $(DEBUGFLAGS) $(OPTFLAGS) $(BUILDFLAGS) $(ENVFLAGS)  $(WFLAGS) $(XAW_FLAG) $<
  endif
  endif




ifeq (,$(XAW_FLAG))
qm:     $(OBJ) $(WINOBJS)
else
qm-aw-defunct:  $(OBJ) $(WINOBJS)
endif
	$(CC)  $(PROFFLAGS)  $(LINKFLAGS) $(LIBS) -o $(EXE_NAME)  $(OBJ) $(WINOBJS)  $(XLIBS) $(XLFLAG) 
  ifndef UNKNOWN_ARCH
	@echo
	@echo "*** you have almost done it, ***"
	@echo "the name of the executable is"
	@echo "     >>>  $(EXE_NAME)  <<<"
	@echo 
	@echo "next check/set PREFIX and"
	@echo "'make install'"
	@echo
	@echo "'make gui' will create a simple GUI"
	@echo
  else
	@echo
	@echo "so far everything seems to have worked"
	@echo "- you might want to try out some of the"
	@echo "more difficult BUILDFLAG options"
  endif

qm-aw:
	@echo "*** sorry, qm-aw target currently not supported ***"
	@echo


:PHONY : install
install: qm
	if [ -z "$$PREFIX" ] ; then 				\
          if [ "`whoami`" = "root" ] ; then 			\
	     PREFIX=/usr/local ;				\
	  else							\
	     PREFIX=$$HOME ;					\
	     echo "make sure $$HOME/bin is in your path"; 	\
	  fi;							\
	fi;							\
	umask 022 ;						\
	mkdir -p $$PREFIX ;					\
	mkdir -p $$PREFIX/bin ;					\
	mkdir -p $$PREFIX/lib/uqlx/romdir ;			\
	mkdir -p $$PREFIX/lib/uqlx/docs ;			\
	cp BOOT.test browse_manual do_install $$PREFIX/lib/uqlx/ ; 	\
	[ -f vmtest ] && cp vmtest $$PREFIX/lib/uqlx/ ; 	\
	[ -f Xgui ] && cp Xgui $$PREFIX/lib/uqlx/ ; 	\
	cp docs/* $$PREFIX/lib/uqlx/docs ;	  		\
	cp romdir/* $$PREFIX/lib/uqlx/romdir ;	  		\
	[ -f $$PREFIX/lib/uqlx/romdir/minerva_rom ] || \
	    ln -s $$PREFIX/lib/uqlx/romdir/min.189 $$PREFIX/lib/uqlx/romdir/minerva_rom ;  \
	cp qm  $$PREFIX/bin/qm	;				\
	ln -sf $$PREFIX/bin/qm  $$PREFIX/bin/qmin ;			\
	ln -sf $$PREFIX/bin/qm  $$PREFIX/bin/qjs ;			\
	ln -sf $$PREFIX/bin/qm  $$PREFIX/bin/qx ;			\
	ln -sf $$PREFIX/bin/qm  $$PREFIX/bin/qxx ;			\
	ln -sf $$PREFIX/bin/qm  $$PREFIX/bin/qxxx ;


Xgui:	util.o Xgui.c
	gcc -o Xgui $(INCLUDES) $(CCFLAGS) $(ENVFLAGS) $(BUILDFLAGS) $(OPTFLAGS) Xgui.c util.o $(LIBS) \
		-lXaw -lXmu -lXt -lXext -lX11 $(XLFLAG) 

gui:	Xgui

## this rules are necessary so that build process can continue
## when docfiles are missing
docs/.dummy:
	mkdir -p docs
	touch docs/.dummy

docs/uqlx.texi: docs/.dummy
	touch docs/uqlx.texi

docs/socket.texi: docs/.dummy
	touch docs/socket.texi
### end dummy doc rules

docs: html_docs dvi_docs

html_docs: docs/uqlx.html docs/socket.html 

docs/uqlx.html: docs/uqlx.texi
	-(cd docs; texi2html uqlx.texi || makeinfo --no-split -o uqlx.html --html uqlx.texi)

docs/socket.html: docs/socket.texi
	-(cd docs; texi2html socket.texi || makeinfo --no-split -o socket.html --html socket.texi )

dvi_docs: docs/uqlx.dvi docs/socket.dvi

docs/uqlx.dvi: docs/uqlx.texi
	-(cd docs; texi2dvi uqlx.texi)

docs/socket.dvi: docs/socket.texi
	-(cd docs; texi2dvi socket.texi)

.PHONY : do_checkin
do_checkin:
	-cvs ci  $(SRC) $(XWSRCS) \
	        $(CONFIGFILES) $(AUX) $(HEADERS) \
		$(DOCS) $(UTILS) $(ROMS)

ci: do_checkin

#	-mkdir RCS
#	cp $(HOME)/archive/uqlxRCS.tar.gz RCS/
#	(cd RCS; gzip -d uqlxRCS.tar.gz; tar -xvf uqlxRCS.tar)
#	rm RCS/uqlxRCS.tar*
#	cp $(HOME)/www/geocities/ql.html docs/
#	ci -l  $(SRC) $(XWSRCS) $(HEADERS) \
#	  $(CONFIGFILES)  $(AUX) \
#	  $(DOCS)


.PHONY : do_archive
do_archive: do_checkin
	(cd RCS; tar -cvf ../uqlxRCS.tar * .gdb* ../docs/*,v; gzip -9 ../uqlxRCS.tar;  mv ../uqlxRCS.tar.gz $(HOME)/archive/ )



# used to be deps: $(SRC) $(XWSRCS) $(CONFIGFILES) $(HEADERS)  $(DOCS)
.PHONY : distr
distr: version do_archive full_save

.PHONY : save
save:	
	- rm -f uqlx.zip
	- find . -type f -print | xargs chmod go+u
	- find . -type d -print | xargs chmod go+u
	- find . -type f -print | xargs chmod go-w
	- find . -type d -print | xargs chmod go-w
	zip -9 -r -q uqlx.zip $(SRC) $(XWSRCS) \
	        $(CONFIGFILES) $(AUX) $(HEADERS) \
		$(DOCS) $(UTILS) $(ROMS) .uqlxrc
	cp uqlx.zip $(HOME)/big/

.PHONY: full_save
full_save: save
	mkdir -p /usr/tmp/p/uqlx-$(NRELSE)
	cd /usr/tmp/p/uqlx-$(NRELSE) ; unzip -q $(HOME)/big/uqlx.zip ; \
	   cd .. ; tar cf - uqlx-$(NRELSE)  | bzip2 -9 >$(HOME)/big/uqlx-$(NRELSE).tar.bz2; \
	   rm -rf uqlx-$(NRELSE)/romdir ; \
	   tar cf - uqlx-$(NRELSE)  | bzip2 -9 >$(HOME)/big/uqlx-noroms-$(NRELSE).tar.bz2 ;\
	   rm -rf uqlx-$(NRELSE)/docs ; \
	   tar cf - uqlx-$(NRELSE)  | bzip2 -9 >$(HOME)/big/uqlx-noroms-nodocs-$(NRELSE).tar.bz2 ;
	rm -rf /usr/tmp/p/uqlx-$(NRELSE)

.PHONY: backup
backup: full_save
	rm $(HOME)/big/uqlx-$(NRELSE).tar.bz2 $(HOME)/big/uqlx-noroms-nodocs-$(NRELSE).tar.bz2


.PHONY : upload
upload: distr docs
	set_version
	insert_banner
	upl_www
	-rm /usr/tmp/p/uqlx.zip /usr/tmp/p/uqlx-noroms.zip /usr/tmp/p/uqlx-noroms-nodocs.zip
	cd $(HOME)/big; zip -9 /usr/tmp/p/uqlx.zip   uqlx-$(NRELSE).tar.bz2  
	cd $(HOME)/big; zip -9 /usr/tmp/p/uqlx-noroms.zip  uqlx-noroms-$(NRELSE).tar.bz2 
	cd $(HOME)/big; zip -9 /usr/tmp/p/uqlx-noroms-nodocs.zip  uqlx-noroms-nodocs-$(NRELSE).tar.bz2 
	rm $(HOME)/big/uqlx-$(NRELSE).tar.bz2 $(HOME)/big/uqlx-noroms-nodocs-$(NRELSE).tar.bz2
	upload_uqlx $(NRELSE)


.PHONY : version
version:
	- rm -f version.c
	echo '#include "QL68000.h"' >> version.c
	echo 'char *release="$(RELSE)";' >> version.c
	echo $(RELSE) > $(HOME)/.uqlx_version

.PHONY : clean
clean:
	- rm -f $(OBJ) $(XTOBJS) $(XLOBJS) qm

### extra .hpr 8.8.99 ###
.PHONY : distclean
distclean:
	- rm -f  $(OBJ) $(XTOBJS) $(XLOBJS) qm qm-aw Xgui qcp qls xheader tags TAGS utils/qcp utils/qls utils/xheader
	- rm -f docs/*aux docs/*cp docs/*dvi docs/*fn docs/*rej docs/*ky docs/*log docs/*pg docs/*toc* docs/*tp docs/*vr docs/*info* docs/*fns docs/[su]*html
	- rm -rf docs/ref *~
	- rm -rf `find . -depth -name "*~" -print`
#		#

.PHONY : tags
tags:	$(SRC) $(XWSRC) $(HEADERS)
	etags  $(SRC) $(XWSRCS) $(HEADERS)

.PHONY : ctags
ctags:	$(SRC) $(XWSRC) $(HEADERS)
	ctags -id $(SRC) $(XWSRCS) $(HEADERS)

.PHONY : bench1
bench1:	qm
	time ./qm -r 256 -s 'lrun "$(PWD)/bench1"'

.PHONY : bench2
bench2: qm
	time ./qm  -r 256 -s 'lrun "$(PWD)/bench2"'


.PHONY : bench1-aw
bench1-aw:	qm-aw
	time ./qm-aw -b 'lrun "$(PWD)/bench1"'

.PHONY : bench2-aw
bench2-aw: qm-aw
	time ./qm-aw -b 'lrun "$(PWD)/bench2"'

twin:
	touch $(XLSRCS) $(XTSRCS) xqlmouse.c unixstuff.c


