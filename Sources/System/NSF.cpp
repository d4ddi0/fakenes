/* FakeNES - A free, portable, Open Source NES emulator.

   nsf.cpp: Implementation of the NSF player.

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
#include "machine.h"
#include "ppu.h"
#include "ppu_int.h" // for palette stuff
#include "timing.h"
#include "types.h"
#include "video.h"

// TODO: Proper support for PAL/NTSC selection.

// Size and number of NSF banks in the $8000 - $FFFF area.  This is also the size of NSF pages.
static const unsigned NSFBankSize = 4096; // 4kiB
static const int NSFBankCount = 8;

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
   std::vector<uint8> data;

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

// Our little NES program to handle calling the init and play routines(cleared by nsf_close()).
static std::vector<uint8> nsfProgram;

// The current song #, this should only be set by play().
static int nsfCurrentSong = 0;

// Legacy 5x6 font.
static FONT* small_font = NULL;

// Function prototypes(defined at bottom).
static void bankswitch(int bank, int page);
static void play(int song);
static real bandpass(uint16* buffer, unsigned size, real sampleRate, int channels, real cutoffLow, real cutoffHigh);

// Opening/closing.
BOOL nsf_open(const UDATA* filename)
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

   // Start with a single page of data.
   int pages = 1;
   nsf.data.resize(pages * NSFBankSize);

   // Calculate the load offset for the first page.
   unsigned offset = nsf.loadAddress & 0x0FFF; // $8000 -> $0000

   // Load the data.
   unsigned counter = offset;
   while(!pack_feof(file)) {
      nsf.data[offset++] = pack_getc(file);
      bytesRead++;

      counter++;
      if(counter >= NSFBankSize) {
         counter = 0;

         // Start a new page, extending and padding the vector as needed.
         pages++;
         nsf.data.resize(pages * NSFBankSize);
      }
   }

   // Close file.
   pack_fclose(file);

   log_printf("NSF: nsf_open(): Loaded %u bytes from '%s'.\n", bytesRead, filename);

   // Return success.
   return true;
}

void nsf_close(void)
{
   if(nsf.data.size() > 0)
      nsf.data.clear();

   if(nsfProgram.size() > 0)
      nsfProgram.clear();
}

// --- NSF player main loop. ---

// Number of audio samples to use for visualization.
static unsigned nsfVisualizationSamples = 0;

// Visualization data, returned by the APU and audio system.
static REAL* nsfAPUVisualizationData = null;
static UINT16* nsfAudioVisualizationData = null;

/* Available visualizations:
      Power bars (bottom left),
      frequency spectrum (bottom right),
      Waveform (top right) */
enum {
   NSFVisualizationPowerLevels = (1 << 0),
   NSFVisualizationFrequencies = (1 << 1),
   NSFVisualizationWaveform    = (1 << 2),
};

static unsigned nsfVisualizationFlags = NSFVisualizationPowerLevels | NSFVisualizationFrequencies |
   NSFVisualizationWaveform;

// Functions to initialize visualizations and clear their state.
static linear void nsfPowerLevelsVisualizationClear(void);
static linear void nsfFrequenciesVisualizationClear(void);
static linear void nsfWaveformVisualizationClear(void);

// Functions to update visualizations(called at 'nsfFrameBPM' times per second).
static linear void nsfPowerLevelsVisualizationUpdate(void);
static linear void nsfFrequenciesVisualizationUpdate(void);
static linear void nsfWaveformVisualizationUpdate(void);

// Functions to draw visualizations(moved out of the main loop since they're complicated).
static linear void nsfPowerLevelsVisualizationDraw(void);
static linear void nsfFrequenciesVisualizationDraw(void);
static linear void nsfWaveformVisualizationDraw(void);

// Button states array, used to detect flip-flops.  This must be synchronized with the input system on every reset.
static bool nsfButtonStates[INPUT_BUTTONS];

// Miscellaneous.
static real nsfPlaybackTimer = 0.0;
static real nsfPlaybackPeriod = 0.0;

/* Set up NSF playback. This is called once after an NSF file is loaded, and must set everyting up for subsequent calls
   to nsf_execute() on a regular basis. The counterpart to this function is nsf_teardown(). */
void nsf_setup(void)
{
   // Load legacy font.
   small_font = video_get_font(VIDEO_FONT_LEGACY);

   // Synchronize our cached button states with the input system.
   for(int index = 0; index < INPUT_BUTTONS; index++) 
      nsfButtonStates[index] = input_get_button_state(INPUT_PLAYER_1, index);

   /* Force the input mode to NSF.  This helps keep anything input-related that the user left enabled(such as periodic
      state saving) from getting in the way of the NSF player. */
   input_mode = INPUT_MODE_NSF;

   // Update timing information.
   nsf_update_timing();

   // Jump to the starting song and queue the first playback cycle.
   play(nsf.startingSong);
   nsfPlaybackTimer = nsfPlaybackPeriod;

   // Clear visualizations.
   nsfPowerLevelsVisualizationClear();
   nsfFrequenciesVisualizationClear();
   nsfWaveformVisualizationClear();
}

