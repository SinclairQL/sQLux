/*
 * (c) UQLX - see COPYRIGHT
 */

/* someday theese might get implemented */
/* - at the moment I enjoy the silence  */

extern volatile Cond soundOn;

short AllocateSound(void);
void DisposeSound(void);
void BeepSound(unsigned char *);
void KillSound(void);
