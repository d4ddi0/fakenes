

static int main_menu_load_rom (void);

static int main_menu_resume (void);

static int main_menu_snapshot (void);

static int main_menu_messages (void);

static int main_menu_exit (void);


static MENU main_menu [] =
{
    { "&Load ROM...", main_menu_load_rom, NIL, 0, NIL },
    {             "",                NIL, NIL, 0, NIL },
    {      "&Resume",   main_menu_resume, NIL, 0, NIL },
    {             "",                NIL, NIL, 0, NIL },
    {    "&Snapshot", main_menu_snapshot, NIL, 0, NIL },
    {             "",                NIL, NIL, 0, NIL },
    { "&Messages...", main_menu_messages, NIL, 0, NIL },
    {             "",                NIL, NIL, 0, NIL },
    {        "E&xit",     main_menu_exit, NIL, 0, NIL },
    {            NIL,                NIL, NIL, 0, NIL }
};


static int machine_speed_menu_ntsc_60_hz (void);

static int machine_speed_menu_pal_50_hz (void);


static MENU machine_speed_menu [] =
{
    { "&NTSC (60 Hz)", machine_speed_menu_ntsc_60_hz, NIL, 0, NIL },
    {              "",                           NIL, NIL, 0, NIL },
    {  "&PAL (50 Hz)",  machine_speed_menu_pal_50_hz, NIL, 0, NIL },
    {             NIL,                           NIL, NIL, 0, NIL }
};


static int machine_state_select_menu_0 (void);

static int machine_state_select_menu_1 (void);

static int machine_state_select_menu_2 (void);

static int machine_state_select_menu_3 (void);

static int machine_state_select_menu_4 (void);

static int machine_state_select_menu_5 (void);

static int machine_state_select_menu_6 (void);

static int machine_state_select_menu_7 (void);

static int machine_state_select_menu_8 (void);

static int machine_state_select_menu_9 (void);


static MENU machine_state_select_menu [] =
{
    { NIL, machine_state_select_menu_0, NIL, 0, NIL },
    {  "",                         NIL, NIL, 0, NIL },
    { NIL, machine_state_select_menu_1, NIL, 0, NIL },
    {  "",                         NIL, NIL, 0, NIL },
    { NIL, machine_state_select_menu_2, NIL, 0, NIL },
    {  "",                         NIL, NIL, 0, NIL },
    { NIL, machine_state_select_menu_3, NIL, 0, NIL },
    {  "",                         NIL, NIL, 0, NIL },
    { NIL, machine_state_select_menu_4, NIL, 0, NIL },
    {  "",                         NIL, NIL, 0, NIL },
    { NIL, machine_state_select_menu_5, NIL, 0, NIL },
    {  "",                         NIL, NIL, 0, NIL },
    { NIL, machine_state_select_menu_6, NIL, 0, NIL },
    {  "",                         NIL, NIL, 0, NIL },
    { NIL, machine_state_select_menu_7, NIL, 0, NIL },
    {  "",                         NIL, NIL, 0, NIL },
    { NIL, machine_state_select_menu_8, NIL, 0, NIL },
    {  "",                         NIL, NIL, 0, NIL },
    { NIL, machine_state_select_menu_9, NIL, 0, NIL },
    { NIL,                         NIL, NIL, 0, NIL }
};


static int machine_state_autosave_menu_disabled (void);

static int machine_state_autosave_menu_10_seconds (void);

static int machine_state_autosave_menu_30_seconds (void);

static int machine_state_autosave_menu_60_seconds (void);


static MENU machine_state_autosave_menu [] =
{
    {      "&Disabled",   machine_state_autosave_menu_disabled, NIL, 0, NIL },
    {               "",                                    NIL, NIL, 0, NIL },
    { "&1: 10 Seconds", machine_state_autosave_menu_10_seconds, NIL, 0, NIL },
    {               "",                                    NIL, NIL, 0, NIL },
    { "&2: 30 Seconds", machine_state_autosave_menu_30_seconds, NIL, 0, NIL },
    {               "",                                    NIL, NIL, 0, NIL },
    { "&3: 60 Seconds", machine_state_autosave_menu_60_seconds, NIL, 0, NIL },
    {              NIL,                                    NIL, NIL, 0, NIL }
};