void nsf_teardown(void)
{
   // Disble visualization.
   if(audio_is_visopen())
      audio_visclose();
}

void nsf_start_frame(void)
{
   // Handle input.
   bool buttonA = input_get_button_state(INPUT_PLAYER_1, INPUT_BUTTON_A);
   if(buttonA &&
      (buttonA != nsfButtonStates[INPUT_BUTTON_A])) {
      int nextSong = nsfCurrentSong - 1;
      if(nextSong < 1)
         nextSong = 1;

      // Jump to the next song and queue the first playback cycle.
      play(nextSong);
      nsfPlaybackTimer = nsfPlaybackPeriod;
   }

   nsfButtonStates[INPUT_BUTTON_A] = buttonA;

   bool buttonB = input_get_button_state(INPUT_PLAYER_1, INPUT_BUTTON_B);
   if(buttonB &&
      (buttonB != nsfButtonStates[INPUT_BUTTON_B])) {
      int nextSong = nsfCurrentSong + 1;
      if(nextSong > nsf.totalSongs)
         nextSong = nsf.totalSongs;

      // Jump to the next song and queue the first playback cycle.
      play(nextSong);
      nsfPlaybackTimer = nsfPlaybackPeriod;
   }

   nsfButtonStates[INPUT_BUTTON_B] = buttonB;

   bool buttonSelect = input_get_button_state(INPUT_PLAYER_1, INPUT_BUTTON_SELECT);
#if 0
   if(buttonSelect &&
      (buttonSelect != nsfButtonStates[INPUT_BUTTON_SELECT]))
      selectPressed = true;
#endif

   nsfButtonStates[INPUT_BUTTON_SELECT] = buttonSelect;
}

void nsf_end_frame(void)
{
   // Update visualizations.
   if(!audio_is_visopen())
      audio_visopen(nsfVisualizationSamples);

   nsfAPUVisualizationData = apu_get_visdata();
   if(nsfAPUVisualizationData) {
      nsfPowerLevelsVisualizationUpdate();

      delete[] nsfAPUVisualizationData;
      nsfAPUVisualizationData = null;
   }

   nsfAudioVisualizationData = audio_get_visdata();
   if(nsfAudioVisualizationData) {
      nsfFrequenciesVisualizationUpdate();
      nsfWaveformVisualizationUpdate();

      delete[] nsfAudioVisualizationData;
      nsfAudioVisualizationData = null;
   }

   // Check if we are drawing, for frame skipping.
   if(!ppu_get_option(PPU_OPTION_ENABLE_RENDERING))
      return;

   // Get a direct reference to the rendering buffer.
   BITMAP* buffer = video_get_render_buffer();
   if(!buffer) {
      WARN_GENERIC();
      return;
   }

   // Since we're not a game we have to clear the rendering buffer manually.
   clear_to_color(buffer, ppu__color_map[0x01]);

   const uint16 textColor = ppu__color_map[0x30];
   textprintf_ex(buffer, small_font, 8,     8 + (12 * 0), textColor, -1, "TITLE:");
   textprintf_ex(buffer, small_font, 8 + 8, 8 + (12 * 1), textColor, -1, (const char *)&nsf.name[0]);
   textprintf_ex(buffer, small_font, 8,     8 + (12 * 2), textColor, -1, "COMPILED BY:");
   textprintf_ex(buffer, small_font, 8 + 8, 8 + (12 * 3), textColor, -1, (const char *)&nsf.artist[0]);
   textprintf_ex(buffer, small_font, 8,     8 + (12 * 4), textColor, -1, "COPYRIGHT:");
   textprintf_ex(buffer, small_font, 8 + 8, 8 + (12 * 5), textColor, -1, (const char *)&nsf.copyright[0]);

   textprintf_ex(buffer, small_font, 8, 8 + (12 * 7), textColor, -1, "Track %d of %d", nsfCurrentSong, nsf.totalSongs);
   textprintf_ex(buffer, small_font, 8, 8 + (12 * 8) + 2, textColor, -1, "Press A for next track");
   textprintf_ex(buffer, small_font, 8, 8 + (12 * 9) + 2, textColor, -1, "Press B for previous track");

   // textprintf_ex(buffer, small_font, 8, 8 + (12 * 11), textColor, -1, "Press SELECT or ESC to exit");

   // Draw visualizations.
   nsfPowerLevelsVisualizationDraw();
   nsfFrequenciesVisualizationDraw();
   nsfWaveformVisualizationDraw();

   // Update the display.
   if(gui_is_active)
      gui_update_display();
   else
      video_update_display();
}

