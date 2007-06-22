/* FakeNES - A free, portable, Open Source NES emulator.

   audio.h: Declarations for the audio interface.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED
#include "apu.h"
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct audio_options_s {
   BOOL enable_output;
   ENUM subsystem;
   int sample_rate_hint;
   int buffer_length_ms_hint;

} audio_options_t;

extern audio_options_t audio_options;

extern volatile int audio_fps;

extern void audio_load_config(void);
extern void audio_save_config(void);
extern int audio_init(void);
extern void audio_exit(void);
extern void audio_update(void);
extern void audio_suspend(void);
extern void audio_resume(void);
extern int audio_open_wav(const UCHAR* filename);
extern void audio_close_wav(void);
extern void audio_visopen(unsigned num_frames);
extern void audio_visclose(void);
extern UINT16* audio_get_visdata(void);

/* Subsystems. */
enum {
   AUDIO_SUBSYSTEM_NONE,
   AUDIO_SUBSYSTEM_ALLEGRO,
   AUDIO_SUBSYSTEM_OPENAL,
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !AUDIO_H_INCLUDED */
