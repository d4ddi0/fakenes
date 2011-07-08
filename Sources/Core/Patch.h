/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef CORE__PATCH_H__INCLUDED
#define CORE__PATCH_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PATCHES 64

typedef struct _MEMORY_PATCH {
   USTRING title;
   UINT16 address;
   UINT8 value, match_value;
   BOOL enabled;

} MEMORY_PATCH;

extern BOOL add_patch(const MEMORY_PATCH* patch);
extern void remove_patch(const MEMORY_PATCH* patch);
extern MEMORY_PATCH* get_patch(const int index);
extern void map_patches(const UINT16 start_address, const UINT16 end_address);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !CORE__PATCH_H__INCLUDED */
