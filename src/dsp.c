/* FakeNES - A free, portable, Open Source NES emulator.

   dsp.c: Implementation of the digital sound processor.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "debug.h"
#include "dsp.h"
#include "log.h"
#include "types.h"

/* Master volume. */
REAL dsp_master_volume = 1.0f;

/* The DSP buffer. */
static DSP_SAMPLE *dsp_buffer = NULL;

/* Parameters passed to dsp_init(). */
static int dsp_buffer_samples  = 0;
static int dsp_buffer_channels = 0;

/* Current write position (for dsp_write()). */
static int dsp_write_sample = 0;

/* Sane minimums and maximums. */
#define DSP_INPUT_CHANNELS_MIN   1
#define DSP_INPUT_CHANNELS_MAX   DSP_MAX_CHANNELS

#define DSP_OUTPUT_CHANNELS_MIN  1
#define DSP_OUTPUT_CHANNELS_MAX  2

#define DSP_SAMPLE_VALUE_MIN  -1.0f
#define DSP_SAMPLE_VALUE_MAX  +1.0f

/* Sample access macros. */
#define DSP_BUFFER_SAMPLE(sample, channel)   \
   dsp_buffer[((sample * dsp_buffer_channels) + channel)]

/* Channel parameters. */
typedef struct _DSP_CHANNEL_PARAMS
{
   BOOL enabled;
   REAL volume, pan;

} DSP_CHANNEL_PARAMS;

static DSP_CHANNEL_PARAMS dsp_channel_params[DSP_MAX_CHANNELS];

/* Effectors. */
static LIST dsp_effector_list = DSP_EFFECTOR_DITHER;

/* WAV writer (see bottom). */
static void dsp_wav_write (void);

/* --- Initialization and deinitialization. --- */

int dsp_init (void)
{
   DEBUG_PRINTF("dsp_init()\n");

   /* Load configuration. */

   dsp_master_volume = get_config_float ("dsp", "master_volume", dsp_master_volume);
   dsp_effector_list = get_config_int   ("dsp", "effector_list", dsp_effector_list);

   /* Clear channel parameters. */
   memset (dsp_channel_params, 0, sizeof (dsp_channel_params));

   /* Return success. */
   return (0);
}

void dsp_exit (void)
{
   /* This function deinitializes the DSP, and destroys any buffers that may
      be allocated. */

   DEBUG_PRINTF("dsp_exit()\n");

   dsp_close_wav ();
   dsp_close ();

   /* Save configuration. */

   set_config_float ("dsp", "master_volume", dsp_master_volume);
   set_config_int   ("dsp", "effector_list", dsp_effector_list);
}

/* --- Buffer manipulation. --- */

int dsp_open (int samples, int channels)
{
   int size;

   DEBUG_PRINTF("dsp_open(samples=%d, channels=%d)\n", samples, channels);
                 
   /* This function initializes the DSP.  Since the DSP is used for creating
      sample buffers suitable for playback on the sound system, this
      function must be called before any attempts to generate audio.

      samples  - The buffer size, in samples.  Note that multiple channels
                 (e.g stereo) count as a single sample here, so if your
                 audio sampling rate was 48 kHz, you would set this to 48000
                 samples if your buffer was exactly 1 second long.

      channels - The number of channels that the buffer will contain.  More
                 than 2 channels are supported.  The panning and volume of
                 each channel can be adjusted by a call to
                 dsp_set_channel_params().

      dsp_write() can be called to store samples in the buffer.  It takes an
      array of DSP_SAMPLE data, with the array size being equal to the
      number of channels in the buffer.  The total dynamic range of the
      samples should not exceed -0.5f or +0.5f.
   */

   /* Clamp values to sane minimums and maximums. */
   channels = fix (channels, DSP_INPUT_CHANNELS_MIN,
      DSP_INPUT_CHANNELS_MAX);

   /* Calculate buffer size in bytes. */
   size = ((samples * channels) * sizeof (DSP_SAMPLE));

   /* Allocate buffer. */
   dsp_buffer = malloc (size);
   if (!dsp_buffer)
   {
      /* Allocation failed. */
      WARN("Failed to create DSP buffer");
      return (1);
   }

   /* Clear buffer. */
   memset (dsp_buffer, 0, size);
   
   /* Set buffer parameters. */
   dsp_buffer_samples  = samples;
   dsp_buffer_channels = channels;

   /* Return success. */
   return (0);
}

