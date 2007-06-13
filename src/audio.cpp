/* FakeNES - A free, portable, Open Source NES emulator.

   audio.cpp: Implementation of the audio interface.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "apu.h"
#include "audio.h"
#include "audio_int.h"
#include "audiolib.h"
#include "common.h"
#include "debug.h"
#include "gui.h"
#include "timing.h"
#include "types.h"

// TODO: .WAV recording functions lifted from the old DSP code.

/* Note that, whenever something is marked as "Read-only outside of the audio system", it means that it should NEVER be
   modified outside of audio.c, audio.h(inline functions only), and audiolib.c.
   If such variables are modified, it could cause problems. */

/* Also note that in the context used here, 'frames' refers to a single sample (mono) or sample pair (stereo) - not a whole
   frame of NES audio as the name might suggest. ;) (A concept that the audio system actually does not understand.) */

// Audio options.  Usually these work more as strong suggestions rather than hard requirements.
audio_options_t audio_options = {
   TRUE,                    // Enable output
   AUDIO_SUBSYSTEM_ALLEGRO, // Subsystem
   -1,                      // Prefered sample rate (Autodetect)
   -1,                      // Prefered buffer length(ms) (Autodetect)
};

// Number of channels.  This usually comes from the APU options.  Read-only outside of the audio system.
int audio_channels = 0;

// Sample rate.  Set by the audio driver.  Read-only outside of the audio system.
int audio_sample_rate = 0;

// Bits per sample.  Set by the audio driver.  Read-only outside of the audio system.
int audio_sample_bits = 0;

/* Whether samples are signed or not.  Set by the audio driver.  Read-only outside of the audio system.

   This only affects the format of samples that are sent to the active subsystem via the audio buffer; it does not affect
   samples in the queue (which should always be unsigned; see below). */
BOOL audio_signed_samples = FALSE;

// Buffer length (in milliseconds).  Set by the audio driver.  Read-only outside of the audio system.
int audio_buffer_length_ms = 0;

// Buffer sizes.  Read-only outside of the audio system.
unsigned audio_buffer_size_frames = 0;  // numframes
unsigned audio_buffer_size_samples = 0; // numframes*numchannels
unsigned audio_buffer_size_bytes = 0;   // numframes*numchannels*(bitspersample/8)

/* FIFO queue to hold audio samples until they can be transfered to the audio buffer.
   Read-only outside of the audio system.

   Samples in the queue should be stored in unsigned 16-bit format.  Any pre-processing such as stereo blending must be done
   prior to storing the samples to the queue, as it won't be done automatically.

   Use audio_queue_sample() (defined in audio.h) to write to the queue in a performance-efficient way.  However, since the
   queue is stored as a vector, that function will only work from C++ code. */
std::vector<uint16> audioQueue;

// Audio buffer, for transfering samples from the queue to the active subsystem in the appropriate format.
static void* audioBuffer = null;

// The number of frames currently present in the audio buffer.
static unsigned audioBufferedFrames = 0;

// Frame rate counter.
volatile int audio_fps = 0;

// Variables for the WAV writer(see bottom).
static FILE* wavFile = null;
static unsigned wavSize = 0;

void audio_load_config(void)
{
   DEBUG_PRINTF("audio_load_config()\n");

   audio_options.enable_output         = true_or_false(get_config_int("audio", "enable_output", audio_options.enable_output));
   audio_options.subsystem             = get_config_int("audio", "subsystem",        audio_options.subsystem);
   audio_options.sample_rate_hint      = get_config_int("audio", "sample_rate",      audio_options.sample_rate_hint);
   audio_options.buffer_length_ms_hint = get_config_int("audio", "buffer_length_ms", audio_options.buffer_length_ms_hint);
}

void audio_save_config(void)
{
   DEBUG_PRINTF("audio_save_config()\n");

   set_config_int("audio", "enable_output",    (audio_options.enable_output ? 1 : 0));
   set_config_int("audio", "subsystem",        audio_options.subsystem);
   set_config_int("audio", "sample_rate",      audio_options.sample_rate_hint);
   set_config_int("audio", "buffer_length_ms", audio_options.buffer_length_ms_hint);
}

