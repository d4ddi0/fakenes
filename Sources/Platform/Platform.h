/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef PLATFORM__PLATFORM_H__INCLUDED
#define PLATFORM__PLATFORM_H__INCLUDED
#include "Common/Global.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Generated by dat2c. */
#include "Data.h"

extern int platform_init(void);
extern void platform_exit(void);

#define DATA(id)           (datafile_data[DATAFILE_ ##id].dat)
#define DATA_INDEX(id)     DATAFILE_##id
#define DATA_TO_BITMAP(id) ((BITMAP *)DATA(id))
#define DATA_TO_FONT(id)   ((FONT *)DATA(id))
#define DATA_TO_RGB(id)    ((RGB *)DATA(id))
#define DATA_TO_SAMPLE(id) ((SAMPLE *)DATA(id))

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !PLATFORM__PLATFORM_H__INCLUDED */
