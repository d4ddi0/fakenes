/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef AUDIO__APU_H__INCLUDED
#define AUDIO__APU_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#include "Core/CPU.h"
#include "Platform/File.h"
#ifdef __cplusplus
extern "C" {
#endif

enum {
   APU_EMULATION_FAST = 0,
   APU_EMULATION_ACCURATE,
   APU_EMULATION_HIGH_QUALITY
};

enum {
   APU_EXSOUND_MMC5,
   APU_EXSOUND_VRC6
};

enum {
   APU_VISDATA_SQUARE_1 = 0,
   APU_VISDATA_SQUARE_2,
   APU_VISDATA_TRIANGLE,
   APU_VISDATA_NOISE,
   APU_VISDATA_DMC,
   APU_VISDATA_MASTER_1,
   APU_VISDATA_MASTER_2,
   APU_VISDATA_ENTRIES
};

typedef struct apu_options_s {
   BOOL enabled;       /* Enable emulation */
   ENUM emulation;     /* Emulation mode */
   BOOL stereo;        /* Stereo output mode */
   BOOL swap_channels; /* Swap stereo channels */
   REAL volume;        /* Global volume */
   BOOL squelch;       /* Force silence. */

   /* Filters. */
   BOOL logarithmic;   /* Logarithmic mapping */
   BOOL agc;           /* Automatic gain control */
   BOOL normalize;     /* Normalize levels */

   /* Channels. */
   BOOL enable_square_1;
   BOOL enable_square_2;
   BOOL enable_triangle;
   BOOL enable_noise;
   BOOL enable_dmc;
   BOOL enable_extra_1;
   BOOL enable_extra_2;
   BOOL enable_extra_3;

} apu_options_t;

extern apu_options_t apu_options;

extern void apu_load_config(void);
extern void apu_save_config(void);
extern int apu_init(void);
extern void apu_exit(void);
extern void apu_reset(void);
extern void apu_reset_exsound(const ENUM exsound_id);
extern void apu_update(void);
extern void apu_clear_exsound(void);
extern void apu_enable_exsound(const ENUM exsound_id);
extern UINT8 apu_read(const UINT16 address);
extern void apu_write(const UINT16 address, UINT8 value);
extern cpu_time_t apu_execute(const cpu_time_t time);
extern void apu_predict_irqs(const cpu_time_t time);
extern void apu_sync_update(void);
extern void apu_load_state(FILE_CONTEXT* file, const int version);
extern void apu_save_state(FILE_CONTEXT* file, const int version);
extern REAL* apu_get_visdata(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !AUDIO__APU_H__INCLUDED */