// Main playback function, called once per scanline by machine_main().
void nsf_execute(const cpu_time_t cycles)
{
   // Check if we need to send a clock pulse to the playback routine.
   for(cpu_time_t cycle = 0; cycle < cycles; cycle++) {
      // This probably isn't the most efficient way to do this, but oh well.
      if(nsfPlaybackTimer > 0.0)
         nsfPlaybackTimer--;
      if(nsfPlaybackTimer <= 0.0) {
         nsfPlaybackTimer += nsfPlaybackPeriod;
         play(nsfCurrentSong);
      }
   }
}

// Synchronizes timing information with outside sources.
void nsf_update_timing(void)
{
   // Calculate beats per second (BPM) for the frame timer.
   const real frameBPM = timing_get_base_frame_rate();

   // Determine how many scanlines to a frame.
   const real scanlinesPerFrame = PPU_TOTAL_LINES;
   const real scanlineBPM = scanlinesPerFrame * frameBPM;

   // Determine how often to update the playback timer, in master clock cycles.
   const real playbackBPM = 1000000.0 / ((machine_type == MACHINE_TYPE_PAL) ? nsf.speedPAL : nsf.speedNTSC);
   nsfPlaybackPeriod = (scanlineBPM / playbackBPM) * SCANLINE_CLOCKS;
   nsfPlaybackTimer = 0.0;

   // How many samples to use for visualization(per frame). */
   nsfVisualizationSamples = (int)floor(audio_sample_rate / frameBPM);

   // Restart visualization.
   if(audio_is_visopen())
      audio_visclose();
}

// Begin "Power Bars" visualization.
static real nsfPowerLevelsVisualizationInputs[APU_VISDATA_ENTRIES];
static real nsfPowerLevelsVisualizationLevels[APU_VISDATA_ENTRIES];
static real nsfPowerLevelsVisualizationOutputs[APU_VISDATA_ENTRIES];
static real nsfPowerLevelsVisualizationPeaks[APU_VISDATA_ENTRIES];

static const real NSFPowerLevelsVisualizationGainTime = 4.0;
static const real NSFPowerLevelsVisualizationOutputScale = 5.0;
static const real NSFPowerLevelsVisualizationPeakFalloffTime = 0.10;

static linear void nsfPowerLevelsVisualizationClear(void)
{
   memset(nsfPowerLevelsVisualizationInputs, 0, sizeof(nsfPowerLevelsVisualizationInputs));
   memset(nsfPowerLevelsVisualizationLevels, 0, sizeof(nsfPowerLevelsVisualizationLevels));
   memset(nsfPowerLevelsVisualizationOutputs, 0, sizeof(nsfPowerLevelsVisualizationOutputs));
   memset(nsfPowerLevelsVisualizationPeaks, 0, sizeof(nsfPowerLevelsVisualizationPeaks));
}

static linear void nsfPowerLevelsVisualizationUpdate(void)
{
   if(!(nsfVisualizationFlags & NSFVisualizationPowerLevels)) {
      // Only update the display if this visualization is currently active.
      return;
   }

   // make sure we have visualization data
   RT_ASSERT(nsfAPUVisualizationData);

   // Get the frame rate and cache it for efficiency.
   const real frameRate = timing_get_frame_rate();

   const int first_channel = APU_VISDATA_SQUARE_1;

   int last_channel;
   if(apu_options.stereo)
      last_channel = APU_VISDATA_MASTER_2;
   else
      last_channel = APU_VISDATA_MASTER_1;

   for(int channel = first_channel; channel <= last_channel; channel++) {
      real& vis = nsfAPUVisualizationData[channel];
      real& input = nsfPowerLevelsVisualizationInputs[channel];
      real& level = nsfPowerLevelsVisualizationLevels[channel];
      real& output = nsfPowerLevelsVisualizationOutputs[channel];
      real& peak = nsfPowerLevelsVisualizationPeaks[channel];

      const real difference = fabs(vis - input);
      input = vis;

      if(input > level) {
         level += NSFPowerLevelsVisualizationGainTime / frameRate;
         if(level > input)
            level = input;
      }
      else if(input < level) {
         level -= NSFPowerLevelsVisualizationGainTime / frameRate;
         if(level < input)
            level = input;
      }

      output = (difference * level) * NSFPowerLevelsVisualizationOutputScale;
      output = fixf(output, 0.0, 1.0);

      if(output >= peak)
         peak = output;
      else {
         peak -= NSFPowerLevelsVisualizationPeakFalloffTime / frameRate;
         if(peak < 0.0)
            peak = 0.0;
      }
   }
}

