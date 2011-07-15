/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef PLATFORM__SAVE_H__INCLUDED
#define PLATFORM__SAVE_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#include "Platform/File.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Max in-file title length for save states and replays. */
/* This only applies to FNSS files of version 1.02 or lower. */
#define SAVE_TITLE_SIZE       16

/* Newer versions include a variable-length title field. */
/* Because the size field is 8-bit, it has an upper limit of 255 bytes. */
#define NEW_SAVE_TITLE_SIZE   255
#define NEW_SAVE_TITLE_SIZE_Z (NEW_SAVE_TITLE_SIZE + 1)

typedef void (*LOAD_STATE_HANDLER)(FILE_CONTEXT *file, const int version);
typedef void (*SAVE_STATE_HANDLER)(FILE_CONTEXT *file, const int version);

extern UDATA* get_replay_title(int, UDATA*, int);
extern BOOL open_replay(int, const char*, const UDATA*);
extern void close_replay(void);
extern BOOL get_replay_data(UINT8*);
extern void save_replay_data(UINT8);
extern UDATA* get_state_title(int, UDATA*, int);
extern BOOL save_state(int, const UDATA*);
extern BOOL load_state(int);
extern BOOL save_state_raw(FILE_CONTEXT*);
extern BOOL load_state_raw(FILE_CONTEXT*);
extern BOOL check_save_state(int);
extern BOOL load_patches(void);
extern BOOL save_patches(void);
extern BOOL load_sram(void);
extern BOOL save_sram(void);
extern UDATA* get_save_path(UDATA*, int);
extern UDATA* fix_save_title(UDATA*, int);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !PLATFORM__SAVE_H__INCLUDED */
