/* Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)

   This program is free software; you can redistribute it and/or
   modify it under the terms of version 2 of the GNU Library General 
   Public License as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
   Library General Public License for more details.  To obtain a 
   copy of the GNU Library General Public License, write to the Free 
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Any permitted reproduction of these routines, in whole or in part,
   must bear this legend.

   Heavily modified for FakeNES by randilyn.
   Portions (c) 2001-2006 FakeNES Team. */

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
extern void apu_update(void);
extern void apu_start_frame(void);
extern void apu_end_frame(void);
extern void apu_set_exsound(ENUM type);
extern UINT8 apu_read(UINT16 address);
extern void apu_write(UINT16 address, UINT8 value);
extern void apu_predict_irqs(cpu_time_t cycles);
extern void apu_save_state(PACKFILE* file, int version);
extern void apu_load_state(PACKFILE* file, int version);

typedef struct apu_options_s {
   BOOL enabled;
   ENUM emulation;
   BOOL stereo;
   BOOL normalize;

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
   APU_EXSOUND_NONE = 0,
   APU_EXSOUND_MMC5,
   APU_EXSOUND_VRC6
};

#ifdef __cplusplus
}
#endif   /* __cplusplus */
#endif   /* !APU_H_INCLUDED */

