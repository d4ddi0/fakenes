/* FakeNES - A free, portable, Open Source NES emulator.

   audio.c: Implementation of the audio interface.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include "apu.h"
#include "audio.h"
#include "common.h"
#include "debug.h"
#include "gui.h"
#include "timing.h"
#include "types.h"

/* Once audio_init() has been called, please do not change the values of
   any of these variables without first calling audio_exit(), otherwise a
   series of memory leaks could occur. */

/* Parameters. */
BOOL audio_enable_output    = TRUE;
ENUM audio_subsystem        = AUDIO_SUBSYSTEM_ALLEGRO;
int  audio_sample_rate      = -1;   /* Autodetect. */
int  audio_sample_size      = -1;   /* Autodetect. */
BOOL audio_unsigned_samples = TRUE;
BOOL audio_interpolation    = TRUE;
int  audio_buffer_length    = 6;

/* Buffer sizes. */
int audio_buffer_frame_size_samples     = 0;
unsigned audio_buffer_frame_size_bytes  = 0;
static int audio_buffer_size_samples    = 0;
static unsigned audio_buffer_size_bytes = 0;

/* Queues. */
typedef struct _AUDIO_QUEUE_FRAME
{
   void *buffer;
   struct _AUDIO_QUEUE_FRAME *prev;
   struct _AUDIO_QUEUE_FRAME *next;

} AUDIO_QUEUE_FRAME;

static AUDIO_QUEUE_FRAME *audio_frames = NULL;
static int audio_frame_count = 0;

typedef struct _AUDIO_QUEUE
{
   AUDIO_QUEUE_FRAME *first;
   AUDIO_QUEUE_FRAME *last;

} AUDIO_QUEUE;

static AUDIO_QUEUE audio_queue;

/* Streams. */
static AUDIOSTREAM *audio_stream = NULL;

/* Helper macros. */
#define AUDIO_STEREO    apu_stereo_mode
#define AUDIO_CHANNELS  (AUDIO_STEREO ? 2 : 1)

/* Function prototypes. */
static int audio_create_queue (void);
static void audio_destroy_queue (void);
static void audio_enqueue (AUDIO_QUEUE_FRAME *);
static AUDIO_QUEUE_FRAME *audio_dequeue (void);
static void audio_output (void *);

/* Miscellaneous. */
volatile int audio_fps = 0;

int audio_init (void)
{
   DEBUG_PRINTF("audio_init()\n");

   /* Load configuration. */

   audio_enable_output = get_config_int ("audio", "enable_output", audio_enable_output);
   audio_subsystem     = get_config_int ("audio", "subsystem",     audio_subsystem);
   audio_sample_rate   = get_config_int ("audio", "sample_rate",   audio_sample_rate);
   audio_sample_size   = get_config_int ("audio", "sample_size",   audio_sample_size);
   audio_interpolation = get_config_int ("audio", "interpolation", audio_interpolation);
   audio_buffer_length = get_config_int ("audio", "buffer_length", audio_buffer_length);

   if (!audio_enable_output)
      return (0);

   switch (audio_subsystem)
   {
      case AUDIO_SUBSYSTEM_ALLEGRO:
      {
         if (audio_interpolation)
            set_mixer_quality (2);
         else
            set_mixer_quality (1);

         set_volume_per_voice (0);
              
         if (install_sound (DIGI_AUTODETECT, MIDI_NONE, NULL) != 0)
            return (1);

         if (digi_driver->id == DIGI_NONE)
            return (2);

         /* Autodetect settings. */

         if (audio_sample_rate == -1)
             audio_sample_rate = get_mixer_frequency ();
         if (audio_sample_size == -1)
             audio_sample_size = get_mixer_bits ();

         /* Allegro always uses unsigned samples. */
         audio_unsigned_samples = TRUE;

         break;
      }

      default:
         WARN_GENERIC();
   }

   /* Calculate buffer sizes. */

   /* Individual frames. */

   audio_buffer_frame_size_samples = (audio_sample_rate /
      timing_get_speed ());
   audio_buffer_frame_size_bytes = ((audio_buffer_frame_size_samples *
      AUDIO_CHANNELS) * (audio_sample_size / 8));

   /* Entire buffer. */

   audio_buffer_size_samples = (audio_buffer_frame_size_samples *
      audio_buffer_length);
   audio_buffer_size_bytes = ((audio_buffer_size_samples * AUDIO_CHANNELS) *
      (audio_sample_size / 8));

   /* Initialize subsystem. */

   switch (audio_subsystem)
   {
      case AUDIO_SUBSYSTEM_ALLEGRO:
      {
         /* Create stream. */

         if (!(audio_stream = play_audio_stream (audio_buffer_size_samples,
            audio_sample_size, AUDIO_STEREO, audio_sample_rate, 255, 128)))
         {
            WARN("Failed to create audio stream");
            return (3);
         }

         /* Pause stream. */
         voice_stop (audio_stream->voice);

         break;
      }

      default:
         WARN_GENERIC();
   }

   /* Create queue and return. */
   return (audio_create_queue ());
}

