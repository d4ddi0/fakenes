/* FakeNES - A free, portable, Open Source NES emulator.

   netplay.h: Declarations for the NetPlay engine.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef NETPLAY_H_INCLUDED
#define NETPLAY_H_INCLUDED
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

#define NETPLAY_DEFAULT_PORT  0x2a03

ENUM netplay_mode;

int netplay_init (void);
void netplay_exit (void);
BOOL netplay_open_server (int);
BOOL netplay_open_client (const char *, int);
void netplay_close (void);
void netplay_process (void);
void netplay_set_nickname (const UCHAR *);
void netplay_send_message (const UCHAR *);

enum
{
   NETPLAY_MODE_INACTIVE,
   NETPLAY_MODE_SERVER_OPEN,
   NETPLAY_MODE_SERVER_CLOSED,
   NETPLAY_MODE_CLIENT
};

enum
{
   NETPLAY_PACKET_CHAT
};

#ifdef __cplusplus
}
#endif
#endif   /* !NETPLAY_H_INCLUDED */
