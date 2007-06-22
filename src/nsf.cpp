/* FakeNES - A free, portable, Open Source NES emulator.

   nsf.cpp: Back-end for the NSF player.

   Copyright (c) 2001-2007, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
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
// TODO: Lots of bankswitching stuff.
// TODO: Support for expansion hardware.

// Structure to hold information about our NSF state.
typedef struct _NSF {
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
   std::vector<uint8> code;

} NSF;

static NSF nsf;

/* bit 0: if clear, this is an NTSC tune
   bit 0: if set, this is a PAL tune
   bit 1: if set, this is a dual PAL/NTSC tune
   bits 2-7: not used. they *must* be 0 */
enum {
   NSF_REGION_PAL  = (1 << 0),
   NSF_REGION_DUAL = (1 << 1),
};

// Function prototypes(defined at bottom).
static linear void play(int song);

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
   while(!pack_feof(file)) {
      nsf.code.push_back(pack_getc(file));
      bytesRead++;
   }

   // Close file.
   pack_fclose(file);

   log_printf("NSF: nsf_open(): Loaded %u bytes from '%s'.\n", bytesRead, filename);

   // Determine if tune is bankswitched.  Ugly, but it works.
   // TODO: Implement support for bankswitched NSFs.
   if((nsf.bankswitch[0] != 0x00) ||
      (nsf.bankswitch[1] != 0x00) ||
      (nsf.bankswitch[2] != 0x00) ||
      (nsf.bankswitch[3] != 0x00) ||
      (nsf.bankswitch[4] != 0x00) ||
      (nsf.bankswitch[5] != 0x00) ||
      (nsf.bankswitch[6] != 0x00) ||
      (nsf.bankswitch[7] != 0x00)) {
      log_printf("NSF: nsf_open(): Bankswitched tunes are not yet supported.\n");
      nsf_close();
      return false;
   }

   // Return success.
   return true;
}

