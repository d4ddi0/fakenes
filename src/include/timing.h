

#ifndef __TIMING_H__
#define __TIMING_H__

#ifdef __cplusplus
extern "C" {
#endif


int machine_type;


#define MACHINE_TYPE_NTSC   0

#define MACHINE_TYPE_PAL    1


int machine_init (void);

void machine_reset (void);


int frame_skip_min;

int frame_skip_max;


volatile int timing_fps;

volatile int timing_hertz;


void suspend_timing (void);

void resume_timing (void);


#define SCANLINE_CLOCKS         114


#define TOTAL_LINES_NTSC        262

#define TOTAL_LINES_PAL         312


#define FIRST_DISPLAYED_LINE    0

#define LAST_DISPLAYED_LINE     239


#define FIRST_VBLANK_LINE       241


#ifdef __cplusplus
}
#endif

#endif /* ! __TIMING_H__ */

