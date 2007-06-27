/* FakeNES - A free, portable, Open Source NES emulator.

   apu.h: External declarations for the APU emulation.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef APU_H_INCLUDED
#define APU_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "core.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
extern void apu_load_config(void);
extern void apu_save_config(void);
extern int apu_init(void);
extern void apu_exit(void);
extern void apu_reset(void);
extern void apu_reset_exsound(ENUM exsound_id);
extern void apu_update(void);
extern void apu_clear_exsound(void);
extern void apu_enable_exsound(ENUM exsound_id);
extern UINT8 apu_read(UINT16 address);
extern void apu_write(UINT16 address, UINT8 value);
extern void apu_predict_irqs(cpu_time_t cycles);
extern void apu_save_state(PACKFILE* file, int version);
extern void apu_load_state(PACKFILE* file, int version);
extern void apu_sync_update(void);
extern REAL* apu_get_visdata(void);

typedef struct apu_options_s {
   BOOL enabled;       /* Enable emulation */
   ENUM emulation;     /* Emulation mode */
   BOOL stereo;        /* Stereo output mode */
   BOOL swap_channels; /* Swap stereo channels */
   REAL volume;        /* Global volume */

   /* Filters. */
   BOOL normalize;     /* Normalize output */

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

enum {
   APU_EMULATION_FAST = 0,
   APU_EMULATION_ACCURATE,
   APU_EMULATION_HIGH_QUALITY,
};

enum {
   APU_EXSOUND_MMC5,
   APU_EXSOUND_VRC6,
};

enum {
   APU_VISDATA_SQUARE_1 = 0,
   APU_VISDATA_SQUARE_2,
   APU_VISDATA_TRIANGLE,
   APU_VISDATA_NOISE,
   APU_VISDATA_DMC,
   APU_VISDATA_MASTER_1,
   APU_VISDATA_MASTER_2,
   APU_VISDATA_ENTRIES,
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !APU_H_INCLUDED */