static linear void nsfPowerLevelsVisualizationDraw(void)
{
   const int first_channel = APU_VISDATA_SQUARE_1;

   int last_channel;
   if(apu_options.stereo)
      last_channel = APU_VISDATA_MASTER_2;
   else
      last_channel = APU_VISDATA_MASTER_1;

   BITMAP* buffer = video_get_render_buffer();
   if(!buffer) {
      WARN_GENERIC();
      return;
   }

   uint8 colorMask;
   if(nsfVisualizationFlags & NSFVisualizationPowerLevels) {
      // Since this visualization is active, use a fullbright display.
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
      const real& output = nsfPowerLevelsVisualizationOutputs[channel];
      const real& peak = nsfPowerLevelsVisualizationPeaks[channel];

      if((channel >= APU_VISDATA_MASTER_1) &&
         apu_options.stereo) {
         bar_height = 4;
         y_spacing = 6;
      }

      // Draw unlit bars.
      for(int dx = x; dx <= max_x; dx += x_spacing)
         vline(buffer, dx, y, (y + bar_height), ppu__color_map[0x2D & colorMask]);

      // Draw partially lit bars.
      int power = Round(bar_width * peak);
      for(int dx = x; dx <= (x + power); dx += x_spacing)
         vline(buffer, dx, y, (y + bar_height), ppu__color_map[0x1C & colorMask]);

      // Draw lit bars.
      power = Round(bar_width * output);
      for(int dx = x; dx <= (x + power); dx += x_spacing)
         vline(buffer, dx, y, (y + bar_height), ppu__color_map[0x2C & colorMask]);

      // Draw peak.
      power = Round(bar_width * peak);
      vline(buffer, (x + power), y, (y + bar_height), ppu__color_map[0x3D & colorMask]);

      y += y_spacing;
      height += y_spacing;
   }

    // Draw labels.
   const uint16 color = ppu__color_map[0x30 & colorMask];
   textprintf_ex(buffer, small_font, 16, 8 + (12 * 13) + 2, color, -1, "Square 1");
   textprintf_ex(buffer, small_font, 16, 8 + (12 * 14) + 2, color, -1, "Square 2");
   textprintf_ex(buffer, small_font, 16, 8 + (12 * 15) + 2, color, -1, "Triangle");
   textprintf_ex(buffer, small_font, 16, 8 + (12 * 16) + 2, color, -1, "   Noise");
   textprintf_ex(buffer, small_font, 16, 8 + (12 * 17) + 2, color, -1, " Digital");
   textprintf_ex(buffer, small_font, 16, 8 + (12 * 18) + 2, color, -1, "  Master");

   if(!(nsfVisualizationFlags & NSFVisualizationPowerLevels)) {
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

      rectfill(buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), ppu__color_map[0x1D]);
      rect(buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), ppu__color_map[0x2D]);

      textprintf_ex(buffer, small_font, tex_x, tex_y, ppu__color_map[0x3D], -1, "Use D-PAD to select");
   }
}
// End "Power Bars" visualization.

// Begin "Frequency Spectrum" visualization.
static real nsfFrequenciesVisualizationLevels[10]; // 10 steps(see below)
static real nsfFrequenciesVisualizationOutputs[10];

static const real NSFFrequenciesVisualizationPowerScale = 256.0; // to normalize for gain
static const real NSFFrequenciesVisualizationGainTime = 2.0;
static const real NSFFrequenciesVisualizationOutputScale = 1.0;

static linear void nsfFrequenciesVisualizationClear(void)
{
   memset(nsfFrequenciesVisualizationLevels, 0, sizeof(nsfFrequenciesVisualizationLevels));
   memset(nsfFrequenciesVisualizationOutputs, 0, sizeof(nsfFrequenciesVisualizationOutputs));
}

static linear void nsfFrequenciesVisualizationUpdate(void)
{
   if(!(nsfVisualizationFlags & NSFVisualizationFrequencies))
      return;

   // make sure we have visualization data
   RT_ASSERT(nsfAudioVisualizationData);

   // Get the frame rate and cache it for efficiency.
   const real frameRate = timing_get_frame_rate();

   // 10 steps of 600Hz each, yielding 2kHz-8kHz
   for(int step = 0; step < 10; step++) {
      real& level = nsfFrequenciesVisualizationLevels[step];
      real& output = nsfFrequenciesVisualizationOutputs[step];

      real power = bandpass(&nsfAudioVisualizationData[0], nsfVisualizationSamples, audio_sample_rate,
         (apu_options.stereo ? 2 : 1), ((step + 2) * 600), (((step + 2) * 600) + 600));
      power = fabs(power);
      power *= NSFFrequenciesVisualizationPowerScale;
      power = fixf(power, 0.0, 1.0);

      if(power > level) {
         level += NSFFrequenciesVisualizationGainTime / frameRate;
         if(level > power)
            level = power;
      }
      else if(power < level) {
         level -= NSFFrequenciesVisualizationGainTime / frameRate;
         if(level < power)
            level = power;
      }

      output = level * NSFFrequenciesVisualizationOutputScale;
      output = fixf(output, 0.0, 1.0);
   }
}