int audio_init(void)
{
   DEBUG_PRINTF("audio_init()\n");

   if(!audio_options.enable_output) {
      // Audio output is disabled - bail out.
      return 0;
   }

   // Determine number of channels.
   audio_channels = (apu_options.stereo ? 2 : 1);

   // Initialize audio library.
   int result = audiolib_init();
   if(result != 0) {
      WARN("I'm sorry, but I couldn't find a suitable audio driver. :<\n"
           "\n"
           "Possible causes for this problem:\n"
           "  - There is no sound hardware present\n"
           "  - The installed sound drivers are not working properly\n"
           "  - The sound API (e.g Allegro or OpenAL) is not cooperating for some reason\n"
           "  - The sound system is already in use by another application\n"
           "\n"
           "Usually, the sound system is just in use by another application.\n"
           "Try again in a few minutes. :)");

      audio_exit();
      return (8 + result);
   }

   // Determine buffer sizes.
   audio_buffer_size_frames = (unsigned)ROUND((audio_sample_rate / 1000.0) * audio_buffer_length_ms);
   audio_buffer_size_samples = (audio_buffer_size_frames * audio_channels);
   audio_buffer_size_bytes = (audio_buffer_size_samples * (audio_sample_bits / 8));
 
   // Allocate buffer.
   audioBuffer = malloc(audio_buffer_size_bytes);
   if(!audioBuffer) {
      WARN("Couldn't allocate audio buffer (out of memory?)");
      audio_exit();
      return 1;
   }

   // Clear buffer.
   memset(audioBuffer, 0, audio_buffer_size_bytes);

   // Begin playing.
   result = audiolib_open_stream();
   if(result != 0) {
      WARN("Call to audiolib_open_stream() failed");
      audio_exit();
      return (16 + result);
   }

   // Return success.
   return 0;
}

void audio_exit(void)
{
   DEBUG_PRINTF("audio_exit()\n");

   // Deinitialize audio library.
   audiolib_exit();

   if(audioBuffer) {
      // Destroy audio buffer.
      free(audioBuffer);
      audioBuffer = null;
      // Clear the frame counter.
      audioBufferedFrames = 0;
   }

   if(audioQueue.size() > 0) {
      // Clear queue.
      audioQueue.clear();
   }
}

void audio_update(void)
{
   // Audio update function, called once per scanline. 
   DEBUG_PRINTF("audio_update()\n");

   if(!audio_options.enable_output)
      return;

   if(audioBufferedFrames < audio_buffer_size_frames) {
      // Determine how many frames are available in the queue.
      const unsigned queuedFrames = (audioQueue.size() / audio_channels);
      if(queuedFrames > 0) {
         // Determine how many frames are available in the buffer.
         const unsigned bufferableFrames = (audio_buffer_size_frames - audioBufferedFrames);
         // Determine the number of frames to copy to the buffer.
         const unsigned framesToCopy = min(queuedFrames, bufferableFrames);

         // Copy frames to the buffer.
         for(unsigned frame = 0; frame < framesToCopy; frame++) {
            // Read/write base addresses (read from queue, write to buffer).
            const unsigned readBase = (frame * audio_channels);
            // audioBufferedFrames changes within the loop(see below), so all we have to do is use it as a write pointer.
            const unsigned writeBase = (audioBufferedFrames * audio_channels);

            for(int channel = 0; channel < audio_channels; channel++) {
               // Fetch a sample from the queue.
               uint16 sample = audioQueue[readBase + channel];

               if(audio_signed_samples) {
                  // Convert to signed.
                  sample ^= 0x8000;
               }

               /* Determine our write offset for the buffer (this remains constant regardless of the value of sample_bits
                  since we cast the buffer to an appropriately sized data type). */
               const unsigned writeOffset = (writeBase + channel);

               // Write our sample to the buffer.
               switch(audio_sample_bits) {
                  case 8: {
                     uint8* buffer = (uint8*)audioBuffer;
                     buffer[writeOffset] = (sample >> 8);

                     break;
                  }

                  case 16: {
                     uint16* buffer = (uint16*)audioBuffer;
                     buffer[writeOffset] = sample;

                     break;
                  }

                  default:
                     WARN_GENERIC();
               }
            }

            // Increment frame counter.
            audioBufferedFrames++;
         }

         // Determine how many samples we copied.
         const unsigned samplesCopied = (framesToCopy * audio_channels);
         const unsigned samplesRemaining = (audioQueue.size() - samplesCopied);
         /* Removed copied samples from the queue.
            Thanks KittyCat! =^-^= */
         memcpy(&audioQueue[0], &audioQueue[samplesCopied], (sizeof(uint16) * samplesRemaining));
         audioQueue.resize(samplesRemaining);
      }
   }

   // Check if the buffer is full.
   if(audioBufferedFrames == audio_buffer_size_frames) {
      // See if we can update the external buffer yet.
      void *audiolibBuffer = audiolib_get_buffer();
      if(audiolibBuffer) {
         // Copy to the external buffer.
         memcpy(audiolibBuffer, audioBuffer, audio_buffer_size_bytes);
         // Let the subsystem have it.
         audiolib_free_buffer();

         // If we're recording a .WAV file, go ahead and write the buffer out to the disk.
         if(wavFile) {
            fwrite(audioBuffer, 1, audio_buffer_size_bytes, wavFile);
            wavSize += audio_buffer_size_bytes;
         }

         // Empty the internal buffer.
         audioBufferedFrames = 0;

         audio_fps += audio_buffer_size_frames;
      }
   }
}

