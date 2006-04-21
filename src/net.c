/* FakeNES - A free, portable, Open Source NES emulator.

   net.c: Implementation of the network interface.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "debug.h"
#include "log.h"
#include "net.h"
#include "types.h"

#ifdef USE_HAWKNL
#include <nl.h>

ENUM net_mode = NET_MODE_INACTIVE;

/* Sockets and groups. */

static NLsocket net_socket = NL_INVALID;

/* Packets & packet buffers. */

#define NET_MAX_PACKET_SIZE         16384
#define NET_MAX_PACKETS_PER_QUEUE   50

typedef struct _NET_PACKET_BUFFER
{
   UINT8 data[NET_MAX_PACKET_SIZE]; /* Buffer containing the packet data.      */
   unsigned size;                   /* Size of the packet data.                */
   unsigned pos;                    /* Current buffer read/write offset.       */
   ENUM pipe;                       /* Pipe to which the packet is to be sent. */

} NET_PACKET_BUFFER;

/* Packet queues. */

typedef struct _NET_PACKET_QUEUE_NODE
{
   NET_PACKET_BUFFER packet;
   struct _NET_PACKET_QUEUE_NODE *prev;
   struct _NET_PACKET_QUEUE_NODE *next;

} NET_PACKET_QUEUE_NODE;

typedef struct _NET_PACKET_QUEUE
{
   NET_PACKET_QUEUE_NODE *first;
   NET_PACKET_QUEUE_NODE *last;
   int size;

} NET_PACKET_QUEUE;

static NET_PACKET_QUEUE net_read_queue;

/* Clients. */

NET_CLIENT net_clients[NET_MAX_CLIENTS];

typedef struct NET_CLIENT_DATA
{
   NLsocket socket;
   NET_PACKET_BUFFER read_buffer;
   NET_PACKET_BUFFER write_buffer;
   NET_PACKET_QUEUE  write_queue;

} NET_CLIENT_DATA;

static NET_CLIENT_DATA net_client_data[NET_MAX_CLIENTS];

/* Function prototypes. */

static void net_print_error (void);
static void net_enqueue (NET_PACKET_QUEUE *, const NET_PACKET_BUFFER *);
static void net_dequeue (NET_PACKET_QUEUE *, NET_PACKET_BUFFER *);
static void net_clear_queue (NET_PACKET_QUEUE *);
static const PACKFILE_VTABLE *net_get_packfile_vtable (void);

/* --- Public functions. --- */

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

   log_printf ("NET: Network initialized.\n");

   return (0);
}

void net_exit (void)
{
   net_close ();

   log_printf ("NET: Shutting down.\n");
   nlShutdown ();
}

int net_open (int port)
{
   /* This function opens a generic, reliable socket on 'port', which may
      then be used either to listen for incoming connections (server) or
      connect to a remote host (client).

      Only one socket opened in this manner may be open at any given time.
      In a server configuration, additional sockets are created as needed
      for each remote client by net_listen().

      'port' should be 0 if trying to connect to a remote host, otherwise
      HawkNL will throw an "App version not supported by DLL" error. */

   if (net_socket != NL_INVALID)
      return (1);

   net_socket = nlOpen (port, NL_RELIABLE);
   if (net_socket == NL_INVALID)
   {
      net_print_error ();
      return (2);
   }

   log_printf ("NET: Root socket opened.\n");

   return (0);
}

