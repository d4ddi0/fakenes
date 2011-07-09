/* FakeNES - A free, portable, Open Source NES emulator.

   save.h: Declarations for the save data routines.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef SAVE_H_INCLUDED
#define SAVE_H_INCLUDED
#include <allegro.h>
#include "common.h"
#include "types.h"
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

UDATA *get_replay_title (int, UDATA *, int);
BOOL open_replay (int, const char *, const UDATA *);
void close_replay (void);
BOOL get_replay_data (UINT8 *);
void save_replay_data (UINT8);
UDATA *get_state_title (int, UDATA *, int);
BOOL save_state (int, const UDATA *);
BOOL load_state (int);
BOOL save_state_raw (PACKFILE *);
BOOL load_state_raw (PACKFILE *);
BOOL check_save_state (int);
BOOL load_patches (void);
BOOL save_patches (void);
BOOL load_sram (void);
BOOL save_sram (void);
UDATA *get_save_path (UDATA *, int);
UDATA *fix_save_title (UDATA *, int);

#ifdef __cplusplus
}
#endif
#endif   /* !SAVE_H_INCLUDED */
