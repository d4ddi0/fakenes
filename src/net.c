/* FakeNES - A free, portable, Open Source NES emulator.

   net.c: Implementation of the network interface.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for etails.
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

#define NET_MAX_PACKETS_PER_QUEUE   50

typedef struct _NET_PACKET_BUFFER
{
   NET_PACKET_HEADER header;              /* Packet header(also present in the buffer on incoming packets). */
   UINT8 data[NET_MAX_PACKET_SIZE_ANY];   /* Buffer containing the packet data. */
   unsigned size;                         /* Size of the packet data. */
   unsigned pos;                          /* Current buffer read/write offset. */

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

/* Clients. */

NET_CLIENT net_clients[NET_MAX_CLIENTS];

typedef struct NET_CLIENT_DATA
{
   NLsocket socket;
   NET_PACKET_BUFFER read_buffer;
   NET_PACKET_QUEUE  read_queue;
   NET_PACKET_BUFFER write_buffer;
   NET_PACKET_QUEUE  write_queue;

} NET_CLIENT_DATA;

static NET_CLIENT_DATA net_client_data[NET_MAX_CLIENTS];

/* Function prototypes. */

static void net_print_error (void);
static void net_enqueue (NET_PACKET_QUEUE *, const NET_PACKET_BUFFER *);
static void net_dequeue (NET_PACKET_QUEUE *, NET_PACKET_BUFFER *);
static void net_clear_queue (NET_PACKET_QUEUE *);

/* --- Public functions. --- */