static linear void nsfFrequenciesVisualizationDraw(void)
{
   BITMAP* buffer = video_get_render_buffer();
   if(!buffer) {
      WARN_GENERIC();
      return;
   }

   uint8 colorMask;
   if(nsfVisualizationFlags & NSFVisualizationFrequencies)
      colorMask = 0xFF;
   else
      colorMask = 0x1D;

   const int x = 162;
   const int bar_width = 4;
   const int bar_spacing = (bar_width + 4);

   const int y_start = (8 + (12 * 13)); // Draw on same line as "Square 1" above.
   const int y_end = (8 + (12 * 17)) + 8; // Just above frequencies text(drawn below).
   const int y = y_end;
   const int max_height = (y_end - y_start);

   for(int step = 0; step < 10; step++) {
      const real& output = nsfFrequenciesVisualizationOutputs[step];

      // Draw bar.
      const int height = Round(max_height * output);
      rectfill(buffer, (x + (bar_spacing * step)), y, ((x + (bar_spacing * step)) + bar_width), (y - height),
         ppu__color_map[0x24 & colorMask]);
   }

   textprintf_ex(buffer, small_font, 152 + 3 + 3, 8 + (12 * 18) + 3, ppu__color_map[0x30 & colorMask], -1,
      "2k   4k   6k   8k");

   if(!(nsfVisualizationFlags & NSFVisualizationFrequencies)) {
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

      rectfill(buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), ppu__color_map[0x1D]);
      rect(buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), ppu__color_map[0x2D]);

      textprintf_ex(buffer, small_font, tex_x, tex_y, ppu__color_map[0x3D], -1, "Use D-PAD to select");
   }
}
// End "Frequency Spectrum" visualization.

// Begin "Waveform" visualization.
static const int NSFWaveformVisualizationStartX = 152;
static const int NSFWaveformVisualizationEndX = (256 - 8);
static const int NSFWaveformVisualizationTotalSteps = (NSFWaveformVisualizationEndX - NSFWaveformVisualizationStartX);

static real nsfWaveformVisualizationOutputs[NSFWaveformVisualizationTotalSteps];
static real nsfWaveformVisualizationStereoOutputsLeft[NSFWaveformVisualizationTotalSteps];
static real nsfWaveformVisualizationStereoOutputsRight[NSFWaveformVisualizationTotalSteps];

static const real NSFWaveformVisualizationOutputScale = 2.0;

static linear void nsfWaveformVisualizationClear(void)
{
   memset(nsfWaveformVisualizationOutputs, 0, sizeof(nsfWaveformVisualizationOutputs));
   memset(nsfWaveformVisualizationStereoOutputsLeft, 0, sizeof(nsfWaveformVisualizationStereoOutputsLeft));
   memset(nsfWaveformVisualizationStereoOutputsRight, 0, sizeof(nsfWaveformVisualizationStereoOutputsRight));
}

static linear void nsfWaveformVisualizationUpdate(void)
{
   if(!(nsfVisualizationFlags & NSFVisualizationWaveform))
      return;

   // make sure we have visualization data
   RT_ASSERT(nsfAudioVisualizationData);

   for(int step = 0; step < NSFWaveformVisualizationTotalSteps; step++) {
      if(apu_options.stereo) {
         real& outputLeft = nsfWaveformVisualizationStereoOutputsLeft[step];
         real& outputRight = nsfWaveformVisualizationStereoOutputsRight[step];

         // Determine sample position.
         const unsigned offset = (nsfVisualizationSamples - (step * 2)) - 1;

         // Fetch and convert samples to floating-point format.
         const real sampleLeft = ((int16)(nsfAudioVisualizationData[offset] ^ 0x8000)) / 32768.0;
         const real sampleRight = ((int16)(nsfAudioVisualizationData[offset + 1] ^ 0x8000)) / 32768.0;

         // Scale and clip samples to output.
         outputLeft = sampleLeft * NSFWaveformVisualizationOutputScale;
         outputLeft = fixf(outputLeft, -1.0, 1.0);

         outputRight = sampleRight * NSFWaveformVisualizationOutputScale;
         outputRight = fixf(outputRight, -1.0, 1.0);
      }
      else {
         real& output = nsfWaveformVisualizationOutputs[step];

         // Determine sample position.
         const unsigned offset = ((nsfVisualizationSamples - step) - 1);

         // Fetch and convert sample to floating-point format.
         const real sample = ((int16)(nsfAudioVisualizationData[offset] ^ 0x8000)) / 32768.0;

         // Scale and clip sample to output.
         output = sample * NSFWaveformVisualizationOutputScale;
         output = fixf(output, -1.0, 1.0);
      }
   }
}

