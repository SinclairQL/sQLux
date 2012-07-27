/* UQLX */

extern int sound_init(int, void *);
extern int sound_open(int, void**);
extern int sound_test(int, char*);
extern void sound_close(int, void *);
extern void sound_io(int, void *);

extern void qm_sound(void);

#define SNDBUFFSIZE  16384
struct sound_data{
  char *buf;
  char *tail;
  pid_t pid;
  int close;
  int reopen;
  int fd;
  int autoflush;
  int dticks;      // max time slices till autoflush
};

struct dsp_ctrl{
  uw32 freq;               // sample rate
  uw32 format;             // sample data format
  uw16 channels;           // 1/2 mono/stereo
  uw16 bits;               // bits per sample per channel
};

extern struct sound_data *delay_list;
