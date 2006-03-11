/* FakeNES - A free, portable, Open Source NES emulator.
   Distributed under the Clarified Artistic License.

   net.c: Implementation of the network interface.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <string.h>
#include "common.h"
#include "debug.h"
#include "net.h"
#include "types.h"

#ifdef USE_HAWKNL
#include <nl.h>

ENUM net_mode = NET_MODE_INACTIVE;

typedef struct NET_CLIENT_DATA
{
   NLsocket *socket;

} NET_CLIENT_DATA;

static NET_CLIENT_DATA net_client_data[NET_MAX_CLIENTS];

static NLsocket net_socket = NL_INVALID;

static void net_print_error (void)
{
   /* Helper function for displaying error messages. */

   NLenum error = nlGetError ();
    
   if (error == NL_SYSTEM_ERROR)
   {
      allegro_message ("Network error: System error: %s\n",
         nlGetSystemErrorStr (nlGetSystemError ()));
   }
   else
   {
      allegro_message ("Network error: HawkNL error: %s\n", nlGetErrorStr
         (error));
   }
}

int net_init (void)
{
   if (nlInit () != NL_TRUE)
   {
      net_print_error ();
      return (1);
   }

   nlSelectNetwork (NL_IP);
   nlHint (NL_REUSE_ADDRESS, NL_TRUE);
   nlEnable (NL_LITTLE_ENDIAN_DATA);

   printf ("NET: Network initialized.\n");

   return (0);
}

void net_exit (void)
{
   net_close ();

   printf ("NET: Shutting down.\n");
   nlShutdown ();
}

int net_open (int port)
{
   /* This function opens a generic, reliable socket on 'port', which may
      then be used either to listen for incoming connections (server) or
      connect to a remote host (client).

      Only one socket opened in this manner may be open at any given time.
      In a server configuration, additional sockets are created as needed
      for each remote client by net_listen(). */

   if (net_socket != NL_INVALID)
      return (1);

   net_socket = nlOpen (port, NL_RELIABLE);
   if (net_socket == NL_INVALID)
   {
      net_print_error ();
      return (2);
   }

   printf ("NET: Root socket opened.\n");

   return (0);
}

void net_close (void)
{
   /* This function closes the socket previously opened by a call to
      net_open(). */

   if (net_socket != NL_INVALID)
   {
      nlClose (net_socket);
      net_socket = NL_INVALID;
      printf ("NET: Root socket closed.\n");
   }
}

void net_clear (void)
{
   /* Clears the net state. */
   net_mode = NET_MODE_INACTIVE;
}

int net_listen (void)
{
   /* This function sets up a socket to listen for incoming connections, or
      (if the socket is already listening) establishes at most one pending
      incoming connection and returns the new client's ID.

      Returns -1 on error, 0 if no incoming connections could be
      established, and 1+ if an incoming connection was established (remote
      client IDs start at 1, since the server is always client 0).

      Any successful calls to this function automatically switch the net
      code into server mode, which must later be reset by a call to
      net_clear() if another mode is desired. */

   NLsocket socket;

   if (nlListen (net_socket) != NL_TRUE)
   {
      net_print_error ();
      return (1);
   }

   if (net_mode == NET_MODE_INACTIVE)
   {
      NET_CLIENT *client = &net_clients[NET_LOCAL_CLIENT];

      /* Switch to server mode. */
      net_mode = NET_MODE_SERVER;

      /* Set up client 0. */

      memset (client, 0, sizeof (NET_CLIENT));

      client->active = TRUE;

      printf ("Client #0 initialized.\n");
   }

   socket = nlAcceptConnection (net_socket);
   if (socket != NL_INVALID)
   {
      int index;

      for (index = NET_FIRST_REMOTE_CLIENT; index < NET_MAX_CLIENTS;
         index++)
      {
         NET_CLIENT      *client = &net_clients[index];
         NET_CLIENT_DATA *data   = &net_client_data[index];

         if (client->active)
            continue;

         /* Initialize structures. */
         memset (client, 0, sizeof (NET_CLIENT));
         memset (data,   0, sizeof (NET_CLIENT_DATA));
         
         /* Set up data. */
         data->socket = socket;

         /* Link data to client. */
         client->data = data;

         /* Activate client. */
         client->active = TRUE;

         printf ("Client #%d initialized.\n", index);

         return (index);
      }

      /* No free client slot could be found. */
      nlClose (socket);

      return (-1);
   }

   return (0);
}

int net_connect (const char *host, int port)
{
   /* This function attempts to connect to 'host' on 'port' using the socket
      previously opened by a call to net_open().

      'host' is usually specified as an IP address, although other forms may
      be valid.

      Returns TRUE and switches the net code into client mode if the
      connection succeeded, or FALSE if the connection failed. */

   NLaddress address;

   RT_ASSERT(host);

   if (net_mode != NET_MODE_INACTIVE)
      return (FALSE);

   /* Construct address. */
   nlStringToAddr (host, &address);
   nlSetAddrPort (&address, port);

   nlEnable (NL_BLOCKING_IO);

   /* Establish connection. */
   if (nlConnect (net_socket, &address) != NL_TRUE)
   {
      net_print_error ();
      nlDisable (NL_BLOCKING_IO);
      return (FALSE);
   }

   nlDisable (NL_BLOCKING_IO);

   /* Switch to client mode. */
   net_mode = NET_MODE_CLIENT;

   return (TRUE);
}

void net_disconnect (void)
{
   /* This function closes a connection previously established by
      net_listen() or net_connect(), by first sending a disconnection
      notice, then closing the socket, and finally clearing the net state.
      */

   if (net_mode == NET_MODE_INACTIVE)
      return;

   net_close ();
   net_clear ();
}

void net_die (void)
{
   /* This is the server equivalent of net_disconnect().

      It literally kills the server, sending an abortion notice to all
      clients, disconnecting each of them one by one, then closing the
      socket, and finally clearing the net state.

      An example of when this might happen is when the NetPlay GUI dialog
      is blatantly closed using the [ X] button at the top-right corner. */

   int index;

   if (net_mode == NET_MODE_INACTIVE)
      return;

   net_close ();
   net_clear ();
}

#else /* USE_HAWKNL */

int net_init (void)
{
   return (1);
}

void net_exit (void)
{
   /* Do nothing. */
}

int net_open (int port)
{
   return (1);
}

void net_close (void)
{
   /* Do nothing. */
}

int net_listen (void)
{
   return (1);
}

int net_connect (const char *host, int port)
{
   RT_ASSERT(host);

   return (FALSE);
}

void net_disconnect (void)
{
   /* Do nothing. */
}

void net_die (void)
{
   /* Do nothing. */
}

#endif   /* !USE_HAWKNL */
