/* Sound updated for SDL2 */
#include <SDL.h>
#include <stdio.h>
#include "debug.h"
#include "QL68000.h"
#include "QL_sound.h"

#ifdef SOUND
/*
 * Structures only used in this file
 */
typedef struct {		// Beep parameters written to ipc8049
    unsigned int length;
    unsigned int pitch;
    unsigned int pitch_2;
    unsigned int grd_x;
    int grd_y;
    unsigned int wrap;
    unsigned int random;
    unsigned int fuzz;
} ipc_sound;

typedef struct {
	SDL_mutex* mutex; 	// Mutex protecting writing to the structure
	int in_use;		// Index to pic_sound buffer being played
	int last_written;	// Latest buffer to play
	ipc_sound beep[3];	// The buffers
} sound_data;

typedef struct {
    unsigned int current_pitch;	// currently playing pitch - may include random
    int random;			// Random number to be added to pitch
    int fuzz;			// Random adjustment in wavelength
    int left;			// Total samples left to write
    int pitch_left;		// Sample left for current pitch
    int half_cycle;		// sample per half period
    int wave_state;		// High or low (with silence in between)
    int cycle_point;		// Current sample position in wave cycle
    int w_count;		// wrap count
    int direction;		// Direction of pitch travel
} current_sound;

/*
 * Global variables
 */
bool sound_enabled = false;	// True if sound enabled successfully

/*
 * Local variables
 */
static int audio_volume; 	// audio volume, 0 to 127

static SDL_AudioDeviceID QLSDLAudio;
static SDL_AudioSpec want;
static SDL_AudioSpec have;

static sound_data sound;
static current_sound c_sound;

/*
 * Local functions
 */
void audioCallback(void* userdata, Uint8* stream, int len);

static void setVolume(int volume);
static void setPitchDuration();
static void getNewPitch();
static void randomAdjust();
static void fuzzAdjust();

static int pitchToHalfSampleCount(int pitch);
static void populateBuffer(int start, int samples, Sint8* buffer, int len);
static void silenceBuffer(int start, Sint8* buffer, int len);

/*
 * Local definitions
 */
#define UNUSED(x) (void)(x)
#define TICK_8049 22917		// Number of IPC ticks per second

#define FREQUENCY 24000		// Requested sampling frequency
#define SAMPLES 256		// Number of samples in a callback


bool initSound(int volume) {
	// Create the sound driver
	if(SDL_Init(SDL_INIT_AUDIO)) {
		if (V1) {
			printf("Audio Failed to initialize: %s\n", SDL_GetError());
		}
		sound_enabled = false;
		return sound_enabled;
	}

	SDL_zero(want);
	want.freq = FREQUENCY;
	want.format = AUDIO_S8;
	want.channels = 1;
	want.samples = SAMPLES;
	want.callback = audioCallback;

	QLSDLAudio = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

	if(!QLSDLAudio) {
		if (V1) {
			printf("Failed to open audio device: %s\n", SDL_GetError());
		}
		sound_enabled = false;
		return sound_enabled;
	}

	sound.mutex = SDL_CreateMutex();

	if (sound.mutex) {
		sound_enabled = true;
		sound.in_use = -1; 		// index in use by callback (-1 = none)
		sound.last_written = -1;	// index last written by by BeepSound (-1 = None)
	}
	setVolume(volume);

	return sound_enabled;
}

void closeSound() {
	sound_enabled = false;

	if (sound.mutex)
		SDL_DestroyMutex(sound.mutex);

	if (QLSDLAudio) {
		SDL_CloseAudioDevice(QLSDLAudio);
	}
}

static void setVolume(int volume) {
	volume = abs(volume);

	if (volume >10)
		volume = 10;

	audio_volume = 12 * volume;
}