void dsp_close (void)
{
   DEBUG_PRINTF("dsp_close()\n");

   if (dsp_buffer)
   {
      /* Deallocate and nullify buffer. */
      free (dsp_buffer);
      dsp_buffer = NULL;
   }
}

void dsp_start (void)
{
   DEBUG_PRINTF("dsp_start()\n");

   /* Clear write pointer. */
   dsp_write_sample = 0;
}

void dsp_write (const DSP_SAMPLE *samples)
{
   int sample, channel;

   RT_ASSERT(samples);

   // DEBUG_PRINTF("dsp_write(samples=0x%x)\n", samples);

   sample = dsp_write_sample;

   for (channel = 0; channel < dsp_buffer_channels; channel++)
   {
      DSP_SAMPLE value;

      value = samples[channel];

      /* Clipping. */
      value *= dsp_master_volume;
      value = fixf (value, DSP_SAMPLE_VALUE_MIN, DSP_SAMPLE_VALUE_MAX);

      DSP_BUFFER_SAMPLE(sample, channel) = value;
   }

   dsp_write_sample++;
}

void dsp_end (void)
{
   DEBUG_PRINTF("dsp_end()\n");

   /* Write data to disk if necessary. */
   dsp_wav_write ();
}

/* --- Channel param manipulation. --- */

void dsp_set_channel_enabled (int channel, ENUM mode, BOOL enabled)
{
   DSP_CHANNEL_PARAMS *params;

   DEBUG_PRINTF("dsp_set_channel_enabled(channel=%d, mode=%d, "
      "enabled=%d)\n", channel, mode, enabled);

   if ((channel < 0) || (channel >= DSP_MAX_CHANNELS))
      WARN_BREAK_GENERIC();

   params = &dsp_channel_params[channel];

   switch (mode)
   {
      case DSP_SET_ENABLED_MODE_SET:
      {
         params->enabled = enabled;

         break;
      }

      case DSP_SET_ENABLED_MODE_INVERT:
      {         
         params->enabled = !params->enabled;

         break;
      }

      default:
         WARN_GENERIC();
   }
}

BOOL dsp_get_channel_enabled (int channel)
{
   DEBUG_PRINTF("dsp_get_channel_enabled(channel=%d)\n", channel);

   if ((channel < 0) || (channel >= DSP_MAX_CHANNELS))
   {
      WARN_GENERIC();
      return (FALSE);
   }

   return (dsp_channel_params[channel].enabled);
}

void dsp_set_channel_params (int channel, REAL volume, REAL pan)
{
   DSP_CHANNEL_PARAMS *params;

   DEBUG_PRINTF("dsp_set_channel_params(channel=%d, volume=%g, pan=%g)\n",
      channel, volume, pan);

   if ((channel < 0) || (channel >= DSP_MAX_CHANNELS))
      WARN_BREAK_GENERIC();

   params = &dsp_channel_params[channel];

   params->volume = volume;
   params->pan    = pan;
}

/* --- Effector manipulation. --- */

void dsp_set_effector_enabled (FLAGS effector, ENUM mode, BOOL enabled)
{
   DEBUG_PRINTF("dsp_set_effector_enabled(effector=0x%x, mode=%d, "
      "enabled=%d)\n", effector, mode, enabled);

   switch (mode)
   {
      case DSP_SET_ENABLED_MODE_SET:
      {
         if (enabled)
            LIST_ADD(dsp_effector_list, effector);
         else
            LIST_REMOVE(dsp_effector_list, effector);

         break;
      }

      case DSP_SET_ENABLED_MODE_INVERT:
      {
         if (LIST_COMPARE(dsp_effector_list, effector))
            LIST_REMOVE(dsp_effector_list, effector);
         else
            LIST_ADD(dsp_effector_list, effector);

         break;
      }

      default:
         WARN_GENERIC();
   }
}

