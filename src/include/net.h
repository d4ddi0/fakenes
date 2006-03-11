
/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   net.h: Declarations for the network interface.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#ifndef NET_H_INCLUDED
#define NET_H_INCLUDED
#include "common.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

#define NET_MAX_CLIENTS 8

typedef struct _NET_CLIENT
{
   BOOL active;      /* Whether slot is occupied or not. */
   USTRING name;     /* Nickname, in Unicode. */
   void *data;       /* Internal data (socket, etc.). */

} NET_CLIENT;

NET_CLIENT net_clients[NET_MAX_CLIENTS];

ENUM net_mode;

int net_init (void);
void net_exit (void);
int net_open (int);
void net_close (void);
int net_listen (void);
int net_connect (const char *, int);
void net_disconnect (void);
void net_die (void);

enum
{
   NET_MODE_INACTIVE,
   NET_MODE_SERVER,
   NET_MODE_CLIENT
};

#define NET_LOCAL_CLIENT         0
#define NET_FIRST_REMOTE_CLIENT  (NET_LOCAL_CLIENT + 1)

#ifdef __cplusplus
}
#endif
#endif   /* !NET_H_INCLUDED */