void net_close (void)
{
   int index;

   if (net_mode == NET_MODE_INACTIVE)
      return;

   /* This function closes all open sockets, resets all variables, and
      clears all buffers. */

   for (index = 0; index < NET_MAX_CLIENTS; index++)
   {
      NET_CLIENT *client = &net_clients[index];
      NET_CLIENT_DATA *data;

      if (!client->active)
         continue;

      data = &net_client_data[index];

      if (data->socket != NL_INVALID)
      {
         /* Close socket. */
         nlClose (data->socket);
      }
                             
      /* Clear queue. */
      net_clear_queue (&data->write_queue);
   }

   nlClose (net_socket);
   net_socket = NL_INVALID;

   log_printf ("NET: Root socket closed.\n");

   /* Clear client data. */
   memset (net_clients,     0, sizeof (net_clients));
   memset (net_client_data, 0, sizeof (net_client_data));

   /* Clear queue. */
   net_clear_queue (&net_read_queue);

   /* Reset net state. */
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
      net_close() if another mode is desired. */

   NLsocket socket;
   NET_CLIENT *client;
   NET_CLIENT_DATA *data;

   if (nlListen (net_socket) != NL_TRUE)
   {
      net_print_error ();
      return (1);
   }

   if (net_mode == NET_MODE_INACTIVE)
   {
      /* Switch to server mode. */
      net_mode = NET_MODE_SERVER;

      log_printf ("NET: Server initialized.\n");

      /* Set up client #0. */

      client = &net_clients[NET_LOCAL_CLIENT];
      data   = &net_client_data[NET_LOCAL_CLIENT];

      /* Initialize structures. */
      memset (client, 0, sizeof (NET_CLIENT));
      memset (data,   0, sizeof (NET_CLIENT_DATA));

      /* Activate client. */
      client->active = TRUE;

      log_printf ("NET: Client #0 initialized.\n");
   }

   socket = nlAcceptConnection (net_socket);
   if (socket != NL_INVALID)
   {
      int index;

      for (index = NET_FIRST_REMOTE_CLIENT; index < NET_MAX_CLIENTS;
         index++)
      {
         client = &net_clients[index];

         if (client->active)
            continue;

         data = &net_client_data[index];

         /* Initialize structures. */
         memset (client, 0, sizeof (NET_CLIENT));
         memset (data,   0, sizeof (NET_CLIENT_DATA));
         
         /* Set up data. */
         data->socket = socket;

         /* Activate client. */
         client->active = TRUE;

         log_printf ("NET: Client #%d initialized.\n", index);

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
   NET_CLIENT *client;
   NET_CLIENT_DATA *data;

   RT_ASSERT(host);

   if (net_mode != NET_MODE_INACTIVE)
      return (FALSE);

   /* Construct address. */
   nlStringToAddr (host, &address);
   nlSetAddrPort (&address, port);

   /* Establish connection. */
   if (nlConnect (net_socket, &address) != NL_TRUE)
   {
      net_print_error ();
      return (FALSE);
   }

   /* Switch to client mode. */
   net_mode = NET_MODE_CLIENT;

   /* Set up client #0. */

   client = &net_clients[NET_LOCAL_CLIENT];
   data   = &net_client_data[NET_LOCAL_CLIENT];

   /* Initialize structures. */
   memset (client, 0, sizeof (NET_CLIENT));
   memset (data,   0, sizeof (NET_CLIENT_DATA));

   /* Activate client. */
   client->active = TRUE;

   log_printf ("NET: Client #0 initialized.\n");

   return (TRUE);
}

void net_process (void)
{
   NET_CLIENT *client;
   NET_CLIENT_DATA *data;
   int index;

   /* This function handles all of the magic of sending and recieving
      packets. */

   if (net_mode == NET_MODE_INACTIVE)
      return;

   for (index = NET_FIRST_REMOTE_CLIENT; index < NET_MAX_CLIENTS; index++)
   {
      /* Grab all incoming packets into the read queue, reading them in in
         buffered fragments if necessary. */

      client = &net_clients[index];

      if (client->active)
         continue;
   }

   client = &net_clients[NET_LOCAL_CLIENT];

   /* TODO: Distribute all outgoing packets here. */
}

PACKFILE *net_open_packet (ENUM pipe)
{
   NET_PACKET_BUFFER *packet = NULL;
   PACKFILE *packfile;

   switch (pipe)
   {
      case NET_PIPE_READ:
      {
         NET_PACKET_QUEUE *queue = &net_read_queue;

         /* Make sure there is data in the queue. */
         if (queue->size == 0)
            return (NULL);

         packet = malloc (sizeof (NET_PACKET_BUFFER));
         if (!packet)
         {
            WARN_GENERIC();
            return (NULL);
         }

         /* Grab a packet from the queue. */
         net_dequeue (queue, packet);

         break;
      }

      case NET_PIPE_WRITE:
      {
         packet = malloc (sizeof (NET_PACKET_BUFFER));
         if (!packet)
         {
            WARN_GENERIC();
            return (NULL);
         }

         memset (packet, 0, sizeof (NET_PACKET_BUFFER));

         /* Note: Packet actually gets written in net_packfile_fclose(). */

         break;
      }

      default:
         WARN_GENERIC();
   }

   /* Set pipe. */
   packet->pipe = pipe;

   /* Open the packet in the form of a PACKFILE. */
   packfile = pack_fopen_vtable (net_get_packfile_vtable (), packet);
   if (!packfile)
   {
      WARN_GENERIC();

      free (packet);
      return (NULL);
   }

   return (packfile);
}

/* --- Private functions. --- */

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

static void net_enqueue (NET_PACKET_QUEUE *queue, const NET_PACKET_BUFFER
   *packet)
{
   NET_PACKET_QUEUE_NODE *node;

   RT_ASSERT(queue);
   RT_ASSERT(packet);

   if (queue->size >= NET_MAX_PACKETS_PER_QUEUE)
      return;

   node = malloc (sizeof (NET_PACKET_QUEUE_NODE));
   if (!node)
   {
      WARN_GENERIC();
      return;
   }

   memcpy (&node->packet, packet, sizeof (NET_PACKET_BUFFER));
   
   queue->last->next = node;
   queue->last = node;

   queue->size++;
}

static void net_dequeue (NET_PACKET_QUEUE *queue, NET_PACKET_BUFFER *packet)
{
   NET_PACKET_QUEUE_NODE *node;

   RT_ASSERT(queue);
   RT_ASSERT(packet);

   if (queue->size <= 0)
      return;

   node = queue->first;

   memcpy (packet, &node->packet, sizeof (NET_PACKET_BUFFER));

   queue->first = node->next;

   free (node);
   
   queue->size--;
}

static void net_clear_queue (NET_PACKET_QUEUE *queue)
{
   /* This function clears a queue, freeing all of the memory used by it's
      nodes.  This is the only safe way to clear a queue. */

   NET_PACKET_QUEUE_NODE *node;
   NET_PACKET_QUEUE_NODE *next;

   RT_ASSERT(queue);

   node = queue->first;

   while ((node) && (next = node->next))
   {
      free (node);
      node = next;
   }

   memset (queue, 0, sizeof (NET_PACKET_QUEUE));
}

static int net_packfile_fclose (void *d)
{
   NET_PACKET_BUFFER *packet;

   RT_ASSERT(d);

   packet = d;

   if (packet->pipe == NET_PIPE_WRITE)
   {
      NET_CLIENT_DATA *data;

      switch (net_mode)
      {
         case NET_MODE_SERVER:
         {
            /* Distribute packet to remote clients. */

            int index;

            for (index = NET_FIRST_REMOTE_CLIENT; index < NET_MAX_CLIENTS;
               index++)
            {
               const NET_CLIENT *client = &net_clients[index];

               if (!client->active)
                  continue;

               data = &net_client_data[index];

               /* Send packet to queue. */
               net_enqueue (&data->write_queue, packet);
            }

            /* Simulate sending to oneself. */
            net_enqueue (&net_read_queue, packet);

            break;
         }

         case NET_MODE_CLIENT:
         {
            data = &net_client_data[NET_LOCAL_CLIENT];

            /* Send packet to queue. */
            net_enqueue (&data->write_queue, packet);

            break;
         }

         default:
            WARN_GENERIC();
      }
   }

   free (packet);

   return (0);
}

static int net_packfile_getc (void *d)
{
   NET_PACKET_BUFFER *packet;
   NLbyte value;

   RT_ASSERT(d);

   packet = d;

   readByte(packet->data, packet->pos, value);

   return (value);
}

static int net_packfile_ungetc (int c, void *d)
{
   RT_ASSERT(d);

   /* Unsupported. */
   return (0);
}

static long net_packfile_fread (void *p, long n, void *d)
{
   NET_PACKET_BUFFER *packet;
   
   RT_ASSERT(p);
   RT_ASSERT(d);

   packet = d;

   readBlock(packet->data, packet->pos, p, n);

   return (n);
}

static int net_packfile_putc (int c, void *d)
{
   NET_PACKET_BUFFER *packet;

   RT_ASSERT(d);

   packet = d;

   writeByte(packet->data, packet->pos, c);

   return (c);
}

static long net_packfile_fwrite (const void *p, long n, void *d)
{
   NET_PACKET_BUFFER *packet;

   RT_ASSERT(p);
   RT_ASSERT(d);

   packet = d;

   writeBlock(packet->data, packet->pos, p, n);

   return (n);
}

static int net_packfile_fseek (void *d, int o)
{
   RT_ASSERT(d);

   /* Unsupported. */
   return (0);
}

static int net_packfile_feof (void *d)
{
   RT_ASSERT(d);

   /* Unsupported. */
   return (FALSE);
}

static int net_packfile_ferror (void *d)
{
   RT_ASSERT(d);
      
   /* Unsupported. */
   return (0);
}

static PACKFILE_VTABLE net_packfile_vtable;

static const PACKFILE_VTABLE *net_get_packfile_vtable (void)
{
   net_packfile_vtable.pf_fclose = net_packfile_fclose;
   net_packfile_vtable.pf_getc   = net_packfile_getc;
   net_packfile_vtable.pf_ungetc = net_packfile_ungetc;
   net_packfile_vtable.pf_fread  = net_packfile_fread;
   net_packfile_vtable.pf_putc   = net_packfile_putc;
   net_packfile_vtable.pf_fwrite = net_packfile_fwrite;
   net_packfile_vtable.pf_fseek  = net_packfile_fseek;
   net_packfile_vtable.pf_feof   = net_packfile_feof;
   net_packfile_vtable.pf_ferror = net_packfile_ferror;

   return (&net_packfile_vtable);
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

void net_process (void)
{
   /* Do nothing. */
}

PACKFILE *net_open_packet (ENUM pipe)
{
   return (NULL);
}

#endif   /* !USE_HAWKNL */