BOOL dsp_get_effector_enabled (FLAGS effector)
{
   DEBUG_PRINTF("dsp_get_effector_enabled(effector=0x%x)\n", effector);

   return (LIST_COMPARE(dsp_effector_list, effector));
}

/* --- Rendering and filters. --- */

/* Helper macros. */

#define DSP_MIXER_LEFT   mixers[0]
#define DSP_MIXER_RIGHT  mixers[1]
#define DSP_OUTPUT_LEFT  outputs[0]
#define DSP_OUTPUT_RIGHT outputs[1]

#define DSP_MIXER_TO_OUTPUT(mixer)  ROUND((mixer * 4294967295.0f))
#define DSP_OUTPUT_SIGNED_BIT       0x80000000
#define DSP_OUTPUT_SHIFTS_8         24
#define DSP_OUTPUT_SHIFTS_16        16

#define DSP_BUFFER_OUTPUT_SAMPLES(count)                          \
   {                                                              \
      int index;                                                  \
                                                                  \
      switch (bits_per_sample)                                    \
      {                                                           \
         case 8:                                                  \
         {                                                        \
            UINT8 *out = buffer;                                  \
                                                                  \
            for (index = 0; index < count; index++)               \
               *out++ = (outputs[index] >> DSP_OUTPUT_SHIFTS_8);  \
                                                                  \
            buffer = out;                                         \
                                                                  \
            break;                                                \
         }                                                        \
                                                                  \
         case 16:                                                 \
         {                                                        \
            UINT16 *out = buffer;                                 \
                                                                  \
            for (index = 0; index < count; index++)               \
               *out++ = (outputs[index] >> DSP_OUTPUT_SHIFTS_16); \
                                                                  \
            buffer = out;                                         \
                                                                  \
            break;                                                \
         }                                                        \
                                                                  \
         default:                                                 \
            WARN_GENERIC();                                       \
      }                                                           \
   }