static linear void nsfWaveformVisualizationDraw(void)
{
   BITMAP* buffer = video_get_render_buffer();
   if(!buffer) {
      WARN_GENERIC();
      return;
   }

   uint8 colorMask;
   if(nsfVisualizationFlags & NSFVisualizationWaveform)
      colorMask = 0xFF;
   else
      colorMask = 0x1D;

   const int x = NSFWaveformVisualizationStartX;
   const int max_x = NSFWaveformVisualizationEndX;

   const int y = (8 + (12 * 7)); // Same line as track number.
   const int max_height = (apu_options.stereo ? 12 : 26);
   const int y_base = (y + max_height);
   const int display_height = (26 * 2); // Height of display(for drawing the D-PAD text further below).

   // Vertical offset for the second box(stereo mode only).
   const int box_spacing = 4;
   const int y_offset = ((max_height * 2) + box_spacing);

   // Draw box background(s).
   if(apu_options.stereo) {
      rectfill(buffer, x, y, max_x, (y + (max_height * 2)), ppu__color_map[0x2D & colorMask]);
      rectfill(buffer, x, (y + y_offset), max_x, ((y + y_offset) + (max_height * 2)),
         ppu__color_map[0x2D & colorMask]);
   }
   else
      rectfill(buffer, x, y, max_x, (y + (max_height * 2)), ppu__color_map[0x2D & colorMask]);

   // Draw bars.
   for(int draw_x = x; draw_x < max_x; draw_x++) {
      const int step = (draw_x - x);

      if(apu_options.stereo) {
         const real& outputLeft = nsfWaveformVisualizationStereoOutputsLeft[step];
         const real& outputRight = nsfWaveformVisualizationStereoOutputsRight[step];

         int power = Round(max_height * outputLeft);
         vline(buffer, draw_x, y_base, (y_base + power), ppu__color_map[0x2A & colorMask]);

         power = Round(max_height * outputRight);
         vline(buffer, draw_x, (y_base + y_offset), ((y_base + y_offset) + power),
            ppu__color_map[0x16 & colorMask]);
      }
      else {
         const real& output = nsfWaveformVisualizationOutputs[step];

         const int power = Round(max_height * output);
         vline(buffer, draw_x, y_base, (y_base + power), ppu__color_map[0x2A & colorMask]);
      }
   }

   // Draw box border(s).
   if(apu_options.stereo) {
      rect(buffer, x, y, max_x, (y + (max_height * 2)), ppu__color_map[0x3D & colorMask]);
      rect(buffer, x, (y + y_offset), max_x, ((y + y_offset) + (max_height * 2)),
         ppu__color_map[0x3D & colorMask]);
   }
   else
      rect(buffer, x, y, max_x, (y + (max_height * 2)), ppu__color_map[0x3D & colorMask]);

   if(!(nsfVisualizationFlags & NSFVisualizationWaveform)) {
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

      rectfill(buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), ppu__color_map[0x1D]);
      rect(buffer, box_x, box_y, (box_x + box_width), (box_y + box_height), ppu__color_map[0x2D]);

      textprintf_ex(buffer, small_font, tex_x, tex_y, ppu__color_map[0x3D], -1, "Use D-PAD to select");
   }
}
// End "Waveform "visualization.

// --- Mapper interface. ---

static int nsf_mapper_init(void);
static void nsf_mapper_reset(void);
static UINT8 nsf_mapper_read(UINT16 address);
static void nsf_mapper_write(UINT16 address, UINT8 value);

const MMC nsf_mapper = {
   10000, // Just set to some bogus number that will never be used.
   "NSF",
   nsf_mapper_init,
   nsf_mapper_reset,
   "NSF\0\0\0\0\0",
   NULL, NULL,
   NULL, NULL, NULL, NULL,
};

// Mapper data for MMC5.
static uint8 nsfMMC5MultiplierMultiplicand = 0x00;
static uint8 nsfMMC5MultiplierMultiplier = 0x00;
static uint16 nsfMMC5MultiplierProduct = 0x0000;

/* Memory to emulate MMC5's ExRAM for limited audio/general purpose use. */
static uint8 nsfMMC5ExRAM[0x5FFF - 0x5C00];

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
   if(nsf.expansionFlags & NSFExpansionMMC5) {
      apu_enable_exsound(APU_EXSOUND_MMC5);

      // Map in MMC5 audio, multiplier, and ExRAM registers.
      cpu_set_read_handler_4k(0x5000, nsf_mapper_read);
      cpu_set_write_handler_4k(0x5000, nsf_mapper_write);
   }
   
   if(nsf.expansionFlags & NSFExpansionVRC6) {
      apu_enable_exsound(APU_EXSOUND_VRC6);

      // Map in VRC6 audio registers.
      cpu_set_write_handler_2k(0x9000, nsf_mapper_write);
      cpu_set_write_handler_2k(0xA000, nsf_mapper_write);
      cpu_set_write_handler_2k(0xB000, nsf_mapper_write);
   }

   // Initialize everything else.
   nsf_mapper_reset();

   // Return success.
   return 0;
}

