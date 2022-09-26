sQLux
=====

**UQLX original Richard Zidlicky**
**sQLux alterations Graeme Gregory**
Copyright © 1996,1997,1998,1999 Richard Zidlicky
Copyright © 2020,2021 Graeme Gregory

Contents
--------

1. [Introduction](#1-Introduction)
2. [Compiling](#2-Compiling)
3. [Installation](#3-Installation)
4. [Customization](#4-Customization)
5. [Program Invocation](#5-Program-Invocation)
6. [Filesystems](#6-Filesystems)
7. [Other Devices](#7-Other-Devices)
8. [Printing](#8-Printing)
9. [SuperBasic Extensions](#9-SuperBasic-Extensions)

1 Introduction
==============

sQLux is a fork of the UQLX Sinclair QL emulator, the original
Introduction is below.

sQLux has been moved for X11 to SDL2 for its graphics output which means it now is functional on Windows and MacOS (although Windows is feature reduced). It has also been extensively developed and tested on 64bit OSes preparing for the near future when CPUs no longer have 32bit instruction set (ARM has already moved in this direction in some devices).

---

UQLX is an software emulator emulating a Sinclair QL on UNIX and similar systems. It works by emulating the 68000 CPU and some hardware. It can handle original JS roms, Minerva roms and many nationalized roms. The focus is more on providing useful extensions than exact emulation of antique hardware. Nevertheless it works well enough even for most games.

UQLX is designed to cooperate neatly with the underlying OS, some of the features and possibilities are:

- don’t waste CPU time when not needed by QDOS programs. Can save you some trouble with the sysadmin if you start it on a Cray accidentally
- launch and control of UNIX programs by the use of the `pty` device
- support for use of QDOS in UNIX scripts, screen IO redirection
- the emulator can fork (clone) itself
- access to every unix file - long filenames
- TCP/IP device for QDOS
- simple GUI with pasting into QL keyboard buffer and some more exotic features

---

[1.1 System Requirements](#11-system-requirements)
[1.2 COPYRIGHT](#12-copyright)

1.1 System Requirements
-----------------------

Nearly any UNIX or similar system will do, provided you have gcc and SDL2 installed.

---

1.2 COPYRIGHT
-------------

New parts of sQLux are license under the zlib license, files are marked where appropriate. (SPDX: Zlib)

Make sure you read the copyright notice carefully before you use, modify or distribute this product or parts of it.

The copyright notice can be found in the file ‘`COPYRIGHT`’ – if not try to email me at `rdzidlic@geocities.com` [(1)](#FOOT1).

---

2 Compiling
===========

Several machines/OS versions are directly supported.

So far the supported machines/OS are:

- Linux on x86, x86_64, arm, arm64
- Windows on x86. x86_64
- MacOS on x86_64, arm64

sQLux requires SDL2 library, so make sure you have installed the development package for that on your distro or OS.

To make it

```
cmake -B build/
cmake --build build/
```

Do not delete the directory where you compiled it unless you know what
you are doing.

The name of the executable will be ’sqlux’.

---

3 Installation
==============


```
cmake --install build/
```

This will install sqlux executable in your default prefix (normally /usr/local/bin).

---

4 Customization
===============

sQLux will look for a configuration file in the following patches in order.

 - $PWD/sqlux.ini (The current working directory)
 - $HOME/.sqluxrc
 - $HOME/.uqlxrc

It is possible to share configuration with uQLx but the ROMIN= format has changed.

---

4.1 About sqlux.ini files
=========================

The sqlux.ini options file uses a `KEY = VALUE` format. The ’`#`’ character can be used to start comments, rest of the line is not evaluated.

The keys available are:

`SYSROM`

The name of the QDOS ROM to boot.

```
SYSROM = js_rom
```

`ROMDIR`

The directory where ’SYSROM’ (and other ROMS) may be found.

```
ROMDIR = /ql/ROMS/
```

`RAMTOP`
The upper limit of memory; usual QDOS rules apply. The value is in kB. Be warned that large values for this will cause long startup delays unless FAST_START is enabled. I have largely tested sQLux with 4MB, but at least Minerva should handle 16MB. Currently sQLux won’t allow more than 16MB, but this could be easily changed if you need more. If a larger screen size is used it has to fit into this value.

```
RAMTOP = 4096
```

`SER1`
The Unix device used for QDOS ser1.

```
SER1 = /dev/ttyS0
```

`SER2`
The Unix device used for QDOS ser2.

```
SER2 = /dev/ttyS1
```

`SER3`
The Unix device used for QDOS ser3.

```
SER3 = /dev/ttyS1
```

`SER4`
The Unix device used for QDOS ser4.

```
SER4 = /dev/ttyS1
```

`PRINT`
The Unix command used to queue print jobs, it used to output data sent to the PRT device. popen() is used to send the data, so you may specify options, flags etc.

```
         PRINT = lpr -Pmy_printer
```

`CPU_HOG`
Define it 0 to make UQLX try to behave multitasking friendly, it will go sleeping when it believes that QDOS is idle. The detection whether QDOS is idle usually works pretty well, but in some cases it may get fooled by very frequent IO, eg an high speed serial connection - in this case define it to 1 to get all time UNIX will give us. Alternatively the `-h` option can be used to enforce CPU_HOG mode.

```
CPU_HOG = 1
```

`FAST_STARTUP`
Set to 1 if you want to skip the usual RAM test(default), or set it to 0 if you want to enjoy the Ram test pattern. Has no effect with the Minerva ROM

```
FAST_STARTUP = 1
```

`SKIP_BOOT`
Skip the F1/F2 screen on boot. 1 to skip, 0 to show screen.

```
SKIP_BOOT = 0
```

`ROMPORT`
`ROMIM`
The ROMPORT/ROMIM specifies the rom to be loaded at 0xC000.

```
ROMPORT = tk2_rom
```

It is assumed that the ROM image can be found in the ROMDIR
directory.

`IOROM1`
The IOROM1 specifies the rom to be loaded at 0x10000.

```
IOROM1 = tk2_rom
```

It is assumed that the ROM image can be found in the ROMDIR
directory.

`IOROM2`
The IOROM2 specifies the rom to be loaded at 0x14000.

```
IOROM2 = tk2_rom
```

It is assumed that the ROM image can be found in the ROMDIR
directory.

`NO_PATCH`
disables patching the ROM, will make a lot of features not work and mainly a debugging feature. 1 to disable patching, 0 to enable patching.

```
NO_PATCH = 0
```

`DEVICE`
All directory devices may be defined in the options file. The format is

```
DEVICE = QDOS_name,  UNIX_pathname[,  flags]
```

`QDOS_name` is the name of the QDOS volume to be defined, eg `FLP1` `WIN6`, `QXL1`. Currently `RAMx` is the only name that receives special attention. UQLX does not enforce any further naming conventions, however most QDOS software requires a 3 chars name length.

`UNIX_pathname` refers to a file, directory or device used to
simulate the QDOS device.

The optional `flags` field supports this options.

`clean`

clean together with a "%x" in the unix pathname can be used to simulate RAMdisk. The "%x" is replaced with the process number at runtime so that multitasking QMs don’t disturb each other and after killing QM the directory is deleted.

`qdos-fs`
`native`

Both flags are synonyms. The qdos-fs option indicates that
`UNIX_pathname` is the name of a file or device in the QDOS
floppy disk or QXL.WIN formats; otherwise a Unix directory is
assumed.

`qdos-like`

applicable only to non-`qdos-fs`. Filenames are not case sensitive and (sub)directory creation mimics SMSQ behaviour.
Dots in filenames/directory names are converted to underscores "\_" on load and directory listing (but the converse is
not true). Be careful not to mix files with the same names on qdos side ie test.c and test\_c in same directory.

`BDI1`
file that is exposed by the BDI interface, this is normally the file from an old style QL-SD device.

```
BDI1 = QL_BDI.bin
```

`BOOT_DEVICE`
changes the device the QDOS rom boots from, in rom the default is mdv1_. Set this to any valid device from above, but it is limited to 4 characters.

```
BOOT_DEVICE = FLP1
```

`WIN_SIZE`
1x, 2x, 3x, max, full, the zoom for the window. max is the maximum size window for the desktop, full is a fullscreen borderless window.

```
WIN_SIZE = max
```

`RESOLUTION`
The screen resolution for sQLux. The same as the -g option, normally used for creating the extended screen.

```
RESOLUTION = 512x512
```

`FILTER`
control the bilinear filtering in SDL when using zoomed screen, 1 enables filter, 0 disables filter. The can produce a smoother screen when zoom is not exact number of pixels, but also has a slight blur effect.

```
FILTER = 1
```

`FIXASPECT`
On BBQL the pixels are not square.   
1 enables sQLux to display a BBQL screen that will fill a 4:3 monitor at full screen.  
2 enables sQLux to display pixels at an aspect ratio matching that for a BBQL.   
0 Displays square pixels  
Defaults to 0

```
FIXASPECT = 1
```

`KBD`
Select the keyboard language. Valid options are `GB`, `DE` and `US`. Defaults to `US`.

```
KBD = DE
```

`SPEED`
Sets the execution speed of the emulator. Useful when running software that was written for an original QL. A value of 1 approximates to the speed of an original QL. Larger values map to multiples of the original QL speed. Specified as a floating point number, so small adjustments can be made if required. Defaults to 0.0 (maximum speed). When running at original speed a faster start-up is achieved by using the JS ROM and setting FAST_START to 1.

```
SPEED = 1.5
```

`SOUND`
Enables sound support. By default sound is enabled at minimum volume (1). A value of 0 disables sound. Values from 1 to 10 enables sound, and sets the output volume.

```
SOUND = 4
```

`JOY1`
The index into SDL2 for a joystick to emulate connection to the QL through the CTL1 port. An integer in the range 1 to 8. 1 maps to the first joystick detected by SDL2.

```
JOY1 = 1
```
`JOY2`
The index into SDL2 for a joystick to emulate connection to the QL through the CTL2 port. An integer in the range 1 to 8. 1 maps to the first joystick detected by SDL2.

```
JOY2 = 1
```
`PALETTE`
0 to use saturated (BRIGHT) colours, 1 to use unsaturated (MUTED) colours, 2 to use a grayscale display. Set to 0 by default (so the display is in saturated colours).

```
PALETTE = 1
```

`SHADER`  Selects shader support. 0 disables shaders. 1 enables a "flat" shader. 2 enables a shader including emulated barrel distortion. Disabled by default.  
Note that sQLux must have been compiled with shader support. See the [Shader documentation](docs/shaders.md) for more details.

```
SHADER = 2
```

`SHADER_FILE`
The path to the GPU shader (written in OpenGL Shading Language) that is loaded when `SHADER` is set to 1 or 2.

```
SHADER_FILE = ../shaders/crt-pi.glsl
```

and here is the example of an actual sqlux.ini file. You will find more recent versions of it with every sQLux distribution.

```
SYSROM = MIN198.rom
ROMIM = TK232.rom
RAMTOP = 4096
PRINT = lpr
CPU_HOG = 0
FAST_STARTUP = 1
DEVICE = MDV1,mdv1/,qdos-like
DEVICE = MDV2,mdv2/,qdos-like
DEVICE = WIN1,~
DEVICE = RAM1,/tmp/.ram1-%x,clean,qdos-like
DEVICE = RAM2,/tmp/.ram2-%x,clean,qdos-like
DEVICE = RAM8,/tmp/.ram8-%x,clean,qdos-like
WIN_SIZE = max
FILTER = 1
FIXASPECT = 1
SOUND = 2
SPEED = 0.8
KBD = GB
```

---

5 Program Invocation
====================

UQLX often prints diagnostic messages to `stdout` and `stderr`, so start it in its own terminal.

During startup, the emulator will attempt to boot from ‘`mdv1_BOOT`’ (Case sensitive, and if not overridden by BOOTDEV) or read commands from the `BOOT` device if one was specified with the ’`-b`’ option.

It appears that in Minerva 1.98 the name of the default `BOOT` file
changed from ‘`mdv1_BOOT`’ to ‘`mdv1_boot`’!! To keep compatibility with other ROMS I would recommend a soft link like

```
ln -s BOOT boot         # in mdv1_
```

---

[5.1 Command Line options](#51-Command-Line-options)
[5.2 BOOT Files](#52-BOOT-Files)
[5.3 ROM Images](#53-ROM-Images)
[5.4 The big screen feature](#54-the-big-screen-feature)
[5.5 Keyboard](#55-Keyboard)

5.1 Command Line options
------------------------

sqlux supports the following command line options; these override
settings in the config file.

```
sqlux [-r RAMTOP] [-f config_file] [-o romname]
      [-b boot_cmd] [-d boot_dev] [-g resolution]
      [-v verbosity_level] [-w windows_size] [-n]
      [-c configuration_line]
```

where:

`-r RAMTOP`

Defines the RAMTOP value in kB. Any enlarged screen also has to fit into this value.

`-c configuration_line`

Configuration line from in sqlux.ini format

```
-c "DEVICE = FLP1,/tmp/flp.img,qdos-native"
```

`-g resolution`

Start with screen size nXm, effective only with Minerva roms. **See big screen**

```
-g 512x512
```

`-f file`

Defines an alternative options file.

```
-f $TMP/sqlux.ini
```

`-h`

Force CPU_HOG mode, take all available CPU time for the emulator.

`-o romname`

Use romname instead of ROM defined in ‘`sqlux.ini`’ file

`-b boot_cmd`

Define commands to be executed when sQLux runs. This is a standard basic line without line number.

```
-b 'PRINT "HELLO WORLD"'
```

`-d boot_dev`

Define the QDOS device the rom will automatically load the BOOT file from. This is limited to 4 characters in ROM. The default in rom is MDV1.

```
-d FLP1
```

`-w window_size`

set the window size, 1x, 2x, 3x, max, full where max is a maximum sized window for the desktop and full is a borderless fullscreen window.

```
-w 2x
```

`-v verbosity_level`

set the verbosity level, so make sQLux quiet set to 0, level 1 is the default for level 2 with added messages.

```
-v 0
```

---

5.2 BOOT Files
--------------

Usually there are no special requirements for sQLux boot files, just remember to store it as ‘`mdv1_BOOT`’ (or other device if using BOOTDEV) - case will be significant in the default installation.

The below is from uQLx manual, but has not been seen in sQLux but may occur in same cases.

---

There seems to be a problem with some versions of `Ptr_gen`. If your boot file fails with random errors near the place where it is loaded, try inserting this after `LRESPR Ptr_gen`:

```
PAUSE#2,1
```

This will not work with JS ROMS, use a for loop or similar to cause the delay there..

You may query the UNIX environment variables and the startup parameters of the emulator from your bootfile - [SuperBasic
Extensions](#9-SuperBasic-Extensions)

---

5.3 ROM Images
--------------

Thanks to Tony Firshman, Minerva v1.89 onwards is now GPL and can be bundled with sQLux - it is ‘`roms/MIN198`’.

For various reasons you may run into some trouble when trying romimages other than the supplied MIN198, or Minerver1.89/JS. Roughly, the known causes are:

- rom image not supported by sQLux. Maybe some old, never tested QDOS roms
- rom image has been patched by GC, SGC or similar. All QL extensions that have something better than a 68008 CPU are likely to do this.
- some TK2 roms fail possibly because they get confused by the UQLX filesystem which is a mix of the basic and FS II. TK232.rom bundled seems to work ok.

5.4 The big screen feature
--------------------------

If you choose to work with a screen bigger than 512x256, there are a few important points.

**Warning: it is possible to define screens bigger than the physical screen - make sure that you know how to move around by your virtual screen manager in this case. Beware that UQLX captures all key events.**

The maximum useable screen size depends on available RAM.

- because of Pointer Environment’s bad habits that the screen must fit with all RAM into 16 MB

Pointer Environment is patched when activated to recognise the new screen parameters - there are ’cleaner’ solutions, unfortunately with severe side effects. If you receive the warning "could not patch PE", you are in serious trouble..

Screen geometry may be slightly adapted to result in clean x-resolution/sd.linel ratio. Length of screen buffer must always be truncated nearest 32K boundary, therefore some screen sizes may result in a certain waste of memory.

5.5 Keyboard
------------

By default sQLux uses scancodes in SDL2, this means the keymap is based on USA keyboard. This is very close to the QL layout and mainly the closest key in physical position is mapped to the equivalent QL key.

This scheme does not take into account international keyboard on the host of international versions of the rom in the emulator. See the `KBD` option to select a British or German keyboard

Alternatively get a USA keyboard or use stickers to give that genuine QL feel.

5.6 Sound
---------
sQLux can optionally generate sound. It supports the SuperBASIC BEEP command. Note that the QL manual states that the length parameter in a BEEP command is in units of 72 microseconds. This is incorrect. The original QL uses a unit length of 43.64 microseconds. sQLux also uses this unit of length.

sQLux attempts to faithfully reproduce the behaviour of the original QL. This includes support for negative values of grd_y, including the correct behaviour for -8. Behaviour when either pitch or pitch_2 equals 255 is also correctly emulated.

All optional parameters, including Fuzzy and Random, are fully supported. The slight click that occurs in the original QL when a pitch change occurs (even for a pitch change of 0) is emulated.

Many thanks to Silvestor from the QLForum for his detailed disassembly of the QL 8049 sound code.

5.6.1 Known Issues
------------------
1. In extreme cases (typically low pitch values, or extensive use of Random and Fuzzy) the timing unit of length can increase from 43.64 microseconds. This can impact the length of sounds and decrease the frequency of notes. This is currently not emulated.
2. The original QL initially generates a square sound wave. This is smoothed by the QL sound hardware. Other resonant frequencies are introduced by the QL case. sQLux only emulates the original square waveform.
3. The SuperBASIC BEEPING command should return true if the QL is making sound. However, the call to the IPC8049 to check if it is making sound is only made every 50/60 Hz. Therefore a call to BEEPING immediately after a BEEP command is issued may return false. This is more likely at faster emulation speeds, but can be easily reproduced on an original QL. Use of the PAUSE command can workaround this issue.

5.7 Joystick
------------
SDL2 is used to provide Joystick support. Analogue or digital joysticks are supported. Axis 0 and axis 1 of the joystick are mapped. Moving the joystick generates key presses. All buttons on the joystick are mapped to the fire key.
As documented by [Sinclair](http://www.dilwyn.me.uk/docs/ebooks/olqlug/QL%20Manual%20-%20Concepts.htm#joystick), Joystick 1 (assumed connected through CTL1) maps to the cursor keys and spacebar. Joystick 2 (assumed connected through CTL2) maps to F1 through to F5.

5.7.1 Known limitations
-----------------------
Currently only devices that are detected by SDL2 as Joysticks are supported. Devices that are *only* identified as GameControllers are not supported.

6 Filesystems
=============

Both QDOS/SMSQ diskimages and the host filesystem can be accessed, for configuration details see the ‘`sqlux.ini`’ file.

It is also possible to use real QDOS floppies, but some care must be taken. Especially disk swaps will only be recognised when all files are closed.

There is no file locking, so try not to use the same file in two processes at same time.

The Host Filesystem can be accessed both translated and untranslated The translated version is used in the default configuration to host ’mdv1_’ and some other devices.

The untranslated version is accessed as the uQVFSx Filesystem, see that section. The uQVFSx Filesystem is good if you want to access a Unix file of which you know the (Unix)filename or simply need very long pathnames.

It can be also used to access raw and special devices, eg the /proc filesystem.

---

[6.1 UNIX Filesystem Interface](#61-UNIX-Filesystem-Interface)
[6.2 QDOS floppy and QXL.WIN](#62-QDOS-floppy-and-QXL-WIN)

6.1 Host Filesystem Interface
-----------------------------

The Host FS Interface provides access to the underlying UNIX (or similar) host filesystem. Standard QDOS and most QDOS-FS II file operations are mapped to UNIX calls, full (sub-)directory access is provided.

This means that whatever filesystems are accessible from Unix (CD, MSDOS, Amiga partitions, ZIP drives ....) are accessible to QDOS programs.

The filenames are translated to a QDOS like syntax, ‘`/`’ maps to ‘`_`’.

Unfortunately this means, that in very rare situations a file may shadow some subdirectory. UNIX names are therefore supported as well, this looks like ‘`mdv1_c68/INCLUDE/stdio_h`’.[(2)](#FOOT2)

The `qdos-like` flag selects whether the filenames are case-sensitive and whether (sub)directory creation will have Unix or SMSQ semantics.

Using the `qdos-like` flag should be restricted to directories reserved for UQLX. If programs other than UQLX create files with names that are not distinguishable in case-insensitive mode the results will be undefined.

The default Unix FS is case-sensitive which can be a real pain with SuperBasic, but works quite good with most other software. Beware that QDOS will attempt to boot from ‘`mdv1_BOOT`’ !

Another lovely source of confusion is using SuperBasic symbols as filenames – SB always remembers the case of the symbol when seen first time and converts to this case subsequently.

Thus

```
open#6,mdv1_BoOt
open#4,mdv1_BOOT      REM still accessing mdv1_BoOt !!
```

Data-type, -space and file version are stored in an one per directory ‘`.-UQLX-`’ file. This means that UNIX hard and soft links for QDOS executables, as well as moving or copying them around by `mv` will fail.

The UNIX directories are visible to QDOS like normal FileSystem II directories. Since QDOS doesn’t use a distinct directory separator, this resulted in a rather complex algorithm for finding files, and in pathologic cases may result in certain files being shadowed.

The ‘`.`’ and ‘`..`’ directories are accessible just like that in QDOS but aren’t listed in directories anymore.

---

6.2 QDOS floppy and QXL.WIN
---------------------------

QXL.WIN files are now supported as well as direct use of floppy/QXL.WIN
devices. Currently, `disk swaps are only recognised when all files on that device are closed!`

UQLX can use ‘`DD`’ or ‘`HD`’ diskimages.[(3)](#FOOT3){#DOCF3} It should be noted that UQLX does not yet work well with unusual floppy formats, even when the files are listed correctly caution is recommended.

Floppy-Diskimages can be taken by ’dd’.

```{.example}
dd if=/dev/fd0 of=DiskImagename
```

On Linux anything works, unless you have a very special floppy use `/dev/fd0` as filename.

7 Other Devices
===============

Here is a description of the `TCP/IP`,`pty`,`ser` and `prt` devices.

---

[7.1 TCP/IP](#71-TCPIP)
[7.2 pty device](#72-pty-device)
[7.3 ser device](#73-ser-device)

---

7.1 TCP/IP
----------

The `TCP/IP` features are described in the files ‘`docs/socket.*`’ that came with this UQLX distribution or here:

7.2 pty device
--------------

`pty*j/t*_programname` *par1* ....

*j*

job control options

`i`

don’t care if child process exits. Default behaviour is to indicate EOF on read after the child process exited and all buffers were read, but theoretically someone might reconnect the tty.

`k`

kill child job after closing the QDOS channel. Default is don’t care.

*t*

translation options

`c`

translate carriage returns

`z`

translate char 26 (`^Z`)as end of file

`t`

translate QDOS <–> ISO-8859-1 font

*program name*

name of program to be executed and parameters

*parm*

arguments to be passed to the invoked program. This can be
unix-style options, filenames etc.

```
10 open#6,pty_csh
20 print#6,"ls -al"
30 print#6,"exit"       rem otherwise we can't detect the end!
40 repeat xx
    if eof(#6) :exit xx
50  input#6,a$:print a$
60 end repeat xx
70 close#6
```

Connect to an NNTP server and post a test message. Most likely you will have to use another NNTP server and change the "From:" to contain a legal address.


```
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

7.3 ser device
--------------

`ser*npht*_*b*baudrate`

except for the `_baudrate`, the options have the same meaning as in QDOS where applicable.

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

use handshake

`I`

ignore handshake

*t*

translation

`R`

no translation

`Z`

recognise `^Z` as EOF

`C`

carriage return

Here is some documentation for `ser` and `pty` devices, originally compiled by Jonathan Hudson

```
ser2hr_b57600   (57600 baud)
```

The data transfer rate appears CPU bound, but 5100 cps TX and 4800 cps RX are achievable for QTPI/ZMODEM.

The serial device names should be specified in the ‘`sqlux.ini`’ file, good choice for Linux is ‘`/dev/ttyS0`’ and ‘`/dev/ttyS1`’.

---

8 Printing
==========

The `prt` device can be used for printing. The data sent to `prt` is piped to the printer command specified in ‘`.~/.uqlxrc`’ which may be overridden or modified by providing additional arguments to the `prt` device.

`prt*ft*_*add_options*!*alt_command*`

*f*

ignore for QDOS compatibility

*t*

translation: use active TRA table

`add_options`

specify additional options to be passed to default printer
command

`alt_command`

specify alternative command to be executed

Data sent to the `prt` device is piped to the specified filter. If you
have QDOS printer drivers for the printer in use, try to send your
output to `lpr`. If this doesn’t work, try following definition in
‘`sqlux.ini`’:

```
PRINT = lpr -Praw
```

If your system doesn’t have a `-Praw` you can add it by editing ‘`/etc/printcap`’ or as a quick hack, just defining

```
PRINT = cat >/dev/lp0
```
For this to work you will probably need to change ‘`/dev/lp0`’ permissions. Obviously this should not be used together with a standard lpd..

*Some care must be used when specifying printer/filter commands: when closing the printer channel uqlx calls `pclose` which waits until the command(s) exits – in this situation uqlx may appear to hang.*

If this happens, kill the filter process from another xterm.

---

9 SuperBasic Extensions
=======================

**Kill_UQLX** *result*

Kill the emulator returning result to the calling program

**UQLX_RELEASE$**

Returns release identification as string

**getXenv$** *name*

Returns value of the (UNIX) environment variable name as string

**getXargc**

Returns the number of arguments that were given to the emulator at startup, options or arguments that have been consumed away by Xtk not counted.

```
qm -m -r 1024 arg1 arg2 arg3

PRINT getXargc          => 4   (arg0=qm !)
```

**getXarg$** *nth*

Returns the nth argument, continuing from above example

```
for i=0 to getXargc-1 : PRINT i, getXarg$(i)
```

results in


```
0       sqlux
1       arg1
2       arg2
3       arg3
4       arg4
5       arg5
```

**getXres**

Returns x-size of screen

**getYres**

Returns y-size of screen

**scr_xlim**

same as `getXres`

**scr_ylim**

same as `getYres`

**scr_base**

Returns the base address of the screen

**scr_llen**

Returns the number of bytes a line on the screen occupies

