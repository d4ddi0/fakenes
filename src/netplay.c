/* FakeNES - A free, portable, Open Source NES emulator.

   netplay.c: Implementation of the NetPlay engine.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include "common.h"
#include "net.h"
#include "netplay.h"
#include "types.h"

ENUM netplay_mode = NETPLAY_MODE_INACTIVE;

int netplay_init (void)
{
   return (0);
}

void netplay_exit (void)
{
   if (netplay_mode != NETPLAY_MODE_INACTIVE)
      netplay_close ();
}

BOOL netplay_open_server (int port)
{
   if (netplay_mode != NETPLAY_MODE_INACTIVE)
      return (FALSE);

   if (net_open (port) > 0)
      return (FALSE);

   netplay_mode = NETPLAY_MODE_SERVER_OPEN;

   return (TRUE);
}

BOOL netplay_open_client (const char *host, int port)
{
   if (netplay_mode != NETPLAY_MODE_INACTIVE)
      return (FALSE);

   if (net_open (0) > 0)
      return (FALSE);

   if (!net_connect (host, port))
   {
      net_close ();
      return (FALSE);
   }

   netplay_mode = NETPLAY_MODE_CLIENT;

   return (TRUE);
}

void netplay_close (void)
{
   if (netplay_mode == NETPLAY_MODE_INACTIVE)
      return;

   net_close ();

   netplay_mode = NETPLAY_MODE_INACTIVE;
}

void netplay_process (void)
{
   if (netplay_mode == NETPLAY_MODE_INACTIVE)
      return;

   switch (netplay_mode)
   {
      case NETPLAY_MODE_SERVER_OPEN:
      {
         net_listen ();

         break;
      }

      default:
         break;
   }

   net_process ();
}

void netplay_set_nickname (const UCHAR *nickname)
{
   /* This function sets the nickname for client #0, which is always the
      local "player". */

   NET_CLIENT *client = &net_clients[NET_LOCAL_CLIENT];

   USTRING_CLEAR(client->nickname);
   ustrncat (client->nickname, nickname, (USTRING_SIZE - 1));
}

void netplay_send_message (const UCHAR *message)
{
   /* This function sends a Unicode chat message over the network from
      client #0, prefixed with the nickname set by netplay_set_nickname().
      */

   NET_CLIENT *client = &net_clients[NET_LOCAL_CLIENT];
   USTRING text;

   /* Prefix message with nickname. */
   USTRING_CLEAR(text);
   uszprintf (text, sizeof (text), "<%s> %s", client->nickname, message);

   /* TODO: Send message over network. */
}
