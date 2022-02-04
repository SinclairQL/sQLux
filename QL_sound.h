/* sQLux Sound */
#ifndef _QL_sound_
#define _QL_sound_

#include <stdbool.h>

extern volatile bool soundOn;
extern bool sound_enabled;

void BeepSound(unsigned char*);
void KillSound(void);

#ifdef SOUND
bool initSound(int volume);
void closeSound();
#endif
#endif
