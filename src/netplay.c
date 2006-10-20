/* FakeNES - A free, portable, Open Source NES emulator.

   netplay.c: Implementation of the NetPlay engine.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include "common.h"
#include "net.h"
#include "netplay.h"
#include "shared/bufferfile.h"
#include "timing.h"
#include "types.h"
#include "video.h"

ENUM netplay_mode = NETPLAY_MODE_INACTIVE;

static void parse_packet (PACKFILE *file);

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

BOOL netplay_open_client (const CHAR *host, int port)
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
   int index;

   if (netplay_mode == NETPLAY_MODE_INACTIVE)
      return;

   /* Sync state. */
   net_process ();

   switch (netplay_mode)
   {
      case NETPLAY_MODE_SERVER_OPEN:
      {
         /* Listen for incoming connections. */
         net_listen ();

         break;
      }

      case NETPLAY_MODE_SERVER_CLOSED:
      case NETPLAY_MODE_CLIENT:
      {
         static int wait_frames = 0;
         UINT8 unused;

         if (wait_frames > 0)
            wait_frames--;
         if (wait_frames > 0)
            break;

         /* Send empty packets so that the connection stays alive. */
         net_send_packet (NETPLAY_PACKET_NULL, &unused, 0);

         /* 5 times per second should be plenty... maybe TOO much. */
         wait_frames = ROUND(timing_get_speed () / 5.0);

         break;
      }

      default:
         break;
   }

   /* Check for incoming packets. */
   for (index = 0; index < NET_MAX_CLIENTS; index++)
   {
      /* Recieve buffer. */
      UINT8 buffer[NET_MAX_PACKET_SIZE_RECIEVE];
      unsigned size;

      /* Grab next packet. */
      size = net_get_packet (index, buffer, sizeof(buffer));
      while (size > 0)
      {
         PACKFILE *file;

         /* Open buffer file. */
         file = BufferFile_open ();
         if (!file)
         {
            WARN_GENERIC();
            continue;
         }

         /* Copy packet to buffer file. */
         pack_fwrite (buffer, size, file);

         /* Parse it. */
         parse_packet (file);

         /* Close buffer file */
         pack_fclose (file);

         /* Grab next packet. */
         size = net_get_packet (index, buffer, sizeof(buffer));
      }
   }
}

void netplay_set_nickname (const UCHAR *nickname)
{
   /* This function sets the nickname for client #0, which is always the
      local "player". */

   NET_CLIENT *client = &net_clients[NET_LOCAL_CLIENT];

   RT_ASSERT(nickname);

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
   PACKFILE *file;
   UINT8 *buffer;
   long size;

   RT_ASSERT(message);

   /* Prefix message with nickname. */
   USTRING_CLEAR(text);
   uszprintf (text, (sizeof (text) - 1), "<%s> %s", client->nickname, message);

   /* Build packet. */
   file = BufferFile_open ();
   if (!file)
   {
      WARN_GENERIC();
      return;
   }

   pack_fwrite (text, ustrsize (text), file);

   /* Send packet. */
   BufferFile_get_buffer (file, &buffer, &size);
   net_send_packet (NETPLAY_PACKET_CHAT, buffer, size);

   pack_fclose (file);
}

/* ---- Private functions ---- */

static void parse_packet (PACKFILE *file)
{
   NET_PACKET_HEADER header;

   RT_ASSERT(file);

   /* Read each field individually for portability... */
   header.tag = pack_getc (file);
   header.size = pack_mgetw (file);

   switch (header.tag)
   {
      case NETPLAY_PACKET_NULL:
      {
         log_printf ("NET: Recieved keep alive packet.");
         break;
      }

      case NETPLAY_PACKET_CHAT:
      {
         unsigned length;
         USTRING text;

         /* Determine how many bytes to read. */
         length = (header.size - sizeof(header));
         if (length == 0)
         {
            WARN("Recieved empty chat packet");
            return;
         }

         /* Load UTF8 string. */
         USTRING_CLEAR(text);
         pack_fread (text, MIN( length, (USTRING_SIZE - 1) ), file);

         /* Display it. */
         video_message (text);

         break;
      }

      default:
      {
         WARN_GENERIC();
         break;
      }
   }
}
