/* FakeNES - A free, portable, Open Source NES emulator.

   nsf.cpp: Back-end for the NSF player.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "apu.h"
#include "audio.h"
#include "audio_int.h"
#include "common.h"
#include "cpu.h"
#include "input.h"
#include "log.h"
#include "nsf.h"
#include "ppu.h"
#include "timing.h"
#include "types.h"
#include "video.h"

// TODO: Proper support for PAL/NTSC selection.

// Size and number of NSF banks in the $8000 - $FFFF area.
static const unsigned NSFBankSize = 4096; // 4kiB
static const int NSFBankCount = 8;

// Maximum size of NSF data, including load offset.
static const unsigned NSFDataSize = 32768; // 32kiB

// Structure to hold information about our NSF state.
typedef struct _NSF {
   // All of these fields are loaded directly from the file.
   uint8 totalSongs;
   uint8 startingSong;
   uint16 loadAddress;
   uint16 initAddress;
   uint16 playAddress;
   uint8 name[32];
   uint8 artist[32];
   uint8 copyright[32];
   uint16 speedNTSC;
   uint8 bankswitch[8];
   uint16 speedPAL;
   uint8 regionFlags;
   uint8 expansionFlags;

   // Determined from the values of bankswitch[] above.   
   bool bankswitched;
   // Data to be mapped into banks, or directly into the 6502's address space in the case of non-bankswitched NSFs.
   uint8 data[NSFDataSize];

} NSF;

static NSF nsf;

/* Region flags:
      bit 0: if clear, this is an NTSC tune
      bit 0: if set, this is a PAL tune
      bit 1: if set, this is a dual PAL/NTSC tune
      bits 2-7: not used. they *must* be 0 */
enum {
   NSFRegionPAL  = (1 << 0),
   NSFRegionDual = (1 << 1),
};

/* Expansion flags:
      bit 0: if set, this song uses VRCVI
      bit 1: if set, this song uses VRCVII
      bit 2: if set, this song uses FDS Sound
      bit 3: if set, this song uses MMC5 audio
      bit 4: if set, this song uses Namco 106
      bit 5: if set, this song uses Sunsoft FME-07 */
enum {
   NSFExpansionVRC6 = (1 << 0),
   NSFExpansionVRC7 = (1 << 1),
   NSFExpansionFDS  = (1 << 2),
   NSFExpansionMMC5 = (1 << 3),
   NSFExpansionN106 = (1 << 4),
   NSFExpansionFME7 = (1 << 5),
};

// Function prototypes(defined at bottom).
static void bankswitch(int bankIn, int bankOut);
static void play(int song);
static real bandpass(uint16* buffer, unsigned size, real sampleRate, real cutoffLow, real cutoffHigh);

// Opening/closing.
BOOL nsf_open(const UCHAR* filename)
{
   // Open file.
   PACKFILE* file = pack_fopen(filename, "r");
   if(!file) {
      log_printf("NSF: nsf_open(): File not found ('%s').\n", filename);
      nsf_close();
      return false;
   }

   unsigned bytesRead = 0;

   /* offset  # of bytes   Function
      ----------------------------
      0000    5   STRING  "NESM",01Ah  ; denotes an NES sound format file */
   uint8 signature[5];
   pack_fread(signature, sizeof(signature), file);
   bytesRead += sizeof(signature);

   // Validate signature.
   if(strncmp((const char*)&signature[0], "NESM\x1A", 5) != 0) {
      log_printf("NSF: nsf_open(): NSF signature is invalid.\n");
      pack_fclose(file);
      nsf_close();
      return false;
   }

   // 0005    1   BYTE    Version number (currently 01h)
   const uint8 version = pack_getc(file);
   bytesRead++;

   // Check version number.
   if(version > 0x01) {
      log_printf("NSF: nsf_open(): NSF version is unsupported.\n");
      pack_fclose(file);
      nsf_close();
      return false;
   }

   // Clear NSF data.
   memset(&nsf, 0, sizeof(nsf));

   // 0006    1   BYTE    Total songs   (1=1 song, 2=2 songs, etc)
   nsf.totalSongs = pack_getc(file);
   bytesRead++;
   // 0007    1   BYTE    Starting song (1= 1st song, 2=2nd song, etc)
   nsf.startingSong = pack_getc(file);
   bytesRead++;

   // 0008    2   WORD    (lo/hi) load address of data (8000-FFFF)
   nsf.loadAddress = pack_igetw(file);
   bytesRead += 2;
   // 000a    2   WORD    (lo/hi) init address of data (8000-FFFF)
   nsf.initAddress = pack_igetw(file);
   bytesRead += 2;
   // 000c    2   WORD    (lo/hi) play address of data (8000-FFFF)
   nsf.playAddress = pack_igetw(file);
   bytesRead += 2;

   // 000e    32  STRING  The name of the song, null terminated
   pack_fread(nsf.name, sizeof(nsf.name), file);
   bytesRead += sizeof(nsf.name);
   // 002e    32  STRING  The artist, if known, null terminated
   pack_fread(nsf.artist, sizeof(nsf.artist), file);
   bytesRead += sizeof(nsf.artist);
   // 004e    32  STRING  The Copyright holder, null terminated
   pack_fread(nsf.copyright, sizeof(nsf.copyright), file);
   bytesRead += sizeof(nsf.copyright);

   // 006e    2   WORD    (lo/hi) speed, in 1/1000000th sec ticks, NTSC
   nsf.speedNTSC = pack_igetw(file);
   bytesRead += 2;

   // 0070    8   BYTE    Bankswitch Init Values
   pack_fread(nsf.bankswitch, sizeof(nsf.bankswitch), file);
   bytesRead += sizeof(nsf.bankswitch);

   // Determine if tune is bankswitched.  Ugly, but it works.
   nsf.bankswitched = ((nsf.bankswitch[0] != 0x00) ||
                       (nsf.bankswitch[1] != 0x00) ||
                       (nsf.bankswitch[2] != 0x00) ||
                       (nsf.bankswitch[3] != 0x00) ||
                       (nsf.bankswitch[4] != 0x00) ||
                       (nsf.bankswitch[5] != 0x00) ||
                       (nsf.bankswitch[6] != 0x00) ||
                       (nsf.bankswitch[7] != 0x00));

   // 0078    2   WORD    (lo/hi) speed, in 1/1000000th sec ticks, PAL
   nsf.speedPAL = pack_igetw(file);
   bytesRead += 2;

   //  007a    1   BYTE    PAL/NTSC bits
   nsf.regionFlags = pack_getc(file);
   bytesRead++;

   // 007b    1   BYTE    Extra Sound Chip Support
   nsf.expansionFlags = pack_getc(file);
   bytesRead++;

   // 007c    4   ----    4 extra bytes for expansion (must be 00h)
   uint8 unused[4];
   pack_fread(unused, sizeof(unused), file);
   bytesRead += sizeof(unused);

   // 0080    nnn ----    The music program/data follows
   unsigned offset = (nsf.loadAddress & 0x0FFF); // $8000 -> $0000
   while(!pack_feof(file)) {
      nsf.data[offset++] = pack_getc(file);
      bytesRead++;
   }

   // Close file.
   pack_fclose(file);

   log_printf("NSF: nsf_open(): Loaded %u bytes from '%s'.\n", bytesRead, filename);

   // Return success.
   return true;
}

