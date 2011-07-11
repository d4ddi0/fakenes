/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef SYSTEM__MAPPER_H__INCLUDED
#define SYSTEM__MAPPER_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#include "Core/CPU.h"
#include "Platform/File.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MMC
{
   int number;
   const char* name;
   int (*init)(void);
   void (*reset)(void);
   const char* id;
   void (*save_state)(FILE_CONTEXT*, const int);
   void (*load_state)(FILE_CONTEXT*, const int);
   void (*save_state_prg)(FILE_CONTEXT*, const int);
   void (*load_state_prg)(FILE_CONTEXT*, const int);
   void (*save_state_chr)(FILE_CONTEXT*, const int);
   void (*load_state_chr)(FILE_CONTEXT*, const int);

} MMC;

extern int mmc_init(void);
extern void mmc_reset(void);
extern void mmc_request(const int);
extern void mmc_force(const MMC*);
extern void (*mmc_scanline_start)(const int);
extern void (*mmc_hblank_start)(const int);
extern void (*mmc_hblank_prefetch_start)(const int);
extern BOOL (*mmc_virtual_scanline_start)(const int);
extern BOOL (*mmc_virtual_hblank_start)(const int);
extern BOOL (*mmc_virtual_hblank_prefetch_start)(const int);
extern void (*mmc_predict_asynchronous_irqs)(const cpu_time_t cycles);
extern void (*mmc_check_vram_banking)(void);
extern void (*mmc_check_address_lines)(const UINT16);
extern int mmc_get_name_table_count(void);
extern int mmc_uses_pattern_vram(void);
extern int mmc_controls_mirroring(void);
extern void mmc_save_state(FILE_CONTEXT*, const int);
extern void mmc_load_state(FILE_CONTEXT*, const int);
extern void mmc_save_state_prg(FILE_CONTEXT*, const int);
extern void mmc_load_state_prg(FILE_CONTEXT*, const int);
extern void mmc_save_state_chr(FILE_CONTEXT*, const int);
extern void mmc_load_state_chr(FILE_CONTEXT*, const int);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !SYSTEM__MAPPER_H__INCLUDED */
