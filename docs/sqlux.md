---
Generator: 'texi2html 5.0'
description: UQLX
distribution: global
keywords: UQLX
resource-type: document
title: UQLX
---

UQLX
====

**Richard Zidlicky**\

Copyright © 1996,1997,1998,1999 Richard Zidlicky

------------------------------------------------------------------------

[]{#Top}

  ---------------- ------------ ---------- ------------------------------------------------------------------ ------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[ &lt;&lt; \]   \[ &lt; \]   \[ Up \]   \[[&gt;](#System-Requirements "Next section in reading order")\]   \[[&gt;&gt;](#Compiling "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------- ------------ ---------- ------------------------------------------------------------------ ------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

  --------------------------------------------------- ---- ----------------------------
  [1 Introduction](#Introduction)                          
  [2 Compiling](#Compiling)                                
  [3 Installation](#Installation)                          not much to do, but
  [4 Customization](#Customization)                        nothing works without this
  [5 Program Invocation](#Program-Invocation)              
  [6 Filesystems](#Filesystems)                            
  [7 Other Devices](#Other-Devices)                        pty,ser,prt
  [8 Printing](#Printing)                                  
  [9 SuperBasic Extensions](#SuperBasic-Extensions)        
  [10 TECHREF](#TECHREF)                                   how it works
  [11 FAQ](#FAQ)                                           
  [12 History](#History)                                   
  [13 Benchmarks](#Benchmarks)                             
  --------------------------------------------------- ---- ----------------------------

[]{#Introduction} []{#Introduction-1}

1 Introduction {#introduction .chapter}
==============

UQLX is an software emulator emulating a Sinclair QL on UNIX and similar
systems. It works by emulating the 68000 CPU and some hardware. It can
handle original JS roms, Minerva roms and many nationalized roms. The
focus is more on providing useful extensions than exact emulation of
antique hardware. Nevertheless it works well enough even for most games.

UQLX is designed to cooperate neatly with the underlying OS, some of the
features and possibilities are:

-   don’t waste CPU time when not needed by QDOS programs. Can save you
    some trouble with the sysadmin if you start it on a Cray
    accidentally
-   launch and control of UNIX programs by the use of the `pty` device
-   support for use of QDOS in UNIX scripts, screen IO redirection
-   the emulator can fork (clone) itself
-   access to every unix file - long filenames
-   TCP/IP device for QDOS
-   simple GUI with pasting into QL keyboard buffer and some more exotic
    features

  ------------------------------------------------- ---- --
  [1.1 System Requirements](#System-Requirements)        
  [1.2 COPYRIGHT](#COPYRIGHT)                            
  ------------------------------------------------- ---- --

------------------------------------------------------------------------

[]{#System-Requirements}

  ------------------------------------------------------------------------------- --------------------------------------------------------------- -------------------------------------- -------------------------------------------------------- ------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Introduction "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Introduction "Previous section in reading order")\]   \[[Up](#Introduction "Up section")\]   \[[&gt;](#COPYRIGHT "Next section in reading order")\]   \[[&gt;&gt;](#Compiling "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------- --------------------------------------------------------------- -------------------------------------- -------------------------------------------------------- ------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#System-Requirements-1}

1.1 System Requirements {#system-requirements .section}
-----------------------

Nearly any UNIX or similar system will do, provided you have gcc and X11
installed. UQLX even works with OS/2, however this port was not
maintained for some time.

**When I say `gcc`, I mean it! You may try pgcc but you have been
warned.**

Typically any improved c compiler couldn’t optimize anything measurable.

-   between 2 and 32 MB SWAP for UQLX at runtime, may require much more
    for compilation. Contact me if this is a problem.
-   gcc, X and GNU make. At least gcc versions 2.7.2.\*,2.8.\*,2.91 and
    2.95.\[12\] work, some broken versions require additional defines to
    compile. egcs generally works well, 2.91.66 does even work with the
    -DUSE\_AREGP option.

------------------------------------------------------------------------

[]{#COPYRIGHT}

  ------------------------------------------------------------------------------- ---------------------------------------------------------------------- -------------------------------------- -------------------------------------------------------- ------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Introduction "Beginning of this chapter or previous chapter")\]   \[[&lt;](#System-Requirements "Previous section in reading order")\]   \[[Up](#Introduction "Up section")\]   \[[&gt;](#Compiling "Next section in reading order")\]   \[[&gt;&gt;](#Compiling "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------- ---------------------------------------------------------------------- -------------------------------------- -------------------------------------------------------- ------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#COPYRIGHT-1}

1.2 COPYRIGHT {#copyright .section}
-------------

Make sure you read the copyright notice carefully before you use, modify
or distribute this product or parts of it.

The copyright notice can be found in the file ‘`COPYRIGHT`’ – if not try
to email me at `rdzidlic@geocities.com` [(1)](#FOOT1){#DOCF1}.

------------------------------------------------------------------------

[]{#Compiling}

  ------------------------------------------------------------------------------- ------------------------------------------------------------ ---------- --------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Introduction "Beginning of this chapter or previous chapter")\]   \[[&lt;](#COPYRIGHT "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#Misc-Hints "Next section in reading order")\]   \[[&gt;&gt;](#Installation "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------- ------------------------------------------------------------ ---------- --------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Compiling-1}

2 Compiling {#compiling .chapter}
===========

Several machines/OS versions are directly supported, others may need a
little guessing. Even if your machine is in the list of supported types,
you may consider using the [Compiling
Preferences](#Compiling-Preferences).

So far the supported machines/OS are:

-   SUN with Solaris
-   Linux on i486 or better, Power PC, Alpha, m68k
-   NetBSD
-   FreeBSD
-   SGI with IRIX, traditionally the most difficult one to compile for
-   SUN running old SunOS, not tested for a long time
-   HP PA with HP-UX, not much tested lately
-   OS/2, not tested for a long time

If your machine is not yet supported, [Unsupported
Machines](#Unsupported-Machines).

To make it on Linux, type

<div class="example">

``` {.example}
./MK.all
make install
```

</div>

other Unices make sure you use gnu-make (called eg gmake on Suns) and

<div class="example">

``` {.example}
make config     # 
make
make install
```

</div>

Do not delete the directory where you compiled it unless you know what
you are doing.

If you build for Linux-x86 then `-DUSE_VM` is enabled by default. This
means that for every new combination of ROMs a patch database must be
built. This happens automatically but can be rather irritating in some
circumstances.

The name of the executable will be ’qm’ resp. ’qm-aw’, depending on the
target and preferences.

If there are any complications like libraries not found, try [Compiling
Preferences](#Compiling-Preferences).

There are some other interesting targets,

-   `make gui` will create a simple GUI for UQLX, only non-aw versions.
    A much better (nicer) GUI is available from Jonathan Hudson’s
    Homepage `http://www.jrhudson.demon.co.uk`
-   `make xaw` *Currently not supported.* Will build the qm-aw version.
    This version is a bit slower (5-20% ) than the `noaw` version. Also
    some features like `Fork_UQLX` will probably never get implemented
    in the `xaw` version.
-   `make noaw` will always build the qm version, on most systems this
    executes a bit better than the `xaw` version
-   `make tags` build emacs TAG table
-   `make docs` will build ‘`docs/uqlx.dvi`’ and ‘`docs/uqlx.info`’

  ------------------------------------------------------------------------------- ---- --
  [2.0.1 Misc Hints](#Misc-Hints)                                                      
  [2.0.2 Compiling Preferences](#Compiling-Preferences)                                
  [2.0.3 Further Options](#Further-Options)                                            
  [2.0.4 Adding CPU specific optimizations](#Adding-CPU-specific-optimizations)        
  [2.0.5 Unsupported Machines](#Unsupported-Machines)                                  
  [2.0.6 Obscure `BUILDFLAGS`](#Obscure-BUILDFLAGS)                                    
  ------------------------------------------------------------------------------- ---- --

------------------------------------------------------------------------

[]{#Misc-Hints}

  ---------------------------------------------------------------------------- ------------------------------------------------------------ ----------------------------------- -------------------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Compiling "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Compiling "Previous section in reading order")\]   \[[Up](#Compiling "Up section")\]   \[[&gt;](#Compiling-Preferences "Next section in reading order")\]   \[[&gt;&gt;](#Installation "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------------------------------- ------------------------------------------------------------ ----------------------------------- -------------------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Misc-Hints-1}

### 2.0.1 Misc Hints {#misc-hints .subsection}

As it turns out, it is always worth to check your `gcc` configuration.
‘`gcc -v`’ will give you the name of specification file.

Recently SUN made some incompatible changes to Solaris, this results in
compiling errors. Newer gcc is the best solution, otherwise patches are
available from me. ’Recently’ was sometimes around 1996 so this is a bit
old news by now.

Some Linux/PC systems have not the correct CPU settings in this
specification file. If your system is Linux and the CPU i486 or better,
it is advisable to define ’\_\_i486\_\_’. You can also do this easily by
an addition to your ‘`~/uqlx_cprefs`’:

<div class="example">

``` {.example}
BUILD_SPECIALFLAGS= -D__i486__
```

</div>

The REGP and AREGP compiling options are incompatible to profiling, use
this to disable them. This also helps if your compiler exits with signal
11..

<div class="example">

``` {.example}
BUILD_SPECIALFLAGS= -DNO_REGP
# or
BUILD_SPECIALFLAGS= -DNO_AREGP
```

</div>

On the other hand if you are very confident about your gcc version you
may explicitly enable some trickier things like `USE_AREGP`. Normally
this is enabled only for known well behaved gcc versions and matters
only for the ix86 architecture anyway.

Specifying compiler version may work like this:

<div class="example">

``` {.example}
make CC="gcc -V 2.7.2.3" config    #compiles with gcc 2.7.2.3 (if installed)
make CC="gcc272" config            #another version of it..
```

</div>

  ------------------------------------------------------- ---- -------------------------------------------------
  [2.0.2 Compiling Preferences](#Compiling-Preferences)        very useful if you have an exotic configuration
  [2.0.5 Unsupported Machines](#Unsupported-Machines)          it may still work...
  [2.0.6 Obscure `BUILDFLAGS`](#Obscure-BUILDFLAGS)            if it doesn’t
  ------------------------------------------------------- ---- -------------------------------------------------

------------------------------------------------------------------------

[]{#Compiling-Preferences}

  ---------------------------------------------------------------------------- ------------------------------------------------------------- ----------------------------------- -------------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Compiling "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Misc-Hints "Previous section in reading order")\]   \[[Up](#Compiling "Up section")\]   \[[&gt;](#Further-Options "Next section in reading order")\]   \[[&gt;&gt;](#Installation "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------------------------------- ------------------------------------------------------------- ----------------------------------- -------------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Compiling-Preferences-1}

### 2.0.2 Compiling Preferences {#compiling-preferences .subsection}

are stored in the file ‘`~/.uqlx_cprefs`’

Following variables can be set:

<div class="example">

``` {.example}
   PREFIX=/usr                          # install here
   DEF_CPU="-mcpu=i586"                 # use if automatic config didn't work
   DEF_CPU= -m68040                     # old style def..
   GENERIC_CPU=yes                      # don't do special optimizations for
# eg Pentium, Cypress or V9 chips, compile generic code (as regarded by gcc..)
# unless this or DEF_CPU set some CPU is guessed
   BUILD_SPECIALFLAGS= -Dxx             # need some strange define flags ?
   LOCAL_LIBPATHS= -L /amd/hx/lib/X11/
   LOCAL_INCLUDES= -I /usr/include/
   DEBUG_FILES= xqlkey.c                # use no optimization for this file(s)
   LOCAL_OPTFLAGS= -O2 -no-strength-reduce
# xqlmouse.c and unixstuff.c don't follow this rule
   BUILD_SPECIALFLAGS= -DNO_REGP -DNO_AREGP # inhibit certain optimization
# especially AREGP can break many gcc versions
   USE_XAW= yes                         # or no - not supported right now
```

</div>

The `DEBUG_FILES` has no effect on certain special files, you have to
change the ‘`Makefile`’ by hand.

------------------------------------------------------------------------

[]{#Further-Options}

  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ ----------------------------------- -------------------------------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Compiling "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Compiling-Preferences "Previous section in reading order")\]   \[[Up](#Compiling "Up section")\]   \[[&gt;](#Adding-CPU-specific-optimizations "Next section in reading order")\]   \[[&gt;&gt;](#Installation "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------------------------------- ------------------------------------------------------------------------ ----------------------------------- -------------------------------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Further-Options-1}

### 2.0.3 Further Options {#further-options .subsection}

Some compiling options which are considered generally useful but for
some reasons may not be defined as default for your architecture. You
may enable any of this options by adding ’-Doption’ to the
BUILD\_SPECIALFLAGS line as described in the previous section.

`XSCREEN`

:   enable big screen support.(enabled as default)

`IPDEV`

:   add TCP/IP devices for QDOS. (enabled on most architectures)

`QVFS`

:   enable direct access to unix-fs using unix-like infinite long
    pathnames!(enabled as default). Also allows access to device special
    and /proc files.

`XVIS`

:   useful for xlib version only, chooses visual to minimize memory
    waste and improve performance. Best selection policy still to be
    found - choosing some other than default visual often has sideffects
    like flashing color pallete.

`U_NODDOT`

:   don’t show ’.’ and ’..’ in directory listings, still accessible

`DEBUG_ROM`

:   enable breakpoints in ROM \[0-64K\], also enables many sorts of
    beautiful crashes. Not very useful anymore because GUI allows this
    interactively.

------------------------------------------------------------------------

[]{#Adding-CPU-specific-optimizations}

  ---------------------------------------------------------------------------- ------------------------------------------------------------------ ----------------------------------- ------------------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Compiling "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Further-Options "Previous section in reading order")\]   \[[Up](#Compiling "Up section")\]   \[[&gt;](#Unsupported-Machines "Next section in reading order")\]   \[[&gt;&gt;](#Installation "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------------------------------- ------------------------------------------------------------------ ----------------------------------- ------------------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Adding-CPU-specific-optimizations-1}

### 2.0.4 Adding CPU specific optimizations {#adding-cpu-specific-optimizations .subsection}

So far there is special treatment for these CPUs/architectures:

`SPARC`

make sure to compile for the right submodel though, use `DEF_CPU`. I
would love to know how to guess this automagically!

`ix86`

`HP-PA RISC`

`MIPS`

`m68k`

For RISC CPUs defining global register variables is the easiest
optimization, all I need to know is a list of register names and their
typical usage - this simple trick often improves performance by &gt;10%.

------------------------------------------------------------------------

[]{#Unsupported-Machines}

  ---------------------------------------------------------------------------- ------------------------------------------------------------------------------------ ----------------------------------- ----------------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Compiling "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Adding-CPU-specific-optimizations "Previous section in reading order")\]   \[[Up](#Compiling "Up section")\]   \[[&gt;](#Obscure-BUILDFLAGS "Next section in reading order")\]   \[[&gt;&gt;](#Installation "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------------------------------- ------------------------------------------------------------------------------------ ----------------------------------- ----------------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Unsupported-Machines-1}

### 2.0.5 Unsupported Machines {#unsupported-machines .subsection}

Try to make it, if this fails due to some library or includes not found
try [Compiling Preferences](#Compiling-Preferences). Also see [Obscure
`BUILDFLAGS`](#Obscure-BUILDFLAGS)

If UQLX compiles fine, but doesn’t execute properly than chances are
good that you have a really exotic CPU not yet known to me. In this case
try to define `BIG_ENDIAN` and `HOST_ALIGN` in ‘`QL68000.h`’ to match
your CPU.

Contact me so that I can add support for your machines; I need the
following information:

-   output of the ’uname’ cmd or other safe way to recognize this
    architecture
-   any special compile flags, libraries etc ...

  --------------------------------------------------- ---- --
  [2.0.6 Obscure `BUILDFLAGS`](#Obscure-BUILDFLAGS)        
  --------------------------------------------------- ---- --

------------------------------------------------------------------------

[]{#Obscure-BUILDFLAGS}

  ---------------------------------------------------------------------------- ----------------------------------------------------------------------- ----------------------------------- ----------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Compiling "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Unsupported-Machines "Previous section in reading order")\]   \[[Up](#Compiling "Up section")\]   \[[&gt;](#Installation "Next section in reading order")\]   \[[&gt;&gt;](#Installation "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------------------------------- ----------------------------------------------------------------------- ----------------------------------- ----------------------------------------------------------- ---------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Obscure-BUILDFLAGS-1}

### 2.0.6 Obscure `BUILDFLAGS` {#obscure-buildflags .subsection}

Usually it is not wise to change `BUILDFLAGS`, but if you want to do it,
here is some info:

`SERIAL`

:   enables ser and pty drivers for Linux. On other OS’s you may need to
    define `-NO_FIONREAD` if `FIONREAD` is not supported. You should
    also define `NEWSERIAL` and `NEWPTY`

`NEWSERIAL`\
`NEWPTY`

:   somewhat changed serial ports, this is required to get them work
    with Minerva ROMs. Can’t be defined independently

`USE_IPC`

:   enable ipc communication, this is needed for GUI.

`XAW`

:   use Xt/Athena Frame around UQLX main window. The problems with SUNs
    OpenWindow now disappeared. This flag is now easier controlled by
    make’s `target` and compiling preferences.

`DO_GRAB`

:   useful to keep window managers from snatching away alt- and ctrl-
    key combinations. Disables ALL WM hotkeys – you must move the
    pointer out of the QL window to have the WM hotkey available again.
    Currently this is the default behavior, unfortunately it interacts
    badly with the broken by design XKB extension. See keyboard for more
    info.

`SH_MEM`

:   use MIT Shared memory to speed up screen conversion. Obviously this
    works only for local screens. While most Xserver/Xlib combinations
    are clever enough to figure this out, other need to disable
    `SH_MEM`. This `X` extension is not very well standardized and
    therefore good for all kinds of trouble..

`VM_SCR`

:   use virtual memory tricks to detect screen changes, this is
    recommended with SYSV systems that support it - your compiler will
    complain if it isn’t. Strangely the XAW appears to use up most of
    the speed advantage...

`USE_VM`

:   detect r/w to QL hardware registers and screen change by doing nasty
    VM tricks. Supported on Solaris and Linux ix86 2.0.x - 2.2.x

`BSD`

:   some BSD related hacks, avoids compile time errors in pty.c
    SunOS&lt;5 needs this as well as the next 2 defines. Worth a try for
    any BSD like systems if it doesn’t compile.

`NO_MEMMOVE if memmove() is missing, try this`\
`NO_GETOPT  .. getopt() .... Actually you would better get and compile`

:   the getopt() library.

Debugging flags:

`VTIME`

:   simulate a virtual time/interrupts by instruction counting, useful
    to get exactly reproducible results with `TRACE`

`TRACE`

:   output loads of information for each instruction executed. Regions
    of code to be traced must be specified in ‘`trace.c`’.

`DEBUG_ROM`

:   enable breakpoints in ROM, also enables many sorts of beautiful
    crashes

------------------------------------------------------------------------

[]{#Installation}

  ---------------------------------------------------------------------------- --------------------------------------------------------------------- ---------- ------------------------------------------------------------ ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Compiling "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Obscure-BUILDFLAGS "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#Customization "Next section in reading order")\]   \[[&gt;&gt;](#Customization "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------------------------------- --------------------------------------------------------------------- ---------- ------------------------------------------------------------ ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Installation-1}

3 Installation {#installation .chapter}
==============

<div class="example">

``` {.example}
make install
```

</div>

This will create the qm, qmin, qjs, qx, qxx, qxxx symlinks to the
executable. The links are created as specified by `PREFIX` which can be
environment var or specified in ‘`~/.uqlx_cprefs`’ - ‘`/usr/local/bin`’
resp. `~/bin`is used as last resort.

The compilation directory should be preserved because thats where all
symlinks point to :)

Upon first invocation `qm` will attempt to create a ‘`~/.uqlxrc`’ file.

Some more caution must be used for systemwide installations, QDOS
software does not expect concurrent access to files and thus all
systemwide QDOS resources should be readonly. Beware, not many QDOS
program’s handle it gracefully if they encounter a readonly file.

Thus all filesystems except floppy and CD are created private per user
by default

------------------------------------------------------------------------

[]{#Customization}

  ------------------------------------------------------------------------------- --------------------------------------------------------------- ---------- ---------------------------------------------------------------------- ---------------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Installation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Installation "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#About-_002euqlxrc-files "Next section in reading order")\]   \[[&gt;&gt;](#Program-Invocation "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------- --------------------------------------------------------------- ---------- ---------------------------------------------------------------------- ---------------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Customization-1}

4 Customization {#customization .chapter}
===============

There are several configuration files, ‘`~/.uqlxrc`’ is by far the most
important one. It is used to configure QL filesystems, rom, ram and
diverse other features.

The other configuration files are ‘`~/.uqlx_cprefs`’ [Compiling
Preferences](#Compiling-Preferences), ‘`Xql`’ used by the standard GUI
and ‘`Xqlaw`’, the qm-aw application defaults file. The later two should
be copied to your application defaults directory, you will probably also
want to change the button names.

  ----------------------------------------------------- ---- --
  [4.1 About .uqlxrc files](#About-_002euqlxrc-files)        
  ----------------------------------------------------- ---- --

------------------------------------------------------------------------

[]{#About-_002euqlxrc-files}

  -------------------------------------------------------------------------------- ---------------------------------------------------------------- --------------------------------------- ----------------------------------------------------------------- ---------------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Customization "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Customization "Previous section in reading order")\]   \[[Up](#Customization "Up section")\]   \[[&gt;](#Program-Invocation "Next section in reading order")\]   \[[&gt;&gt;](#Program-Invocation "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------------- ---------------------------------------------------------------- --------------------------------------- ----------------------------------------------------------------- ---------------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#About-_002euqlxrc-files-1}

4.1 About .uqlxrc files {#about-.uqlxrc-files .section}
-----------------------

uqlx looks for an options file ‘`~/.uqlxrc`’ This file describes all
other settings uqlx requires; thus uqlx can exist in any directory as
long as ‘`.uqlxrc`’ is setup properly. The default name ‘`~/.uqlxrc`’
can be overridden by specifying the `-f` option at startup.

At first installation ’make install’ will create a simple ‘`~/.uqlxrc`’,
you may edit it by hand or use Jonathan Hudson’s fantastic GUI. Get it
from `http://www.jrhudson.demon.co.uk`

The uqlx options file uses a `KEY = VALUE` format. The ’`#`’ character
can be used to start comments, rest of the line is not evaluated.

The keys available are:

`SYSROM`

:   The name of the QDOS ROM to boot.

    <div class="example">

    ``` {.example}
             SYSROM = js_rom
    ```

    </div>

`ROMDIR`

:   The directory where ’SYSROM’ (and other ROMS) may be found.

    <div class="example">

    ``` {.example}
             ROMDIR = /ql/ROMS/
    ```

    </div>

`RAMTOP`

:   The upper limit of memory; usual QDOS rules apply. The value is in
    kB. Be warned that large values for this will cause long startup
    delays unless FAST\_START is enabled. I have largely tested UQLX
    with 4MB, but at least Minerva should handle 16MB. Currently UQLX
    won’t allow more than 16MB, but this could be easily changed if you
    need more. If a larger screen size is used it has to fit into this
    value.

    <div class="example">

    ``` {.example}
             RAMTOP = 4096
    ```

    </div>

`COLOUR`

:   The usage of a colour or mono display. Values are 0 for mono, 1 for
    colour. This may be used to simulate grayscale on a color monitor,
    not the opposite unfortunately. This option may get overridden to
    mono if a specific visual or visual class is requested as specified
    below.

    <div class="example">

    ``` {.example}
             COLOUR = 1
    ```

    </div>

`XVID`

:   specify X visual ID to be used. Overrides `XDEPTH` and `XVIS_CLASS`.
    See ‘xdpyinfo‘ for list of available visuals.

    <div class="example">

    ``` {.example}
            XVID = 0x24
    ```

    </div>

`XVIS_CLASS`

:   Specify preferred XVisualClass. This will affect whether color or
    mono is used and color cell allocation policy. Try experimenting if
    you have color palette flashing.

    <div class="example">

    ``` {.example}
            XVIS_CLASS = StaticColor
    ```

    </div>

`XDEPTH`

:   Specify preferred display depth to be used. Should be 8 where
    possible

`REAL_WHITE`

:   Set to 0 if you prefer greyish screens like me - useful with bad
    VUD’s with insufficient refresh rates.. Redefines QL white to Gray95

`SIZE_1`\
`SIZE_2`\
`SIZE_3`

:   Screen size definitions to be used for fast selection via program
    name argument(’x’,’xx’,’xx’). See Program Name.

    <div class="example">

    ``` {.example}
             SIZE_1 = 640x400
             SIZE_2 = 1024x768
             SIZE_3 = 4096x4096  
    ```

    </div>

`SER1`

:   The Unix device used for QDOS ser1.

    <div class="example">

    ``` {.example}
             SER1 = /dev/ttyS0
    ```

    </div>

`SER2`

:   The Unix device used for QDOS ser2.

    <div class="example">

    ``` {.example}
             SER1 = /dev/ttyS1
    ```

    </div>

`PRINT`

:   The Unix command used to queue print jobs, it used to output data
    sent to the PRT device. popen() is used to send the data, so you may
    specify options, flags etc.

    <div class="example">

    ``` {.example}
             PRINT = lpr -Pmy_printer
    ```

    </div>

`CPU_HOG`

:   Define it 0 to make UQLX try to behave multitasking friendly, it
    will go sleeping when it believes that QDOS is idle. The detection
    whether QDOS is idle usually works pretty well, but in some cases it
    may get fooled by very frequent IO, eg an high speed serial
    connection - in this case define it to 1 to get all time UNIX will
    give us. Alternatively the `-h` option can be used to enforce
    CPU\_HOG mode. Largely obsolete now as it may be toggled through
    GUI.

    <div class="example">

    ``` {.example}
             CPU_HOG = 1
    ```

    </div>

`FAST_START`

:   Set to 1 if you want to skip the usual RAM test(default), or set it
    to 0 if you want to enjoy the Ram test pattern.

    <div class="example">

    ``` {.example}
             FAST_START=1
    ```

    </div>

`ROMIMG`

:   The ROMIMG option defines additional ROMS to be loaded at specific
    addresses. These should include TK2 if required.

    <div class="example">

    ``` {.example}
             ROMIMG = tk2_rom,0xc000
    ```

    </div>

    It is assumed that the ROM image can be found in the ROMDIR
    directory. The address should be specified in ’C’ numeric format.

`XKEY_ON`

:   A value of 1 can be used to indicate that qm should start with the
    alternative input method. This involves using ungrabbed keyboard (if
    configured) and preferring the X11 input method over QDOS
    translation of key events See section [Keyboard](#Keyboard). You
    might prefer this when you have a non-english keyboard and don’t use
    many special QL key combinations. The downside is that typical QL
    hotkeys are very often interpreted by window managers - these won’t
    be available for QDOS programs and may additionally screw up your
    desktop or even kill applications. Default is 0.

`XKEY_SWITCH`

:   Defines Keysym to be used to switch keyboard input method See
    section [Keyboard](#Keyboard). The Keysym name should be in the form
    returned by ’xev’, ie without the leading ’XK\_’. It should be
    accessible without modifiers. Default is to use the "F11" key.

    <div class="example">

    ``` {.example}
             XKEY_SWITCH = F16      
    ```

    </div>

`DO_GRAB`

:   Whether to do keyboard grabbing. This is used to avoid confusion
    when window manager would try to interpret QDOS key like ALT-F1.
    Proper fix is to disable it and get a ICCM compliant wm (eg.
    windowmaker) Enabling it will interfere with the broken Xkb
    extension See section [Keyboard](#Keyboard)

    <div class="example">

    ``` {.example}
             DO_GRAB = 0
    ```

    </div>

`XKEY_ALT`

:   Defines Keysym to be used as (additional) alternative to the Alt
    keys to simulate QDOS ALT. Reason for this is that many window
    managers catch away the Alt keys to use them as their hotkeys.
    Should be accessible without modifiers. Default "F12"

    <div class="example">

    ``` {.example}
            XKEY_ALT = Mode_switch  ## frequently this is Alt_R
    ```

    </div>

`STRICT_LOCK`

:   Controls whether strict locking applies for disk image files, the
    alternative being advisory locking. True by default, disable if you
    hate the ugly warnings. BTW never rely on locking in UNIX anyway.

    <div class="example">

    ``` {.example}
             STRICT_LOCK=1
    ```

    </div>

`DEVICE`

:   All directory devices may be defined in the options file. The format
    is

    <div class="example">

    ``` {.example}
    DEVICE = QDOS_name,  UNIX_pathname[,  flags]
    ```

    </div>

    `QDOS_name` is the name of the QDOS volume to be defined, eg `FLP1`,
    `WIN6`, `QXL1`. Currently `RAMx` is the only name that receives
    special attention. UQLX does not enforce any further naming
    conventions, however most QDOS software requires a 3 chars name
    length.

    `UNIX_pathname` refers to a file, directory or device used to
    simulate the QDOS device.

    The optional `flags` field supports this options.

    `clean`

    :   clean together with a "%x" in the unix pathname can be used to
        simulate RAMdisk. The "%x" is replaced with the process number
        at runtime so that multitasking QMs don’t disturb each other and
        after killing QM the directory is deleted.

    `qdos-fs`\
    `native`

    :   Both flags are synonyms. The qdos-fs option indicates that
        `UNIX_pathname` is the name of a file or device in the QDOS
        floppy disk or QXL.WIN formats; otherwise a Unix directory is
        assumed.

    `qdos-like`

    :   applicable only to non-`qdos-fs`. Filenames are not case
        sensitive and (sub)directory creation mimics SMSQ behaviour.

Devices may be removed from the device list by not supplying a unit
(volume) number. This is useful if some devices that are defined by
default, eg ’mdv’,’flp’ are unused.

<div class="example">

``` {.example}
         DEVICE = CD
```

</div>

Would remove the above default CDROM specification.

Some device mapping and other options are supplied as default; in
addition, the following defaults are also set.

<div class="example">

``` {.example}
         SYSROM = jsrom
         ROMDIR = /ql/
         RAMTOP = 4096
         COLOUR = 1

         PRINT = /usr/bin/lpr 
         CPU_HOG = 1
```

</div>

Note that no additional ROM (tk2) is defined by default.

and here is the example of an actual .uqlxrc file. You will find more
recent versions of it with every UQLX distribution.

<div class="example">

``` {.example}
SYSROM = js_rom                         # default ROM to use
ROMIM = tk2_rom,0xc000                  # extra ROM
ROMDIR = ~/qm/romdir/                   # ...search them here
RAMTOP = 16384
DEVICE = MDV1,~/qm/qldata/              # this directory will be accessible as 'mdv1_'
DEVICE = MDV2,~/qm/qlsoft/
DEVICE = FLP1,~/qm/DiskImage2,qdos-fs   # 'flp1_' is the image of a real QL floppy..
DEVICE = FLP2,~/qm/DiskImage,qdos-fs
DEVICE = FLP3,~/qm/DiskImage3,qdos-fs
DEVICE = WIN1,~/
DEVICE = WIN2,/
DEVICE = WIN3,~/PiQ/
DEVICE = RAM1,/tmp/.ram1-%x/,clean      # temporay dirs, cleared after exit
DEVICE = RAM2,/tmp/.ram2-%x/,clean
DEVICE = RAM3,/tmp/.ram3-%x/,clean
DEVICE = RAM4,/tmp/.ram4-%x/,clean
DEVICE = RAM5,/tmp/.ram5-%x/,clean
DEVICE = RAM6,/tmp/.ram6-%x/,clean
DEVICE = RAM7,/tmp/.ram7-%x/,clean
DEVICE = RAM8,/tmp/.ram8-%x/,clean
DEVICE = CD1                            # devices we don't want
DEVICE = MS1
COLOUR = 0                              # simulate MONO, 1=COLOR monitor

PRINT = lpr                    # printer spooler prog used by PRT port
CPU_HOG = 0                    # don't burn CPU power in QDOS scheduler loop
FAST_START = 1                 # skip ramtest
```

</div>

------------------------------------------------------------------------

[]{#Program-Invocation}

  -------------------------------------------------------------------------------- -------------------------------------------------------------------------- ---------- ----------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Customization "Beginning of this chapter or previous chapter")\]   \[[&lt;](#About-_002euqlxrc-files "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#Program-Name "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------------- -------------------------------------------------------------------------- ---------- ----------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Program-Invocation-1}

5 Program Invocation {#program-invocation .chapter}
====================

If compiled with `-DUSE_VM` - currently Linux-x86 and SPARC, a patch
database must be built on first startup. This is usually done
automatically.

UQLX often prints diagnostic messages to `stdout` and `stderr`, so start
it in its own xterm.

During startup, the emulator will attempt to boot from ‘`mdv1_BOOT`’
(Case sensitive !!) or read commands from the `BOOT` device if one was
specified with the ’`-s`’ or ’`-b`’ option.

It appears that in Minerva 1.98 the name of the default `BOOT` file
changed from ‘`mdv1_BOOT`’ to ‘`mdv1_boot`’!! To keep compatibility with
other ROMS I would recommend a soft link like

<div class="example">

``` {.example}
ln -s BOOT boot         # in mdv1_
```

</div>

  ------------------------------------------------------------------- ---- ---------------------------------------------
  [5.1 Program Name](#Program-Name)                                        important shortcut to select configurations
  [5.2 Command Line options](#Command-Line-options)                        to override defaults
  [5.3 BOOT Files](#BOOT-Files)                                            a few points to know
  [5.4 GUI](#GUI)                                                          
  [5.5 Signals - Terminating UQLX](#Signals-_002d-Terminating-UQLX)        
  [5.6 ROM Images](#ROM-Images)                                            
  [5.7 The big screen feature](#The-big-screen-feature)                    
  [5.8 X Window Managers](#X-Window-Managers)                              
  [5.9 Keyboard](#Keyboard)                                                
  [5.10 Scripting](#Scripting)                                             
  ------------------------------------------------------------------- ---- ---------------------------------------------

------------------------------------------------------------------------

[]{#Program-Name}

  ------------------------------------------------------------------------------------- --------------------------------------------------------------------- -------------------------------------------- ------------------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Program-Invocation "Previous section in reading order")\]   \[[Up](#Program-Invocation "Up section")\]   \[[&gt;](#Command-Line-options "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- --------------------------------------------------------------------- -------------------------------------------- ------------------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Program-Name-1}

5.1 Program Name {#program-name .section}
----------------

‘`qm`’ is the recommended name for normal usage. A different name will
be interpreted as follows: if either ’`min`’ or ’`js`’ is part of the
program name, `qm` will attempt to use a ‘`minerva_rom`’ resp ‘`js_rom`’
for the emulation, overriding any options or defaults.

Thus if you often need to switch between Minerva and JS roms, or even
want to have both running at the same time, the easiest way to manage is
to arrange some soft links approximately like this:

<div class="example">

``` {.example}
         ln -s qm qjs
         ln -s qm qmin
```

</div>

If the program name contains an ’`x`’, ’SIZE\_X’ and ‘`minerva_rom`’
will OVERRIDE other defaults. Likewise, ’`xx`’ will trigger ’SIZE\_XX’
and ’`xxx`’ ’SIZE\_XXX’. The current compiled in defaults for screen
size are

<div class="example">

``` {.example}
         SIZE_X   = 632x400
         SIZE_XX  = 720x512
         SIZE_XXX = 800x600
```

</div>

------------------------------------------------------------------------

[]{#Command-Line-options}

  ------------------------------------------------------------------------------------- --------------------------------------------------------------- -------------------------------------------- --------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Program-Name "Previous section in reading order")\]   \[[Up](#Program-Invocation "Up section")\]   \[[&gt;](#BOOT-Files "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- --------------------------------------------------------------- -------------------------------------------- --------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Command-Line-options-1}

5.2 Command Line options {#command-line-options .section}
------------------------

uqlx supports the following command line options; these override
settings in ‘`~/.uqlxrc.`’ Note that options in turn can be overridden
by program name as described above.

<div class="example">

``` {.example}
         qm [-r RAMTOP] [-i] [[-c][-m]] [-f file] [-h] [-o romname] 
            [-s [string]] [-b [string]]
```

</div>

where:

`-r RAMTOP`

:   Defines the RAMTOP value in kB. Any enlarged screen also has to fit
    into this value.

`-c`

:   Forces colour mode.

`-m`

:   Forces mono mode (even on a colour X display).

`-g nXm`

:   Start with screen size nXm, effective only with Minerva roms. **See
    big screen**

`-f file`

:   Defines an alternative options file.

`-h`

:   Force CPU\_HOG mode, take all available CPU time for the emulator.

`-o romname`

:   Use romname instead of ROM defined in ‘`.uqlxrc`’ file

`-s boot_cmd`

:   No attempt is made to make a connection to the Xserver, See section
    [Scripting](#Scripting). `boot_cmd` must be present, it defines a
    QDOS ’BOOT’ device to be used, see `-b` option.

`-b boot_cmd`

:   Define QDOS ’BOOT’ device that will return the `boot_cmd` string on
    read. The `boot_cmd` should be a string of the form
    `"10 lrun mdv1_progxx"` or similar; quoting newlines is tricky and
    therefore only 1 line expressions are recommended. A QDOS newline
    char is automatically appended to the string.

`-i`

:   Start with UQLX window iconised - if supported by window manager

------------------------------------------------------------------------

[]{#BOOT-Files}

  ------------------------------------------------------------------------------------- ----------------------------------------------------------------------- -------------------------------------------- -------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Command-Line-options "Previous section in reading order")\]   \[[Up](#Program-Invocation "Up section")\]   \[[&gt;](#GUI "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- ----------------------------------------------------------------------- -------------------------------------------- -------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#BOOT-Files-1}

5.3 BOOT Files {#boot-files .section}
--------------

Usually there are no special requirements for UQLX boot files, just
remember to store it as ‘`mdv1_BOOT`’ - case will be significant in the
default installation.

There seems to be a problem with some versions of `Ptr_gen`. If your
boot file fails with random errors near the place where it is loaded,
try inserting this after `LRESPR Ptr_gen`:

<div class="example">

``` {.example}
PAUSE#2,1
```

</div>

This will not work with JS ROMS, use a for loop or similar to cause the
delay there..

You may query the UNIX environment variables and the startup parameters
of the emulator from your bootfile - [SuperBasic
Extensions](#SuperBasic-Extensions)

------------------------------------------------------------------------

[]{#GUI}

  ------------------------------------------------------------------------------------- ------------------------------------------------------------- -------------------------------------------- ----------------------------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#BOOT-Files "Previous section in reading order")\]   \[[Up](#Program-Invocation "Up section")\]   \[[&gt;](#Signals-_002d-Terminating-UQLX "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- ------------------------------------------------------------- -------------------------------------------- ----------------------------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#GUI-1}

5.4 GUI {#gui .section}
-------

The supplied GUI is intentionally very simple - it is supposed to
compile and work on every system with X without requiring additional
toolkits etc. Seems only the old Athena Widget set meets this
requirement :-(

Resources are in ‘`Xql`’, you may change button names, fonts or even use
own graphics.

The GUI supports this actions:

`Paste to keyboard Queue`

:   .. should be obvious.

`Clone UQLX`

:   Create an exact copy of this UQLX program, complete with jobs etc.
    Files are kept open mainly for the UnixFS, accessing same floppy or
    QXL devices from two applications will cause problems.

`ROM breakpoints`

:   Toggle whether setting ROM breakpoints is possible. This will
    completely remove ROM protection so use only when needed.

`Redraw QL screen`

:   May be useful in some circumstances..

`XKeyLookup`

:   Switch whether X or QDOS key translation method is applied, similar
    as `XKEY_ON` in ‘`~/.uqlxrc`’ resp. the key configured by
    `XKEY_SWITCH`

`cpu hog`

:   Toggle CPU saving mode, similar to `-h` or `CPU_HOG`

------------------------------------------------------------------------

[]{#Signals-_002d-Terminating-UQLX}

  ------------------------------------------------------------------------------------- ------------------------------------------------------ -------------------------------------------- --------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#GUI "Previous section in reading order")\]   \[[Up](#Program-Invocation "Up section")\]   \[[&gt;](#ROM-Images "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- ------------------------------------------------------ -------------------------------------------- --------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Signals-_002d-Terminating-UQLX-1}

5.5 Signals - Terminating UQLX {#signals---terminating-uqlx .section}
------------------------------

UQLX handles all signals, currently they will have this effect:

`SIGINT`

:   typically bound to `^C` will do a ’soft termination’, cleaning up
    all temporary files and directories.

`SIGQUIT`

:   typically bound to `^\` will cause an immediate exit, this is useful
    in the unlikely case that `SIGINT` fails due to some recursive
    error.

`SIGABRT`

:   is generated internally for debugging reasons when UQLX encounters
    an virtual memory error it is unable to handle.

------------------------------------------------------------------------

[]{#ROM-Images}

  ------------------------------------------------------------------------------------- --------------------------------------------------------------------------------- -------------------------------------------- --------------------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Signals-_002d-Terminating-UQLX "Previous section in reading order")\]   \[[Up](#Program-Invocation "Up section")\]   \[[&gt;](#The-big-screen-feature "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- --------------------------------------------------------------------------------- -------------------------------------------- --------------------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#ROM-Images-1}

5.6 ROM Images {#rom-images .section}
--------------

Thanks to Tony Firshman, Minerva v1.89 is now PD and can be bundled with
UQLX - it is ‘`romdir/min.189`’.

For various reasons you may run into some trouble when trying romimages
other than the supplied js\_rom or min.189. Roughly, the known causes
are:

-   rom image not supported by UQLX. Maybe some old, never tested QDOS
    roms
-   rom image has been patched by GC, SGC or similar. All QL extensions
    that have something better than a 68008 CPU are likely to do this.

    On a SGC, according to Zeljko Nastasic you may retrieve the original
    ROM by this command:

    <div class="example">

    ``` {.example}
    SBYTES <device><filename>,hex('400000'),48*1024
    ```

    </div>

-   some TK2 roms fail possibly because they get confused by the UQLX
    filesystem which is a mix of the basic and FS II. Use some older rom
    if possible, 2.10 works very well for me.

  ------------------------------------------------------- ---- --
  [5.7 The big screen feature](#The-big-screen-feature)        
  [5.9 Keyboard](#Keyboard)                                    
  ------------------------------------------------------- ---- --

------------------------------------------------------------------------

[]{#The-big-screen-feature}

  ------------------------------------------------------------------------------------- ------------------------------------------------------------- -------------------------------------------- ---------------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#ROM-Images "Previous section in reading order")\]   \[[Up](#Program-Invocation "Up section")\]   \[[&gt;](#X-Window-Managers "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- ------------------------------------------------------------- -------------------------------------------- ---------------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#The-big-screen-feature-1}

5.7 The big screen feature {#the-big-screen-feature .section}
--------------------------

If you choose to work with a screen bigger than 512x256, there are a few
important points.

**Warning: it is possible to define screens bigger than the physical
screen - make sure that you know how to move around by your virtual
screen manager in this case. Beware that UQLX captures all key events.**

The maximum useable screen size depends on available RAM and X server
habits. If at all possible try to run UQLX with 8bit depth - see
customisation. For 8bit depth the memory requirements will be
approximately x\*y\*1.25 bytes + whatever the Xserver allocates. This
can be quite a lot because UQLX uses backing store and shared memory
when available, thus turning of this features in the Makefile or Xserver
may improve performance for very big screen sizes by a factor of 2.

If you are an extremist, there are several window managers that will
allow you to have rather big screen sizes, eg fvwm. The maximum I have
tested so far is 8192x4096 with SunOS/Solaris. The theoretical limit is
- because of Pointer Environment’s bad habits that the screen must fit
with all RAM into 16 MB

Pointer Environment is patched when activated to recognise the new
screen parameters - there are ’cleaner’ solutions, unfortunately with
severe side effects. If you receive the warning "could not patch PE",
you are in serious trouble..

Screen geometry may be slightly adapted to result in clean
x-resolution/sd.linel ratio. Length of screen buffer must always be
truncated nearest 32K boundary, therefore some screen sizes may result
in a certain waste of memory.

------------------------------------------------------------------------

[]{#X-Window-Managers}

  ------------------------------------------------------------------------------------- ------------------------------------------------------------------------- -------------------------------------------- ------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#The-big-screen-feature "Previous section in reading order")\]   \[[Up](#Program-Invocation "Up section")\]   \[[&gt;](#Keyboard "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- ------------------------------------------------------------------------- -------------------------------------------- ------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#X-Window-Managers-1}

5.8 X Window Managers {#x-window-managers .section}
---------------------

Some of them work better than others, and some don’t work at all. UQLX
is slightly more demanding on them because it has to do keyboard
grabbing and warps the mouse pointer. ICCM compliant wm’s (those that
don’t grab events from clients) should be preferred where possible.

Enlightenment with gnome is reported to freeze (pretty old report by
now).

Tested to work: olvwm, fvwm2, afterstep, windowmaker. Many of these have
settings which can make a big improvement.

------------------------------------------------------------------------

[]{#Keyboard}

  ------------------------------------------------------------------------------------- -------------------------------------------------------------------- -------------------------------------------- -------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#X-Window-Managers "Previous section in reading order")\]   \[[Up](#Program-Invocation "Up section")\]   \[[&gt;](#Scripting "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- -------------------------------------------------------------------- -------------------------------------------- -------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Keyboard-1}

5.9 Keyboard {#keyboard .section}
------------

There are 2 points of concern when emulating a QDOS type keyboard under
X11:

First, many window managers try to interpret key events that are
meaningful for QDOS programs and invoke their own actions - this is
almost always undesired. To resolve the conflict I have implemented
keyboard grabbing. Unfortunately, due to a design flaw of the X11/XKB
extension, keyboard grabbing doesn’t work with many non-us keyboards.
Typically some keys accessed by AltGr or similar do no longer give
correct results in UQLX.

Best cure is do NOT use XKB. Despite what many national-howtos say Xkb
is completely optional and only wastes resources. To disable it,add
following lines in ‘`/etc/XF86Config`’

<div class="example">

``` {.example}
XkbDisable
RightAlt    ModeShift
```

</div>

You can then safely set `DO_GRAB = 0` in the config file. To get exactly
the same bindings with or without Xkb you need to use ‘`~/.Xmodmap`’.
Create it in a xkb-enabled session like this:

<div class="example">

``` {.example}
xmodmap -pke >~/.Xmodmap
```

</div>

Second problem, there are often key combination for which both QDOS and
X11 have meaningful interpretations and that are difficult to generate
otherwise.

To resolve the problems, I have added a switch, switching this states:

-   default state: \[grabbed keyboard\] & preferred QDOS interpretation
    of key events, indicated by arrow mouse cursor (indication only when
    PE cursor inactive!)
-   \[ungrabbed keyboard\] & preferred X11 interpretation of key events,
    indicated by crossbar mouse cursor. All characters returned by ’xev’
    should be available in this state.

The switch is bound to a key, F11 by default and in default state unless
configured otherwise See section [About .uqlxrc
files](#About-_002euqlxrc-files) `XKEY_SWITCH`. Switching the keyboard
mode can be also done using the GUI

To make life with XkB easier, it is also possible to define a special
key that is interpreted as QDOS’ Alt key by the emulator.

Be warned, that when the keyboard is ungrabbed, the X11 window manager
may react strangely to some of your hotkeys. Perhaps the easiest way to
avoid clashes in this case would be to redefine the ’meta’ modifier to
something completely unusual like F13.

Here is a short description what the xql\_key() approximately does in
case you want to mess around with it.

X itself is pretty straightforward but the code is a bit complicated for
the reasons outlined above. Additionally I am trying to circumvent some
problems with vendor specific keyboards. Eg some SUN keyboards return
Keysyms like ’F35’ instead of XK\_Next - not even programs like Netscape
or Xemacs can cope with it without being hacked (which is far easier
with Xemacs btw). And don’t even mention SGI keyboards and WIN95 X
emulation to me ...

The result is a 3 level key-combination translation scheme:

<div class="example">

``` {.example}
X defines:
1) keycodes - each keyboard button has a number that is completely OS dependent
   This number is only useful to find out what KeySyms are associated with the
   button as I do in keycode_from_XKeycode()
   keycodes are used to see how X translates them - if the same result could 
   be achieved with "less" modifiers, a QDOS translation is preferred.

   state - is a value indicating which modifiers are to be applied to the 
   keycode. Unfortunately state is wrong when keyboard is grabbed and XKB 
   being used.

2) keysyms - gives some portable name to keycodes and is to some extent 
   in an 'unportable way' dependent on the modifiers (shift, ctrl, meta).
   The Keysym data type is an integer that is associated to the keysymname by 
   XKeysymToString().
   Keysyms associated with ASCII chars have the ASCII value of the char - some
   tests in the code are based on this.

3) XLookupString is basically the default ASCII/ISO value associated with the key
   combination described by the XEvent. Unfortunately XlookupString is no 
   longer supported - it is not politically correct in the days of i18n.
   The advertised replacements for it are not only complete overkill for 
   a QDOS emulator that understands less than the ISO-1 charset by definition 
   - they would also increase the complexity of this 3 level translation 
   scheme to ludicrous levels.
```

</div>

------------------------------------------------------------------------

[]{#Scripting}

  ------------------------------------------------------------------------------------- ----------------------------------------------------------- -------------------------------------------- ---------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Keyboard "Previous section in reading order")\]   \[[Up](#Program-Invocation "Up section")\]   \[[&gt;](#Filesystems "Next section in reading order")\]   \[[&gt;&gt;](#Filesystems "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- ----------------------------------------------------------- -------------------------------------------- ---------------------------------------------------------- --------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Scripting-1}

5.10 Scripting {#scripting .section}
--------------

`qm` can now be used to run QDOS programs as scripts. This feature is
not yet completed but I already used it in some `cron` commands. With
the `pty_` device and this facility QDOS will soon rival `perl` as an
extension language.

Both the `-b` and `-s` options can be used to pass a command string that
overrides the usual ‘`mdv1_BOOT`’ actions. `-s` additionally inhibits
any attempt to open an X window to display the QL screen.

With the `-s` option, screen IO will be redirected to fd 0 for input, fd
1 for output. Piping input into UQLX is somewhat difficult, not only
aren’t special codes (\^C,break) working, also very strange things may
happen at eof. Currently piping input to Minerva doesn’t work reliably.

Since it turns out that quoting SuperBasic characters or newlines can be
extremely complex I would recommend passing only very simple command
strings, something like

<div class="example">

``` {.example}
qm -s "10 lrun mdv1_prog"
```

</div>

should work without any problems with any shell, a single chr\$(10) is
appended to each command string.

There are a few SuperBasic extensions provided to pass arguments to
scripts and access environment variables, namely `getXargc`, `getXarg$`
and `getXenv$`. see section [SuperBasic
Extensions](#SuperBasic-Extensions)

------------------------------------------------------------------------

[]{#Filesystems}

  ------------------------------------------------------------------------------------- ------------------------------------------------------------ ---------- ------------------------------------------------------------------------ ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Program-Invocation "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Scripting "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#UNIX-Filesystem-Interface "Next section in reading order")\]   \[[&gt;&gt;](#Other-Devices "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------------- ------------------------------------------------------------ ---------- ------------------------------------------------------------------------ ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Filesystems-1}

6 Filesystems {#filesystems .chapter}
=============

Both QDOS/SMSQ diskimages and the UNIX host filesystem can be accessed,
for configuration details see the ‘`.uqlxrc`’ file.

It is also possible to use real QDOS floppies, but some care must be
taken. Especially disk swaps will only be recognised when all files are
closed.

There is no file locking for the UnixFS. QDOS diskimages and QXL.WIN
files are locked before accessed, because the potential for damage is
much greater. However not every Unix will support locking block devices
- test this if you intend to run more than 1 emulators at a time.

**File locking is mandatory ONLY when the mode (chmod(2)) of the file is
set accordingly. This is what UQLX does by default - see customisation.
However with the strict locking enabled some problems remain, eg if some
another program opens the image file before UQLX. It seems file locking
can never be done absolutely foolproof in Unix, the results may depend
on the particular brand of Unix.**

**When UQLX fails to establish a lock for some reason it will proceed
with a warning.**

Readonly access is implemented for all types of filesystems simply by
respecting the UNIX file modes and returning `ERR.RO`. In UnixFS this is
on a per file basis, whereas on floppy/QXL.WIN this controls the whole
volume. Most QDOS programs seem to ignore ERR.RO btw which can be a
problem.

The Unix Filesystem can be accessed both translated and untranslated.
The translated version is used in the default configuration to host
’mdv1\_’ and some other devices.

The untranslated version is accessed as the uQVFSx Filesystem, see that
section. The uQVFSx Filesystem is good if you want to access a Unix file
of which you know the (Unix)filename or simply need very long pathnames.
It can be also used to access raw and special devices, eg the /proc
filesystem.

  ------------------------------------------------------------- ---- -------------------------------
  [6.1 UNIX Filesystem Interface](#UNIX-Filesystem-Interface)        Make Unix-FS QDOS compatible
  [6.2 QDOS floppy and QXL.WIN](#QDOS-floppy-and-QXL_002eWIN)        
  [6.3 uQVFSx Filesystem](#uQVFSx-Filesystem)                        Accessing very long pathnames
  ------------------------------------------------------------- ---- -------------------------------

------------------------------------------------------------------------

[]{#UNIX-Filesystem-Interface}

  ------------------------------------------------------------------------------ -------------------------------------------------------------- ------------------------------------- -------------------------------------------------------------------------- ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Filesystems "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Filesystems "Previous section in reading order")\]   \[[Up](#Filesystems "Up section")\]   \[[&gt;](#QDOS-floppy-and-QXL_002eWIN "Next section in reading order")\]   \[[&gt;&gt;](#Other-Devices "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------ -------------------------------------------------------------- ------------------------------------- -------------------------------------------------------------------------- ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#UNIX-Filesystem-Interface-1}

6.1 UNIX Filesystem Interface {#unix-filesystem-interface .section}
-----------------------------

The UNIX FS Interface provides access to the underlying UNIX (or
similar) host filesystem. Standard QDOS and most QDOS-FS II file
operations are mapped to UNIX calls, full (sub-)directory access is
provided.

This means that whatever filesystems are accessible from Unix (CD,
MSDOS, Amiga partitions, ZIP drives ....) are accessible to QDOS
programs.

The filenames are translated to a QDOS like syntax, ‘`/`’ maps to ‘`_`’.
Unfortunately this means, that in very rare situations a file may shadow
some subdirectory. UNIX names are therefore supported as well, this
looks like ‘`mdv1_c68/INCLUDE/stdio_h`’.[(2)](#FOOT2){#DOCF2}

The `qdos-like` flag selects whether the filenames are case-sensitive
and whether (sub)directory creation will have Unix or SMSQ semantics.

Using the `qdos-like` flag should be restricted to directories reserved
for UQLX. If programs other than UQLX create files with names that are
not distinguishable in case-insensitive mode the results will be
undefined.

The default Unix FS is case-sensitive which can be a real pain with
SuperBasic, but works quite good with most other software. Beware that
QDOS will attempt to boot from ‘`mdv1_BOOT`’ !

Another lovely source of confusion is using SuperBasic symbols as
filenames – SB always remembers the case of the symbol when seen first
time and converts to this case subsequently.

Thus

<div class="example">

``` {.example}
open#6,mdv1_BoOt
open#4,mdv1_BOOT      REM still accessing mdv1_BoOt !!
```

</div>

Data-type, -space and file version are stored in an one per directory
‘`.-UQLX-`’ file. This means that UNIX hard and soft links for QDOS
executables, as well as moving or copying them around by `mv` will fail.
There are some utilities in the ‘`utils`’ subdirectory that can be used
to manipulate the entries. Depending on the underlying Unix filesystem
there may be problems to create or access a file ‘`.-UQLX-`’, so exotic
filesystems like ’umsdos’ are only very limited QDOS compatible.

The UNIX directories are visible to QDOS like normal FileSystem II
directories. Since QDOS doesn’t use a distinct directory separator, this
resulted in a rather complex algorithm for finding files, and in
pathologic cases may result in certain files being shadowed.

The ‘`.`’ and ‘`..`’ directories are accessible just like that in QDOS
but aren’t listed in directories anymore.

------------------------------------------------------------------------

[]{#QDOS-floppy-and-QXL_002eWIN}

  ------------------------------------------------------------------------------ ---------------------------------------------------------------------------- ------------------------------------- ------------------------------------------------------------ ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Filesystems "Beginning of this chapter or previous chapter")\]   \[[&lt;](#UNIX-Filesystem-Interface "Previous section in reading order")\]   \[[Up](#Filesystems "Up section")\]   \[[&gt;](#qxl_005ffschk "Next section in reading order")\]   \[[&gt;&gt;](#Other-Devices "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------ ---------------------------------------------------------------------------- ------------------------------------- ------------------------------------------------------------ ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#QDOS-floppy-and-QXL_002eWIN-1}

6.2 QDOS floppy and QXL.WIN {#qdos-floppy-and-qxl.win .section}
---------------------------

QXL.WIN files are now supported as well as direct use of floppy/QXL.WIN
devices. Currently,
`disk swaps are only recognised when all files on that device are closed!`

UQLX can use ‘`DD`’ or ‘`HD`’ diskimages.[(3)](#FOOT3){#DOCF3} It should
be noted that UQLX does not yet work well with unusual floppy formats,
even when the files are listed correctly caution is recommended.

Floppy-Diskimages can be taken by ’dd’, although ’cat’ usually works as
well (and much faster on SUN). On SUNOS the exact syntax is

<div class="example">

``` {.example}
dd if=/dev/rfd0 of=DiskImagename
```

</div>

- unfortunately it does rarely work as easily on Solaris.

Man pages of ‘`dd`’ or ‘`tar`’ usually give good hints about the floppy
names, one possible complication is Volume Management, refer to ’man
vold’.

On Linux anything works, unless you have a very special floppy use
`/dev/fd0` as filename.

  ------------------------------------ ---- --
  [6.2.1 qxl\_fschk](#qxl_005ffschk)        
  ------------------------------------ ---- --

------------------------------------------------------------------------

[]{#qxl_005ffschk}

  ------------------------------------------------------------------------------ ------------------------------------------------------------------------------ ----------------------------------------------------- ---------------------------------------------------------------- ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Filesystems "Beginning of this chapter or previous chapter")\]   \[[&lt;](#QDOS-floppy-and-QXL_002eWIN "Previous section in reading order")\]   \[[Up](#QDOS-floppy-and-QXL_002eWIN "Up section")\]   \[[&gt;](#uQVFSx-Filesystem "Next section in reading order")\]   \[[&gt;&gt;](#Other-Devices "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------ ------------------------------------------------------------------------------ ----------------------------------------------------- ---------------------------------------------------------------- ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#qxl_005ffschk-1}

### 6.2.1 qxl\_fschk {#qxl_fschk .subsection}

This program is now supplied in the ‘`utils`’ subdirectory. It should be
run before exchanging QXL.WIN filesystems images with SMSQ software, it
will spot some potential compatibility problems and detect many types of
filesystem damage.

------------------------------------------------------------------------

[]{#uQVFSx-Filesystem}

  ------------------------------------------------------------------------------ ---------------------------------------------------------------- ------------------------------------- ------------------------------------------------------------ ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Filesystems "Beginning of this chapter or previous chapter")\]   \[[&lt;](#qxl_005ffschk "Previous section in reading order")\]   \[[Up](#Filesystems "Up section")\]   \[[&gt;](#Other-Devices "Next section in reading order")\]   \[[&gt;&gt;](#Other-Devices "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------ ---------------------------------------------------------------- ------------------------------------- ------------------------------------------------------------ ----------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#uQVFSx-Filesystem-1}

6.3 uQVFSx Filesystem {#uqvfsx-filesystem .section}
---------------------

UQLX now offers access to (almost) unlimited pathname lengths. So far I
have tested pathnames up to 1017 bytes which appears to be some limit in
Solaris, 4000 being the supposed limit of the current UQLX
implementation. However you should be warned that TK2 and/or maybe some
QDOS versions might mess up and fail to report ERR\_NF when you are
trying to access a long pathname that can’t be opened.

The syntax has some similarities with QVFS (also many differences), and
if practical I intend to make it QVFS compatible in the future. QVFS is
a new filesystem (c) by Hans-Peter Recktenwald - see
`http://www.snafu.de/~phpr/qhpqvfs.html` for more info.

To access a unix file through uQVFSx specify its full pathname in unix
syntax. Sometimes uQVFSx will also replace ’\_’ by ’/’ if it matches
this way - this is only intended as an compatibility hack to make some
antiqued programs work with it.

A leading ’/’ or ’XVFS\_’ is enforced to recognise an uQVFSx name.

Also, if for some reason you are not sure the filename is unique within
QDOS devices, prefix it with ’XVFS\_’

Examples:

<div class="example">

``` {.example}
view '/etc/motd'
view 'XVFS_/etc/motd'
view '/usr/include/sys/signal.h'
view '/usr_include_sys_signal.h'
```

</div>

You should be able to do all file operations defined for normal UQLX
filesystems except directory operations. Hoping that QVFS becomes stable
and widely accepted, I will add directories etc.

For now, getting directories works like this:

<div class="example">

``` {.example}
copy 'pty_ls -a /usr/include' to con_
spl 'pty_ls -a /usr/lib',#1
```

</div>

Of course any options ‘`ls`’ recognizes can be used, the `-a` option is
essential otherwise you could trigger a `SPL` bug that closes `#1` in
the 2nd example! See section [pty device](#pty-device) for details.

When this filesystem is used to access special files (devices), only a
restricted set of `trap#3` commands will work for this file.

------------------------------------------------------------------------

[]{#Other-Devices}

  ------------------------------------------------------------------------------ -------------------------------------------------------------------- ---------- --------------------------------------------------------- ------------------------------------------ --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Filesystems "Beginning of this chapter or previous chapter")\]   \[[&lt;](#uQVFSx-Filesystem "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#TCP_002fIP "Next section in reading order")\]   \[[&gt;&gt;](#Printing "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ------------------------------------------------------------------------------ -------------------------------------------------------------------- ---------- --------------------------------------------------------- ------------------------------------------ --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Other-Devices-1}

7 Other Devices {#other-devices .chapter}
===============

Here is a description of the `TCP/IP`,`pty`,`ser` and `prt` devices.

  ------------------------------- ---- ----------------------------------------------------
  [7.1 TCP/IP](#TCP_002fIP)            
  [7.2 pty device](#pty-device)        launch and control unix applications through QDOS!
  [7.3 ser device](#ser-device)        serial communication
  ------------------------------- ---- ----------------------------------------------------

------------------------------------------------------------------------

[]{#TCP_002fIP}

  -------------------------------------------------------------------------------- ---------------------------------------------------------------- --------------------------------------- --------------------------------------------------------- ------------------------------------------ --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Other-Devices "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Other-Devices "Previous section in reading order")\]   \[[Up](#Other-Devices "Up section")\]   \[[&gt;](#pty-device "Next section in reading order")\]   \[[&gt;&gt;](#Printing "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------------- ---------------------------------------------------------------- --------------------------------------- --------------------------------------------------------- ------------------------------------------ --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#TCP_002fIP-1}

7.1 TCP/IP {#tcpip .section}
----------

The `TCP/IP` features are described in the files ‘`docs/socket.*`’ that
came with this UQLX distribution or here:

<div class="example">

``` {.example}
http://www.geocities.com/SiliconValley/rdzidlic/socket_main.html
```

</div>

------------------------------------------------------------------------

[]{#pty-device}

  -------------------------------------------------------------------------------- ------------------------------------------------------------- --------------------------------------- --------------------------------------------------------- ------------------------------------------ --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Other-Devices "Beginning of this chapter or previous chapter")\]   \[[&lt;](#TCP_002fIP "Previous section in reading order")\]   \[[Up](#Other-Devices "Up section")\]   \[[&gt;](#ser-device "Next section in reading order")\]   \[[&gt;&gt;](#Printing "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------------- ------------------------------------------------------------- --------------------------------------- --------------------------------------------------------- ------------------------------------------ --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#pty-device-1}

7.2 pty device {#pty-device .section}
--------------

`pty`*jt*`_program name` *par1* ....

*j*

:   job control options

    `i`

    :   don’t care if child process exits. Default behaviour is to
        indicate EOF on read after the child process exited and all
        buffers were read, but theoretically someone might reconnect the
        tty.

    `k`

    :   kill child job after closing the QDOS channel. Default is don’t
        care.

*t*

:   translation options

    `c`

    :   translate carriage returns

    `z`

    :   translate char 26 (`^Z`)as end of file

    `t`

    :   translate QDOS &lt;–&gt; ISO-8859-1 font

*program name*

:   name of program to be executed and parameters

*parn*

:   arguments to be passed to the invoked program. This can be
    unix-style options, filenames etc.

    A special syntax allows the specification of UQLX filenames and
    redirection: \[following stuff isn’t yet implemented at all\]

    `' (single quote,\')`

    :   can be used avoid expansion of the following symbols

    `'#name'`

    :   denotes a QDOS filename. The path of the underlying Unix file is
        looked up and passed as argument to the program.

        The next example will run german ispell on the file
        ‘`mdv1_kant.tex`’. Note that even though the *t* flag is
        selected it applies only to the `pty` I/O - thus if
        ‘`mdv1_kant.tex`’ contains any non ASCII codes a separate
        translation is required.

        <div class="example">

        ``` {.example}
        open#4,"ptyt_ispell -d #mdv1_kant.tex"
        ```

        </div>

    `[n]>`\
    `[n]>>`\
    `[n]<`

    :   redirects stdin/out/err or fd `n` when given

A pty is a pseudo-terminal that enables you to run a program as if it
were connected to a real tty. The terminal output of the program is is
available as input from the connected QDOS channel, likewise output into
the QDOS channel appears as input from its terminal to the program. The
pty device works similarly as a pair of pipes, with the difference that
the launched program believes to execute on a real terminal, and the
same channel is used for i/o and error output.

examples:

<div class="example">

``` {.example}
open#6,pty_date     REM get unix time and date
input$#6,a$         REM should be same like QDOS date
print a$:close#6
```

</div>

A bit more complicated example, `'pty_ls -al'` would do the same easier.

<div class="example">

``` {.example}
10 open#6,pty_csh
20 print#6,"ls -al"
30 print#6,"exit"       rem otherwise we can't detect the end!
40 repeat xx
    if eof(#6) :exit xx
50  input#6,a$:print a$
60 end repeat xx
70 close#6
```

</div>

Connect to an NNTP server and post a test message. Most likely you will
have to use another NNTP server and change the "From:" to contain a
legal address.

<div class="example">

``` {.example}
100 PRINT "NNTP Posting software"
110 PRINT
120 OPEN#6,'pty_telnet  rznews.rrze.uni-erlangen.de 119'
130 get_resp
140 get_resp
150 get_resp
160 get_resp
170 PRINT "** Connected **"
180 PRINT#6,"post"
190 get_resp
200 PRINT#6,"Newsgroups: alt.test"
210 PRINT#6,"Subject: test"
220 PRINT#6,"From: test@alt.test"
230 PRINT#6
240 PRINT#6,"test"
250 PRINT#6,"test"
260 PRINT#6,"test"
270 PRINT#6
280 PRINT#6,"."
290 PRINT#6
300 get_resp
310 PRINT#6,"quit"
315 REPeat xx:get_resp:IF EOF(#6) :EXIT xx
320 CLOSE#6
330 DEFine PROCedure get_resp
340  INPUT#6,c$:PRINT c$
350 END DEFine 
360 DEFine PROCedure sa
370 SAVE_O mdv1_script
380 END DEFine 
```

</div>

If the pty driver (pty.c) compiles on your system; try setting the QTPI
"Comm Dev" setting to ’pty\_/bin/login’ (or, better, "pty\_/sbin/agetty
-L -h 38400 -" on a Linux box). The pty device takes one parameter, the
Unix command to run. If no command is given, it tries /bin/sh as a login
shell. Usually it is not even necessary to give the full path of the
command as above.

The pty code is taken from the ’emu’ X Window terminal program; the
original archive contains many clues on getting it to work on different
platforms. You can find out about ’emu’ from emu@pcs.com.

*In this context, beware of an*` TK2` *bug -* ` SPL ` *may accidentally
close a basic channel passed as* `#ch ` *if the channel returns an
immediate* `EOF ` *as the* `pty `*device can do*

<div class="example">

``` {.example}
SPL 'pty_ls emptydir',#1        REM #1 gets closed! (-:
```

</div>

------------------------------------------------------------------------

[]{#ser-device}

  -------------------------------------------------------------------------------- ------------------------------------------------------------- --------------------------------------- ------------------------------------------------------- ------------------------------------------ --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Other-Devices "Beginning of this chapter or previous chapter")\]   \[[&lt;](#pty-device "Previous section in reading order")\]   \[[Up](#Other-Devices "Up section")\]   \[[&gt;](#Printing "Next section in reading order")\]   \[[&gt;&gt;](#Printing "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------------- ------------------------------------------------------------- --------------------------------------- ------------------------------------------------------- ------------------------------------------ --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#ser-device-1}

7.3 ser device {#ser-device .section}
--------------

`ser`*npht*`_`*b*baudrate

except for the `_baudrate`, the options have the same meaning as in QDOS
where applicable.

*n*

unit number, currently 1,2

*p*

parity

`O`

`E`

`M`

`S`

*h*

handshake

`H`

:   use handshake

`I`

:   ignore handshake

*t*

translation

`R`

:   no translation

`Z`

:   recognise `^Z` as EOF

`C`

:   carriage return

Here is some documentation for `ser` and `pty` devices, originally
compiled by Jonathan Hudson

WARNING: this implementation is tested only on Linux, implementing it
for other OS/architectures may require nontrivial changes. The `pty`
drivers are also tested on Solaris.

Serial and pty (pseudo terminal drivers) are available for uqlx. Due to
the machine specific nature of these devices, some work may be required
to make them work on non-Linux systems. In particular, the
`ioctl(..,FIONREAD,..)` call may not be supported.

The serial device (1 and 2) takes an extra parameter, the baud rate.
`MT.BAUD` is also supported, but as it only works to 19200 baud, it is
not much use. The extended syntax is, for example:

<div class="example">

``` {.example}
        ser2hr_b57600   (57600 baud)
```

</div>

The data transfer rate appears CPU bound, but 5100 cps TX and 4800 cps
RX are achievable for QTPI/ZMODEM.

The serial device names should be specified in the ‘`.uqlxrc`’ file,
good choice for Linux is ‘`/dev/ttyS0`’ and ‘`/dev/ttyS1`’.

------------------------------------------------------------------------

[]{#Printing}

  -------------------------------------------------------------------------------- ------------------------------------------------------------- ---------- -------------------------------------------------------------------- ------------------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Other-Devices "Beginning of this chapter or previous chapter")\]   \[[&lt;](#ser-device "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#SuperBasic-Extensions "Next section in reading order")\]   \[[&gt;&gt;](#SuperBasic-Extensions "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------------- ------------------------------------------------------------- ---------- -------------------------------------------------------------------- ------------------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Printing-1}

8 Printing {#printing .chapter}
==========

The `prt` device can be used for printing. The data sent to `prt` is
piped to the printer command specified in ‘`.~/.uqlxrc`’ which may be
overridden or modified by providing additional arguments to the `prt`
device.

`prt`*ft*`_`*add\_options*`!`*alt\_command*

*f*

:   ignore for QDOS compatibility

*t*

:   translation: use active TRA table

    `add_options`

    :   specify additional options to be passed to default printer
        command

    `alt_command`

    :   specify alternative command to be executed

Data sent to the `prt` device is piped to the specified filter. If you
have QDOS printer drivers for the printer in use, try to send your
output to `lpr`. If this doesn’t work, try following definition in
‘`.~/.uqlxrc`’:

<div class="example">

``` {.example}
PRINT = lpr -Praw
```

</div>

If your system doesn’t have a `-Praw` you can add it by editing
‘`/etc/printcap`’ or as a quick hack, just defining

<div class="example">

``` {.example}
PRINT = cat >/dev/lp0
```

</div>

For this to work you will probably need to change ‘`/dev/lp0`’
permissions. Obviously this should not be used together with a standard
lpd..

*Some care must be used when specifying printer/filter commands: when
closing the printer channel uqlx calls `pclose` which waits until the
command(s) exits – in this situation uqlx may appear to hang.*

If this happens, kill the filter process from another xterm.

------------------------------------------------------------------------

[]{#SuperBasic-Extensions}

  --------------------------------------------------------------------------- ----------------------------------------------------------- ---------- ------------------------------------------------------ ----------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#Printing "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Printing "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#TECHREF "Next section in reading order")\]   \[[&gt;&gt;](#TECHREF "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  --------------------------------------------------------------------------- ----------------------------------------------------------- ---------- ------------------------------------------------------ ----------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#SuperBasic-Extensions-1}

9 SuperBasic Extensions {#superbasic-extensions .chapter}
=======================

[]{#index-Kill_005fUQLX}Procedure: **Kill\_UQLX** *result*

:   Kill the emulator returning result to the calling program

<!-- -->

[]{#index-UQLX_005fRELEASE_0024}Function: **UQLX\_RELEASE\$**

:   Returns release identification as string

<!-- -->

[]{#index-getXenv_0024}Function: **getXenv\$** *name*

:   Returns value of the (UNIX) environment variable name as string

<!-- -->

[]{#index-Fork_005fUQLX}Function: **Fork\_UQLX**

:   Create an exact copy of this UQLX process. A new Xwindow is created,
    files on directory device drivers recreated. However beware that a
    file that remained opened during a fork may now be writable by two
    or more UQLX instances. Also using same stream i/o channel from both
    instances of the process will result in chaos, especially `pty`
    channels have to loose EOF.

    It appears as if UQLX and/or X can get easily confused when
    `Fork_UQLX` is applied while the mouse pointer/focus is in the UQLX
    window. This doesn’t seem to be a serious problem, as forking should
    be most useful when used in scripts anyway.

    Returns `pid` for the parent process, `0` for its child.

    An utterly useful example program is:

    <div class="example">

    ``` {.example}
    10 for i=1 to 4
    20 print Fork_UQLX
    30 end for i
    ```

    </div>

<!-- -->

[]{#index-getXargc}Function: **getXargc**

:   Returns the number of arguments that were given to the emulator at
    startup, options or arguments that have been consumed away by Xtk
    not counted.

    <div class="example">

    ``` {.example}
    qm -m -r 1024 arg1 arg2 arg3

    PRINT getXargc          => 4   (arg0=qm !)
    ```

    </div>

<!-- -->

[]{#index-getXarg_0024}Function: **getXarg\$** *nth*

:   Returns the nth argument, continuing from above example

    <div class="example">

    ``` {.example}
    for i=0 to getXargc-1 : PRINT i, getXarg$(i)
    ```

    </div>

    results in

    <div class="example">

    ``` {.example}
    0       /user80/rdzidlic/qm/qm
    1       arg1
    2       arg2
    3       arg3
    4       arg4
    5       arg5
    ```

    </div>

<!-- -->

[]{#index-getXres}Function: **getXres**

:   Returns x-size of screen

<!-- -->

[]{#index-getYres}Function: **getYres**

:   Returns y-size of screen

<!-- -->

[]{#index-scr_005fxlim}Function: **scr\_xlim**

:   same as `getXres`

<!-- -->

[]{#index-scr_005fylim}Function: **scr\_ylim**

:   same as `getYres`

------------------------------------------------------------------------

[]{#TECHREF}

  ---------------------------------------------------------------------------------------- ------------------------------------------------------------------------ ---------- --------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#SuperBasic-Extensions "Beginning of this chapter or previous chapter")\]   \[[&lt;](#SuperBasic-Extensions "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#ByteOrder-Issues "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------------------------------------------- ------------------------------------------------------------------------ ---------- --------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#TECHREF-1}

10 TECHREF {#techref .chapter}
==========

For a general overview of the Emulator see also Daniele’s original
QM.README.

  -------------------------------------------------------------------------------- ---- --
  [10.1 ByteOrder Issues](#ByteOrder-Issues)                                            
  [10.2 Debugging](#Debugging)                                                          
  [10.3 ROMs and Patching](#ROMs-and-Patching)                                          
  [10.4 Calling 68K code from the emulator](#Calling-68K-code-from-the-emulator)        
  [10.5 Directory Device Drivers](#Directory-Device-Drivers)                            
  [10.6 Patch Database](#Patch-Database)                                                
  [10.7 Device Drivers](#Device-Drivers)                                                
  -------------------------------------------------------------------------------- ---- --

------------------------------------------------------------------------

[]{#ByteOrder-Issues}

  -------------------------------------------------------------------------- ---------------------------------------------------------- --------------------------------- ----------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#TECHREF "Previous section in reading order")\]   \[[Up](#TECHREF "Up section")\]   \[[&gt;](#Memory "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ---------------------------------------------------------- --------------------------------- ----------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#ByteOrder-Issues-1}

10.1 ByteOrder Issues {#byteorder-issues .section}
---------------------

  ------------------------------------------------------------------ ---- --
  [10.1.1 Memory](#Memory)                                                
  [10.1.2 PC - the program counter](#PC-_002d-the-program-counter)        
  [10.1.3 Registers](#Registers)                                          
  ------------------------------------------------------------------ ---- --

------------------------------------------------------------------------

[]{#Memory}

  -------------------------------------------------------------------------- ------------------------------------------------------------------- ------------------------------------------ --------------------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#ByteOrder-Issues "Previous section in reading order")\]   \[[Up](#ByteOrder-Issues "Up section")\]   \[[&gt;](#PC-_002d-the-program-counter "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ------------------------------------------------------------------- ------------------------------------------ --------------------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Memory-1}

### 10.1.1 Memory {#memory .subsection}

QDOS memory is always in 68k format, that is big-endian. There are some
possible ways access memory:

1.  raw always takes absolute (host) address, only byte order conversion
    is done here.
    <div class="example">

    ``` {.example}
            RW(),WW()       read,write 16bit word
            RL(),WL()                  32bit
    ```

    </div>

2.  cooked always takes QL address, checks bounds, adds QLmem base and
    determines what sort of memory is used(ROM.screen,RAM,hardware)

    <div class="example">

    ``` {.example}
            w8 ReadHWByte(w32 addr);
            w8 ReadByte(w32 addr);
            w16 ReadWord(w32 addr);
            w32 ReadLong(w32 addr);
            void WriteByte(w32 addr,w8 d);
            void WriteWord(w32 addr,w16 d);
            void WriteLong(w32 addr,w32 d);
    ```

    </div>

    This must be done only in a very controlled way, bad alignment or
    other errors may cause very hard to debug m68k exceptions

3.  arbitrary, eg strcpy() or read() into QL memory you must carefully
    obey all restrictions and call ChangedMemory(from,to) to indicate
    the change to screen-flush manager

------------------------------------------------------------------------

[]{#PC-_002d-the-program-counter}

  -------------------------------------------------------------------------- --------------------------------------------------------- ------------------------------------------ -------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Memory "Previous section in reading order")\]   \[[Up](#ByteOrder-Issues "Up section")\]   \[[&gt;](#Registers "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- --------------------------------------------------------- ------------------------------------------ -------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#PC-_002d-the-program-counter-1}

### 10.1.2 PC - the program counter {#pc---the-program-counter .subsection}

the `pc` is a global variable used as program counter; it is somewhat
special in that it already has the base of QL memory, "theROM" added to
it. Thus if you need the QL-relative pc value `((long)pc-(long)theROM)`
will do.

------------------------------------------------------------------------

[]{#Registers}

  -------------------------------------------------------------------------- ------------------------------------------------------------------------------- ------------------------------------------ -------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#PC-_002d-the-program-counter "Previous section in reading order")\]   \[[Up](#ByteOrder-Issues "Up section")\]   \[[&gt;](#Debugging "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ------------------------------------------------------------------------------- ------------------------------------------ -------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Registers-1}

### 10.1.3 Registers {#registers .subsection}

are an array of w32’s, always stored in host format byte order.
`reg[8]`,`*(reg+8)`,`aReg[0]` all refer to A0

byte and word access to registers is done using address calculation,
thus

<div class="example">

``` {.example}
        *(uw8*)((Ptr)reg+RBO)           is d0.b
        *(uw16*)((Ptr)(reg+1)+RWO)      is d1.w
```

</div>

A more complicated example is:

<div class="example">

``` {.example}
        (w32)*(((w16*)((Ptr)reg+RWO+((displ>>10)&60))))+
                        aReg[code&7]+(w32)((w8)displ));
```

</div>

this will compute the the xx(An,Rn.w) address, `code`, is the first
instruction word and `displ` the extend instruction code.

------------------------------------------------------------------------

[]{#Debugging}

  -------------------------------------------------------------------------- ------------------------------------------------------------ --------------------------------- ---------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Registers "Previous section in reading order")\]   \[[Up](#TECHREF "Up section")\]   \[[&gt;](#Trace "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ------------------------------------------------------------ --------------------------------- ---------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Debugging-1}

10.2 Debugging {#debugging .section}
--------------

It is recommended to compile with the ‘`-ggdb3`’ option if you use gdb,
this flag can be set eg by setting ‘`LOCAL_OPTFLAGS`’. Older gcc
versions only support ‘`-ggdb`’

If you are interested in a certain file, set ’DEBUG\_FILES = file.c’ in
\~/.uqlx\_cprefs so that this file is compiled for debugging. If you
think you might want to debug UQLX, but don’t know what exactly, you may
need to compile without the ’-fomit-frame-pointer’, otherwise `bt`
(backtrace) won’t work.

To work with `gdb` you should ensure that the ‘`.gdbinit`’ file supplied
with UQLX is used.

start qm within gdb:

<div class="example">

``` {.example}
>gdb qm
gdb>run
```

</div>

if you run into an exception,

<div class="example">

``` {.example}
gdb> call DbgInfo()
```

</div>

will give me an idea what is happening.

It is also possible to disable the ROM protection (See section
[GUI](#GUI)) to allow breakpoints in ROM, See section [GUI](#GUI)

It is possible to activate normal QL debuggers from `gdb` by typing

<div class="example">

``` {.example}
qldbg
continue
```

</div>

This causes UQLX to generate a `trap# $e` exception with the current
`PC` as "breakpoint address"(usually this points 2 bytes before you
would expect!). This is useful if you wish to see what happens after a
certain UQLX function (eg driver call) returns to normal 68K execution
mode.

If you suspect a certain function or region of code of causing some
trouble, email me the TRACE output with exact description of how it was
generated (including modified tracetable and software used) - see next
section.

  ------------------------ ---- --
  [10.2.1 Trace](#Trace)        
  ------------------------ ---- --

------------------------------------------------------------------------

[]{#Trace}

  -------------------------------------------------------------------------- ------------------------------------------------------------ ----------------------------------- ---------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Debugging "Previous section in reading order")\]   \[[Up](#Debugging "Up section")\]   \[[&gt;](#ROMs-and-Patching "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ------------------------------------------------------------ ----------------------------------- ---------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Trace-1}

### 10.2.1 Trace {#trace .subsection}

I have added selective tracing, if you want to activate just add the
`-DTRACE` in Makefile, recompile everything and customise tracetable in
trace.c to suit your needs. For technical reasons all values are printed
before the instruction is executed. This means that "code" and "PC" are
not sync. Thus if you get output like this

<div class="example">

``` {.example}
DebugInfo: PC=48de, code=4eba, SupervisorMode: no USP=fffbc SSp=28480 A7=fffc8
Register Dump:   Dn             An
0                      0             107
1               50000000             d88
2                      0            8d2b
3               a0000000             186
4                      0             ce8
5                     df            8cee
6                      1           ff068
7                      0           fffc8
Trace : RI.MULT+4
DebugInfo: PC=48e2, code=48e7, SupervisorMode: no USP=fffbc SSp=28480 A7=fffbc
Register Dump:   Dn             An
0                      0             107
1               50000000             d88
2                      0            8d2b
3               a0000000             186
4                      0             ce8
5                     df            8cee
6                      1           ff068
7                      0           fffbc
```

</div>

beware that code=48e7 belongs to PC=48de and to find the instruction;
gdb&gt; print table\[0x48e7\] however the program must have been started
before this, otherwise ’table’ would not have been initialised.

------------------------------------------------------------------------

[]{#ROMs-and-Patching}

  -------------------------------------------------------------------------- -------------------------------------------------------- --------------------------------- --------------------------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Trace "Previous section in reading order")\]   \[[Up](#TECHREF "Up section")\]   \[[&gt;](#Calling-68K-code-from-the-emulator "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- -------------------------------------------------------- --------------------------------- --------------------------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#ROMs-and-Patching-1}

10.3 ROMs and Patching {#roms-and-patching .section}
----------------------

the supplied JS\_ROM is original. The patching process is implemented in
QL\_configure.c and is designed to work for many js-similar ROMS without
change.

The patches are:

-   correct some table access routine at 0x250 that apparently causes
    crashes of QLs &gt;2MB
-   install special handlers for IPC access and MDV vectors

Installing handlers is done by overwriting the ROM at a suitable address
with a specific A-line opcode and modifying the emulators instruction
table to call a specified handler function (instead of just initiating
an A-line exception).

This handler function is then responsible to check that indeed it was
called from a well defined location and not just by accident - this
usually is a statement in the form

<div class="example">

``` {.example}
if((long)gPC-(long)theROM-2==EMUL_IPC_LOC)
{      if(IPC_Command()) rts();
        else table[code=0x40e7]();
}
else
{      exception=4;
        extraFlag=true;
        nInst2=nInst;
        nInst=0;
        }
```

</div>

where `EMUL_IPC_LOC` is the stored address of the patch. If it is indeed
the location, the handler function can access QL memory and registers
using the techniques outlined above, or if a special condition is not
met proceed emulation as if there was no patch at all - this technique
is demonstrated in QL\_hardware:QL\_KeyTrans

Note that if there is an rts() in any routine, this will modify the
emulators A7 and PC but not do any other action (as returning from the
c-function itself). Thus you might write ’rts();rts();rts(); return;’
and it may make perfectly sense, depending on the context of the patch.

------------------------------------------------------------------------

[]{#Calling-68K-code-from-the-emulator}

  -------------------------------------------------------------------------- -------------------------------------------------------------------- --------------------------------- ----------------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#ROMs-and-Patching "Previous section in reading order")\]   \[[Up](#TECHREF "Up section")\]   \[[&gt;](#Directory-Device-Drivers "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- -------------------------------------------------------------------- --------------------------------- ----------------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Calling-68K-code-from-the-emulator-1}

10.4 Calling 68K code from the emulator {#calling-68k-code-from-the-emulator .section}
---------------------------------------

this is perfectly possible, provided QDOS is an a state where it allows
this particular request. A trap\#0 call may be done as

<div class="example">

``` {.example}
savedA0=*aReg;
QLtrap(1,0,20000l);
printf("QDOS vars at %x, trap res=%d, RTOP=%d\n",aReg[0],reg[0],RTOP);
*aReg=savedA0;
```

</div>

The 200000l is the instruction-count limit may be chosen arbitrarily -
but must not be exceeded. Similarly, the following will do a vector
call:

<div class="example">

``` {.example}
QLvector(0xd0,200000);
```

</div>

However not all vectors may be safely called, those that manipulate its
return point may fail.

Following must be arranged:

-   saving and restoring all registers
-   ensuring no QDOS context switch happens within the code

The risk of encountering a context switch is reduced by switching off
the 50Hz interrupts in such calls, often other precautions may be wise
(clearing SV.POLM or entering SU mode)

------------------------------------------------------------------------

[]{#Directory-Device-Drivers}

  -------------------------------------------------------------------------- ------------------------------------------------------------------------------------- --------------------------------- ------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Calling-68K-code-from-the-emulator "Previous section in reading order")\]   \[[Up](#TECHREF "Up section")\]   \[[&gt;](#Patch-Database "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ------------------------------------------------------------------------------------- --------------------------------- ------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Directory-Device-Drivers-1}

10.5 Directory Device Drivers {#directory-device-drivers .section}
-----------------------------

are defined in QL\_files.h and designed to serve every kind of filling
systems attached to UQLX. Currently the code is very messy so don’t
bother to understand it.

------------------------------------------------------------------------

[]{#Patch-Database}

  -------------------------------------------------------------------------- --------------------------------------------------------------------------- --------------------------------- ------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Directory-Device-Drivers "Previous section in reading order")\]   \[[Up](#TECHREF "Up section")\]   \[[&gt;](#Device-Drivers "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- --------------------------------------------------------------------------- --------------------------------- ------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Patch-Database-1}

10.6 Patch Database {#patch-database .section}
-------------------

This is stored in `~/.uqlxpatch` directory if the emulator was compiled
with `-DUSE_VM`. There is a checksum to identify a ROM (everything from
0 to 96K is checksummed) and entries to mark from where this ROM
accesses the QL HW registers.

The files must be writable! UQLX usually takes care of that, just don’t
try neat tricks like `'umask 222'`.

This files are not meant to be user editable but can be deleted anytime
- in fact this may be necessary if something went seriously wrong.

------------------------------------------------------------------------

[]{#Device-Drivers}

  -------------------------------------------------------------------------- ----------------------------------------------------------------- --------------------------------- ---------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Patch-Database "Previous section in reading order")\]   \[[Up](#TECHREF "Up section")\]   \[[&gt;](#Memory-Management "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ----------------------------------------------------------------- --------------------------------- ---------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Device-Drivers-1}

10.7 Device Drivers {#device-drivers .section}
-------------------

simple (nondir) devices like the printer "PRT" are handled in
QL\_driver.c The attempted approach is to have a single generic driver
handling all simple device drivers which can then be written in C
without detailed knowledge of how QDOS drivers actually work and without
having to care about alignment and/or byte order problems when accessing
QL memory.

To add a new driver, add an entry to the `QL_driver.c:Drivers[]` table.
The definition should include the open/io/close routines and information
about name decoding.

The members of ’struct DRV’:

`ref`

:   intern use only

`init`

:   pointer to an init routine that will be called at the time the
    driver is linked into QDOS.

`open_test`

:   routine to test whether `'name'` is a legal channel name for this
    channel.

    should return:

    `0`

    :   not this device

    `1`

    :   ok, name fully decoded

    `-1`

    :   bad name

    If NAME\_PARS points to an appropriate structure the ’decode\_name’
    utility can be used to do the decoding. The open\_test routine
    should not itself allocate any memory, decoded parameters might be
    passed in an array of global values to the ’open’ routine that gets
    called subsequently.

`open:`

:   this routine should do the actual open and allocate memory for
    status information. See section [Memory
    Management](#Memory-Management)

    returns:

    `0`

    :   success

    `<>0 & reg[0]= QDOS_ERR set:`

    :   error

`close`

:   may return memory allocated by open etc.

`io`

:   routine to do QDOS io,See section [io\_handle](#io_005fhandle)

`slot`

:   not yet used

  ------------------------------------------------ ---- ------------------------------------------------
  [10.7.1 Memory Management](#Memory-Management)        where to store channel specific data
  [10.7.2 io\_handle](#io_005fhandle)                   translate QDOS trap\#3 semantics to read/write
  [10.7.3 decode\_name](#decode_005fname)               
  [10.7.4 Name Description](#Name-Description)          
  [10.7.5 Examples](#Examples)                          
  ------------------------------------------------ ---- ------------------------------------------------

------------------------------------------------------------------------

[]{#Memory-Management}

  -------------------------------------------------------------------------- ----------------------------------------------------------------- ---------------------------------------- ------------------------------------------------------------ ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Device-Drivers "Previous section in reading order")\]   \[[Up](#Device-Drivers "Up section")\]   \[[&gt;](#io_005fhandle "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ----------------------------------------------------------------- ---------------------------------------- ------------------------------------------------------------ ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Memory-Management-1}

### 10.7.1 Memory Management {#memory-management .subsection}

The `open` routine should malloc() a block of memory for each channel it
manages, it should return a pointer at this block via its second
parameter. Subsequent calls to dev.io and dev.close will receive this
pointer as an argument (observe the \*\* in the declaration!) The driver
should not attempt to store anything in the QDOS channel definition
block!

------------------------------------------------------------------------

[]{#io_005fhandle}

  -------------------------------------------------------------------------- -------------------------------------------------------------------- ---------------------------------------- -------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Memory-Management "Previous section in reading order")\]   \[[Up](#Device-Drivers "Up section")\]   \[[&gt;](#decode_005fname "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- -------------------------------------------------------------------- ---------------------------------------- -------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#io_005fhandle-1}

### 10.7.2 io\_handle {#io_handle .subsection}

is designed to ease the task of writing dev.io routines, very similar to
QDOS io.serio vector. Calls that may block must be handled specifically.
Also, be warned that most unix io calls can get interrupted by the
SIGALRM used for emulating interrupts.

------------------------------------------------------------------------

[]{#decode_005fname}

  -------------------------------------------------------------------------- ---------------------------------------------------------------- ---------------------------------------- --------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#io_005fhandle "Previous section in reading order")\]   \[[Up](#Device-Drivers "Up section")\]   \[[&gt;](#Name-Description "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ---------------------------------------------------------------- ---------------------------------------- --------------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#decode_005fname-1}

### 10.7.3 decode\_name {#decode_name .subsection}

is an utility function similar to io.name.

<div class="example">

``` {.example}
int decode_name(char *name, struct NAME_PARS *ndescr, open_arg *parblk)
```

</div>

`ndescr`

:   is description of the channel name synatx

`parblk`

:   points to an array of (long long) that should be large enough for
    all parameters.

`open_arg`

:   is union used to store either char \* or int type arguments

------------------------------------------------------------------------

[]{#Name-Description}

  -------------------------------------------------------------------------- ------------------------------------------------------------------ ---------------------------------------- ------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#decode_005fname "Previous section in reading order")\]   \[[Up](#Device-Drivers "Up section")\]   \[[&gt;](#Examples "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ------------------------------------------------------------------ ---------------------------------------- ------------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Name-Description-1}

### 10.7.4 Name Description {#name-description .subsection}

see examples.

There are a few extensions, most notably `parse_nseparator` provides a
way to parse text delimited by 2 characters or one character and end of
name.

Care should be taken not to take certain chars as delimiters and/or
options that are differently mapped in Unix and QDOS, eg backtick and
pound.

Also consider that for option letters and separators upper and lower
case is not distinguished for alphabetic characters.

------------------------------------------------------------------------

[]{#Examples}

  -------------------------------------------------------------------------- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Name-Description "Previous section in reading order")\]   \[[Up](#Device-Drivers "Up section")\]   \[[&gt;](#FAQ "Next section in reading order")\]   \[[&gt;&gt;](#FAQ "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ------------------------------------------------------------------- ---------------------------------------- -------------------------------------------------- ------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Examples-1}

### 10.7.5 Examples {#examples .subsection}

’PRT’ is a simple example driver, if you want to study the channel
opening syntax consider the example `'bg'` bogus driver(\#define TEST in
QL\_driver.c)

An example call to open a ’bg’ channel is:

<div class="example">

``` {.example}
open#4,'bg299F_119/999x333-string 1 ****xxx++++-,string====,,,,,,'
```

</div>

------------------------------------------------------------------------

[]{#FAQ}

  -------------------------------------------------------------------------- ----------------------------------------------------------- ---------- ------------------------------------------------------ ----------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#TECHREF "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Examples "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#History "Next section in reading order")\]   \[[&gt;&gt;](#History "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ----------------------------------------------------------- ---------- ------------------------------------------------------ ----------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#FAQ-1}

11 FAQ {#faq .chapter}
======

I have just started it so there is not yet any structure in it, just a
few points that did strike me. Even with the best docs there are many
gotcha’s ;-)

`The AltGr keys don't work even if I use the X11 lookup method.`

:   You probably also have the XKEY\_ALT bound to `Mode_switch` – thus
    an extra char `"\255"` is inserted with each `AltGr` key

`How do I access Unix devices from UQLX?`

:   The uQVFSx Filesystem does it, provided you don’t need any ioctls,
    [uQVFSx Filesystem](#uQVFSx-Filesystem). For more complicated cases
    writing a device driver for UQLX is not as difficult and many
    examples as well as description exist.

------------------------------------------------------------------------

[]{#History}

  ---------------------------------------------------------------------- ------------------------------------------------------ ---------- ---------------------------------------------------------------------- -------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#FAQ "Beginning of this chapter or previous chapter")\]   \[[&lt;](#FAQ "Previous section in reading order")\]   \[ Up \]   \[[&gt;](#Daniele-Terdinas-README "Next section in reading order")\]   \[[&gt;&gt;](#Benchmarks "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------------------------- ------------------------------------------------------ ---------- ---------------------------------------------------------------------- -------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#History-1}

12 History {#history .chapter}
==========

  ---------------------------------------------------------- ---- --
  [12.1 Daniele Terdinas README](#Daniele-Terdinas-README)        
  ---------------------------------------------------------- ---- --

------------------------------------------------------------------------

[]{#Daniele-Terdinas-README}

  -------------------------------------------------------------------------- ---------------------------------------------------------- --------------------------------- ------------------------------------------------------------- -------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#History "Beginning of this chapter or previous chapter")\]   \[[&lt;](#History "Previous section in reading order")\]   \[[Up](#History "Up section")\]   \[[&gt;](#Q_002demuLator "Next section in reading order")\]   \[[&gt;&gt;](#Benchmarks "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ---------------------------------------------------------- --------------------------------- ------------------------------------------------------------- -------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Daniele-Terdinas-README-1}

12.1 Daniele Terdinas README {#daniele-terdinas-readme .section}
----------------------------

This is the original README I received with Daniele’s sources; refer to
TECHREF for updates. Much of the information contained herein is more or
less outdated.

  ---------------------------------------------------------------------- ---- --
  [12.1.1 Q-emuLator](#Q_002demuLator)                                        
  [12.1.2 A brief note about endianess](#A-brief-note-about-endianess)        
  [12.1.3 Daniels FAQ](#Daniels-FAQ)                                          
  ---------------------------------------------------------------------- ---- --

------------------------------------------------------------------------

[]{#Q_002demuLator}

  -------------------------------------------------------------------------- -------------------------------------------------------------------------- ------------------------------------------------- --------------------------------------------------------------------------- -------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#History "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Daniele-Terdinas-README "Previous section in reading order")\]   \[[Up](#Daniele-Terdinas-README "Up section")\]   \[[&gt;](#A-brief-note-about-endianess "Next section in reading order")\]   \[[&gt;&gt;](#Benchmarks "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- -------------------------------------------------------------------------- ------------------------------------------------- --------------------------------------------------------------------------- -------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Q_002demuLator-1}

### 12.1.1 Q-emuLator {#q-emulator .subsection}

A brief comment about the 68K emulator sources.

These are the sources (written in the C language) of the 68000 emulator
which are used in my Sinclair QL emulator for Macintosh.

The sources are not complete: a few procedures are missing, as they are
not portable and you should rewrite them according to the target
platform. In particular all the parts regarding the hardware are missing
(except for the clock register); in any case they are not part of the
68008 emulator, but specific of the QL emulator.

A 68000 instruction code is 16 bit long (eventually followed by other
data or addressing mode specifications), so there are 65536 possible
codes. The idea is to use the instruction code as an index in a table of
functions’ addresses, and to execute the corresponding function. The
table is 65536 x 4 bytes long, i.e. 256K. Writing all the 65536
functions would be quite a long task, so I wrote only one function for
each instruction type: for example there is only one function to emulate
the MOVE.B instruction; the function’s address is put in all the table’s
positions corresponding to the code (binary encoded) 0001xxxxxxxxxxxx,
where x may be either 0 or 1, covering all the possible bit patterns. In
this way the MOVE.B function is called for all the 0001xxxxxxxxxxxx
instructions codes, like 0001000000000001 (MOVE.B D1,D0) or
0001110110111100 (MOVE.B \#imm,d(A6,RN), where d, RN and imm are
specified in the bytes following the instruction code). This generic
MOVE.B function execute the right operation by first interpreting the
bits marked x (in 0001xxxxxxxxxxxx); in the example to interpret the x
bits means finding the address modes of the source and destination
operands. This method allows for future optimizations: writing a greater
number of more specialized functions results in better performance. In
the MOVE.B example, we could write another function to emulate the
0001yyy000000xxx codes (MOVE.B Dx,Dy), which would speed up the
execution of the MOVE.B instruction with registers as source and
destination. Now this function (when it is called) knows already the
source and destination addressing modes (registers), avoiding the
overhead of the code-interpretation phase, as was the case with the
generic MOVE.B function.

The instruction-code fetching loop (in the ExecuteLoop procedure), which
looks up in the table the address of the corresponding function and
executes it, was rewritten in assembly language: as this loop is
executed hundred million times, it must be fully optimized.

The 68000’s registers (Dn, An, PC, SR) are memorized in global
variables.

Computing the flags during arithmetic operations is a problem: the C
language doesn’t allow a direct flags’ control (for example an overflow
caused by an addition can’t be easily detected: the only way to detect
it is by arguments like ’if the addends are positive and the result is
less then an addend, then an overflow has occurred’, etc.) and computing
the flags leads to a large number of complicated tests, resulting in
slower emulation.

Another problem are the read/write operations from/to the QL memory. I
decided (although it is a slow method) that all these operations can’t
be performed directly, but only through the
ReadByte/ReadWord/ReadLong/WriteByte/WriteWord/WriteLong functions. In
this way these six functions can check the address and behave correctly
depending on it; the following cases are possible: - QL’s RAM: the
standard case - QL’s ROM: can only be read. Write operations have no
effect. - QL’s video memory: when writing to it the emulator must draw
the corresponding pixels to a Macintosh window. This operation requires
a fair amount of time. To speed it up a little the effective drawing
operation is delayed as long as the modified bytes are adjacent to each
other. So the final conversion may involve entire rows instead of single
pixels. - QL’s hardware: the hardware ports behaviour must be emulated.
- any other address: writing has no effect, to avoid to randomly
overwrite the Macintosh memory. This means that whatever the QL does,
the Mac environment can’t be corrupted; in other words these controls on
each read/write operations result in a program which is very stable.

Exception processing: exceptions are caused by instructions (like TRAP
or CHK), by the 50/60Hz interrupt (generated by a timer), by
reading/writing words or long words to odd addresses, or by the trace
bit when it is set. The exception is called by putting the desired
exception number in the ’exception’ variable (otherwise the variable is
0). The variable is not checked after each instruction, to avoid slowing
down the instruction fetch&execute loop (ExecuteLoop). The loop counter
(variable ’nInst’, determining how many instructions have to be executed
yet) is zeroed instead, causing the loop to be exited. The fact that the
end of the loop was due to an exception and is not a normal loop
termination is signalled by setting the ’extraFlag’ variable to true;
the old ’nInst’ value is saved in ’nInst2’, so that it will be restored
when the loop will be re-entered after causing the exception. The
exception calling process consists in the following steps (see the 68000
manual): - enter supervisor mode and save SR and PC on the stack - load
the new PC from the ’exception handlers’ list, located in low memory

Addressing modes: operands are fetched through the ’GetFromEA’
(EA=Effective Address) functions and written to memory through the
’PutToEA’ functions. Often a memory location is read and then written by
the same instruction (i.e. it is modified). (eg: ADDQ.W \#1,(A0): the
word at the A0 address is read, incremented and written back). Using the
two GetFromEA and PutToEA calls would be a waste, because the address
decoding is performed twice; for this reason a ModifyAtEA function is
provided, which do the same as GetFromEA but saves in global variables
some information about the decoded address (Mac memory address, access
type: ram/rom/video/ecc...). Then the RewriteEA function is used to
rewrite data at the same address. RewriteEA takes advantage of the saved
information. Addresses (like a value in an An register) are maintained
as QL addresses, which are different from the real addresses: for
example the QL address 0 doesn’t correspond to the Mac address 0,
because this part of memory is used by the Mac operating system. The
read/write functions must relocate the address. The only exception to
this rule is the program counter, which is an address in the Mac memory,
to make the fetch&execute loop more efficient.

The types of int employed by the emulator are 8/16/32 bit signed and
unsigned. They are defined (file QLtypes.h) with the names
w8,w16,w32,uw8,uw16,uw32 (u means unsigned, w word, the number is the
width expressed in bit). The definitions are based on the basic C types
(char,int,short,long). Depending on the particular compiler and computer
which is used, it might be necessary to modify these definitions. For
example on the Mac an int is 32 bit, while on old 8086 systems an int is
usually 16 bit.

The file ql68000Init.c contains the code to fill in the 256K function
table. The functions which emulate the 68000 instructions are contained
in two files, because a unique file would be too long. The first file
contains the instructions with name beginning with a letter between a
and o, the second between p and z, plus (at the end) the many variants
of the shift instructions.

June, 4th 1995 Daniele Terdina

------------------------------------------------------------------------

[]{#A-brief-note-about-endianess}

  -------------------------------------------------------------------------- ----------------------------------------------------------------- ------------------------------------------------- ---------------------------------------------------------- -------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#History "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Q_002demuLator "Previous section in reading order")\]   \[[Up](#Daniele-Terdinas-README "Up section")\]   \[[&gt;](#Daniels-FAQ "Next section in reading order")\]   \[[&gt;&gt;](#Benchmarks "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ----------------------------------------------------------------- ------------------------------------------------- ---------------------------------------------------------- -------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#A-brief-note-about-endianess-1}

### 12.1.2 A brief note about endianess {#a-brief-note-about-endianess .subsection}

The 68000 processors are big endian, i.e. when they store in memory a
word or a long they store the most significant byte first (in lower
memory addresses). If you want to compile the emulator for little endian
machines (like Intel processors), you must perform byte swapping when
transferring word or long data between memory and registers. Luckily
almost all the accesses to memory in the emulator code are performed by
using the functions ReadByte, ReadWord, ReadLong, WriteByte, WriteWord,
WriteLong, so it is sufficient to add byte swapping in this functions.
However, there are also a few places where memory is accessed directly,
and you should change the code. Some (but probably not all) of these
places are: 1) all the accesses to immediate operands in the GetFromEA
and similar functions 2) all 16 bit displacement, both in the
GetFromEA-type functions and in jump or other particular instructions
(e.g.: bra, jsr, stop) 3) all the other places where the program counter
is used; in particular the instruction fetch and dispatch loop. I
suggest that to save time the instruction code is not swapped at all:
the swapping should be performed when filling the 256K emulator table
instead, so that the code, although with the two bytes in reverse order,
can directly be used as an index in the modified emulator table 4) all
the optimized functions which access to partial registers. For example,
when adding a byte to d0, the add\_b\_dn() function directly accesses
the fourth byte in the chunk of memory containing the register values.
On little endian processors this byte should be the first in the table
instead of the fourth. Similar changes are needed for many other
instructions which use only the byte or word lowest part of registers

------------------------------------------------------------------------

[]{#Daniels-FAQ}

  -------------------------------------------------------------------------- ------------------------------------------------------------------------------- ------------------------------------------------- --------------------------------------------------------- -------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#History "Beginning of this chapter or previous chapter")\]   \[[&lt;](#A-brief-note-about-endianess "Previous section in reading order")\]   \[[Up](#Daniele-Terdinas-README "Up section")\]   \[[&gt;](#Benchmarks "Next section in reading order")\]   \[[&gt;&gt;](#Benchmarks "Next chapter")\]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- ------------------------------------------------------------------------------- ------------------------------------------------- --------------------------------------------------------- -------------------------------------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Daniels-FAQ-1}

### 12.1.3 Daniels FAQ {#daniels-faq .subsection}

Q: What is RamMap (in the read/write QL memory functions, in file
QL68000\_general.c)?

A: RamMap is an array. Each position represents a 32K memory chunk (so
the array has 32 positions to cover the 68008’s 1M addressing space).
The value in the array represents the operations which are allowed on
the corresponding memory chunk: bit 0=read permission bit 1=write
permission bit 2=video memory bit 3=hardware register ie: ram has value
3, rom 1, unused addresses 0, video ram 7, hw registers 8.

Q: How do you manage the 50 hz interrupt of the 8049 and the Keyboard
interrupt?

A: First: I don’t emulate the keyboard interrupt. Anyway the keyboard is
polled by QDOS every 50th of second via the IPC commands 1 (test IPC
status) and 8 (read characters from the IPC keyboard buffer). As for the
50Hz interrupt, I have a timer which sets some variables every 20ms.
These variables cause an interrupt 2 exception: pendingInterrupt=2;
\*((uw8\*)theROM+0x280a0l)=16; extraFlag=true; nInst2=nInst; nInst=0;
Before this, however, the routine checks if interrupt are enabled and if
so set the interrupt mask (=8). It also disables further 50Hz
interrupts. When the QDOS I2 handler is called, it reads from an hw
register the interrupt mask and the value 8 tells it that the cause of
the interrupt was the 50hz interrupt. When the QDOS handler has
completed, it reenables the interrupts by writing to an hardware
register.

<div class="example">

``` {.example}
Daniele Terdina               e-mail: sistest@ictp.trieste.it
                              Feedback is always welcome!
```

</div>

------------------------------------------------------------------------

[]{#Benchmarks}

  -------------------------------------------------------------------------- -------------------------------------------------------------- ---------- ------------ ---------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[&lt;&lt;](#History "Beginning of this chapter or previous chapter")\]   \[[&lt;](#Daniels-FAQ "Previous section in reading order")\]   \[ Up \]   \[ &gt; \]   \[ &gt;&gt; \]                   \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  -------------------------------------------------------------------------- -------------------------------------------------------------- ---------- ------------ ---------------- --- --- --- --- ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

[]{#Benchmarks-1}

13 Benchmarks {#benchmarks .chapter}
=============

These are mainly interesting for me, but also intended to give new users
some idea what to expect. Remember, these are the times with JSROM, you
can get much better times with Minerva and/or Lightning

After trying around I implemented a method that may be considered
reasonably precise. The preferred method is now this:

<div class="example">

``` {.example}
make bench1             # mainly CPU emulation
make bench2             # mainly text output
```

</div>

I have again changed the details for `bench1` so these are now *old*
results:

  --------------------------------------------- ----------------- --------------------------
  Hardware/OS                                   bench1            bench2
  SUNW SPARCserver 1000, 384 MB                 18.4r,11.3u,.5s   105.9r,41.1u,4.6s
  Linux i686                                    xx,5.87u,.11s,    too slow X connection...
  Linux i80486DX, 33MHz, 256kB Cache 32MB RAM   xx,58.4u,1.s      xx,199.4u,.9s
  --------------------------------------------- ----------------- --------------------------

Here are some old benchmarks:

<div class="example">

``` {.example}
B1:
10 t=date
20 for i=1 to 2000:a=sin(1)
30 print date-t

and B2:
10 t=date
15 cls
20 for i=1 to 1000:print i,
30 print date-t


The Benchmarks also show how much effect some of the optimisations
show; the old value is taken without the -DFASTLOOP optimisation.


BENCHMARKS:             | B1                    | B2
Machine                 | old value             | old value
                        |       new result      |       new result(s)
---------------------------------------------------------------------
SPARC on remote         |  4s                   | 9-15s
  display               |                       |
---------------------------------------------------------------------
Linux/i486DX-33,8MB,64k | 28s                   | 71s
memory increased to 20MB|       20s             |       49s
...       256Kcache/32MB|               19s     |               53s
---------------------------------------------------------------------
Linux/486DX2-66,12MB,   | 19s                   | 35s
no L2 cache             |                       |
---------------------------------------------------------------------
Linux/P133/32Mb/S3      |  4s                   | 9s
Trio/1G Scsi            |                       |
---------------------------------------------------------------------
Linux/Cyrix 686 PR166   |  3.1s                 |  6.3s
                        |       2.7             |       6.5s
---------------------------------------------------------------------
Linux AMD 5x86 P75 UMC  |                       |
256KB Cache, 16 MB      |       8s              |       19s
---------------------------------------------------------------------
HP-9000/715-65/64MB     |  7s                   | 15s  
---------------------------------------------------------------------
SGI MIPS R5000/64MB     |  4s                   |  8s
---------------------------------------------------------------------
```

</div>

------------------------------------------------------------------------

[]{#SEC_Foot}

  ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

Footnotes
=========

### [(1)](#DOCF1){#FOOT1}

try `rdzidlic@cip.informatik.uni-erlangen.de` or
`rdzidlic@mailandnews.com` if that fails

### [(2)](#DOCF2){#FOOT2}

for the very daring, a compile option in ‘`uxfile.c`’ will avoid file
name translation altogether – technically this has only an effect when
opening directories

### [(3)](#DOCF3){#FOOT3}

reportedly some users used hacked diskimages to allow up to 100 MB per
image

------------------------------------------------------------------------

[]{#SEC_Contents}

  ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

Table of Contents
=================

<div class="contents">

-   [1 Introduction](#Introduction){#toc-Introduction-1}
    -   [1.1 System
        Requirements](#System-Requirements){#toc-System-Requirements-1}
    -   [1.2 COPYRIGHT](#COPYRIGHT){#toc-COPYRIGHT-1}
-   [2 Compiling](#Compiling){#toc-Compiling-1}
    -   [2.0.1 Misc Hints](#Misc-Hints){#toc-Misc-Hints-1}
    -   [2.0.2 Compiling
        Preferences](#Compiling-Preferences){#toc-Compiling-Preferences-1}
    -   [2.0.3 Further
        Options](#Further-Options){#toc-Further-Options-1}
    -   [2.0.4 Adding CPU specific
        optimizations](#Adding-CPU-specific-optimizations){#toc-Adding-CPU-specific-optimizations-1}
    -   [2.0.5 Unsupported
        Machines](#Unsupported-Machines){#toc-Unsupported-Machines-1}
    -   [2.0.6 Obscure
        `BUILDFLAGS`](#Obscure-BUILDFLAGS){#toc-Obscure-BUILDFLAGS-1}

[3 Installation](#Installation){#toc-Installation-1}
[4 Customization](#Customization){#toc-Customization-1}
-   [4.1 About .uqlxrc
    files](#About-_002euqlxrc-files){#toc-About-_002euqlxrc-files-1}

[5 Program Invocation](#Program-Invocation){#toc-Program-Invocation-1}
-   [5.1 Program Name](#Program-Name){#toc-Program-Name-1}
-   [5.2 Command Line
    options](#Command-Line-options){#toc-Command-Line-options-1}
-   [5.3 BOOT Files](#BOOT-Files){#toc-BOOT-Files-1}
-   [5.4 GUI](#GUI){#toc-GUI-1}
-   [5.5 Signals - Terminating
    UQLX](#Signals-_002d-Terminating-UQLX){#toc-Signals-_002d-Terminating-UQLX-1}
-   [5.6 ROM Images](#ROM-Images){#toc-ROM-Images-1}
-   [5.7 The big screen
    feature](#The-big-screen-feature){#toc-The-big-screen-feature-1}
-   [5.8 X Window
    Managers](#X-Window-Managers){#toc-X-Window-Managers-1}
-   [5.9 Keyboard](#Keyboard){#toc-Keyboard-1}
-   [5.10 Scripting](#Scripting){#toc-Scripting-1}

[6 Filesystems](#Filesystems){#toc-Filesystems-1}
-   [6.1 UNIX Filesystem
    Interface](#UNIX-Filesystem-Interface){#toc-UNIX-Filesystem-Interface-1}
-   [6.2 QDOS floppy and
    QXL.WIN](#QDOS-floppy-and-QXL_002eWIN){#toc-QDOS-floppy-and-QXL_002eWIN-1}
    -   [6.2.1 qxl\_fschk](#qxl_005ffschk){#toc-qxl_005ffschk-1}
-   [6.3 uQVFSx
    Filesystem](#uQVFSx-Filesystem){#toc-uQVFSx-Filesystem-1}

[7 Other Devices](#Other-Devices){#toc-Other-Devices-1}
-   [7.1 TCP/IP](#TCP_002fIP){#toc-TCP_002fIP-1}
-   [7.2 pty device](#pty-device){#toc-pty-device-1}
-   [7.3 ser device](#ser-device){#toc-ser-device-1}

[8 Printing](#Printing){#toc-Printing-1}
[9 SuperBasic
Extensions](#SuperBasic-Extensions){#toc-SuperBasic-Extensions-1}
[10 TECHREF](#TECHREF){#toc-TECHREF-1}
-   [10.1 ByteOrder Issues](#ByteOrder-Issues){#toc-ByteOrder-Issues-1}
    -   [10.1.1 Memory](#Memory){#toc-Memory-1}
    -   [10.1.2 PC - the program
        counter](#PC-_002d-the-program-counter){#toc-PC-_002d-the-program-counter-1}
    -   [10.1.3 Registers](#Registers){#toc-Registers-1}
-   [10.2 Debugging](#Debugging){#toc-Debugging-1}
    -   [10.2.1 Trace](#Trace){#toc-Trace-1}
-   [10.3 ROMs and
    Patching](#ROMs-and-Patching){#toc-ROMs-and-Patching-1}
-   [10.4 Calling 68K code from the
    emulator](#Calling-68K-code-from-the-emulator){#toc-Calling-68K-code-from-the-emulator-1}
-   [10.5 Directory Device
    Drivers](#Directory-Device-Drivers){#toc-Directory-Device-Drivers-1}
-   [10.6 Patch Database](#Patch-Database){#toc-Patch-Database-1}
-   [10.7 Device Drivers](#Device-Drivers){#toc-Device-Drivers-1}
    -   [10.7.1 Memory
        Management](#Memory-Management){#toc-Memory-Management-1}
    -   [10.7.2 io\_handle](#io_005fhandle){#toc-io_005fhandle-1}
    -   [10.7.3 decode\_name](#decode_005fname){#toc-decode_005fname-1}
    -   [10.7.4 Name
        Description](#Name-Description){#toc-Name-Description-1}
    -   [10.7.5 Examples](#Examples){#toc-Examples-1}

[11 FAQ](#FAQ){#toc-FAQ-1}
[12 History](#History){#toc-History-1}
-   [12.1 Daniele Terdinas
    README](#Daniele-Terdinas-README){#toc-Daniele-Terdinas-README-1}
    -   [12.1.1 Q-emuLator](#Q_002demuLator){#toc-Q_002demuLator-1}
    -   [12.1.2 A brief note about
        endianess](#A-brief-note-about-endianess){#toc-A-brief-note-about-endianess-1}
    -   [12.1.3 Daniels FAQ](#Daniels-FAQ){#toc-Daniels-FAQ-1}

[13 Benchmarks](#Benchmarks){#toc-Benchmarks-1}

</div>

------------------------------------------------------------------------

[]{#SEC_About}

  ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------
  \[[Top](#Introduction "Cover (top) of document")\]   \[[Contents](#SEC_Contents "Table of contents")\]   \[Index\]   \[[?](#SEC_About "About (help)")\]
  ---------------------------------------------------- --------------------------------------------------- ----------- ------------------------------------

About This Document
===================

This document was generated on *January 15, 2017* using [*texi2html
5.0*](http://www.nongnu.org/texi2html/).

The buttons in the navigation panels have the following meaning:

  Button           Name          Go to                                           From 1.2.3 go to
  ---------------- ------------- ----------------------------------------------- ------------------
  \[ &lt;&lt; \]   FastBack      Beginning of this chapter or previous chapter   1
  \[ &lt; \]       Back          Previous section in reading order               1.2.2
  \[ Up \]         Up            Up section                                      1.2
  \[ &gt; \]       Forward       Next section in reading order                   1.2.4
  \[ &gt;&gt; \]   FastForward   Next chapter                                    2
  \[Top\]          Top           Cover (top) of document                          
  \[Contents\]     Contents      Table of contents                                
  \[Index\]        Index         Index                                            
  \[ ? \]          About         About (help)                                     

where the **Example** assumes that the current position is at
**Subsubsection One-Two-Three** of a document of the following
structure:

-   1\. Section One
    -   1.1 Subsection One-One
        -   ...
    -   1.2 Subsection One-Two
        -   1.2.1 Subsubsection One-Two-One
        -   1.2.2 Subsubsection One-Two-Two
        -   1.2.3 Subsubsection One-Two-Three     **&lt;== Current
            Position**
        -   1.2.4 Subsubsection One-Two-Four
    -   1.3 Subsection One-Three
        -   ...
    -   1.4 Subsection One-Four

------------------------------------------------------------------------

This document was generated on *January 15, 2017* using [*texi2html
5.0*](http://www.nongnu.org/texi2html/).\
