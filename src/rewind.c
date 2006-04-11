/* FakeNES - A free, portable, Open Source NES emulator.

   rewind.c: Implementation of the the game rewinder.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "debug.h"
#include "rewind.h"
#include "save.h"
#include "timing.h"
#include "types.h"
#ifdef USE_ZLIB
#include <zlib.h>
#endif

/* Maximum buffer file data size.  Buffer files are only used as temporary
   storage while saving/loading snapshots, therefor their size does not
   determine the amount of memory actually allocated for each frame in the
   queue.  The total size of a raw save state must not exceed this value. */
#define MAX_BUFFER_FILE_DATA_SIZE   131072

/* Whether or not real-time game rewinding is enabled.  Disabling this sweet
   feature can give a significant speed boost and reduced memory usage.

   Default: TRUE
   */
static BOOL enabled = TRUE;

/* Frame rate at which snapshots are saved/loaded.  Larger values mean
   smoother backtracking at the cost of memory and a more constant speed it.

   This should be a fractional value that represents a normalized percentage
   of the current emulation speed (e.g, 0.5 of 50 Hz would be 25 FPS).

   Default: 0.5
   Minimum: Value of EPSILON
   Maximum: 1.0
   */
static REAL frame_rate = 0.5f;

/* How long the rewinder can backtrack, in seconds.  This is combined with
   'frame_rate' to form the final value of 'max_queue_size'.

   Default: 10
   Minimum: 1
   Maximum: NONE
   */
static int seconds = 10;

/* Compression level (when available).

   Default: 1
   Minimum: 0 (disabled)
   Maximum: 9
   */
static int compression_level = 1;

/* Maximum number of enries in the queue, computed from 'seconds'. */
static int max_queue_size = 0;

/* Frame counter, used to enforce the frame rate. */
static int wait_frames = 0;

/* Queue. */

typedef struct _QUEUE_FRAME
{
   void *data;
   unsigned data_size;
   struct _QUEUE_FRAME *prev;
   struct _QUEUE_FRAME *next;

} QUEUE_FRAME;

typedef struct _QUEUE
{
   QUEUE_FRAME *first;
   QUEUE_FRAME *last;
   int size;

} QUEUE;

static QUEUE queue;

/* Queue routines (defined later). */
static BOOL pack (void *, long *);
static BOOL unpack (void *, long *, void *, long);
static BOOL enqueue (QUEUE_FRAME *);
static QUEUE_FRAME *unenqueue (void);
static QUEUE_FRAME *dequeue (void);

int rewind_init (void)
{
   /* Load configuration. */

   enabled           = get_config_int   ("rewind", "enabled",    enabled);
   frame_rate        = get_config_float ("rewind", "frame_rate", frame_rate);
   seconds           = get_config_int   ("rewind", "seconds",    seconds);
   compression_level = get_config_int   ("rewind", "compress",   compression_level);

   /* Enforce sane limits. */

   if (frame_rate < EPSILON)
      frame_rate = EPSILON;
   if (frame_rate > 1.0f)
      frame_rate = 1.0f;

   if (seconds < 1)
      seconds = 1;

   if (compression_level < 0)
      compression_level = 0;
   if (compression_level > 9)
      compression_level = 9;

   /* Calculate rough maximum queue size.
   
      TODO: Add the necessary code here (and elsewhere) to make this
      automatically adjust to the current emulation speed.  As-is, it
      produces a queue about 20% larger than necessary for PAL.
      */
   max_queue_size = ROUND(((60.0f * frame_rate) * seconds));

   /* Clear everything. */
   rewind_clear ();

   /* Return success. */
   return (0);
}

void rewind_exit (void)
{
   /* Clear everything. */
   rewind_clear ();

   /* Save configuration. */

   set_config_int   ("rewind", "enabled",    enabled);
   set_config_float ("rewind", "frame_rate", frame_rate);
   set_config_int   ("rewind", "seconds",    seconds);

#ifdef USE_ZLIB

   set_config_int ("rewind", "compress", compression_level);

#endif
}

void rewind_clear (void)
{
   QUEUE_FRAME *frame;

   if (!enabled)
      return;

   /* Clear queue. */

   /* Get first frame in queue. */
   frame = queue.first;

   while (frame)
   {
      QUEUE_FRAME *next = frame->next;

      if (frame->data)
      {
         /* Destroy frame data buffer. */
         free (frame->data);
      }

      /* Destroy frame. */
      free (frame);

      /* Advance to the next frame. */
      frame = next;
   }

   queue.first = NULL;
   queue.last  = NULL;
   queue.size  = 0;

   /* Clear frame counter. */
   wait_frames = 0;
}

typedef struct _BUFFER_FILE
{
   UINT8 data[MAX_BUFFER_FILE_DATA_SIZE];
   long pos, max;

} BUFFER_FILE;

static const PACKFILE_VTABLE *get_packfile_vtable (void);

