/* sQLux Sound */
#ifndef _QL_sound_
#define _QL_sound_

#include <stdbool.h>

extern volatile bool soundOn;

void BeepSound(unsigned char*);
void KillSound(void);

#ifdef SOUND
void initSound(int volume);
void closeSound();
#endif
#endif
