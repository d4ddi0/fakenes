/* FakeNES - A portable, Open Source NES and Famicom emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "APU.h"
#include "Audio.h"
#include "AudioLib.hpp"
#include "Local.hpp"

/* TODO: Fix WAV recording stuff up to work properly on big-endian platforms(currently it produces a big-endian ordered
         WAV file, I think). */


/* Note that, whenever something is marked as "Read-only outside of the audio system", it means that it should NEVER be
   modified outside of audio.c, audio.h(inline functions only), and audiolib.c.
   If such variables are modified, it could cause problems. */

/* Also note that in the context used here, 'frames' refers to a single sample (mono) or sample pair (stereo) - not a whole
   frame of NES audio as the name might suggest. ;) (A concept that the audio system actually does not understand.) */

// Audio options.  Usually these work more as strong suggestions rather than hard requirements.
audio_options_t audio_options = {
   TRUE,                      // Enable output
   AUDIO_SUBSYSTEM_AUTOMATIC, // Subsystem
   -1,                        // Prefered sample rate (Autodetect)
   -1,                        // Prefered buffer length(ms) (Autodetect)
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
static FILE_CONTEXT* wavFile = null;
static unsigned wavSize = 0;

/* Visualization buffer.  This is an actual ring buffer (not a fake one like the audio buffer) into which all data from the
   audio queue eventually passes when visualization is enabled. */
static unsigned audioVisBufferSize = 0;
static uint16* audioVisBuffer = null;
static unsigned audioVisBufferOffset = 0;
static unsigned audioVisBufferHead = 0;
static unsigned audioVisBufferTail = 0;

void audio_load_config(void)
{
   DEBUG_PRINTF("audio_load_config()\n");

   audio_options.enable_output         = get_config_bool("audio", "enable_output",    audio_options.enable_output);
   audio_options.subsystem             = get_config_int ("audio", "subsystem",        audio_options.subsystem);
   audio_options.sample_rate_hint      = get_config_int ("audio", "sample_rate",      audio_options.sample_rate_hint);
   audio_options.buffer_length_ms_hint = get_config_int ("audio", "buffer_length_ms", audio_options.buffer_length_ms_hint);
}

void audio_save_config(void)
{
   DEBUG_PRINTF("audio_save_config()\n");

   set_config_bool("audio", "enable_output",    audio_options.enable_output);
   set_config_int ("audio", "subsystem",        audio_options.subsystem);
   set_config_int ("audio", "sample_rate",      audio_options.sample_rate_hint);
   set_config_int ("audio", "buffer_length_ms", audio_options.buffer_length_ms_hint);
}

int audio_init(void)
{
   DEBUG_PRINTF("audio_init()\n");

   if(!audio_options.enable_output) {
      // Audio output is disabled - bail out.
      return 0;
   }

   // Determine number of channels.
   audio_channels = apu_options.stereo ? 2 : 1;

   // Initialize audio library.
   int result = audiolib_init();
   if(result != 0) {
      WARN("I'm sorry, but I couldn't find a suitable audio driver. :<\n"
           "\n"
           "Possible causes for this problem:\n"
           "  - There is no sound hardware present\n"
           "  - The installed sound drivers are not working properly\n"
           "  - The sound API (e.g Allegro, SDL or OpenAL) is not cooperating for some reason\n"
           "  - The sound system is already in use by another application\n"
           "\n"
           "Usually, the sound system is just in use by another application.\n"
           "Try again in a few minutes. :)");

      audio_exit();
      return 8 + result;
   }

   // Determine buffer sizes.
   audio_buffer_size_frames = (unsigned)Round((audio_sample_rate / 1000.0) * audio_buffer_length_ms);
   audio_buffer_size_samples = audio_buffer_size_frames * audio_channels;
   audio_buffer_size_bytes = audio_buffer_size_samples * (audio_sample_bits / 8);
 
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
      return 16 + result;
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

   if(wavFile)
      audio_close_wav();

   if(audioVisBuffer)
      audio_visclose();
}

void audio_update(void)
{
   // Audio update function, called once per scanline. 
   DEBUG_PRINTF("audio_update()\n");

   if(!audio_options.enable_output)
      return;

   // Check if the buffer is full.
   if(audioBufferedFrames == audio_buffer_size_frames) {
      // See if we can update the driver buffer yet.
      void* audiolibBuffer = audiolib_get_buffer(audioBuffer);
      if(audiolibBuffer) {
         if(audiolibBuffer != audioBuffer) {
            // Copy to external buffer.
            memcpy(audiolibBuffer, audioBuffer, audio_buffer_size_bytes);
         }

         // Let the subsystem have it.
         audiolib_free_buffer(audiolibBuffer);

         // Empty the internal buffer.
         audioBufferedFrames = 0;

         audio_fps += audio_buffer_size_frames;
      }
      else {
         /* This code simulates a ring buffer for when the output system can't keep up with the emulation (e.g while using
            fast forward), by scrapping the oldest data in the buffer and adding the new data(see below).

            This is functionally identical to using an actual ring buffer, though not nearly as efficient. */

         // Determine how many frames are available in the queue.
         const unsigned queuedFrames = audioQueue.size() / audio_channels;
         if(queuedFrames > 0) {
            // Determine how many frames we want to make room for.
            const unsigned framesToAdd = Minimum<unsigned>(queuedFrames, audio_buffer_size_frames);
            // Make room for the frames in the buffer.
            const unsigned framesToMove = audioBufferedFrames - framesToAdd;
            if(framesToMove > 0) {
               const unsigned copyBase = ((audioBufferedFrames - framesToMove) * audio_channels) * (audio_sample_bits / 8);
               const unsigned samplesToMove = framesToMove * audio_channels;
               const unsigned bytesToMove = samplesToMove * (audio_sample_bits / 8);

               uint8* buffer = (uint8*)audioBuffer;
               memcpy(&buffer[0], &buffer[copyBase], bytesToMove);
            }

            audioBufferedFrames -= framesToAdd;
         }
      }
   }

   if(audioBufferedFrames < audio_buffer_size_frames) {
      // Determine how many frames are available in the queue.
      const unsigned queuedFrames = audioQueue.size() / audio_channels;
      if(queuedFrames > 0) {
         // Determine how many frames are available in the buffer.
         const unsigned bufferableFrames = audio_buffer_size_frames - audioBufferedFrames;
         // Determine the number of frames to copy to the buffer.
         const unsigned framesToCopy = Minimum<unsigned>(queuedFrames, bufferableFrames);

         // Copy frames to the buffer.
         for(unsigned frame = 0; frame < framesToCopy; frame++) {
            // Read/write base addresses (read from queue, write to buffer).
            const unsigned readBase = frame * audio_channels;
            // audioBufferedFrames changes within the loop(see below), so all we have to do is use it as a write pointer.
            const unsigned writeBase = audioBufferedFrames * audio_channels;

            for(int channel = 0; channel < audio_channels; channel++) {
               // Fetch a sample from the queue.
               uint16 sample = audioQueue[readBase + channel];

               if(audioVisBuffer) {
                  // Buffer it for visualization.
                  audioVisBuffer[audioVisBufferOffset] = sample;
                  audioVisBufferOffset++;
                  if(audioVisBufferOffset > audioVisBufferTail) { 
                     // The buffer has filled up.
                     if(audioVisBufferOffset > (audioVisBufferSize - 1))
                        audioVisBufferOffset = 0;

                     // Move head and tail, wrapping around if neccessary.
                     audioVisBufferHead++;
                     if(audioVisBufferHead > (audioVisBufferSize - 1))
                        audioVisBufferHead = 0;

                     audioVisBufferTail++;
                     if(audioVisBufferTail > (audioVisBufferSize - 1))
                        audioVisBufferTail = 0;
                  }
               }

               if(audio_signed_samples) {
                  // Convert to signed.
                  sample ^= 0x8000;
               }

               /* Determine our write offset for the buffer (this remains constant regardless of the value of sample_bits
                  since we cast the buffer to an appropriately sized data type). */
               const unsigned writeOffset = writeBase + channel;

               // Write our sample to the buffer.
               switch(audio_sample_bits) {
                  case 8: {
                     // Reduce to 8 bits.
                     sample >>= 8;

                     uint8* buffer = (uint8*)audioBuffer;
                     buffer[writeOffset] = sample;

                     if(wavFile) {
                        if(audio_signed_samples) {
                           // Convert to unsigned.
                           sample ^= 0x80;
                        }

                        wavFile->write_byte(wavFile, sample);
                        wavSize++;
                     }

                     break;
                  }

                  case 16: {
                     uint16* buffer = (uint16*)audioBuffer;
                     buffer[writeOffset] = sample;

                     if(wavFile) {
                        if(!audio_signed_samples) {
                           // Convert to signed.
                           sample ^= 0x8000;
                        }

                        // putc(sample & 0xFF, wavFile);
                        // putc((sample & 0xFF00) >> 8, wavFile);
			wavFile->write_word(wavFile, sample);
                        wavSize += 2;
                     }

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
         const unsigned samplesCopied = framesToCopy * audio_channels;
         const unsigned samplesRemaining = audioQueue.size() - samplesCopied;
         /* Removed copied samples from the queue.
            Thanks KittyCat! =^-^= */
         memcpy(&audioQueue[0], &audioQueue[samplesCopied], sizeof(uint16) * samplesRemaining);
         audioQueue.resize(samplesRemaining);
      }
   }

   // Check if the buffer is full.
   if(audioBufferedFrames == audio_buffer_size_frames) {
      // See if we can update the driver buffer yet.
      void* audiolibBuffer = audiolib_get_buffer(audioBuffer);
      if(audiolibBuffer) {
         if(audiolibBuffer != audioBuffer) {
            // Copy to external buffer.
            memcpy(audiolibBuffer, audioBuffer, audio_buffer_size_bytes);
         }

         // Let the subsystem have it.
         audiolib_free_buffer(audiolibBuffer);

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

int audio_open_wav(const UTF_STRING* filename)
{
   Safeguard(filename);

   /* Open file. */
   wavFile = open_file(filename, FILE_MODE_WRITE, FILE_ORDER_INTEL);
   if(!wavFile)
      return 1;

   // Skip header space.
   wavFile->seek_to(wavFile, WAV_HEADER_SIZE);

   // Clear size counter.
   wavSize = 0;

   // Return success.
   return 0;
}

void audio_close_wav(void)
{
   if(wavFile) {
      // Write header.
      wavFile->seek_to(wavFile, 0);

      WAVRIFFTypeChunk riff;
      riff.chunkID = WAV_ID('R','I','F','F');
      riff.chunkSize = (WAV_HEADER_SIZE + wavSize) - 8;
      riff.riffType = WAV_ID('W','A','V','E');
      wavFile->write(wavFile, (void*)&riff, sizeof(riff));

      /* Available formats (compressions):
         0x0001 	WAVE_FORMAT_PCM 	PCM
         0x0003 	WAVE_FORMAT_IEEE_FLOAT 	IEEE float
         0x0006 	WAVE_FORMAT_ALAW 	8-bit ITU-T G.711 A-law
         0x0007 	WAVE_FORMAT_MULAW 	8-bit ITU-T G.711 µ-law
         0xFFFE 	WAVE_FORMAT_EXTENSIBLE 	Determined by SubFormat */
      WAVFormatChunk fmt;
      fmt.chunkID = WAV_ID('f','m','t',' ');
      fmt.chunkSize = sizeof(fmt) - 8;
      fmt.formatTag = 0x0001; // No compression.
      fmt.channels = audio_channels;
      fmt.samplesPerSec = audio_sample_rate;
      fmt.avgBytesPerSec = (audio_sample_rate * audio_channels) * (audio_sample_bits / 8);
      fmt.blockAlign = audio_channels * (audio_sample_bits / 8);
      fmt.bitsPerSample = audio_sample_bits;
      wavFile->write(wavFile, (void*)&fmt, sizeof(fmt));
      
      WAVDataChunk data;
      data.chunkID = WAV_ID('d','a','t','a');
      data.chunkSize = (sizeof(data) + wavSize) - 8;
      wavFile->write(wavFile, (void*)&data, sizeof(data));

      // Close file.
      wavFile->close(wavFile);
      wavFile = null;
      // Clear counter.
      wavSize = 0;
   }
}

// --- Visualization support ---
void audio_visopen(unsigned num_frames)
{
   // Attempts to open a visualization buffer (no error checking - use audio_get_visdata() for that instead).
   audioVisBufferSize = num_frames * audio_channels;
   audioVisBuffer = new uint16[audioVisBufferSize];
}

void audio_visclose(void)
{
   if(audioVisBuffer) {
      // Destroy visualization buffer.
      delete[] audioVisBuffer;
      audioVisBuffer = null;
   }

   audioVisBufferSize = 0;
}


BOOL audio_is_visopen(void)
{
   return (audioVisBufferSize > 0);
}

UINT16* audio_get_visdata(void)
{
   // Gets visualization data from the visualization buffer.  Used by the NSF player, but might be used by the normal
   // HUD later on.
   // Remember to delete[] it when you're done with it!
   
   if(!audioVisBuffer) {
      WARN_GENERIC();
      return null;
   }

   UINT16* visdata = new UINT16[audioVisBufferSize];
   if(!visdata) {
      WARN_GENERIC();
      return null;
   }

   memcpy(&visdata[0], &audioVisBuffer[0], audioVisBufferSize * sizeof(uint16));
   return visdata;
}