static void nsf_mapper_reset(void)
{
   // TODO: PAL/NTSC selection stuff here (setting machine_type?).  Probably needs changes to the timing system to work.

   // Clear mapper data.
   nsfMMC5MultiplierMultiplicand = 0x00;
   nsfMMC5MultiplierMultiplier = 0x00;
   nsfMMC5MultiplierProduct = 0x0000;

   memset(nsfMMC5ExRAM, 0, sizeof(nsfMMC5ExRAM));

   // Reset ExSound.
   if(nsf.expansionFlags & NSFExpansionMMC5)
      apu_reset_exsound(APU_EXSOUND_MMC5);
   if(nsf.expansionFlags & NSFExpansionVRC6)
      apu_reset_exsound(APU_EXSOUND_VRC6);
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
      else if((address >= 0x5C00) && (address <= 0x5FFF)) {
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
      else if((address >= 0x5C00) && (address <= 0x5FFF)) {
         // MMC5 ExRAM(write).
         nsfMMC5ExRAM[address - 0x5C00] = value;
         return;
      }
   }

   if(nsf.expansionFlags & NSFExpansionVRC6) {
      if(((address >= 0x9000) && (address <= 0x9002)) ||
         ((address >= 0xA000) && (address <= 0xA002)) ||
         ((address >= 0xB000) && (address <= 0xB002))) {
         // VRC6 audio.
         apu_write(address, value);
         return;
      }
   }
}

// Helper functions.
static void bankswitch(int bank, int page)
{
   /* Maps the 4kiB bank number 'bank' into the address range associated with the 4kiB page number 'page' (there are 8
      banks in the $8000-$FFFF range).  Banks are always mapped as read-only, since they occupy the ROM area. */

   // The NSF bankswitching registers are 8 bits wide, so I'm assuming a maximum of 255 pages.
   if(((bank < 0) || (bank > NSFBankCount)) ||
      ((page < 0) || (page > 255))) {
      WARN_GENERIC();
      return;
   }

   cpu_set_read_address_4k(0x8000 + (bank * NSFBankSize), (UINT8*)&nsf.data[page * NSFBankSize]);
}

