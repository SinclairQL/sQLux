/* sQLux Sound */
#ifndef _QL_sound_
#define _QL_sound_

#include <stdbool.h>

extern volatile bool soundOn;

void initSound(int volume);
void BeepSound(unsigned char*);
void KillSound(void);

#ifdef SOUND
void closeSound();
#endif
#endif
