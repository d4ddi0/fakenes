/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef SYSTEM__NSF_H__INCLUDED
#define SYSTEM__NSF_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#include "Core/CPU.h"
#include "System/Mapper.h"
#ifdef __cplusplus
extern "C" {
#endif

extern BOOL nsf_open(const UDATA* filename);
extern void nsf_close(void);
extern void nsf_setup(void);
extern void nsf_teardown(void);
extern void nsf_start_frame(void);
extern void nsf_end_frame(void);
extern void nsf_execute(const cpu_time_t cycles);
extern void nsf_update_timing(void);

extern const MMC nsf_mapper;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !SYSTEM__NSF_H__INCLUDED */