static int machine_state_menu_select (void);

static int machine_state_menu_save (void);

static int machine_state_menu_restore (void);


static MENU machine_state_menu [] =
{
    {   "S&elect",                        NIL,   machine_state_select_menu, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    {     "&Save",    machine_state_menu_save,                         NIL, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    {  "&Restore", machine_state_menu_restore,                         NIL, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    { "&Autosave",                        NIL, machine_state_autosave_menu, 0, NIL },
    {         NIL,                        NIL,                         NIL, 0, NIL }
};


static int machine_menu_reset (void);

static int machine_menu_status (void);

static int machine_menu_patches (void);


static MENU machine_menu [] =
{
    {      "&Reset",   machine_menu_reset,                NIL, 0, NIL },
    {            "",                  NIL,                NIL, 0, NIL },
    {     "&Status",  machine_menu_status,                NIL, 0, NIL },
    {            "",                  NIL,                NIL, 0, NIL },
    {      "S&peed",                  NIL, machine_speed_menu, 0, NIL },
    {            "",                  NIL,                NIL, 0, NIL },
    { "&Patches...", machine_menu_patches,                NIL, 0, NIL },
    {            "",                  NIL,                NIL, 0, NIL },
    {      "St&ate",                  NIL, machine_state_menu, 0, NIL },
    {           NIL,                  NIL,                NIL, 0, NIL }
};


static int netplay_protocol_menu_tcpip (void);

static int netplay_protocol_menu_spx (void);


static MENU netplay_protocol_menu [] =
{
    { "&TCP/IP", netplay_protocol_menu_tcpip, NIL, 0, NIL },
    {        "",                         NIL, NIL, 0, NIL },
    {    "&SPX",   netplay_protocol_menu_spx, NIL, 0, NIL },
    {       NIL,                         NIL, NIL, 0, NIL }
};


static int netplay_server_menu_start (void);

static int netplay_server_menu_stop (void);


static MENU netplay_server_menu [] =
{
    { "&Start", netplay_server_menu_start, NIL,          0, NIL },
    {       "",                       NIL, NIL,          0, NIL },
    {  "S&top",  netplay_server_menu_stop, NIL, D_DISABLED, NIL },
    {      NIL,                       NIL, NIL,          0, NIL }
};


static int netplay_client_menu_connect (void);

static int netplay_client_menu_disconnect (void);


static MENU netplay_client_menu [] =
{
    { "&Connect...",    netplay_client_menu_connect, NIL,          0, NIL },
    {            "",                            NIL, NIL,          0, NIL },
    { "&Disconnect", netplay_client_menu_disconnect, NIL, D_DISABLED, NIL },
    {           NIL,                            NIL, NIL,          0, NIL }
};


static MENU netplay_menu [] =
{
    { "&Protocol", NIL, netplay_protocol_menu, 0, NIL },
    {          "", NIL,                   NIL, 0, NIL },
    {   "&Server", NIL,   netplay_server_menu, 0, NIL },
    {          "", NIL,                   NIL, 0, NIL },
    {   "&Client", NIL,   netplay_client_menu, 0, NIL },
    {         NIL, NIL,                   NIL, 0, NIL }
};


static int options_audio_mixing_channels_stereo_menu_classic (void);

static int options_audio_mixing_channels_stereo_menu_enhanced (void);

static int options_audio_mixing_channels_stereo_menu_accurate (void);


static MENU options_audio_mixing_channels_stereo_menu [] =
{
    {  "&Classic",  options_audio_mixing_channels_stereo_menu_classic, NIL, 0, NIL },
    {          "",                                                NIL, NIL, 0, NIL },
    { "&Enhanced", options_audio_mixing_channels_stereo_menu_enhanced, NIL, 0, NIL },
    {          "",                                                NIL, NIL, 0, NIL },
    { "&Accurate", options_audio_mixing_channels_stereo_menu_accurate, NIL, 0, NIL },
    {         NIL,                                                NIL, NIL, 0, NIL }
};


static int options_audio_mixing_channels_menu_mono (void);


static MENU options_audio_mixing_channels_menu [] =
{
    {   "&Mono", options_audio_mixing_channels_menu_mono,                                       NIL, 0, NIL },
    {        "",                                     NIL,                                       NIL, 0, NIL },
    { "&Stereo",                                     NIL, options_audio_mixing_channels_stereo_menu, 0, NIL },
    {       NIL,                                     NIL,                                       NIL, 0, NIL }
};


static int options_audio_mixing_speed_menu_8000_hz (void);

static int options_audio_mixing_speed_menu_11025_hz (void);

static int options_audio_mixing_speed_menu_16000_hz (void);

static int options_audio_mixing_speed_menu_22050_hz (void);

static int options_audio_mixing_speed_menu_32000_hz (void);

static int options_audio_mixing_speed_menu_44100_hz (void);

static int options_audio_mixing_speed_menu_48000_hz (void);

static int options_audio_mixing_speed_menu_80200_hz (void);

static int options_audio_mixing_speed_menu_96000_hz (void);


static MENU options_audio_mixing_speed_menu [] =
{
    {  "&1: 8000 Hz",  options_audio_mixing_speed_menu_8000_hz, NIL, 0, NIL },
    {             "",                                      NIL, NIL, 0, NIL },
    { "&2: 11025 Hz", options_audio_mixing_speed_menu_11025_hz, NIL, 0, NIL },
    {             "",                                      NIL, NIL, 0, NIL },
    { "&3: 16000 Hz", options_audio_mixing_speed_menu_16000_hz, NIL, 0, NIL },
    {             "",                                      NIL, NIL, 0, NIL },
    { "&4: 22050 Hz", options_audio_mixing_speed_menu_22050_hz, NIL, 0, NIL },
    {             "",                                      NIL, NIL, 0, NIL },
    { "&5: 32000 Hz", options_audio_mixing_speed_menu_32000_hz, NIL, 0, NIL },
    {             "",                                      NIL, NIL, 0, NIL },
    { "&6: 44100 Hz", options_audio_mixing_speed_menu_44100_hz, NIL, 0, NIL },
    {             "",                                      NIL, NIL, 0, NIL },
    { "&7: 48000 Hz", options_audio_mixing_speed_menu_48000_hz, NIL, 0, NIL },
    {             "",                                      NIL, NIL, 0, NIL },
    { "&8: 80200 Hz", options_audio_mixing_speed_menu_80200_hz, NIL, 0, NIL },
    {             "",                                      NIL, NIL, 0, NIL },
    { "&9: 96000 Hz", options_audio_mixing_speed_menu_96000_hz, NIL, 0, NIL },
    {            NIL,                                      NIL, NIL, 0, NIL }
};


static int options_audio_mixing_quality_menu_low_8_bit (void);

static int options_audio_mixing_quality_menu_high_16_bit (void);

static int options_audio_mixing_quality_menu_dithering (void);


static MENU options_audio_mixing_quality_menu [] =
{
    {   "&Low (8 bits)",   options_audio_mixing_quality_menu_low_8_bit, NIL, 0, NIL },
    {                "",                                           NIL, NIL, 0, NIL },
    { "&High (16 bits)", options_audio_mixing_quality_menu_high_16_bit, NIL, 0, NIL },
    {                "",                                           NIL, NIL, 0, NIL },
    {      "&Dithering",   options_audio_mixing_quality_menu_dithering, NIL, 0, NIL },
    {               NIL,                                           NIL, NIL, 0, NIL }
};


static int options_audio_mixing_advanced_menu_reverse_stereo (void);


static MENU options_audio_mixing_advanced_menu [] =
{
    { "&Reverse Stereo", options_audio_mixing_advanced_menu_reverse_stereo, NIL, 0, NIL },
    {               NIL,                                               NIL, NIL, 0, NIL }
};


static MENU options_audio_mixing_menu [] =
{
    {    "&Speed", NIL,    options_audio_mixing_speed_menu, 0, NIL },
    {          "", NIL,                                NIL, 0, NIL },
    { "&Channels", NIL, options_audio_mixing_channels_menu, 0, NIL },
    {          "", NIL,                                NIL, 0, NIL },
    {  "&Quality", NIL,  options_audio_mixing_quality_menu, 0, NIL },
    {          "", NIL,                                NIL, 0, NIL },
    { "&Advanced", NIL, options_audio_mixing_advanced_menu, 0, NIL },
    {         NIL, NIL,                                NIL, 0, NIL }
};


static int options_audio_effects_menu_linear_echo (void);

static int options_audio_effects_menu_surround_sound (void);


static MENU options_audio_effects_menu [] =
{
    {    "&Linear Echo",    options_audio_effects_menu_linear_echo, NIL, 0, NIL },
    {                "",                                       NIL, NIL, 0, NIL },
    { "&Surround Sound", options_audio_effects_menu_surround_sound, NIL, 0, NIL },
    {               NIL,                                       NIL, NIL, 0, NIL }
};


static int options_audio_filter_low_pass_menu_simple (void);

static int options_audio_filter_low_pass_menu_weighted (void);

static int options_audio_filter_low_pass_menu_dynamic (void);


static MENU options_audio_filter_low_pass_menu [] =
{
    {   "&Simple",   options_audio_filter_low_pass_menu_simple, NIL, 0, NIL },
    {          "",                                         NIL, NIL, 0, NIL },
    { "&Weighted", options_audio_filter_low_pass_menu_weighted, NIL, 0, NIL },
    {          "",                                         NIL, NIL, 0, NIL },
    {  "&Dynamic",  options_audio_filter_low_pass_menu_dynamic, NIL, 0, NIL },
    {         NIL,                                         NIL, NIL, 0, NIL }
};


static int options_audio_filter_menu_none (void);

static int options_audio_filter_menu_high_pass (void);


static MENU options_audio_filter_menu [] =
{     
    {      "&None",      options_audio_filter_menu_none,                                NIL, 0, NIL },
    {           "",                                 NIL,                                NIL, 0, NIL },
    {  "&Low Pass",                                 NIL, options_audio_filter_low_pass_menu, 0, NIL },
    {           "",                                 NIL,                                NIL, 0, NIL },
    { "&High Pass", options_audio_filter_menu_high_pass,                                NIL, 0, NIL },
    {          NIL,                                 NIL,                                NIL, 0, NIL }
};


static int options_audio_channels_menu_square_1 (void);

static int options_audio_channels_menu_square_2 (void);

static int options_audio_channels_menu_triangle (void);

static int options_audio_channels_menu_noise (void);

static int options_audio_channels_menu_dmc (void);

static int options_audio_channels_menu_exsound (void);


static MENU options_audio_channels_menu [] =
{
    { "&Square 1", options_audio_channels_menu_square_1, NIL, 0, NIL },
    {          "",                                  NIL, NIL, 0, NIL },
    { "S&quare 2", options_audio_channels_menu_square_2, NIL, 0, NIL },
    {          "",                                  NIL, NIL, 0, NIL },
    { "&Triangle", options_audio_channels_menu_triangle, NIL, 0, NIL },
    {          "",                                  NIL, NIL, 0, NIL },
    {    "&Noise",    options_audio_channels_menu_noise, NIL, 0, NIL },
    {          "",                                  NIL, NIL, 0, NIL },
    {      "&DMC",      options_audio_channels_menu_dmc, NIL, 0, NIL },
    {          "",                                  NIL, NIL, 0, NIL },
    {  "&ExSound",  options_audio_channels_menu_exsound, NIL, 0, NIL },
    {         NIL,                                  NIL, NIL, 0, NIL }
};


static int options_audio_advanced_menu_ideal_triangle (void);


static MENU options_audio_advanced_menu [] =
{
    { "&Ideal Triangle", options_audio_advanced_menu_ideal_triangle, NIL, 0, NIL },
    {               NIL,                                        NIL, NIL, 0, NIL }
};


static int options_audio_record_menu_start (void);

static int options_audio_record_menu_stop (void);


static MENU options_audio_record_menu [] =
{
    { "&Start", options_audio_record_menu_start, NIL,          0, NIL },
    {       "",                             NIL, NIL,          0, NIL },
    {  "S&top",  options_audio_record_menu_stop, NIL, D_DISABLED, NIL },
    {      NIL,                             NIL, NIL,          0, NIL }
};


static int options_audio_menu_enabled (void);


static MENU options_audio_menu [] =
{
    {  "&Enabled", options_audio_menu_enabled,                         NIL, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    {   "&Mixing",                        NIL,   options_audio_mixing_menu, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    {  "Effec&ts",                        NIL,  options_audio_effects_menu, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    {   "&Filter",                        NIL,   options_audio_filter_menu, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    { "&Channels",                        NIL, options_audio_channels_menu, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    { "&Advanced",                        NIL, options_audio_advanced_menu, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    {   "&Record",                        NIL,   options_audio_record_menu, 0, NIL },
    {         NIL,                        NIL,                         NIL, 0, NIL }
};


static int options_video_resolution_menu_256_224 (void);

static int options_video_resolution_menu_256_240 (void);

static int options_video_resolution_menu_256_256 (void);

static int options_video_resolution_menu_320_240 (void);

static int options_video_resolution_menu_400_300 (void);

static int options_video_resolution_menu_512_384 (void);

static int options_video_resolution_menu_512_480 (void);

static int options_video_resolution_menu_640_400 (void);

static int options_video_resolution_menu_640_480 (void);


static MENU options_video_resolution_menu [] =
{
    { "&1: 256x224", options_video_resolution_menu_256_224, NIL, 0, NIL },
    {            "",                                   NIL, NIL, 0, NIL },
    { "&2: 256x240", options_video_resolution_menu_256_240, NIL, 0, NIL },
    {            "",                                   NIL, NIL, 0, NIL },
    { "&3: 256x256", options_video_resolution_menu_256_256, NIL, 0, NIL },
    {            "",                                   NIL, NIL, 0, NIL },
    { "&4: 320x240", options_video_resolution_menu_320_240, NIL, 0, NIL },
    {            "",                                   NIL, NIL, 0, NIL },
    { "&5: 400x300", options_video_resolution_menu_400_300, NIL, 0, NIL },
    {            "",                                   NIL, NIL, 0, NIL },
    { "&6: 512x384", options_video_resolution_menu_512_384, NIL, 0, NIL },
    {            "",                                   NIL, NIL, 0, NIL },
    { "&7: 512x480", options_video_resolution_menu_512_480, NIL, 0, NIL },
    {            "",                                   NIL, NIL, 0, NIL },
    { "&8: 640x400", options_video_resolution_menu_640_400, NIL, 0, NIL },
    {            "",                                   NIL, NIL, 0, NIL },
    { "&9: 640x480", options_video_resolution_menu_640_480, NIL, 0, NIL },
    {           NIL,                                   NIL, NIL, 0, NIL }
};


static int options_video_colors_menu_few_8_bit (void);

static int options_video_colors_menu_many_15_bit (void);

static int options_video_colors_menu_lots_16_bit (void);


static MENU options_video_colors_menu [] =
{
    {   "&Few (8-bit)",   options_video_colors_menu_few_8_bit, NIL, 0, NIL },
    {               "",                                   NIL, NIL, 0, NIL },
    { "&Many (15-bit)", options_video_colors_menu_many_15_bit, NIL, 0, NIL },
    {               "",                                   NIL, NIL, 0, NIL },
    { "&Lots (16-bit)", options_video_colors_menu_lots_16_bit, NIL, 0, NIL },
    {              NIL,                                   NIL, NIL, 0, NIL }
};


static int options_video_blitter_menu_automatic (void);

static int options_video_blitter_menu_normal (void);

static int options_video_blitter_menu_stretched (void);

static int options_video_blitter_menu_interpolated (void);

static int options_video_blitter_menu_2xsoe (void);

static int options_video_blitter_menu_2xscl (void);

static int options_video_blitter_menu_super_2xsoe (void);

static int options_video_blitter_menu_super_2xscl (void);


static MENU options_video_blitter_menu [] =
{
    {    "&Automatic",    options_video_blitter_menu_automatic, NIL, 0, NIL },
    {              "",                                     NIL, NIL, 0, NIL },
    {       "&Normal",       options_video_blitter_menu_normal, NIL, 0, NIL },
    {              "",                                     NIL, NIL, 0, NIL },
    {    "&Stretched",    options_video_blitter_menu_stretched, NIL, 0, NIL },
    {              "",                                     NIL, NIL, 0, NIL },
    { "&Interpolated", options_video_blitter_menu_interpolated, NIL, 0, NIL },
    {              "",                                     NIL, NIL, 0, NIL },
    {        "&2xSOE",        options_video_blitter_menu_2xsoe, NIL, 0, NIL },
    {              "",                                     NIL, NIL, 0, NIL },
    {        "2&xSCL",        options_video_blitter_menu_2xscl, NIL, 0, NIL },
    {              "",                                     NIL, NIL, 0, NIL },
    {  "Sup&er 2xSOE",  options_video_blitter_menu_super_2xsoe, NIL, 0, NIL },
    {              "",                                     NIL, NIL, 0, NIL },
    {  "Su&per 2xSCL",  options_video_blitter_menu_super_2xscl, NIL, 0, NIL },
    {             NIL,                                     NIL, NIL, 0, NIL }
};


static int options_video_filters_scanlines_menu_high (void);

static int options_video_filters_scanlines_menu_medium (void);

static int options_video_filters_scanlines_menu_low (void);


static MENU options_video_filters_scanlines_menu [] =
{
    {  "&High (100%)",   options_video_filters_scanlines_menu_high, NIL, 0, NIL },
    {              "",                                         NIL, NIL, 0, NIL },
    { "&Medium (50%)", options_video_filters_scanlines_menu_medium, NIL, 0, NIL },
    {              "",                                         NIL, NIL, 0, NIL },
    {    "&Low (25%)",    options_video_filters_scanlines_menu_low, NIL, 0, NIL },
    {             NIL,                                         NIL, NIL, 0, NIL }
};


static MENU options_video_filters_menu [] =
{
    { "&Scanlines", NIL, options_video_filters_scanlines_menu, 0, NIL },
    {          NIL, NIL,                                  NIL, 0, NIL }
};


static int options_video_layers_menu_sprites_a (void);

static int options_video_layers_menu_sprites_b (void);

static int options_video_layers_menu_background (void);


static MENU options_video_layers_menu [] =
{
    {  "&Sprites A",  options_video_layers_menu_sprites_a, NIL, 0, NIL },
    {            "",                                  NIL, NIL, 0, NIL },
    {  "S&prites B",  options_video_layers_menu_sprites_b, NIL, 0, NIL },
    {            "",                                  NIL, NIL, 0, NIL },
    { "&Background", options_video_layers_menu_background, NIL, 0, NIL },
    {           NIL,                                  NIL, NIL, 0, NIL }
};


static int options_video_palette_menu_default (void);

static int options_video_palette_menu_grayscale (void);

static int options_video_palette_menu_gnuboy (void);

static int options_video_palette_menu_nester (void);

static int options_video_palette_menu_nesticle (void);

static int options_video_palette_menu_custom (void);


static MENU options_video_palette_menu [] =
{
    {   "&Default",   options_video_palette_menu_default, NIL, 0, NIL },
    {           "",                                  NIL, NIL, 0, NIL },
    { "&Grayscale", options_video_palette_menu_grayscale, NIL, 0, NIL },
    {           "",                                  NIL, NIL, 0, NIL },
    {    "gn&uboy",    options_video_palette_menu_gnuboy, NIL, 0, NIL },
    {           "",                                  NIL, NIL, 0, NIL },
    {    "&NESter",    options_video_palette_menu_nester, NIL, 0, NIL },
    {           "",                                  NIL, NIL, 0, NIL },
    {  "N&ESticle",  options_video_palette_menu_nesticle, NIL, 0, NIL },
    {           "",                                  NIL, NIL, 0, NIL },
    {    "&Custom",    options_video_palette_menu_custom, NIL, 0, NIL },
    {          NIL,                                  NIL, NIL, 0, NIL }
};


static int options_video_advanced_menu_force_window (void);


static MENU options_video_advanced_menu [] =
{
    { "&Force Window", options_video_advanced_menu_force_window, NIL, 0, NIL },
    {             NIL,                                      NIL, NIL, 0, NIL }
};


static int options_video_menu_vsync (void);


static MENU options_video_menu [] =
{
    { "&Resolution",                        NIL, options_video_resolution_menu, 0, NIL },
    {            "",                        NIL,                           NIL, 0, NIL },
    {     "&Colors",                        NIL,     options_video_colors_menu, 0, NIL },
    {            "",                        NIL,                           NIL, 0, NIL },
    {    "&Blitter",                        NIL,    options_video_blitter_menu, 0, NIL },
    {            "",                        NIL,                           NIL, 0, NIL },
    {    "&Filters",                        NIL,    options_video_filters_menu, 0, NIL },
    {            "",                        NIL,                           NIL, 0, NIL },
    {      "&VSync",   options_video_menu_vsync,                           NIL, 0, NIL },
    {            "",                        NIL,                           NIL, 0, NIL },
    {     "&Layers",                        NIL,     options_video_layers_menu, 0, NIL },
    {            "",                        NIL,                           NIL, 0, NIL },
    {    "&Palette",                        NIL,    options_video_palette_menu, 0, NIL },
    {            "",                        NIL,                           NIL, 0, NIL },
    {   "&Advanced",                        NIL,   options_video_advanced_menu, 0, NIL },
    {           NIL ,                       NIL,                           NIL, 0, NIL }
};


static MENU options_menu [] =
{
    { "&Audio", NIL, options_audio_menu, 0, NIL },
    {       "", NIL,                NIL, 0, NIL },
    { "&Video", NIL, options_video_menu, 0, NIL },
    {      NIL, NIL,                NIL, 0, NIL }
};


static int help_menu_shortcuts (void);

static int help_menu_about (void);


static MENU help_menu [] =
{
    { "&Shortcuts...", help_menu_shortcuts, NIL, 0, NIL },
    {              "",                 NIL, NIL, 0, NIL },
    {     "&About...",     help_menu_about, NIL, 0, NIL },
    {             NIL,                 NIL, NIL, 0, NIL }
};


static MENU top_menu [] =
{ 
    {    "&Main", NIL,    main_menu, 0, NIL },
    { "M&achine", NIL, machine_menu, 0, NIL },
    { "&Options", NIL, options_menu, 0, NIL },
    { "&NetPlay", NIL, netplay_menu, 0, NIL },
    {    "&Help", NIL,    help_menu, 0, NIL },
    {        NIL, NIL,          NIL, 0, NIL }
};