void nsf_close(void)
{
   // Do nothing.
}

// Main loop.
static volatile int nsfFrameTicks = 0;

static void nsfFrameTimer(void)
{
   nsfFrameTicks++;
}
END_OF_STATIC_FUNCTION(nsfFrameTimer);

void nsf_main(void)
{
   /* To keep everything in sync, we have to clock the playback timer via the scanline timer, which is in turn clocked by
      the frame master timer. */

   // Switch to direct timing mode so that speed modifiers and the like do not skew the NSF timing.
   const ENUM saved_timing_mode = timing_mode;
   timing_mode = TIMING_MODE_DIRECT;
   timing_update_mode();

   // Calculate beats per second (BPM) for the frame timer.
   const real frameBPM = timing_get_speed();

   // Determine how many scanlines to a frame.
   const real scanlinesPerFrame = ((machine_type == MACHINE_TYPE_PAL) ? TOTAL_LINES_PAL : TOTAL_LINES_NTSC);
   const real scanlineBPM = (scanlinesPerFrame *  frameBPM);

   // Determine how often to update the playback timer, in scanline counts.
   real playbackTimer = 0.0;
   const real playbackBPM = (1000000.0 / ((machine_type == MACHINE_TYPE_PAL) ? nsf.speedPAL : nsf.speedNTSC));
   const real playbackPeriod = (scanlineBPM / playbackBPM);

   // Jump to the starting song.
   int currentSong = nsf.startingSong;
   play(currentSong);

   // Queue first playback cycle.
   cpu_context.PC.word = nsf.playAddress;
   playbackTimer += playbackPeriod;

   // How many samples to use for visualization(per frame). */
   const unsigned visualizationSamples = (int)floor(audio_sample_rate / frameBPM);

   // Enable visualization.
   audio_visopen(visualizationSamples);

   /* 0 = none (top left),
      1 = power bars (bottom left),
      2 = frequency spectrum (bottom right),
      3 = waveform (top right) */
   static const int visualizationNone = 0;
   static const int visualizationPowerBars = 1;
   static const int visualizationFrequencySpectrum = 2;
   static const int visualizationWaveform = 3;
   int currentVisualization = visualizationWaveform;

   // Install frame timer.
   LOCK_VARIABLE(nsfFrameTicks);
   LOCK_FUNCTION(nsfFrameTimer);
   install_int_ex(nsfFrameTimer, BPS_TO_TIMER(frameBPM));

   bool buttonLeft = false, buttonRight = false, buttonUp = false, buttonDown = false;
   bool buttonA = false, buttonB = false, buttonSelect = false;

   while(!buttonSelect) {
      int cached_nsfFrameTicks = nsfFrameTicks;
      nsfFrameTicks -= cached_nsfFrameTicks;

      if(cached_nsfFrameTicks > 0) {
         while(cached_nsfFrameTicks > 0) {
            cached_nsfFrameTicks--;

            // Checking our inputs only once per frame should be enough without introducing additional overhead.
            input_process();

            // Directional controls select between the visualizations in a rather unterrific way.
            bool buttonLeftX = input_get_button_state(INPUT_PLAYER_1, INPUT_BUTTON_LEFT);
            if(buttonLeftX &&
               (buttonLeftX != buttonLeft)) {
               // frequency spectrum->power bars
               if(currentVisualization == visualizationFrequencySpectrum)
                  currentVisualization = visualizationPowerBars;
               // waveform->none
               if(currentVisualization == visualizationWaveform)
                  currentVisualization = visualizationNone;
            }

            buttonLeft = buttonLeftX;

            bool buttonRightX = input_get_button_state(INPUT_PLAYER_1, INPUT_BUTTON_RIGHT);
            if(buttonRightX &&
               (buttonRightX != buttonRight)) {
               // power bars->frequency spectrum
               if(currentVisualization == visualizationPowerBars)
                  currentVisualization = visualizationFrequencySpectrum;
               // none->waveform
               if(currentVisualization == visualizationNone)
                  currentVisualization = visualizationWaveform;
            }

            buttonRight = buttonRightX;

            bool buttonUpX = input_get_button_state(INPUT_PLAYER_1, INPUT_BUTTON_UP);
            if(buttonUpX &&
               (buttonUpX != buttonUp)) {
               // power bars->none
               if(currentVisualization == visualizationPowerBars)
                  currentVisualization = visualizationNone;
               // frequency spectrum->waveform
               if(currentVisualization == visualizationFrequencySpectrum)
                  currentVisualization = visualizationWaveform;
            }

            buttonUp = buttonUpX;

            bool buttonDownX = input_get_button_state(INPUT_PLAYER_1, INPUT_BUTTON_DOWN);
            if(buttonDownX &&
               (buttonDownX != buttonDown)) {
               // none->power bars
               if(currentVisualization == visualizationNone)
                  currentVisualization = visualizationPowerBars;
               // waveform->frequency spectrum
               if(currentVisualization == visualizationWaveform)
                  currentVisualization = visualizationFrequencySpectrum;
            }

            buttonDown = buttonDownX;

            bool buttonAX = input_get_button_state(INPUT_PLAYER_1, INPUT_BUTTON_A);
            if(buttonAX &&
               (buttonAX != buttonA)) {
               currentSong--;
               if(currentSong < 1)
                  currentSong = 1;

               play(currentSong);

               // Clear frame timer.
               nsfFrameTicks -= nsfFrameTicks;

               // Queue first playback cycle.
               cpu_context.PC.word = nsf.playAddress;
               playbackTimer = playbackPeriod;
            }

            buttonA = buttonAX;

            bool buttonBX = input_get_button_state(INPUT_PLAYER_1, INPUT_BUTTON_B);
            if(buttonBX &&
               (buttonBX != buttonB)) {
               currentSong++;
               if(currentSong > nsf.totalSongs)
                  currentSong = nsf.totalSongs;

               play(currentSong);

               // Clear frame timer.
               nsfFrameTicks -= nsfFrameTicks;

               // Queue first playback cycle.
               cpu_context.PC.word = nsf.playAddress;
               playbackTimer = playbackPeriod;
            }

            buttonB = buttonBX;

            // Since this button can only be triggered once, there's no sense in detecting flip flops.
            buttonSelect = input_get_button_state(INPUT_PLAYER_1, INPUT_BUTTON_SELECT);

            for(int scanline = 0; scanline < scanlinesPerFrame; scanline++) {
               if(playbackTimer > 0.0)
                  playbackTimer--;
               if(playbackTimer <= 0.0) {
                  playbackTimer += playbackPeriod;

                  // Jump to play routine.
                  cpu_context.PC.word = nsf.playAddress;
               }

               apu_predict_irqs(SCANLINE_CLOCKS);
               cpu_execute(SCANLINE_CLOCKS);

            }

            // I hope that only updating once per frame is ok.  main() does it once per scanline.
            apu_sync_update();
            audio_update();
         }

         // Normally a game would initialize the PPU palette, but since we're not a game we have to clear it manually.
         ppu_clear_palette();

         clear_to_color(video_buffer, ppu_get_background_color());

         textprintf_ex(video_buffer, small_font, 8,     8 + (12 * 0), 1 + 0x30, -1, "TITLE:");
         textprintf_ex(video_buffer, small_font, 8 + 8, 8 + (12 * 1), 1 + 0x30, -1, (const char *)&nsf.name[0]);
         textprintf_ex(video_buffer, small_font, 8,     8 + (12 * 2), 1 + 0x30, -1, "COMPILED BY:");
         textprintf_ex(video_buffer, small_font, 8 + 8, 8 + (12 * 3), 1 + 0x30, -1, (const char *)&nsf.artist[0]);
         textprintf_ex(video_buffer, small_font, 8,     8 + (12 * 4), 1 + 0x30, -1, "COPYRIGHT:");
         textprintf_ex(video_buffer, small_font, 8 + 8, 8 + (12 * 5), 1 + 0x30, -1, (const char *)&nsf.copyright[0]);

         textprintf_ex(video_buffer, small_font, 8, 8 + (12 * 7), 1 + 0x30, -1, "Track %d of %d", currentSong, nsf.totalSongs);
         textprintf_ex(video_buffer, small_font, 8, 8 + (12 * 8) + 2, 1 + 0x30, -1, "Press A for next track");
         textprintf_ex(video_buffer, small_font, 8, 8 + (12 * 9) + 2, 1 + 0x30, -1, "Press B for previous track");

         textprintf_ex(video_buffer, small_font, 8, 8 + (12 * 11), 1 + 0x30, -1, "Press SELECT to exit");

         // Begin "Power Bars" visualization.
         {
            static real inputs[APU_VISDATA_ENTRIES];
            static real levels[APU_VISDATA_ENTRIES];
            static real outputs[APU_VISDATA_ENTRIES];
            static real peaks[APU_VISDATA_ENTRIES];
            static bool initialized = false;

            if(!initialized) {
               // Initialize data.
               memset(inputs, 0, sizeof(inputs));
               memset(levels, 0, sizeof(levels));
               memset(outputs, 0, sizeof(outputs));
               memset(peaks, 0, sizeof(peaks));
               initialized = true;
            }

            const int first_channel = APU_VISDATA_SQUARE_1;
            
            int last_channel;
            if(apu_options.stereo)
               last_channel = APU_VISDATA_MASTER_2;
            else
               last_channel = APU_VISDATA_MASTER_1;

            uint8 colorMask;
            if(currentVisualization == visualizationPowerBars) {
               // Only update the display if this is the currently active visualization.
               REAL* visdata = apu_get_visdata();
               if(visdata) {
                  for(int channel = first_channel; channel <= last_channel; channel++) {
                     const real difference = fabs(visdata[channel] - inputs[channel]);
                     inputs[channel] = visdata[channel];

                     if(inputs[channel] > levels[channel])
                        levels[channel] += (4.0 / frameBPM);
                     else if(inputs[channel] < levels[channel])
                        levels[channel] -= (4.0 / frameBPM);

                     outputs[channel] = ((difference * levels[channel]) * 5.0);
                     outputs[channel] = fixf(outputs[channel], 0.0, 1.0);

                     if(outputs[channel] >= peaks[channel])
                        peaks[channel] = outputs[channel];
                     else
                        peaks[channel] -= (0.10 / frameBPM);
                  }

                  // Destroy it.
                  delete[] visdata;
               }

               // Since this is the currently active visualization, use a fullbright display.
               colorMask = 0xFF;
            }
            else {
               // The display is inactive, darken it by ANDing all colors with $1D.
               colorMask = 0x1D;
            }

            // Draw channel VUs.
            int x = 64;
            const int max_x = 132;
            const int bar_width = (max_x - x);
            const int x_spacing = 2;

            int y = (8 + (12 * 13)); // Draw on same line as "Square 1" above.
            int bar_height = 8;
            int y_spacing = 12;
            int height = 0; // Used for drawing "Use D-PAD to select" box(further down).

            for(int channel = first_channel; channel <= last_channel; channel++) {
               if((channel >= APU_VISDATA_MASTER_1) &&
                  apu_options.stereo) {
                  bar_height = 4;
                  y_spacing = 6;
               }

               // Draw unlit bars.
               for(int dx = x; dx <= max_x; dx += x_spacing)
                  vline(video_buffer, dx, y, (y + bar_height), 1 + (0x2D & colorMask));

               // Draw partially lit bars.
               int power = (int)ROUND(bar_width * peaks[channel]);
               for(int dx = x; dx <= (x + power); dx += x_spacing)
                  vline(video_buffer, dx, y, (y + bar_height), 1 + (0x1C & colorMask));

               // Draw lit bars.
               power = (int)ROUND(bar_width * outputs[channel]);
               for(int dx = x; dx <= (x + power); dx += x_spacing)
                  vline(video_buffer, dx, y, (y + bar_height), 1 + (0x2C & colorMask));

               // Draw peak.
               power = (int)ROUND(bar_width * peaks[channel]);
               vline(video_buffer, (x + power), y, (y + bar_height), 1 + (0x3D & colorMask));

               y += y_spacing;
               height += y_spacing;
            }

            // Draw labels.
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 13) + 2, 1 + (0x30 & colorMask), -1, "Square 1");
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 14) + 2, 1 + (0x30 & colorMask), -1, "Square 2");
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 15) + 2, 1 + (0x30 & colorMask), -1, "Triangle");
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 16) + 2, 1 + (0x30 & colorMask), -1, "   Noise");
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 17) + 2, 1 + (0x30 & colorMask), -1, " Digital");
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 18) + 2, 1 + (0x30 & colorMask), -1, "  Master");

            if(currentVisualization != visualizationPowerBars) {
               // Draw "Use D-PAD to select" text.
               const int center_x = (16 + ((max_x - 16) / 2));
               const int center_y = (y - ((height + 4) / 2));

               const int tex_width = text_length(small_font, "Use D-PAD to select");
               const int tex_height = text_height(small_font);
               const int tex_x = (center_x - (tex_width / 2));
               const int tex_y = (center_y - (tex_height / 2));

               const int box_width = (4 + tex_width + 4);
               const int box_height = (4 + tex_height + 4);
               const int box_x = (center_x - (box_width / 2));
               const int box_y = (center_y - (box_height / 2));

               rectfill(video_buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), 1 + 0x1D);
               rect(video_buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), 1 + 0x2D);

               textprintf_ex(video_buffer, small_font, tex_x, tex_y, 1 + 0x3D, -1, "Use D-PAD to select");
            }
         }
         // End "Power Bars" visualization.

         // Begin "Frequency Spectrum" visualization.
         {
            static real levels[10]; // 10 steps(see below)
            static bool initialized = false;

            if(!initialized) {
               // Initialize data.
               memset(levels, 0, sizeof(levels));
               initialized = true;
            }

            uint8 colorMask;
            if(currentVisualization == visualizationFrequencySpectrum) {
               // Only update the display if this is the currently active visualization.
               UINT16* visdata = audio_get_visdata();
               if(visdata) {
                  // 10 steps of 600Hz each, yielding 2kHz-8kHz
                  for(int step = 0; step < 10; step++) {
                     real power = bandpass(&visdata[0], visualizationSamples, audio_sample_rate, ((step + 2) * 600), (((step + 2) * 600) + 600));
                     power = fabs(power);
                     power *= 256.0;
                     power = fixf(power, 0.0, 1.0);

                     if(power > levels[step])
                        levels[step] += (2.0 / frameBPM);
                     else if(power < levels[step])
                        levels[step] -= (2.0 / frameBPM);
                  }
     
                  // Destroy it.
                  delete[] visdata;
               }

               // Since this is the currently active visualization, use a fullbright display.
               colorMask = 0xFF;
            }
            else {
               // The display is inactive, darken it by ANDing all colors with $1D.
               colorMask = 0x1D;
            }

            const int x = 162;
            const int bar_width = 4;
            const int bar_spacing = (bar_width + 4);

            const int y_start = (8 + (12 * 13)); // Draw on same line as "Square 1" above.
            const int y_end = (8 + (12 * 17)) + 8; // Just above frequencies text(drawn below).
            const int y = y_end;
            const int max_height = (y_end - y_start);

            for(int step = 0; step < 10; step++) {
               // Draw bar.
               const int height = (int)ROUND(max_height * levels[step]);
               rectfill(video_buffer, (x + (bar_spacing * step)), y, ((x + (bar_spacing * step)) + bar_width), (y - height), 1 + (0x24 & colorMask));
            }

            textprintf_ex(video_buffer, small_font, 152 + 3 + 3, 8 + (12 * 18) + 3, 1 + (0x30 & colorMask), -1, "2k   4k   6k   8k");

            if(currentVisualization != visualizationFrequencySpectrum) {
               // Draw "Use D-PAD to select" text.
               const int center_x = (153 + 3 + 3 + (text_length(small_font, "2k   4k   6k   8k") / 2));
               const int center_y = (y_start + ((max_height + 12) / 2));

               const int tex_width = text_length(small_font, "Use D-PAD to select");
               const int tex_height = text_height(small_font);
               const int tex_x = (center_x - (tex_width / 2));
               const int tex_y = (center_y - (tex_height / 2));

               const int box_width = (4 + tex_width + 4);
               const int box_height = (4 + tex_height + 4);
               const int box_x = (center_x - (box_width / 2));
               const int box_y = (center_y - (box_height / 2));

               rectfill(video_buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), 1 + 0x1D);
               rect(video_buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), 1 + 0x2D);

               textprintf_ex(video_buffer, small_font, tex_x, tex_y, 1 + 0x3D, -1, "Use D-PAD to select");
            }
         }
         // End "Frequency Spectrum" visualization.

         // Begin "Waveform" visualization.
         {
            const int x = 152;
            const int max_x = (256 - 8);
            const int num_steps = (max_x - x);

            static real steps[num_steps];
            static real stereo_steps_left[num_steps];
            static real stereo_steps_right[num_steps];
            static bool initialized = false;

            if(!initialized) {
               memset(steps, 0, sizeof(steps));
               memset(stereo_steps_left, 0, sizeof(stereo_steps_left));
               memset(stereo_steps_right, 0, sizeof(stereo_steps_right));
               initialized = true;
            }

            uint8 colorMask;
            if(currentVisualization == visualizationWaveform) {
               // Only update the display if this is the currently active visualization.
               UINT16* visdata = audio_get_visdata();
               if(visdata) {
                  for(int draw_x = x; draw_x < max_x; draw_x++) {
                     // Determine step index.
                     const int step = (draw_x - x);

                     if(apu_options.stereo) {
                        // Determine sample position.
                        const unsigned offset = ((visualizationSamples - (step * 2)) - 1);

                        // Fetch and convert samples to signed format.
                        const int16 sample_left = (visdata[offset] ^ 0x8000);
                        const int16 sample_right = (visdata[offset + 1] ^ 0x8000);

                        // Scale samples and clip.
                        real sample_left_f = (sample_left / 8192.0);
                        sample_left_f = fixf(sample_left_f, -1.0, 1.0);

                        real sample_right_f = (sample_right / 8192.0);
                        sample_right_f = fixf(sample_right_f, -1.0, 1.0);

                        // Save for later.
                        stereo_steps_left[step] = sample_left_f;
                        stereo_steps_right[step] = sample_right_f;
                     }
                     else {
                        // Determine sample position.
                        const unsigned offset = ((visualizationSamples - step) - 1);

                        // Fetch and convert sample to signed format.
                        const int16 sample = (visdata[offset] ^ 0x8000);

                        // Scale sample and clip.
                        real sample_f = (sample / 16384.0);
                        sample_f = fixf(sample_f, -1.0, 1.0);

                        // Save for later.
                        steps[step] = sample_f;
                     }
                  }

                  // Destroy it.
                  delete[] visdata;
               }

               // Since this is the currently active visualization, use a fullbright display.
               colorMask = 0xFF;
            }
            else {
               // The display is inactive, darken it by ANDing all colors with $1D.
               colorMask = 0x1D;
            }

            const int y = (8 + (12 * 7)); // Same line as track number.
            const int max_height = (apu_options.stereo ? 12 : 26);
            const int y_base = (y + max_height);
            const int display_height = (26 * 2); // Height of display(for drawing the D-PAD text further below).

            // Vertical offset for the second box(stereo mode only).
            const int box_spacing = 4;
            const int y_offset = ((max_height * 2) + box_spacing);

            // Draw box background(s).
            if(apu_options.stereo) {
               rectfill(video_buffer, x, y, max_x, (y + (max_height * 2)), 1 + (0x2D & colorMask));
               rectfill(video_buffer, x, (y + y_offset), max_x, ((y + y_offset) + (max_height * 2)), 1 + (0x2D & colorMask));
            }
            else
               rectfill(video_buffer, x, y, max_x, (y + (max_height * 2)), 1 + (0x2D & colorMask));

            // Draw bars.
            for(int draw_x = x; draw_x < max_x; draw_x++) {
               const int step = (draw_x - x);

               if(apu_options.stereo) {
                  int power = (int)ROUND(max_height * stereo_steps_left[step]);
                  vline(video_buffer, draw_x, y_base, (y_base + power), 1 + (0x2A & colorMask));

                  power = (int)ROUND(max_height * stereo_steps_right[step]);
                  vline(video_buffer, draw_x, (y_base + y_offset), ((y_base + y_offset) + power), 1 + (0x16 & colorMask));
               }
               else {
                  const int power = (int)ROUND(max_height * steps[step]);
                  vline(video_buffer, draw_x, y_base, (y_base + power), 1 + (0x2A & colorMask));
               }
            }

            // Draw box border(s).
            if(apu_options.stereo) {
               rect(video_buffer, x, y, max_x, (y + (max_height * 2)), 1 + (0x3D & colorMask));
               rect(video_buffer, x, (y + y_offset), max_x, ((y + y_offset) + (max_height * 2)), 1 + (0x3D & colorMask));
            }
            else
               rect(video_buffer, x, y, max_x, (y + (max_height * 2)), 1 + (0x3D & colorMask));

            if(currentVisualization != visualizationWaveform) {
               // Draw "Use D-PAD to select" text.
               const int center_x = (x + ((max_x - x) / 2));
               const int center_y = (y + (display_height / 2));

               const int tex_width = text_length(small_font, "Use D-PAD to select");
               const int tex_height = text_height(small_font);
               const int tex_x = (center_x - (tex_width / 2));
               const int tex_y = (center_y - (tex_height / 2));

               const int box_width = (4 + tex_width + 4);
               const int box_height = (4 + tex_height + 4);
               const int box_x = (center_x - (box_width / 2));
               const int box_y = (center_y - (box_height / 2));

               rectfill(video_buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), 1 + 0x1D);
               rect(video_buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), 1 + 0x2D);

               textprintf_ex(video_buffer, small_font, tex_x, tex_y, 1 + 0x3D, -1, "Use D-PAD to select");
            }
         }
         // End "Waveform "visualization.

         video_blit(screen);
      }

      if(cpu_usage == CPU_USAGE_NORMAL)
         rest(0);
      else if(cpu_usage == CPU_USAGE_PASSIVE)
         rest(1);
   }

   // Remove frame timer.
   remove_int(nsfFrameTimer);

   // Disble visualization.
   audio_visclose();

   // Switch back to indirect timing mode and return.
   timing_mode = saved_timing_mode;
   timing_update_mode();

   /* Since this function is usually called from the GUI, we have to clear the keyboard buffer to prevent any keypresses
      that we recieved from reaching the GUI, otherwise hangs and other issues could occur. */
   clear_keybuf();
}