static void play(int song)
{
   if(song != nsfCurrentSong) {
      // The requested song is different from our current song - start a new song.

      // We need to reset before initializing each tune.
      // Note that a call to machine_reset() also implies a call to nsf_mapper_reset(), so we only have to do the former.
      machine_reset();

      // Since machine_reset() calls input_reset(), we need to resynchronize our cached input states.
      for(int index = 0; index < INPUT_BUTTONS; index++) 
         nsfButtonStates[index] = input_get_button_state(INPUT_PLAYER_1, index);

      // Another reset-related issue: input_reset() resets the input mode, so we want to force it back to NSF.
      input_mode = INPUT_MODE_NSF;

      // All annoying reset issues aside, now we're good to play NSFs again...

      //  Clear all RAM at 0000h-07ffh.
      for(uint16 address = 0x0000; address <= 0x07FF; address++)
         cpu_write(address, 0x00);

      // Clear all RAM at 6000h-7fffh.
      for(uint16 address = 0x6000; address <= 0x7FFF; address++)
         cpu_write(address, 0x00);

      // Init the sound registers by writing 00h to 04000-0400Fh,
      for(uint16 address = 0x4000; address <= 0x400F; address++)
         apu_write(address, 0x00);

      // 10h to 4010h,
      apu_write(0x4010, 0x10);

      // and 00h to 4011h-4013h.
      for(uint16 address = 0x4011; address <= 0x4013; address++)
         apu_write(address, 0x00);
  
      // Set volume register 04015h to 00fh.
      // Annotation: Huh?  I think he means to do this...
      apu_write(0x4015, 0x0F);

      // If this is a banked tune, load the bank values from the header into 5ff8-5fffh.
      if(nsf.bankswitched) {
         for(int bank = 0; bank < NSFBankCount; bank++)
            bankswitch(bank, nsf.bankswitch[bank]);
      }

      /* To initialize a tune, we set the accumulator to the song number(minus one), and the X register to the desired
         region (0=NTSC, 1=PAL), and then jump to the init address.
         However, because the initialization routine returns with an RTS, we cannot simply call it in an emulator, because
         it will return to nothing and the CPU will begin executing code off in la-la land.
         This is further complicated by the fact that we have no way of knowing how long the routine will last, and thus we
         do not know how many cycles to execute, and just taking a guess is bad for compatibility.
         We can solve both problems by writing a small 6502 program and uploading it to RAM somewhere that jumps to the
         init address for us, and then jams the CPU upon return.
         The jamming keeps the CPU from executing any more code, and also allows us to monitor the jammed flag to know when
         the initialization routine has finished executing.
         Since we're already writing the program, we'll go ahead and have it set up the other registers for us as well.
         This minimizes the amount of direct modifications we have to make to the CPU context, which is a good thing. */

      // Clear our program area.
      if(nsfProgram.size() > 0)
         nsfProgram.resize(0);

      // LDA #$II
      const uint8 loadA = song - 1;
      nsfProgram.push_back(0xA9);
      nsfProgram.push_back(loadA);

      // LDX #$II
      const uint8 loadX = (machine_type == MACHINE_TYPE_PAL) ? 1 : 0;
      nsfProgram.push_back(0xA2);
      nsfProgram.push_back(loadX);

      // JSR $AAAA
      uint8 jumpLow = nsf.initAddress & 0x00FF;
      uint8 jumpHigh = (nsf.initAddress & 0xFF00) >> 8;

      nsfProgram.push_back(0x20);
      nsfProgram.push_back(jumpLow);
      nsfProgram.push_back(jumpHigh);

      // JAM (Note that only the $F2 variation of this opcode is supported by our current CPU emulation.)
      nsfProgram.push_back(0xF2);

      // Fill the rest with NOPs, just in case.
      while(nsfProgram.size() < 2048)
         nsfProgram.push_back(0xEA);

      // We'll upload the entire thing to a 2 kB page starting at $1000 and ending at $17FF, which should be unused.
      cpu_set_read_address_2k(0x1000, &nsfProgram[0]);

      // Set the program counter to the address of our program.
      cpu_context.PC.word = 0x1000;

      // Execute until the initialization routine finishes and RTSes to the JAM opcode.
      while(!cpu_context.Jammed) {
         apu_predict_irqs(100);
         cpu_execute(100);
      }

      // Unjam the CPU now that we're done.
      cpu_context.Jammed = FALSE;

      // Note for the playback routine...

      // Clear our program area.
      if(nsfProgram.size() > 0)
         nsfProgram.resize(0);

      // JSR $AAAA
      jumpLow = nsf.playAddress & 0x00FF;
      jumpHigh = (nsf.playAddress & 0xFF00) >> 8;

      nsfProgram.push_back(0x20);
      nsfProgram.push_back(jumpLow);
      nsfProgram.push_back(jumpHigh);

      // JMP $AAAA - Repeatedly jumps to itself to harmlessly idle the CPU once the JSR has finished.
      const uint16 jumpAddress = 0x1000 + nsfProgram.size(); // hehe
      jumpLow = jumpAddress & 0x00FF;
      jumpHigh = (jumpAddress & 0xFF00) >> 8;

      nsfProgram.push_back(0x4C);
      nsfProgram.push_back(jumpLow);
      nsfProgram.push_back(jumpHigh);

      // Fill the rest with NOPs, just in case.
      while(nsfProgram.size() < 2048)
         nsfProgram.push_back(0xEA);

      // Upload to $1000 - $17FF.
      cpu_set_read_address_2k(0x1000, &nsfProgram[0]);

      // Update song counter.
      nsfCurrentSong = song;
   }

   // Set the program counter to the address of our program.
   cpu_context.PC.word = 0x1000;
}

static real bandpass(uint16* buffer, unsigned size, real sampleRate, int channels, real cutoffLow, real cutoffHigh)
{
   /* Helper function for the frequency spectrum visualizer.  When passed a buffer, the samples within it are analyzed and
      a (crude) band pass filter performed on them using the specified cutoff frequencies.
      The resulting power level (average value) is then returned.
      The buffer is expected to consist of unsigned 16bit samples. */

   RT_ASSERT(buffer);

   // Lowpass.
   real timer = 0.0;
   real period = sampleRate / Maximum(Epsilon, cutoffHigh);

   int32 accumulator = 0;
   int counter = 0;

   for(unsigned offset = 0; offset < size; offset++) {
      if(timer > 0.0) {
         timer--;
         if(timer > 0.0)
            continue;
      }

      timer += period;

      int32 sample = 0;
      for(int channel = 0; channel < channels; channel++)
         sample += (int16)(buffer[(offset * channels) + channel] ^ 0x8000);

      accumulator += (int16)Round((real)sample / channels);
      counter++;
   }

   accumulator = (int32)Round((real)accumulator / counter);

   // Highpass.
   timer = 0.0;
   period = sampleRate / Maximum(Epsilon, cutoffLow);

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

      int32 sample = 0;
      for(int channel = 0; channel < channels; channel++)
         sample += (int16)(buffer[(offset * channels) + channel] ^ 0x8000);

      accumulator += (int16)Round((real)sample / channels);
      counter++;
   }

   accumulator = (int32)Round((real)accumulator / counter);
   accumulator = saved_accumulator - accumulator;

   return accumulator / 32768.0;
}