void dsp_render (void *buffer, int channels, int bits_per_sample, BOOL
   unsigned_samples)
{
   int sample, channel;

   /* This function performs all applicable filtering on a completed DSP
      buffer (filled by dsp_write()), and stores the samples in the output
      buffer using the specified format.

      buffer           - A pointer to the output buffer.  It must be large
                         enough to accomodate all of the samples in the DSP
                         buffer across all of the output channels.

      channels         - Number of channels to mix to.  Must be set to 1 for
                         mono, or 2 for stereo.  Surround sound (3 or more
                         channels) output is not yet supported by the
                         renderer.

      bits_per_sample  - The depth of each sample written to the output
                         buffer.  Must be set to 8 or 16.  24 and 32 bits
                         per sample may be supported in the future.

      unsigned_samples - Whether or not the output samples will be centered
                         around 0 (FALSE), or half of their dynamic range
                         (TRUE).
   */

   RT_ASSERT(buffer);

   DEBUG_PRINTF("dsp_render(buffer=0x%x,channels=%d,bits_per_sample=%d,"
      "unsigned_samples=%d)\n", buffer, channels, bits_per_sample,
         unsigned_samples);

   if (!dsp_buffer)
      WARN_BREAK("dsp_buffer is null");

   /* Clamp values to sane minimums and maximums. */
   channels = fix (channels, DSP_OUTPUT_CHANNELS_MIN,
      DSP_OUTPUT_CHANNELS_MAX);

   if ((bits_per_sample != 8) &&
       (bits_per_sample != 16))
   {
      /* Force default. */
      bits_per_sample = 16;
   }

   /* Quantize and store. */
   for (sample = 0; sample < dsp_buffer_samples; sample++)
   {
      DSP_SAMPLE mixers[DSP_OUTPUT_CHANNELS_MAX];
      INT32 outputs[DSP_OUTPUT_CHANNELS_MAX];

      DSP_MIXER_LEFT  = 0;
      DSP_MIXER_RIGHT = 0;

      for (channel = 0; channel < dsp_buffer_channels; channel++)
      {
         DSP_CHANNEL_PARAMS *params = &dsp_channel_params[channel];
         DSP_SAMPLE input;

         if (!params->enabled)
            continue;

         input = (DSP_BUFFER_SAMPLE(sample, channel) * params->volume);
         input /= 2.0f; /* For stereo mixing. */

         if (params->pan < (0 - EPSILON))
         {
            DSP_MIXER_LEFT  += input;
            DSP_MIXER_RIGHT += (input / 2.0f);
         }
         else if (params->pan > (0 + EPSILON))
         {
            DSP_MIXER_LEFT  += (input / 2.0f);
            DSP_MIXER_RIGHT += input;
         }
         else
         {
            DSP_MIXER_LEFT  += input;
            DSP_MIXER_RIGHT += input;
         }
      }

      /* Clipping. */

      DSP_MIXER_LEFT  = fixf (DSP_MIXER_LEFT,  DSP_SAMPLE_VALUE_MIN,
         DSP_SAMPLE_VALUE_MAX);
      DSP_MIXER_RIGHT = fixf (DSP_MIXER_RIGHT, DSP_SAMPLE_VALUE_MIN,
         DSP_SAMPLE_VALUE_MAX);

      if (dsp_effector_list & DSP_EFFECTOR_SWAP_CHANNELS)
      {
         /* Swap stereo channels. */

         DSP_SAMPLE old_left;

         old_left = DSP_MIXER_LEFT;
         DSP_MIXER_LEFT = DSP_MIXER_RIGHT;
         DSP_MIXER_RIGHT = old_left;
      }

      /* Convert back to signed integer. */

      DSP_OUTPUT_LEFT  = DSP_MIXER_TO_OUTPUT(DSP_MIXER_LEFT);
      DSP_OUTPUT_RIGHT = DSP_MIXER_TO_OUTPUT(DSP_MIXER_RIGHT);

      if (unsigned_samples)
      {
         /* Convert signed to unsigned. */

         DSP_OUTPUT_LEFT  ^= DSP_OUTPUT_SIGNED_BIT;
         DSP_OUTPUT_RIGHT ^= DSP_OUTPUT_SIGNED_BIT;
      }

      if (dsp_effector_list & DSP_EFFECTOR_DITHER)
      {
         /* Dithering. */

         DSP_OUTPUT_LEFT ^= ((DSP_OUTPUT_LEFT & (DSP_OUTPUT_SIGNED_BIT >>
            bits_per_sample)) << 1);
         DSP_OUTPUT_RIGHT ^= ((DSP_OUTPUT_RIGHT & (DSP_OUTPUT_SIGNED_BIT >>
            bits_per_sample)) << 1);
      }

      /* Quantize and output samples. */

      switch (channels)
      {
         case 1:  /* Mono. */
         {
            /* Downmix to mono. */

            DSP_OUTPUT_LEFT >>= 1;
            DSP_OUTPUT_LEFT += (DSP_OUTPUT_RIGHT >> 1);

            DSP_BUFFER_OUTPUT_SAMPLES(1);

            break;
         }

         case 2:  /* Stereo. */
         {
            DSP_BUFFER_OUTPUT_SAMPLES(2);

            break;
         }

         default:
            WARN_GENERIC();
      }
   }
}

/* --- WAV writer functions. --- */

/* TODO: Fix this stuff up to work properly on big-endian platforms
   (currently it produces a big-endian ordered WAV file, I think). */

typedef struct
{
   UINT32 chunk_id;
   INT32  chunk_size;
   UINT32 riff_type;

} dsp_wav_riff_type_chunk_t;

typedef struct
{
   UINT32 chunk_id;
   INT32  chunk_size;
   INT16  format_tag;
   UINT16 channels;
   UINT32 samples_per_sec;
   UINT32 avg_bytes_per_sec;
   UINT16 block_align;
   UINT16 bits_per_sample;

} dsp_wav_format_chunk_t;

typedef struct
{
   UINT32 chunk_id;
   INT32  chunk_size;

} dsp_wav_data_chunk_t;

#define DSP_WAV_ID(a,b,c,d)   ((d << 24) | (c << 16) | (b << 8) | a)

#define WAV_HEADER_SIZE \
   (sizeof (dsp_wav_riff_type_chunk_t) +  \
    sizeof (dsp_wav_format_chunk_t) +  \
    sizeof (dsp_wav_data_chunk_t))

