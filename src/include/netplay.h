

/*

FakeNES - A portable, Open Source NES emulator.

netplay.h: Declarations for the netplay engine.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#ifndef __NETPLAY_H__
#define __NETPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif


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


#ifdef __cplusplus
}
#endif

#endif /* ! __NETPLAY_H__ */