void BeepSound(unsigned char *arg) {
	int mask = (arg[5] + (int)(arg[4] << 8) +
		   (int)(arg[3] << 16) + (int)(arg[2] << 24));

	/*
	int bit;

	for (int j=0; j<16; ++j) {
		bit = 2 * j + 1;
		(mask & (0x1 << bit)) ? printf("1") : printf("0");
		--bit;
		(mask & (0x1 << bit)) ? printf("1") : printf("0");

		printf(" %i %x\n", j+6, arg[j+6]);
	}
	printf("Mask: %0x\n",mask);
	*/

	// Find correct param structure
	int write_num;

	soundOn = true; // Sound will soon be on!

	SDL_LockMutex(sound.mutex);
	for (write_num=0; write_num<3; ++write_num) {
		if ((write_num != sound.in_use) && (write_num != sound.last_written))
			break;
	}

	sound.beep[write_num].pitch = (((mask & 0x1) ? arg[7] : arg[6]));
	sound.beep[write_num].pitch_2 = (((mask & 0x10) ? arg[9] : arg[8]));

	sound.beep[write_num].grd_x = arg[10] | ((arg[11] & 0x7f) << 8);
	sound.beep[write_num].length = arg[12] | ((arg[13] & 0x7f) << 8);

	sound.beep[write_num].grd_y = ((mask & 0x00010000) ? arg[15] : arg[14]) & 0xf;
	if (sound.beep[write_num].grd_y > 7)
		sound.beep[write_num].grd_y -= 0x10;

	sound.beep[write_num].wrap = ((mask & 0x00100000) ? arg[17] : arg[16]) & 0xf;
	sound.beep[write_num].random = ((mask & 0x01000000) ? arg[19] : arg[18]) & 0xf;
	sound.beep[write_num].fuzz = ((mask & 0x10000000) ? arg[21] : arg[20]) & 0xf;

	// Calculate data and write
	sound.last_written = write_num;
	SDL_UnlockMutex(sound.mutex);

	/*
	printf("length %u pitch %u pitch2 %u grd_x %u grd_y %u wrap %u fuzz %u random %u\n",
	sound.beep[write_num].length, sound.beep[write_num].pitch, sound.beep[write_num].pitch_2,
	sound.beep[write_num].grd_x, sound.beep[write_num].grd_y, sound.beep[write_num].wrap,
	sound.beep[write_num].fuzz, sound.beep[write_num].random);
	*/

	// Always unpause the sound here, in case the callback has paused itself
	if (sound.beep[write_num].length > 0)
		SDL_PauseAudioDevice(QLSDLAudio, 0);
}

void KillSound(){
	SDL_LockMutex(sound.mutex);
	sound.in_use = -1;
	sound.last_written = -1;
	SDL_UnlockMutex(sound.mutex);
}

void audioCallback(void* userdata, Uint8* stream, int len) {
	UNUSED(userdata);

	int written = 0;		// Total samples written this callback
	int to_write = 0;		// Samples to write in next iteration

	bool new_found = false;
	SDL_LockMutex(sound.mutex);

	if (c_sound.left < 0) {
		// Used all of the buffer, so no longer in use
		sound.in_use = -1;
	}

	if ((sound.last_written >=0) && (sound.last_written != sound.in_use)) {
		sound.in_use = sound.last_written;
		sound.last_written = -1;	// Have taken latest buffer
		new_found = true;
	}

	SDL_UnlockMutex(sound.mutex);

	if (new_found) {
		c_sound.current_pitch = (sound.beep[sound.in_use].grd_y < 0)
					? sound.beep[sound.in_use].pitch_2
					: sound.beep[sound.in_use].pitch;
		c_sound.random = 0;  	// No randomness on first pitch
		c_sound.fuzz = 0;
		c_sound.half_cycle = pitchToHalfSampleCount(c_sound.current_pitch);
		c_sound.left = (sound.beep[sound.in_use].length * have.freq) / TICK_8049;

		setPitchDuration();
		c_sound.cycle_point = 0;
		c_sound.wave_state = 0;
		c_sound.direction = 1;
		c_sound.w_count = sound.beep[sound.in_use].wrap;
		fuzzAdjust();
	}

	if ((c_sound.left < 0) || (sound.in_use == -1)){
		SDL_PauseAudioDevice(QLSDLAudio, 1);
		soundOn = false;
	}
	else {
		do {
			if (c_sound.pitch_left == 0) {
				// Play one note forever
				to_write = len - written;
			}
			else if (c_sound.pitch_left > (len - written)) {
				// Can fill buffer with current note
				to_write = len - written;
				c_sound.pitch_left -= to_write;
				if (c_sound.left) {
					c_sound.left -= to_write;
				}
			}
			else {
				// Need to change note or stop playing
				to_write = c_sound.pitch_left;
				if (c_sound.left) {
					if (c_sound.left > c_sound.pitch_left) {
						c_sound.left -= to_write;
				    	}
					else {
						c_sound.left = -1;
					}
				}
				c_sound.pitch_left = -1;
			}

			populateBuffer(written, to_write, (Sint8*)stream, len);
			written += to_write;

			if (written < len) {
				// Reached the end of the pitch
				// New pitch, or silence?
				if (c_sound.left >= 0) {
					getNewPitch();
					setPitchDuration();
				}
				else {
					silenceBuffer(written, (Sint8*)stream, len);
					written = len;
				}
			}
		}
		while (written < len);
	}
}