// Mapper interface.
static int nsf_mapper_init(void);
static void nsf_mapper_reset(void);
static UINT8 nsf_mapper_read(UINT16 address);
static void nsf_mapper_write(UINT16 address, UINT8 value);

const MMC nsf_mapper =
{
   10000, // Just set to some bogus number that will never be used.
   (const UINT8*)"NSF",
   nsf_mapper_init,
   nsf_mapper_reset,
   (const UINT8*)"NSF\0\0\0\0\0",
   NULL, NULL,
};

// Mapper data for MMC5.
static uint8 nsfMMC5MultiplierMultiplicand = 0x00;
static uint8 nsfMMC5MultiplierMultiplier = 0x00;
static uint16 nsfMMC5MultiplierProduct = 0x0000;

/* Memory to emulate MMC5's ExRAM for limited audio/general purpose use.
   NSF spec says this tops out at $5FF5, not $5FFF. */
static uint8 nsfMMC5ExRAM[0x5FF5 - 0x5C00];

static int nsf_mapper_init(void)
{
   if(nsf.bankswitched) {
      // Install handler for bankswitching registers (actual initial bankswitching is done from nsf_mapper_reset()).
      cpu_set_write_handler_2k(0x5800, nsf_mapper_write);
   }
   else {
      // Map data directly into the CPU address space.
      cpu_set_read_address_32k(0x8000, (UINT8*)&nsf.data[0]);
   }

   // Set up ExSound, if applicable.
   // TODO: Support for multiple ExSound chips simultaneously?  Would require modifications to the APU...
   if(nsf.expansionFlags & NSFExpansionMMC5) {
      apu_set_exsound(APU_EXSOUND_MMC5);

      // Map in MMC5 audio, multiplier, and ExRAM registers.
      cpu_set_read_handler_4k(0x5000, nsf_mapper_read);
      cpu_set_write_handler_4k(0x5000, nsf_mapper_write);
   }
   else if(nsf.expansionFlags & NSFExpansionVRC6) {
      apu_set_exsound(APU_EXSOUND_VRC6);

      // Map in VRC6 audio registers.
      cpu_set_write_handler_2k(0x9000, nsf_mapper_write);
      cpu_set_write_handler_2k(0xA000, nsf_mapper_write);
      cpu_set_write_handler_2k(0xB000, nsf_mapper_write);
   }
   else
      apu_set_exsound(APU_EXSOUND_NONE);

   // Initialize everything else.
   nsf_mapper_reset();

   // Return success.
   return 0;
}

