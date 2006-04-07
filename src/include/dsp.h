/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   dsp.h: Declarations for the digital sound processor.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef DSP_H_INCLUDED
#define DSP_H_INCLUDED
#include "apu.h"
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

#define DSP_MAX_CHANNELS   APU_CHANNELS

typedef REAL DSP_SAMPLE;

int dsp_init (void);
void dsp_exit (void);
int dsp_open (int, int);
void dsp_close (void);
void dsp_clear (void);
void dsp_write (const DSP_SAMPLE *);
void dsp_set_channel_enabled (int, ENUM, BOOL);
BOOL dsp_get_channel_enabled (ENUM);
void dsp_set_channel_params (int, REAL, REAL);
void dsp_set_effector_enabled (FLAGS, ENUM, BOOL);
BOOL dsp_get_effector_enabled (FLAGS);
void dsp_render (void *, int, int, BOOL);

enum
{
   DSP_SET_ENABLED_MODE_SET,
   DSP_SET_ENABLED_MODE_INVERT
};

enum
{
   DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_1 = (1 << 0),
   DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_2 = (1 << 1),
   DSP_EFFECTOR_LOW_PASS_FILTER_TYPE_3 = (1 << 2),
   DSP_EFFECTOR_HIGH_PASS_FILTER       = (1 << 3),
   DSP_EFFECTOR_DELTA_SIGMA_FILTER     = (1 << 4),
   DSP_EFFECTOR_WIDE_STEREO_TYPE_1     = (1 << 5),
   DSP_EFFECTOR_WIDE_STEREO_TYPE_2     = (1 << 6),
   DSP_EFFECTOR_WIDE_STEREO_TYPE_3     = (1 << 7),
   DSP_EFFECTOR_SWAP_CHANNELS          = (1 << 8),
   DSP_EFFECTOR_DITHER                 = (1 << 9)
};

/* Helper macros. */

#define DSP_ENABLE_CHANNEL_EX(channel, enable)  \
   dsp_set_channel_enabled (channel, DSP_SET_ENABLED_MODE_SET, enable)

#define DSP_ENABLE_EFFECTOR(effector)  \
   dsp_set_effector_enabled (effector, DSP_SET_ENABLED_MODE_SET, TRUE)
#define DSP_DISABLE_EFFECTOR(effector) \
   dsp_set_effector_enabled (effector, DSP_SET_ENABLED_MODE_SET, FALSE)
#define DSP_TOGGLE_EFFECTOR(effector)  \
   dsp_set_effector_enabled (effector, DSP_SET_ENABLED_MODE_INVERT, 0)

#ifdef __cplusplus
}
#endif
#endif   /* !DSP_H_INCLUDED */