BOOL rewind_save_snapshot (void)
{
   QUEUE_FRAME *frame;
   PACKFILE *file;
   BUFFER_FILE buffer;
   long size;
   REAL speed;

   if (!enabled)
      return (FALSE);

   if (wait_frames > 0)
      wait_frames--;
   if (wait_frames > 0)
      return (FALSE);

   if (queue.size >= max_queue_size)
   {
      /* The queue is currently full - dequeue and destroy the oldest frame
         to make room for the new one. */
      frame = dequeue ();
      if (!frame)
         return (FALSE);

      if (frame->data)
      {
         /* Destroy frame data buffer. */
         free (frame->data);
      }

      /* Clear frame for reuse. */
      /* memset (frame, 0, sizeof (QUEUE_FRAME)); */
   }
   else
   {
      /* Allocate a new frame. */
      frame = malloc (sizeof (QUEUE_FRAME));
      if (!frame)
         return (FALSE);

      /* Clear frame. */
      /* memset (frame, 0, sizeof (QUEUE_FRAME)); */
   }

   /* Clear buffer. */
   buffer.pos = 0;
   buffer.max = sizeof (buffer.data);

   /* Open buffer file. */
   file = pack_fopen_vtable (get_packfile_vtable (), &buffer);
   if (!file)
   {
      free (frame);
      return (FALSE);
   }

   /* Save snapshot. */
   /* TODO: Check for failure here. */
   save_state_raw (file);

   /* Close buffer file. */
   pack_fclose (file);

   /* Get data size. */
   size = buffer.pos;

   /* Compress data. */
   if (!pack (buffer.data, &size))
   {
      free (frame);
      return (FALSE);
   }

   /* Allocate frame data buffer. */
   frame->data = malloc (size);
   if (!frame->data)
   {
      free (frame);
      return (FALSE);
   }

   /* Copy data to frame data buffer. */
   memcpy (frame->data, buffer.data, size);

   /* Set data size. */
   frame->data_size = size;

   /* Enqueue frame. */
   if (!enqueue (frame))
   {
      /* Enqueue failed. */
      free (frame->data);
      free (frame);
      return (FALSE);
   }

   /* Set frame counter. */

   speed = timing_get_speed ();

   wait_frames = ROUND((speed / (speed * frame_rate)));

   /* Return success. */
   return (TRUE);
}

BOOL rewind_load_snapshot (void)
{
   QUEUE_FRAME *frame;
   PACKFILE *file;
   BUFFER_FILE buffer;
   long size;
   REAL speed;

   if (!enabled)
      return (FALSE);

   if (wait_frames > 0)
      wait_frames--;
   if (wait_frames > 0)
      return (FALSE);

   if (queue.size <= 0)
   {
      /* Queue is empty. */
      return (FALSE);
   }

   /* Fetch most recent frame. */
   frame = unenqueue ();
   if (!frame)
      return (FALSE);

   if (!frame->data)
   {
      /* This shouldn't have been allowed to slip through. */
      free (frame);
      return (FALSE);
   }

   /* Clear buffer. */
   buffer.pos = 0;
   buffer.max = sizeof (buffer.data);

   size = buffer.max;

   /* Uncompress data. */
   if (!unpack (buffer.data, &size, frame->data, frame->data_size))
   {
      free (frame->data);
      free (frame);
      return (FALSE);
   }

   /* Open buffer file. */
   file = pack_fopen_vtable (get_packfile_vtable (), &buffer);
   if (!file)
   {
      free (frame->data);
      free (frame);
      return (FALSE);
   }

   /* Load snapshot. */
   /* TODO: Check for failure here. */
   load_state_raw (file);

   /* Close buffer file. */
   pack_fclose (file);

   /* Destroy frame data buffer. */
   free (frame->data);

   /* Destroy frame. */
   free (frame);

   /* Set frame counter. */

   speed = timing_get_speed ();

   wait_frames = ROUND((speed / (speed * frame_rate)));

   /* Return success. */
   return (TRUE);
}

/* --- Internal functions. --- */

static BOOL pack (void *buffer, long *size)
{
   /* In-place Deflate compression. */

   void *packbuf;
   long packsize;

   RT_ASSERT(buffer);
   RT_ASSERT(size);

#ifdef USE_ZLIB

   if (compression_level == 0)
   {
      /* Nothing to do. */
      return (TRUE);
   }

   /* We add 16 bytes of slack just in case. */
   packsize = (((*size * 1.01f) + 12) + 16);

   packbuf = malloc (packsize);
   if (!packbuf)
      return (FALSE);

   if (compress2 (packbuf, &packsize, buffer, *size, compression_level) !=
      Z_OK)
   {
      free (packbuf);
      return (FALSE);
   }

   memcpy (buffer, packbuf, packsize);
   *size = packsize;
   
   free (packbuf);

#endif   /* USE_ZLIB */

   return (TRUE);
}