static void nsf_mapper_reset(void)
{
   // TODO: PAL/NTSC selection stuff here (setting machine_type?).  Probably needs changes to the timing system to work.

   //  Clear all RAM at 0000h-07ffh.
   for(uint16 address = 0x0000; address <= 0x07FF; address++)
      cpu_write(address, 0x00);

   // Clear all RAM at 6000h-7fffh.
   for(uint16 address = 0x6000; address <= 0x7FFF; address++)
      cpu_write(address, 0x00);

   // Init the sound registers by writing 00h to 04000-0400Fh,
   for(uint16 address = 0x4000; address <= 0x400F; address++)
      cpu_write(address, 0x00);

   // 10h to 4010h,
   cpu_write(0x4010, 0x10);

   // and 00h to 4011h-4013h.
   for(uint16 address = 0x4011; address <= 0x4013; address++)
      cpu_write(address, 0x00);
  
   // Set volume register 04015h to 00fh.
   // Annotation: Huh?  I think he means to do this...
   cpu_write(0x4015, 0x0F);

   // If this is a banked tune, load the bank values from the header into 5ff8-5fffh.
   if(nsf.bankswitched) {
      for(int bank = 0; bank < NSFBankCount; bank++)
         bankswitch(bank, nsf.bankswitch[bank]);
   }

   // Clear mapper data.
   nsfMMC5MultiplierMultiplicand = 0x00;
   nsfMMC5MultiplierMultiplier = 0x00;
   nsfMMC5MultiplierProduct = 0x0000;

   memset(nsfMMC5ExRAM, 0, sizeof(nsfMMC5ExRAM));
}