static void getNewPitch() {
	int change = sound.beep[sound.in_use].grd_y;
	unsigned int try_pitch = c_sound.current_pitch;

	if (change) {
		if (change == -8)
		{
			// we cycle through the whole set of pitches
			// this is confirmed on a BBQL
			c_sound.current_pitch = (c_sound.current_pitch + 248) % 0x100;
		}
		else {
			// Now have -7 to +7
			try_pitch += change * c_sound.direction;
			try_pitch &= 0xff;

			if ((try_pitch > sound.beep[sound.in_use].pitch) &&
				(try_pitch < sound.beep[sound.in_use].pitch_2)) {
				// Pitch in range
				c_sound.current_pitch = try_pitch;
			}
			else {
				// Reached the end of the sequence - are we wrapping?
				if (c_sound.w_count > 0) {
					// We are not so go back to the original pitch
					c_sound.current_pitch = ((change * c_sound.direction) < 0)
					? sound.beep[sound.in_use].pitch_2
					: sound.beep[sound.in_use].pitch;

					if (c_sound.w_count != 15) {
						--c_sound.w_count;
					}
				}
				else {
					// Use the pitch we have calculated
					c_sound.current_pitch = try_pitch;
					c_sound.w_count = sound.beep[sound.in_use].wrap;
					c_sound.direction *= -1;
				}
			}
		}
	}
	randomAdjust();
	c_sound.half_cycle = pitchToHalfSampleCount(c_sound.current_pitch +
							c_sound.random +
							c_sound.fuzz);

	// Emulate the BBQL "click"
	c_sound.cycle_point =
		((c_sound.cycle_point + 1) < c_sound.half_cycle)
		? c_sound.cycle_point + 1 : c_sound.cycle_point - 1;
}

/*
 * Fuzz adjust is called whenever the wave_state changes
 */
static void fuzzAdjust() {

	if (sound.beep[sound.in_use].fuzz > 7) {
		int val = (sound.beep[sound.in_use].fuzz - 7);
		c_sound.fuzz = rand() % (0x1 << val);
		c_sound.half_cycle =
		 pitchToHalfSampleCount(c_sound.current_pitch +
					c_sound.random +
					c_sound.fuzz);
	}
	else {
		c_sound.fuzz = 0;
	}
}

/*
 * Random adjust is called whenever a pich change is evaluated
 */
static void randomAdjust()
{
	if (sound.beep[sound.in_use].random > 7) {
		int val = (sound.beep[sound.in_use].random - 7);
		c_sound.random = rand() % (0x1 << val);

	}
	else {
		c_sound.random = 0;
	}
}

static void setPitchDuration()
{
	c_sound.pitch_left = (sound.beep[sound.in_use].grd_x * have.freq) / TICK_8049;

	// Bound pitch_left, if it is bigger than left
	if (c_sound.left) {
		if (c_sound.pitch_left) {
			c_sound.pitch_left = (c_sound.pitch_left > c_sound.left)
						? c_sound.left : c_sound.pitch_left;
		}
		else {
			c_sound.pitch_left = c_sound.left;
		}
	}
}

/*
 * The number of samples in one *half cycle* for a given QL pich value
 */
static int pitchToHalfSampleCount(int pitch)
{
	// Correct for off by 1 in ROM
	pitch = (pitch + 255) % 256;

	float b = pitch + 10.6;

	return (int)((have.freq * b / TICK_8049) + 0.5f);
}

static void populateBuffer(int start, int samples, Sint8* buffer, int len)
{
	if (!c_sound.wave_state){
		c_sound.wave_state = -1;
		fuzzAdjust();
		c_sound.cycle_point = 0;
	}
	int buffer_pos = start;

	while (buffer_pos < (samples + start)) {
		buffer[buffer_pos++] = audio_volume * c_sound.wave_state;
		++c_sound.cycle_point;

		if (c_sound.cycle_point >= (c_sound.half_cycle)) {
			c_sound.wave_state *= -1;
			fuzzAdjust();
			c_sound.cycle_point = 0;
		}
	}
}

static void silenceBuffer(int start, Sint8* buffer, int len)
{
	int buffer_pos = start;
	while (buffer_pos < len) {
		buffer[buffer_pos++] = 0;
	}
	c_sound.wave_state = 0;
	c_sound.cycle_point = 0;
}
#else
void BeepSound(unsigned char *arg) {}
void KillSound(){}
#endif
