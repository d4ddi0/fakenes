

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

netplay.h: Declarations for the NetPlay engine.

Copyright (c) 2001-2006, Randy McDowell.
Copyright (c) 2001-2006, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef NETPLAY_H_INCLUDED

#define NETPLAY_H_INCLUDED


#include "misc.h"


#define NETPLAY_PORT    0x2a03


int netplay_protocol;


enum
{
    NETPLAY_PROTOCOL_TCPIP,

    NETPLAY_PROTOCOL_SPX
};


int netplay_server_active;

int netplay_client_active;


int netplay_init (void);

void netplay_exit (void);


int netplay_open_server (void);

void netplay_close_server (void);


int netplay_poll_server (void);


int netplay_open_client (const UINT8 *);

void netplay_close_client (void);


#endif /* ! NETPLAY_H_INCLUDED */