static BOOL unpack (void *outbuf, long *max, void *buffer, long size)
{
   /* Same as above, but decompression instead.  If zlib is not available,
      this performs a simple buffer copy instead.

      outbuf - The output buffer.
      max    - The maximum size of the output buffer (will be set to it's
               actual length after decompression).
      buffer - The input buffer.
      size   - The size of the data in the input buffer. */

   RT_ASSERT(outbuf);
   RT_ASSERT(max);
   RT_ASSERT(buffer);

#ifdef USE_ZLIB

   if (compression_level == 0)
   {
      /* Perform a normal buffer copy. */

      memcpy (outbuf, buffer, size);
      *max = size;
   }

   if (uncompress (outbuf, max, buffer, size) != Z_OK)
      return (FALSE);

#else /* USE_ZLIB */

   /* Perform a normal buffer copy. */

   memcpy (outbuf, buffer, size);
   *max = size;

#endif   /* !USE_ZLIB */

   return (TRUE);
}

static BOOL enqueue (QUEUE_FRAME *frame)
{
   QUEUE_FRAME *last;

   if (queue.size >= max_queue_size)
      return (FALSE);

   /* Get last frame in queue. */
   last = queue.last;

   if (last)
   {
      /* Add frame to queue. */
      queue.last = frame;
   
      /* Set up links. */
      last->next = frame;
      frame->prev = last;
      frame->next = NULL;
   }
   else
   {
      /* Queue is empty, add this as the first and last frame. */
      queue.first = frame;
      queue.last  = frame;

      /* Clear links. */
      frame->prev = NULL;
      frame->next = NULL;
   }

   /* Increment counter. */
   queue.size++;

   /* Return success. */
   return (TRUE);
}

static QUEUE_FRAME *unenqueue (void)
{
   QUEUE_FRAME *frame;

   if (queue.size <= 0)
   {
      /* Queue is empty. */
      return (NULL);
   }

   /* Get last frame in queue. */
   frame = queue.last;

   /* Remove frame from queue. */
   queue.last = frame->prev;

   /* Clear links. */

   if (queue.last)
      queue.last->next = NULL;

   frame->prev = NULL;
   frame->next = NULL;

   /* Decrement counter. */
   queue.size--;

   if (queue.size <= 0)
   {
      /* Queue is empty again - clear stale root pointer. */
      queue.first = NULL;
   }

   return (frame);
}

static QUEUE_FRAME *dequeue (void)
{
   QUEUE_FRAME *frame;

   if (queue.size <= 0)
   {
      /* Queue is empty. */
      return (NULL);
   }

   /* Get first frame in queue. */
   frame = queue.first;

   /* Remove frame from queue. */
   queue.first = frame->next;

   /* Clear links. */

   if (queue.first)
      queue.first->prev = NULL;

   frame->prev = NULL;
   frame->next = NULL;

   /* Decrement counter. */
   queue.size--;

   return (frame);
}

static int buffered_fclose (void *userdata)
{
   /* It seems that we have to provide this function to keep Allegro from
      crashing on calls to pack_fclose(). ;) */

   RT_ASSERT(userdata);

   /* Do nothing. */

   return (0);
}

static int buffered_getc (void *userdata)
{
   BUFFER_FILE *buffer;

   RT_ASSERT(userdata);

   buffer = userdata;

   if (buffer->pos >= buffer->max)
      return (0);

   return (buffer->data[buffer->pos++]);
}                  

static long buffered_fread (void *p, long n, void *userdata)
{
   BUFFER_FILE *buffer;
   long max;

   RT_ASSERT(p);
   RT_ASSERT(userdata);

   buffer = userdata;

   if (buffer->pos >= buffer->max)
      return (0);

   max = (buffer->max - buffer->pos);
   if (n > max)
      n = max;

   memcpy (p, (buffer->data + buffer->pos), n);
   buffer->pos += n;

   return (n);
}

static int buffered_putc (int c, void *userdata)
{
   BUFFER_FILE *buffer;

   RT_ASSERT(userdata);

   buffer = userdata;

   if (buffer->pos >= buffer->max)
      return (0);

   buffer->data[buffer->pos++] = c;

   return (c);
}
                                                               
static long buffered_fwrite (const void *p, long n, void *userdata)
{
   BUFFER_FILE *buffer;
   long max;

   RT_ASSERT(p);
   RT_ASSERT(userdata);

   buffer = userdata;

   if (buffer->pos >= buffer->max)
      return (0);

   max = (buffer->max - buffer->pos);
   if (n > max)
      n = max;

   memcpy ((buffer->data + buffer->pos), p, n);
   buffer->pos += n;

   return (n);
}

static PACKFILE_VTABLE packfile_vtable;

static const PACKFILE_VTABLE *get_packfile_vtable (void)
{
   PACKFILE_VTABLE *vtable = &packfile_vtable;

   memset (vtable, 0, sizeof (PACKFILE_VTABLE));

   vtable->pf_fclose = buffered_fclose;
   vtable->pf_getc   = buffered_getc;
   vtable->pf_fread  = buffered_fread;
   vtable->pf_putc   = buffered_putc;
   vtable->pf_fwrite = buffered_fwrite;

   return (vtable);
}