int net_init (void)
{
   if (nlInit () != NL_TRUE)
   {
      net_print_error ();
      return (1);
   }

   /* We want TCP/IP protocol. */
   nlSelectNetwork (NL_IP);
   nlHint (NL_REUSE_ADDRESS, NL_TRUE);

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

      /* See if we have an open socket. */
      if (data->socket != NL_INVALID)
      {
         /* Close socket. */
         nlClose (data->socket);
      }
                             
      /* Clear buffers. */
      data->read_buffer.size  = 0;
      data->read_buffer.pos   = 0;
      data->write_buffer.size = 0;
      data->write_buffer.pos  = 0;

      /* Clear queues. */
      net_clear_queue (&data->read_queue);
      net_clear_queue (&data->write_queue);
   }

   /* Close root socket. */
   nlClose (net_socket);
   net_socket = NL_INVALID;

   log_printf ("NET: Root socket closed.\n");

   /* Clear client data. */
   memset (net_clients,     0, sizeof(net_clients));
   memset (net_client_data, 0, sizeof(net_client_data));

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

   /* Start listening. */
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
      memset (client, 0, sizeof(NET_CLIENT));
      memset (data,   0, sizeof(NET_CLIENT_DATA));

      /* Activate client. */
      client->active = TRUE;

      log_printf ("NET: Client #0 initialized.\n");
   }

   /* Accept any pending connections. */
   socket = nlAcceptConnection (net_socket);
   if (socket != NL_INVALID)
   {
      /* A new connection has been established... */

      int index;

      for (index = NET_FIRST_REMOTE_CLIENT; index < NET_MAX_CLIENTS; index++)
      {
         client = &net_clients[index];

         /* Make sure this slot isn't already in use. */
         if (client->active)
            continue;

         data = &net_client_data[index];

         /* Initialize structures. */
         memset (client, 0, sizeof(NET_CLIENT));
         memset (data,   0, sizeof(NET_CLIENT_DATA));
         
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

int net_connect (const CHAR *host, int port)
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
   memset (client, 0, sizeof(NET_CLIENT));
   memset (data,   0, sizeof(NET_CLIENT_DATA));

   /* Activate client. */
   client->active = TRUE;

   log_printf ("NET: Client #0 initialized.\n");

   return (TRUE);
}

void net_process (void)
{
   /* This function handles all of the magic of sending and recieving
      packets. */

   NET_CLIENT *client;
   NET_CLIENT_DATA *data;
   int index;

   if (net_mode == NET_MODE_INACTIVE)
      return;

   for (index = NET_FIRST_REMOTE_CLIENT; index < NET_MAX_CLIENTS; index++)
   {
      /* Recieve buffer. */
      UINT8 buffer[NET_MAX_PACKET_SIZE_RECIEVE];
      unsigned size;

      client = &net_clients[index];

      if (!client->active)
         continue;

      data = &net_client_data[index];

      /* Gather incoming packets. */

      size = nlRead (data->socket, &buffer, sizeof(buffer));
      if (size > 0)
      {
         unsigned pos;

         for (pos = 0; pos < size; pos++)
         {
            data->read_buffer.data[data->read_buffer.pos++] = buffer[pos];
            data->read_buffer.size++;
   
            if (data->read_buffer.size == sizeof(NET_PACKET_HEADER))
            {
               /* Extract header. */
               memcpy (&data->read_buffer.header, data->read_buffer.data, sizeof(data->read_buffer.header));

               /* Byte swap size field as needed. */
               data->read_buffer.header.size = nlSwaps (data->read_buffer.header.size);
            }
            else if ((data->read_buffer.size > sizeof(NET_PACKET_HEADER)) &&
                     (data->read_buffer.size == data->read_buffer.header.size))
            {
               /* Queue packet. */
               net_enqueue (&data->read_queue, &data->read_buffer);

               /* Clear buffer. */
               data->read_buffer.size = 0;
               data->read_buffer.pos = 0;
            }
         }
      }

      /* Distribute outgoing packets. */

      if (data->write_buffer.size > 0)
      {
         /* Transfer as much data as possible in one shot. */
         size = nlWrite (data->socket, (data->write_buffer.data + data->write_buffer.pos), data->write_buffer.size);
         if (size > 0)
         {
            data->write_buffer.size -= size;
            data->write_buffer.pos += size;
         }
      }

      if ((data->write_buffer.size == 0) &&
          (data->write_queue.size > 0))
      {
         /* Reload buffer with a packet from the queue. */
         net_dequeue (&data->write_queue, &data->write_buffer);
      }
   }
}

unsigned net_get_packet (ENUM client_id, void *buffer, unsigned size)
{
   NET_CLIENT *client;
   NET_CLIENT_DATA *data;
   NET_PACKET_BUFFER packet_buffer;
   unsigned length;

   RT_ASSERT(buffer);

   /* Make sure we were given a valid client ID. */
   if ( (client_id < 0) || (client_id >= NET_MAX_CLIENTS) )
      return (0);

   client = &net_clients[client_id];

   if (!client->active)
      return (0);

   data = &net_client_data[client_id];

   /* Make sure there is data in the queue. */
   if (data->read_queue.size == 0)
      return (0);

   /* Fetch a packet from the queue. */
   net_dequeue (&data->read_queue, &packet_buffer);

   /* Determine how much data to transfer. */
   length = MIN(NET_MAX_PACKET_SIZE_RECIEVE, size);

   /* Transfer the data from the packet buffer to the buffer. */
   memcpy (buffer, packet_buffer.data, length);

   /* Return the amount of data transfered. */
   return (length);
}
                                          
unsigned net_send_packet (UINT8 tag, void *buffer, unsigned size)
{
   unsigned length;
   NET_PACKET_BUFFER packet;
   NET_CLIENT *client;
   NET_CLIENT_DATA *data;
   int index;

   RT_ASSERT(buffer);

   /* Determine how much data to transfer. */
   length = MIN(NET_MAX_PACKET_SIZE_SEND, size);

   /* Transfer data from the buffer to the packet buffer. */
   memcpy ( ( packet.data + NET_PACKET_HEADER_SIZE ), buffer, length);

   /* Clear read/write pointer. */
   packet.pos = 0;

   /* Determine how large the packet will be. */
   packet.size = ( length + NET_PACKET_HEADER_SIZE );

   /* Byte swap size field as needed. */
   packet.size = nlSwaps (packet.size);

   /* Build the packet header. */
   packet.header.size = packet.size;
   packet.header.tag = tag;

   /* Embed the header in the data. */
   memcpy (packet.data, &packet.header, sizeof(packet.header));

   /* Distribute the packet to all remote clients. */
   for (index = NET_FIRST_REMOTE_CLIENT; index < NET_MAX_CLIENTS; index++)
   {
      client = &net_clients[index];

      if (!client->active)
         continue;

      data = &net_client_data[index];

      /* Send packet to write queue. */
      net_enqueue (&data->write_queue, &packet);
   }

   data = &net_client_data[NET_LOCAL_CLIENT];

   /* Simulate sending to oneself. */
   net_enqueue (&data->read_queue, &packet);

   /* Return the amount distributed. */
   return (length);
}

/* --- Private functions. --- */

static void net_print_error (void)
{
   /* Helper function for displaying error messages. */

   NLenum error;

   error = nlGetError ();
   if (error == NL_SYSTEM_ERROR)
      allegro_message ("Network error: System error: %s\n", nlGetSystemErrorStr (nlGetSystemError ()));
   else
      allegro_message ("Network error: HawkNL error: %s\n", nlGetErrorStr (error));
}

static void net_enqueue (NET_PACKET_QUEUE *queue, const NET_PACKET_BUFFER *packet)
{
   NET_PACKET_QUEUE_NODE *node;

   RT_ASSERT(queue);
   RT_ASSERT(packet);

   if (queue->size >= NET_MAX_PACKETS_PER_QUEUE)
   {
      log_printf ("NET: Warning: Queue full");
      return;
   }

   node = malloc (sizeof(NET_PACKET_QUEUE_NODE));
   if (!node)
   {
      WARN_GENERIC();
      return;
   }

   memcpy (&node->packet, packet, sizeof(NET_PACKET_BUFFER));
   
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

   memcpy (packet, &node->packet, sizeof(NET_PACKET_BUFFER));

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

   memset (queue, 0, sizeof(NET_PACKET_QUEUE));
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

int net_connect (const CHAR *host, int port)
{
   RT_ASSERT(host);

   return (FALSE);
}

void net_process (void)
{
   /* Do nothing. */
}

unsigned net_get_packet (ENUM client_id, void *buffer, unsigned size)
{
   RT_ASSERT(buffer);

   return (0);
}

unsigned net_send_packet (void *buffer, unsigned size)
{
   RT_ASSERT(buffer);

   return (0);
}

#endif   /* !USE_HAWKNL */