static UINT8 nsf_mapper_read(UINT16 address)
{
   if(nsf.expansionFlags & NSFExpansionMMC5) {
      if(address == 0x5205) {
         // MMC5 multiplier(lower 8 bits of result).
         return (nsfMMC5MultiplierProduct & 0x00FF);
      }
      else if(address == 0x5206) {
         // MMC5 multiplier(upper 8 bits of result).
         return ((nsfMMC5MultiplierProduct & 0xFF00) >> 8);
      }
      else if((address >= 0x5C00) && (address <= 0x5FF5)) {
         // MMC5 ExRAM(read).
         return nsfMMC5ExRAM[address - 0x5C00];
      }
   }

   return 0x00;
}

static void nsf_mapper_write(UINT16 address, UINT8 value)
{
   if((address >= 0x5FF8) && (address <= 0x5FFF)) {
      // NSF bankswitching.
      const int bank = (address - 0x5FF8);
      bankswitch(bank, value);
      return;
   }

   if(nsf.expansionFlags & NSFExpansionMMC5) {
      if((address >= 0x5000) && (address <= 0x5015)) {
         // MMC5 audio.
         apu_write(address, value);
         return;
      }
      else if(address == 0x5205) {
         // MMC5 multiplier(8 bits multiplicand).
         nsfMMC5MultiplierMultiplicand = value;
         // Remember to cast everything to uint16 before the multiply otherwise we could get an overflow!
         nsfMMC5MultiplierProduct = ((uint16)nsfMMC5MultiplierMultiplicand * (uint16)nsfMMC5MultiplierMultiplier);

         return; 
      }
      else if(address == 0x5206) {
         // MMC5 multiplier(8 bits multiplier);
         nsfMMC5MultiplierMultiplier = value;
         nsfMMC5MultiplierProduct = ((uint16)nsfMMC5MultiplierMultiplicand * (uint16)nsfMMC5MultiplierMultiplier);

         return; 
      }
      else if((address >= 0x5C00) && (address <= 0x5FF5)) {
         // MMC5 ExRAM(write).
         nsfMMC5ExRAM[address - 0x5C00] = value;
         return;
      }
   }

   if(nsf.expansionFlags & NSFExpansionVRC6) {
      if(((address >= 0x9000) && (address <= 0x9001)) ||
         ((address >= 0xA000) && (address <= 0xA002)) ||
         ((address >= 0xB000) && (address <= 0xB002))) {
         // VRC6 audio.
         apu_write(address, value);
         return;
      }
   }
}

