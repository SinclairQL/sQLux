/*
	QLAY - Sinclair QL emulator
	Copyright Jan Venema 1998
	QL keys defines
*/

#define QL_A		0x1c
#define QL_B		0x2c
#define QL_C		0x2b
#define QL_D		0x1e
#define QL_E		0x0c
#define QL_F		0x24
#define QL_G		0x26
#define QL_H		0x1a
#define QL_I		0x12
#define QL_J		0x1f
#define QL_K		0x22
#define QL_L		0x18
#define QL_M		0x2e
#define QL_N		0x06
#define QL_O		0x17
#define QL_P		0x1d
#define QL_Q		0x0b
#define QL_R		0x14
#define QL_S		0x23
#define QL_T		0x0e
#define QL_U		0x0f
#define QL_V		0x04
#define QL_W		0x11
#define QL_X		0x03
#define QL_Y		0x16
#define QL_Z		0x29

#define QL_0		0x0d
#define QL_1		0x1b
#define QL_2		0x09
#define QL_3		0x19
#define QL_4		0x3e
#define QL_5		0x3a
#define QL_6		0x0a
#define QL_7		0x3f
#define QL_8		0x08
#define QL_9		0x10

#define QL_F1		0x39
#define QL_F2		0x3b
#define QL_F3		0x3c
#define QL_F4		0x38
#define QL_F5		0x3d

#define QL_UP		0x32
#define QL_DOWN		0x37
#define QL_LEFT		0x31
#define QL_RIGHT	0x34

#define QL_SPACE	0x36
#define QL_TAB		0x13
#define QL_ENTER	0x30
#define QL_ESCAPE	0x33
#define QL_CAPSLOCK	0x21
#define QL_LBRACKET	0x20
#define QL_RBRACKET	0x28
#define QL_SEMICOLON	0x27
#define QL_COMMA	0x07
#define QL_PERIOD	0x2a
#define QL_SLASH	0x05
#define QL_BACKSLASH	0x35
#define QL_QUOTE	0x2f
#define QL_POUND	0x2d
#define QL_MINUS	0x15
#define QL_EQUAL	0x25
#define QL_SS		0x45

#define QL_SHIFT	0x400
#define QL_CTRL		0x200
#define QL_ALT		0x100

#define QL_KP		0x80			/* keypad key */

/* shifted characters */
#define QLSH_POUND	QL_SHIFT|QL_POUND	/* backquote US */
#define QLSH_1		QL_SHIFT|QL_1
#define QLSH_2		QL_SHIFT|QL_2
#define QLSH_3		QL_SHIFT|QL_3
#define QLSH_4		QL_SHIFT|QL_4
#define QLSH_5		QL_SHIFT|QL_5
#define QLSH_6		QL_SHIFT|QL_6
#define QLSH_7		QL_SHIFT|QL_7
#define QLSH_8		QL_SHIFT|QL_8
#define QLSH_9		QL_SHIFT|QL_9
#define QLSH_0		QL_SHIFT|QL_0
#define QLSH_MINUS	QL_SHIFT|QL_MINUS
#define QLSH_EQUAL	QL_SHIFT|QL_EQUAL
#define QLSH_BACKSLASH	QL_SHIFT|QL_BACKSLASH
#define QLSH_LBRACKET	QL_SHIFT|QL_LBRACKET
#define QLSH_RBRACKET	QL_SHIFT|QL_RBRACKET
#define QLSH_SEMICOLON	QL_SHIFT|QL_SEMICOLON
#define QLSH_QUOTE	QL_SHIFT|QL_QUOTE
#define QLSH_COMMA	QL_SHIFT|QL_COMMA
#define QLSH_PERIOD	QL_SHIFT|QL_PERIOD
#define QLSH_SLASH	QL_SHIFT|QL_SLASH

/* special scancode translations */
#define QL_SC_56	0x40