void audio_suspend(void)
{
   DEBUG_PRINTF("audio_suspend()\n");

   if (!audio_options.enable_output)
      return;

   audiolib_suspend();
}

void audio_resume(void)
{
   DEBUG_PRINTF("audio_resume()\n");

   if (!audio_options.enable_output)
      return;

   audiolib_resume();
}

// --- WAV recording functions. ---
/* TODO: Fix this stuff up to work properly on big-endian platforms
   (currently it produces a big-endian ordered WAV file, I think). */

// TODO: Move this stuff into another file?
// 
typedef struct _WAVRIFFTypeChunk {
   uint32 chunkID;
   int32 chunkSize;
   uint32 riffType;

} WAVRIFFTypeChunk;

typedef struct _WAVFormatChunk {
   uint32 chunkID;
   int32 chunkSize;
   int16 formatTag;
   uint16 channels;
   uint32 samplesPerSec;
   uint32 avgBytesPerSec;
   uint16 blockAlign;
   uint16 bitsPerSample;

} WAVFormatChunk;

typedef struct _WAVDataChunk {
   uint32 chunkID;
   int32 chunkSize;

} WAVDataChunk;

#define WAV_ID(a,b,c,d) ((d << 24) | (c << 16) | (b << 8) | a)

#define WAV_HEADER_SIZE (sizeof(WAVRIFFTypeChunk) + \
                         sizeof(WAVFormatChunk) + \
                         sizeof(WAVDataChunk))

int audio_open_wav(const UCHAR* filename)
{
   /* Open file. */
   wavFile = fopen(filename, "wb");
   if(!wavFile)
      return 1;

   // Skip header space.
   fseek(wavFile, WAV_HEADER_SIZE, SEEK_SET);

   // Clear size counter.
   wavSize = 0;

   // Return success.
   return 0;
}

void audio_close_wav(void)
{
   if(wavFile) {
      // Write header.
      fseek(wavFile, 0, SEEK_SET);

      WAVRIFFTypeChunk riff;
      riff.chunkID = WAV_ID('R','I','F','F');
      riff.chunkSize = ((WAV_HEADER_SIZE + wavSize) - 8);
      riff.riffType = WAV_ID('W','A','V','E');
      fwrite(&riff, sizeof(riff), 1, wavFile);

      WAVFormatChunk fmt;
      fmt.chunkID = WAV_ID('f','m','t',' ');
      fmt.chunkSize = (sizeof(fmt) - 8);
      fmt.formatTag = 1; // No compression.
      fmt.channels = audio_channels;
      fmt.samplesPerSec = audio_sample_rate;
      fmt.avgBytesPerSec = ((audio_sample_rate * audio_channels) * (audio_sample_bits / 8));
      fmt.blockAlign = (audio_channels * (audio_sample_bits / 8));
      fmt.bitsPerSample = audio_sample_bits;
      fwrite(&fmt,  sizeof(fmt),  1, wavFile);
      
      WAVDataChunk data;
      data.chunkID = WAV_ID('d','a','t','a');
      data.chunkSize = ((sizeof(data) + wavSize) - 8);
      fwrite(&data, sizeof(data), 1, wavFile);

      // Close file.
      fclose(wavFile);
      wavFile = null;
      // Clear counter.
      wavSize = 0;
   }
}
