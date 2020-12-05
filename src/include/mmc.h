/* FakeNES - A free, portable, Open Source NES emulator.

   mmc.h: Declarations for the MMC emulations.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef MMC_H_INCLUDED
#define MMC_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "cpu.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MMC
{
   int number;
   const char *name;
   int (*init) (void);
   void (*reset) (void);
   const char *id;
   void (*save_state) (PACKFILE *, const int);
   void (*load_state) (PACKFILE *, const int);
   void (*save_state_prg) (PACKFILE *, const int);
   void (*load_state_prg) (PACKFILE *, const int);
   void (*save_state_chr) (PACKFILE *, const int);
   void (*load_state_chr) (PACKFILE *, const int);

} MMC;

extern int mmc_init (void);
extern void mmc_reset (void);
extern void mmc_request (const int);
extern void mmc_force (const MMC *);
extern int (*mmc_hblank_start) (const int);
extern int (*mmc_scanline_start) (const int);
extern int (*mmc_scanline_end) (const int);
extern void (*mmc_predict_irqs) (const cpu_time_t cycles);
extern void (*mmc_check_latches) (const UINT16);
extern int mmc_get_name_table_count (void);
extern int mmc_uses_pattern_vram (void);
extern void mmc_save_state (PACKFILE *, const int);
extern void mmc_load_state (PACKFILE *, const int);
extern void mmc_save_state_prg (PACKFILE *, const int);
extern void mmc_load_state_prg (PACKFILE *, const int);
extern void mmc_save_state_chr (PACKFILE *, const int);
extern void mmc_load_state_chr (PACKFILE *, const int);

#ifdef __cplusplus
}
#endif   /* __cplusplus */
#endif   /* !MMC_H_INCLUDED */