// Helper functions.
static void bankswitch(int bankIn, int bankOut)
{
   /* Maps the 4kB bank number 'bankOut' into the address range associated with the 4kB bank number 'bankIn' (there are 8
      total in the $8000-$FFFF range).  Banks are always mapped as read-only, since they occupy the ROM area. */

   if(((bankIn < 0) || (bankIn > NSFBankCount)) ||
      ((bankOut < 0) || (bankOut > NSFBankCount))) {
      WARN_GENERIC();
      return;
   }

   cpu_set_read_address_4k(0x8000 + (bankIn * NSFBankSize), (UINT8*)&nsf.data[bankOut * NSFBankSize]);
}

static void play(int song)
{
   // We need to reset before initializing each tune.
   nsf_mapper_reset();

   /* To initialize a tune, we set the accumulator to the song number(minus one), and the X register to the desired
      region (0=NTSC, 1=PAL), and then jump to the init address.
      However, because the initialization routine returns with an RTS, we cannot simply call it in an emulator, because
      it will return to nothing and the CPU will begin executing code off in la-la land.
      This is further complicated by the fact that we have no way of knowing how long the routine will last, and thus we do
      not know how many cycles to execute, and just taking a guess is bad for compatibility.
      We can solve both problems by writing a small 6502 program and uploading it to RAM somewhere that jumps to the
      init address for us, and then jams the CPU upon return.
      The jamming keeps the CPU from executing any more code, and also allows us to monitor the jammed flag to know when
      the initialization routine has finished executing.
      Since we're already writing the program, we'll go ahead and have it set up the other registers for us as well.  This
      minimizes the amount of direct modifications we have to make to the CPU context, which is a good thing. */

   const uint8 loadA = (song - 1);
   const uint8 loadX = ((machine_type == MACHINE_TYPE_PAL) ? 1 : 0);
   const uint8 jumpLow = (nsf.initAddress & 0x00FF);
   const uint8 jumpHigh = ((nsf.initAddress & 0xFF00) >> 8);

   std::vector<uint8> program;

   // LDA #$II
   program.push_back(0xA9);
   program.push_back(loadA);
   // LDX #$II
   program.push_back(0xA2);
   program.push_back(loadX);
   // JSR $AAAA
   program.push_back(0x20);
   program.push_back(jumpLow);
   program.push_back(jumpHigh);
   // JAM (Note that only the $F2 variation of this opcode is supported by our current CPU emulation.)
   program.push_back(0xF2);
   // Fill the rest with NOPs, just in case.
   while(program.size() < 2048)
      program.push_back(0xEA);

   // We'll upload the entire thing to a 2 kB page starting at $1000 and ending at $17FF, which should be unused.
   cpu_set_read_address_2k(0x1000, &program[0]);

   // Set the program counter to the address of our program.
   cpu_context.PC.word = 0x1000;

   // Execute until the initialization routine finishes and RTSes to the JAM opcode.
   while(!cpu_context.Jammed) {
      apu_predict_irqs(100);
      cpu_execute(100);
   }

   // Unjam the CPU now that we're done.
   cpu_context.Jammed = FALSE;
}