void audio_exit (void)
{
   DEBUG_PRINTF("audio_exit()\n");

   if (audio_enable_output)
   {
      /* Destroy queue. */
      audio_destroy_queue ();

      if (audio_stream)
      {
         /* Destroy stream. */
         stop_audio_stream (audio_stream);
      }

      /* Remove sound driver. */
      remove_sound ();
   }

   /* Save configuration. */

   set_config_int ("audio", "enable_output", audio_enable_output);
   set_config_int ("audio", "subsystem",     audio_subsystem);
   set_config_int ("audio", "sample_rate",   audio_sample_rate);
   set_config_int ("audio", "sample_size",   audio_sample_size);
   set_config_int ("audio", "interpolation", audio_interpolation);
   set_config_int ("audio", "buffer_length", audio_buffer_length);
}

void audio_update (void)
{
   static int wait_frames = 0;
   void *buffer;

   DEBUG_PRINTF("audio_update()\n");

   if (!audio_enable_output)
      return;

   if (wait_frames > 0)
      wait_frames--;
   if (wait_frames > 0)
      return;

   buffer = get_audio_stream_buffer (audio_stream);
   if (!buffer)
      return;

   /* Botch for AUDIOSTREAMs not working like they're supposed to. */
   wait_frames = audio_buffer_length;

   /* Write entire queue to buffer. */
   audio_output (buffer);

   free_audio_stream_buffer (audio_stream);

   /* Play stream if we haven't already. */
   voice_start (audio_stream->voice);
}

void audio_suspend (void)
{
   DEBUG_PRINTF("audio_suspend()\n");

   if (!audio_enable_output)
      return;

   voice_stop (audio_stream->voice);
}

void audio_resume (void)
{
   DEBUG_PRINTF("audio_resume()\n");

   if (!audio_enable_output)
      return;

   voice_start (audio_stream->voice);
}

static AUDIO_QUEUE_FRAME *floaty_frame = NULL;

void *audio_get_buffer (void)
{
   DEBUG_PRINTF("audio_get_buffer()\n");

   if (!audio_enable_output)
      return (NULL);

   floaty_frame = audio_dequeue ();
   if (!floaty_frame)
   {
      WARN_GENERIC();
      return (NULL);
   }

   return (floaty_frame->buffer);
}

void audio_free_buffer (void)
{
   DEBUG_PRINTF("audio_free_buffer()\n");

   if (!audio_enable_output)
      return;

   if (!floaty_frame)
   {
      WARN_GENERIC();
      return;
   }

   audio_enqueue (floaty_frame);
   floaty_frame = NULL;
}

/* --- Internal functions. --- */

