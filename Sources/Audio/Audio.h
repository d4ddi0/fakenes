/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef AUDIO__AUDIO_H__INCLUDED
#define AUDIO__AUDIO_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#include "Toolkit/Unicode.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Subsystems. */
enum {
   AUDIO_SUBSYSTEM_NONE = 0,
   AUDIO_SUBSYSTEM_AUTOMATIC,
   AUDIO_SUBSYSTEM_SAFE,
   AUDIO_SUBSYSTEM_ALLEGRO,
   AUDIO_SUBSYSTEM_OPENAL
};

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
extern int audio_open_wav(const UTF_STRING* filename);
extern void audio_close_wav(void);
extern void audio_visopen(unsigned num_frames);
extern void audio_visclose(void);
extern BOOL audio_is_visopen(void);
extern UINT16* audio_get_visdata(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !AUDIO__AUDIO_H__INCLUDED */