void nsf_close(void)
{
   // Clear program data.
   if(nsf.code.size() > 0)
      nsf.code.clear();
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

   // How many samples to use for visualization(per frame).  150 is good for most cases. */
   const unsigned visualizationSamples = 150;

   // Enable visualization.
   audio_visopen(visualizationSamples);

   /* 0 = none (top left),
      1 = power bars (bottom left),
      2 = frequency spectrum (bottom right),
      3 = waveform (top right) */
   int currentVisualization = 1;
   const int visualizationNone = 0;
   const int visualizationPowerBars = 1;
   const int visualizationFrequencySpectrum = 2;
   const int visualizationWaveform = 3;

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
         textprintf_ex(video_buffer, small_font, 8, 8 + (12 * 8), 1 + 0x30, -1, "Press A for next track");
         textprintf_ex(video_buffer, small_font, 8, 8 + (12 * 9), 1 + 0x30, -1, "Press B for previous track");

         textprintf_ex(video_buffer, small_font, 8, 8 + (12 * 11), 1 + 0x30, -1, "Press SELECT to exit");

         textprintf_ex(video_buffer, small_font, 160 + 8, 8 + (12 * 12), 1 + 0x30, -1, "FREQUENCIES");
         textprintf_ex(video_buffer, small_font, 160,     8 + (12 * 18), 1 + 0x30, -1, "150Hz 1kHz 8kHz");

         REAL* visdata1 = apu_get_visdata();
         if(visdata1) {
            static real inputs[APU_VISDATA_ENTRIES];
            static real outputs[APU_VISDATA_ENTRIES];
            static real peaks[APU_VISDATA_ENTRIES];

            // If the display is inactive darken it by ANDing all color palette entries with $1D.
            const uint8 colorMask = ((currentVisualization == visualizationPowerBars) ? 0xFF : 0x1D);

            // Draw channel VUs.
            int x = 64;
            const int max_x = 128;
            const int x_spacing = 2;

            int y = (8 + (12 * 13)); // Draw on same line as "Square 1" above.
            int line_height = 8;
            int y_spacing = 12;

            const int first_channel = APU_VISDATA_SQUARE_1;
            
            int last_channel;
            if(apu_options.stereo)
               last_channel = APU_VISDATA_MASTER_2;
            else
               last_channel = APU_VISDATA_MASTER_1;

            for(int channel = first_channel; channel <= last_channel; channel++) {
               if((channel >= APU_VISDATA_MASTER_1) &&
                  apu_options.stereo) {
                  line_height = 4;
                  y_spacing = 6;
               }

               if(currentVisualization == visualizationPowerBars) {
                  // Only update the display if this is the currently active visualization.
                  const real difference = fabs(visdata1[channel] - inputs[channel]);
                  inputs[channel] = visdata1[channel];
   
                  outputs[channel] = ((difference / 2.0) + 0.5);

                  if(outputs[channel] >= peaks[channel])
                     peaks[channel] = outputs[channel];
                  else
                     peaks[channel] -= (0.10 / frameBPM);
               }

               // Draw unlit bars.
               for(int dx = x; dx <= max_x; dx += x_spacing)
                  vline(video_buffer, dx, y, (y + line_height), 1 + (0x2D & colorMask));

               // Draw lit bars.
               int power = (int)ROUND(max_x * outputs[channel]);
               for(int dx = x; dx <= power; dx += x_spacing)
                  vline(video_buffer, dx, y, (y + line_height), 1 + (0x21 & colorMask));

               // Draw peak.
               power = (int)ROUND(max_x * peaks[channel]);
               vline(video_buffer, power, y, (y + line_height), 1 + (0x30 & colorMask));

               y += y_spacing;
            }

            // Draw labels.
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 13), 1 + (0x30 & colorMask), -1, "Square 1");
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 14), 1 + (0x30 & colorMask), -1, "Square 2");
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 15), 1 + (0x30 & colorMask), -1, "Triangle");
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 16), 1 + (0x30 & colorMask), -1, "   Noise");
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 17), 1 + (0x30 & colorMask), -1, " Digital");
            textprintf_ex(video_buffer, small_font, 16, 8 + (12 * 18), 1 + (0x30 & colorMask), -1, "  Master");

            // Destroy it.
            delete[] visdata1;
         }

         // TODO: Fix stupid variable naming due to conflicts up above.
         UINT16* visdata3 = audio_get_visdata();
         if(visdata3) {
            // If the display is inactive darken it by ANDing all color palette entries with $1D.
            const uint8 colorMask = ((currentVisualization == visualizationWaveform) ? 0xFF : 0x1D);

            if(apu_options.stereo) {
               const int x = 152;
               const int max_x = (256 - 8); 

               const int y = (8 + (12 * 7)); // Same line as track number.
               const int max_height = 10;
               const int y_base = (y + max_height);

               // Vertical offset for the second box.
               const int box_spacing = 4;
               const int y_offset = ((max_height * 2) + box_spacing);

               if(currentVisualization == visualizationWaveform) {
                  for(int draw_x = x; draw_x < max_x; draw_x++) {
                     // Determine sample position.
                     const unsigned offset = ((((draw_x - x) * (max_x - x)) / visualizationSamples) * 2);

                     // Fetch and convert samples to signed format.
                     const int16 sample_left = (visdata3[offset] ^ 0x8000);
                     const int16 sample_right = (visdata3[offset + 1] ^ 0x8000);

                     // Normalize samples.
                     real sample_left_f = (sample_left / 16384.0);
                     sample_left_f = fixf(sample_left_f, -1.0, 1.0);

                     real sample_right_f = (sample_right / 16384.0);
                     sample_right_f = fixf(sample_right_f, -1.0, 1.0);

                     // Draw bars.
                     int power = (int)ROUND(max_height * sample_left_f);
                     vline(video_buffer, draw_x, y_base, (y_base + power), 1 + 0x2A & (colorMask));

                     power = (int)ROUND(max_height * sample_right_f);
                     vline(video_buffer, draw_x, (y_base + y_offset), ((y_base + y_offset) + power), 1 + (0x16 & colorMask));
                  }
               }

               // Draw boxes.
               rect(video_buffer, x, y, max_x, (y + (max_height * 2)), 1 + (0x30 & colorMask));
               rect(video_buffer, x, (y + y_offset), max_x, ((y + y_offset) + (max_height * 2)), 1 + (0x30 & colorMask));
            }
            else {
               const int x = 152;
               const int max_x = (256 - 8); 

               const int y = (8 + (12 * 7)); // Same line as track number.
               const int max_height = 24;
               const int y_base = (y + max_height);

               if(currentVisualization == visualizationWaveform) {
                  for(int draw_x = x; draw_x < max_x; draw_x++) {
                     // Determine sample position.
                     const unsigned offset = (((draw_x - x) * (max_x - x)) / visualizationSamples);

                     // Fetch and convert sample to signed format.
                     const int16 sample = (visdata3[offset] ^ 0x8000);

                     // Normalize sample.
                     real sample_f = (sample / 16384.0);
                     sample_f = fixf(sample_f, -1.0, 1.0);

                     // Draw bar.
                     const int power = (int)ROUND(max_height * sample_f);
                     vline(video_buffer, draw_x, y_base, (y_base + power), 1 + (0x2A & colorMask));
                  }
               }

               // Draw box.
               rect(video_buffer, x, y, max_x, (y + (max_height * 2)), 1 + (0x30 & colorMask));
            }

            // Destroy it.
            delete[] visdata3;
         }

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
const MMC nsf_mapper =
{
   10000, // Just set to some bogus number that will never be used.
   (const UINT8*)"NSF",
   nsf_mapper_init,
   nsf_mapper_reset,
   (const UINT8*)"NSF\0\0\0\0\0",
   NULL, NULL,
};

int nsf_mapper_init(void)
{
   // Load the data into the 6502's address space starting at the specified load address.
   /* TODO: Loading this directly into the CPU ramspace is probably not a good idea, especially for bankswitched ROMs.
      Better to allocate a block of memory here and install an overloading read handler for it. */
   memcpy(&cpu_ram[nsf.loadAddress], &nsf.code[0], nsf.code.size());

   // Nothing is mapped into the ROM space by default so we have to do it manually.
   cpu_set_read_address_32k(0x8000, &cpu_ram[0x8000]);

   // Return success.
   return 0;
}

void nsf_mapper_reset(void)
{
   // TODO: PAL/NTSC selection stuff here (setting machine_type).

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
   // TODO: Support bankswitched NSFs.
}

// Helper functions.
static linear void play(int song)
{
   // Supposedly we should reset before playing each tune, but I'm not sure about this.
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