static int audio_create_queue (void)
{
   int size;
   int index;

   /* Save frame count so we don't get memory leaks if 'audio_buffer_length'
      is changed before audio_destroy_queue() is called. */
   audio_frame_count = audio_buffer_length;

   /* Allocate frames. */

   size = (sizeof (AUDIO_QUEUE_FRAME) * audio_frame_count);

   audio_frames = malloc (size);
   if (!audio_frames)
   {
      WARN_GENERIC();
      return (1);
   }

   /* Clear frames. */
   memset (audio_frames, 0, size);

   /* Allocate frame buffers. */

   for (index = 0; index < audio_frame_count; index++)
   {
      AUDIO_QUEUE_FRAME *frame = &audio_frames[index];

      size = audio_buffer_frame_size_bytes;

      frame->buffer = malloc (size);
      if (!frame->buffer)
      {
         /* Possible memory leak here. */
         WARN_GENERIC();
         free (audio_frames);
         return (2);
      }

      /* Clear buffer. */
      memset (frame->buffer, 0, size);
   }

   /* Clear queue. */
   memset (&audio_queue, 0, sizeof (audio_queue));

   /* Map frames into the queue. */

   for (index = 0; index < audio_frame_count; index++)
   {
      AUDIO_QUEUE_FRAME *frame = &audio_frames[index];

      if (index > 0)
         frame->prev = &audio_frames[(index - 1)];
      if (index < (audio_frame_count - 1))
         frame->next = &audio_frames[(index + 1)];
   }

   audio_queue.first = &audio_frames[0];
   audio_queue.last  = &audio_frames[(audio_frame_count - 1)];

   /* Create ring. */
   /* Note that because of this we have to be careful to only read the queue
      in a single direction (either forward or backward, but not both at
      the same time), otherwise we could end up with an infinite loop. */
   audio_queue.first->prev = audio_queue.last;
   audio_queue.last->next  = audio_queue.first;

   /* Return success. */
   return (0);
}

static void audio_destroy_queue (void)
{
   if (audio_frames)
   {
      int index;

      for (index = 0; index < audio_frame_count; index++)
      {
         AUDIO_QUEUE_FRAME *frame = &audio_frames[index];

         if (frame->buffer)
            free (frame->buffer);
      }

      free (audio_frames);
      audio_frames = NULL;

      /* Reset frame counter. */
      audio_frame_count = 0;
   }

   /* Clear queue. */
   memset (&audio_queue, 0, sizeof (audio_queue));
}         

static void audio_enqueue (AUDIO_QUEUE_FRAME *frame)
{
   AUDIO_QUEUE_FRAME *last;

   RT_ASSERT(frame);

   /* Get last frame in queue. */
   last = audio_queue.last;

   /* Set up links. */
   last->next = frame;
   frame->prev = last;
   frame->next = audio_queue.first;

   /* Add frame to queue. */
   audio_queue.last = frame;
}

static AUDIO_QUEUE_FRAME *audio_dequeue (void)
{
   AUDIO_QUEUE_FRAME *frame;

   /* Grab first frame in queue. */
   frame = audio_queue.first;

   /* Remove frame from queue. */
   audio_queue.first = frame->next;

   /* Clear links. */
   audio_queue.first->prev = audio_queue.last;
   frame->prev = NULL;
   frame->next = NULL;

   /* Return frame. */
   return (frame);
}

static void audio_output (void *buffer)
{
   int frame_size, buffer_size;
   AUDIO_QUEUE_FRAME *frame;
   unsigned offset = 0;

   RT_ASSERT(buffer);

   frame_size  = audio_buffer_frame_size_bytes;
   buffer_size = audio_buffer_size_bytes;

   /* Get first frame in queue. */
   frame = audio_queue.first;

   while (frame)
   {
      if (offset >= buffer_size)
      {
         /* Buffer is full. */
         return;
      }

      /* Copy frame data to output buffer. */
      memcpy ((buffer + offset), frame->buffer, frame_size);

      /* Advance write pointer. */
      offset += frame_size;

      /* Advance to next frame in queue. */
      frame = frame->next;

      /* Increment FPS counter. */
      audio_fps++;
   }
}