static FILE    *dsp_wav_file            = NULL;
static unsigned dsp_wav_size            = 0;
static int      dsp_wav_sample_rate     = 0;
static int      dsp_wav_channels        = 0;
static int      dsp_wav_bits_per_sample = 0;

int dsp_open_wav (const UCHAR *filename, int sample_rate, int channels, int
   bits_per_sample)
{
   RT_ASSERT(filename);

   DEBUG_PRINTF("dsp_open_wav(filename=%s,sample_rate=%d,channels=%d,"
      "bits_per_sample=%d)\n", filename, sample_rate, channels,
         bits_per_sample);

   if ((channels != 1) &&
       (channels != 2))
   {
      /* Unsupported channel count. */
      return (1);
   }

   if ((bits_per_sample != 8) &&
       (bits_per_sample != 16))
   {
      /* Unsupported sample depth. */
      return (2);
   }

   /* Open file. */
   dsp_wav_file = fopen (filename, "wb");
   if (!dsp_wav_file)
      return (3);

   /* Skip header space. */
   fseek (dsp_wav_file, WAV_HEADER_SIZE, SEEK_SET);

   /* Clear size counter. */
   dsp_wav_size = 0;

   /* Set parameters. */
   dsp_wav_sample_rate     = sample_rate;
   dsp_wav_channels        = channels;
   dsp_wav_bits_per_sample = bits_per_sample;

   /* Return success. */
   return (0);
}

void dsp_close_wav (void)
{
   DEBUG_PRINTF("dsp_close_wav()\n");

   if (dsp_wav_file)
   {
      dsp_wav_riff_type_chunk_t riff;
      dsp_wav_format_chunk_t    fmt;
      dsp_wav_data_chunk_t      data;
      unsigned size;

      /* Build RIFF chunks. */

      riff.chunk_id   = DSP_WAV_ID('R','I','F','F');
      riff.chunk_size = ((WAV_HEADER_SIZE + dsp_wav_size) - 8);
      riff.riff_type  = DSP_WAV_ID('W','A','V','E');

      fmt.chunk_id          = DSP_WAV_ID('f','m','t',' ');
      fmt.chunk_size        = (sizeof (fmt) - 8);
      fmt.format_tag        = 1;  /* No compression. */
      fmt.channels          = dsp_wav_channels;
      fmt.samples_per_sec   = dsp_wav_sample_rate;
      fmt.avg_bytes_per_sec = ((dsp_wav_sample_rate * dsp_wav_channels) *
         (dsp_wav_bits_per_sample / 8));
      fmt.block_align       = (dsp_wav_channels * (dsp_wav_bits_per_sample /
         8));
      fmt.bits_per_sample   = dsp_wav_bits_per_sample;
      
      data.chunk_id   = DSP_WAV_ID('d','a','t','a');
      data.chunk_size = ((sizeof (data) + dsp_wav_size) - 8);

      /* Write WAV header. */

      fseek (dsp_wav_file, 0, SEEK_SET);

      fwrite (&riff, sizeof (riff), 1, dsp_wav_file);
      fwrite (&fmt,  sizeof (fmt),  1, dsp_wav_file);
      fwrite (&data, sizeof (data), 1, dsp_wav_file);

      /* Close file. */
      fclose (dsp_wav_file);
   }
}

static void dsp_wav_write (void)
{
   unsigned size;
   void *buffer;

   DEBUG_PRINTF("dsp_wav_write()\n");

   if (!dsp_wav_file)
      return;

   /* Calculate buffer size. */
   size = ((dsp_buffer_samples * dsp_wav_channels) *
      (dsp_wav_bits_per_sample / 8));

   /* Allocate rendering buffer. */

   buffer = malloc (size);
   if (!buffer)
      WARN_BREAK_GENERIC();

   /* Render waveform. */
   dsp_render (buffer, dsp_wav_channels, dsp_wav_bits_per_sample,
      ((dsp_wav_bits_per_sample == 8) ? TRUE : FALSE));

   /* Write rendering buffer to disk. */
   fwrite (buffer, size, 1, dsp_wav_file);

   /* Increment size counter. */
   dsp_wav_size += size;

   /* Free rendering buffer. */
   free (buffer);
}