static real bandpass(uint16* buffer, unsigned size, real sampleRate, real cutoffLow, real cutoffHigh)
{
   /* Helper function for the frequency spectrum visualizer.  When passed a buffer, the samples within it are analyzed and
      a (crude) band pass filter performed on them using the specified cutoff frequencies.
      The resulting power level (average value) is then returned.
      The buffer is expected to consist of unsigned 16bit samples of a single channel.
      TODO: Handle multiple channels gracefully. */

   RT_ASSERT(buffer);

   // Lowpass.
   real timer = 0.0;
   real period = (sampleRate / MAX(EPSILON, cutoffHigh));

   int32 accumulator = 0;
   int counter = 0;

   for(unsigned offset = 0; offset < size; offset++) {
      if(timer > 0.0) {
         timer--;
         if(timer > 0.0)
            continue;
      }

      timer += period;

      const int16 sample = (buffer[offset] ^ 0x8000);
      accumulator += sample;
      counter++;
   }

   accumulator /= counter;

   // Highpass.
   timer = 0.0;
   period = (sampleRate / MAX(cutoffLow, EPSILON));

   const int32 saved_accumulator = accumulator;
   accumulator = 0;
   counter = 0;

   for(unsigned offset = 0; offset < size; offset++) {
      if(timer > 0.0) {
         timer--;
         if(timer > 0.0)
            continue;
      }

      timer += period;

      const int16 sample = (buffer[offset] ^ 0x8000);
      accumulator += sample;
      counter++;
   }

   accumulator /= counter;
   accumulator = (saved_accumulator - accumulator);

   return (accumulator / 32768.0);
}
