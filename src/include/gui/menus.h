

static int main_state_select_menu_0 (void);

static int main_state_select_menu_1 (void);

static int main_state_select_menu_2 (void);

static int main_state_select_menu_3 (void);

static int main_state_select_menu_4 (void);

static int main_state_select_menu_5 (void);

static int main_state_select_menu_6 (void);

static int main_state_select_menu_7 (void);

static int main_state_select_menu_8 (void);

static int main_state_select_menu_9 (void);


static MENU main_state_select_menu [] =
{
    { NIL, main_state_select_menu_0, NIL, 0, NIL },
    {  "",                      NIL, NIL, 0, NIL },
    { NIL, main_state_select_menu_1, NIL, 0, NIL },
    {  "",                      NIL, NIL, 0, NIL },
    { NIL, main_state_select_menu_2, NIL, 0, NIL },
    {  "",                      NIL, NIL, 0, NIL },
    { NIL, main_state_select_menu_3, NIL, 0, NIL },
    {  "",                      NIL, NIL, 0, NIL },
    { NIL, main_state_select_menu_4, NIL, 0, NIL },
    {  "",                      NIL, NIL, 0, NIL },
    { NIL, main_state_select_menu_5, NIL, 0, NIL },
    {  "",                      NIL, NIL, 0, NIL },
    { NIL, main_state_select_menu_6, NIL, 0, NIL },
    {  "",                      NIL, NIL, 0, NIL },
    { NIL, main_state_select_menu_7, NIL, 0, NIL },
    {  "",                      NIL, NIL, 0, NIL },
    { NIL, main_state_select_menu_8, NIL, 0, NIL },
    {  "",                      NIL, NIL, 0, NIL },
    { NIL, main_state_select_menu_9, NIL, 0, NIL },
    { NIL,                      NIL, NIL, 0, NIL }
};


static int main_state_autosave_menu_disabled (void);

static int main_state_autosave_menu_10_seconds (void);

static int main_state_autosave_menu_30_seconds (void);

static int main_state_autosave_menu_60_seconds (void);


static MENU main_state_autosave_menu [] =
{
    {      "&Disabled",   main_state_autosave_menu_disabled, NIL, 0, NIL },
    {               "",                                 NIL, NIL, 0, NIL },
    { "&1: 10 Seconds", main_state_autosave_menu_10_seconds, NIL, 0, NIL },
    {               "",                                 NIL, NIL, 0, NIL },
    { "&2: 30 Seconds", main_state_autosave_menu_30_seconds, NIL, 0, NIL },
    {               "",                                 NIL, NIL, 0, NIL },
    { "&3: 60 Seconds", main_state_autosave_menu_60_seconds, NIL, 0, NIL },
    {              NIL,                                 NIL, NIL, 0, NIL }
};


static int main_state_menu_select (void);

static int main_state_menu_save (void);

static int main_state_menu_restore (void);


static MENU main_state_menu [] =
{
    {   "S&elect",                     NIL,   main_state_select_menu, 0, NIL },
    {          "",                     NIL,                         NIL, 0, NIL },
    {     "&Save",    main_state_menu_save,                         NIL, 0, NIL },
    {          "",                     NIL,                         NIL, 0, NIL },
    {  "&Restore", main_state_menu_restore,                         NIL, 0, NIL },
    {          "",                     NIL,                         NIL, 0, NIL },
    { "&Autosave",                     NIL, main_state_autosave_menu, 0, NIL },
    {         NIL,                     NIL,                         NIL, 0, NIL }
};


static int main_replay_select_menu_0 (void);

static int main_replay_select_menu_1 (void);

static int main_replay_select_menu_2 (void);

static int main_replay_select_menu_3 (void);

static int main_replay_select_menu_4 (void);


static MENU main_replay_select_menu [] =
{
    { NIL, main_replay_select_menu_0, NIL, 0, NIL },
    {  "",                       NIL, NIL, 0, NIL },
    { NIL, main_replay_select_menu_1, NIL, 0, NIL },
    {  "",                       NIL, NIL, 0, NIL },
    { NIL, main_replay_select_menu_2, NIL, 0, NIL },
    {  "",                       NIL, NIL, 0, NIL },
    { NIL, main_replay_select_menu_3, NIL, 0, NIL },
    {  "",                       NIL, NIL, 0, NIL },
    { NIL, main_replay_select_menu_4, NIL, 0, NIL },
    { NIL,                       NIL, NIL, 0, NIL }
};


static int main_replay_record_menu_start (void);

static int main_replay_record_menu_stop (void);


static MENU main_replay_record_menu [] =
{
    { "&Start", main_replay_record_menu_start, NIL,          0, NIL },
    {       "",                           NIL, NIL,          0, NIL },
    {  "S&top",  main_replay_record_menu_stop, NIL, D_DISABLED, NIL },
    {      NIL,                           NIL, NIL,          0, NIL }
};


static int main_replay_play_menu_start (void);

static int main_replay_play_menu_stop (void);


static MENU main_replay_play_menu [] =
{
    { "&Start", main_replay_play_menu_start, NIL,          0, NIL },
    {       "",                         NIL, NIL,          0, NIL },
    {  "S&top",  main_replay_play_menu_stop, NIL, D_DISABLED, NIL },
    {      NIL,                         NIL, NIL,          0, NIL }
};


static MENU main_replay_menu [] =
{
    { "&Select", NIL, main_replay_select_menu, 0, NIL },
    {        "", NIL,                     NIL, 0, NIL },
    { "&Record", NIL, main_replay_record_menu, 0, NIL },
    {        "", NIL,                     NIL, 0, NIL },
    {   "&Play", NIL,   main_replay_play_menu, 0, NIL },
    {       NIL, NIL,                     NIL, 0, NIL }
};


static int main_menu_load_rom (void);

static int main_menu_resume (void);

static int main_menu_reset (void);

static int main_menu_snapshot (void);

static int main_menu_messages (void);

static int main_menu_exit (void);


static MENU main_menu [] =
{
    { "&Load ROM...", main_menu_load_rom,              NIL, 0, NIL },
    {             "",                NIL,              NIL, 0, NIL },
    {      "&Resume",   main_menu_resume,              NIL, 0, NIL },
    {             "",                NIL,              NIL, 0, NIL },
    {       "R&eset",    main_menu_reset,              NIL, 0, NIL },
    {             "",                NIL,              NIL, 0, NIL },
    {       "&State",                NIL,  main_state_menu, 0, NIL },
    {             "",                NIL,              NIL, 0, NIL },
    {      "Re&play",                NIL, main_replay_menu, 0, NIL },
    {             "",                NIL,              NIL, 0, NIL },
    {    "S&napshot", main_menu_snapshot,              NIL, 0, NIL },
    {             "",                NIL,              NIL, 0, NIL },
    { "&Messages...", main_menu_messages,              NIL, 0, NIL },
    {             "",                NIL,              NIL, 0, NIL },
    {        "E&xit",     main_menu_exit,              NIL, 0, NIL },
    {            NIL,                NIL,              NIL, 0, NIL }
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


static int options_gui_theme_menu_classic (void);

static int options_gui_theme_menu_stainless_steel (void);

static int options_gui_theme_menu_zero_4 (void);


static MENU options_gui_theme_menu [] =
{
    {         "&1: Classic",         options_gui_theme_menu_classic, NIL, 0, NIL },
    {                    "",                                    NIL, NIL, 0, NIL },
    { "&2: stainless Steel", options_gui_theme_menu_stainless_steel, NIL, 0, NIL },
    {                    "",                                    NIL, NIL, 0, NIL },
    {          "&3: Zero 4",          options_gui_theme_menu_zero_4, NIL, 0, NIL },
    {                   NIL,                                    NIL, NIL, 0, NIL }
};


static MENU options_gui_menu [] =
{
    { "&Theme", NIL, options_gui_theme_menu, 0, NIL },
    {      NIL, NIL,                    NIL, 0, NIL }
};


static int options_system_menu_ntsc_60_hz (void);

static int options_system_menu_pal_50_hz (void);


static MENU options_system_menu [] =
{
    { "&NTSC (60 Hz)", options_system_menu_ntsc_60_hz, NIL, 0, NIL },
    {              "",                            NIL, NIL, 0, NIL },
    {  "&PAL (50 Hz)",  options_system_menu_pal_50_hz, NIL, 0, NIL },
    {             NIL,                            NIL, NIL, 0, NIL }
};


static int options_audio_mixing_channels_menu_mono (void);

static int options_audio_mixing_channels_menu_pseudo_stereo_mode_1 (void);

static int options_audio_mixing_channels_menu_pseudo_stereo_mode_2 (void);

static int options_audio_mixing_channels_menu_pseudo_stereo_mode_3 (void);


static MENU options_audio_mixing_channels_menu [] =
{
    {                   "&Mono",                 options_audio_mixing_channels_menu_mono, NIL, 0, NIL },
    {                        "",                                                     NIL, NIL, 0, NIL },
    { "Pseudo &Stereo (Mode 1)", options_audio_mixing_channels_menu_pseudo_stereo_mode_1, NIL, 0, NIL },
    {                        "",                                                     NIL, NIL, 0, NIL },
    { "Pseudo S&tereo (Mode 2)", options_audio_mixing_channels_menu_pseudo_stereo_mode_2, NIL, 0, NIL },
    {                        "",                                                     NIL, NIL, 0, NIL },
    { "Pseudo St&ereo (Mode 3)", options_audio_mixing_channels_menu_pseudo_stereo_mode_3, NIL, 0, NIL },
    {                       NIL,                                                     NIL, NIL, 0, NIL }
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

static int options_audio_effects_menu_spatial_stereo_mode_1 (void);

static int options_audio_effects_menu_spatial_stereo_mode_2 (void);

static int options_audio_effects_menu_spatial_stereo_mode_3 (void);


static MENU options_audio_effects_menu [] =
{
    {             "&Linear Echo",           options_audio_effects_menu_linear_echo, NIL, 0, NIL },
    {                         "",                                              NIL, NIL, 0, NIL },
    { "&Spatial Stereo (Mode 1)", options_audio_effects_menu_spatial_stereo_mode_1, NIL, 0, NIL },
    {                         "",                                              NIL, NIL, 0, NIL },
    { "S&patial Stereo (Mode 2)", options_audio_effects_menu_spatial_stereo_mode_2, NIL, 0, NIL },
    {                         "",                                              NIL, NIL, 0, NIL },
    { "Sp&atial Stereo (Mode 3)", options_audio_effects_menu_spatial_stereo_mode_3, NIL, 0, NIL },
    {                        NIL,                                              NIL, NIL, 0, NIL }
};


static int options_audio_filters_menu_low_pass_mode_1 (void);

static int options_audio_filters_menu_low_pass_mode_2 (void);

static int options_audio_filters_menu_low_pass_mode_3 (void);

static int options_audio_filters_menu_high_pass (void);


static MENU options_audio_filters_menu [] =
{     
    { "&Low Pass (Mode 1)", options_audio_filters_menu_low_pass_mode_1, NIL, 0, NIL },
    {                   "",                                        NIL, NIL, 0, NIL },
    { "L&ow Pass (Mode 2)", options_audio_filters_menu_low_pass_mode_2, NIL, 0, NIL },
    {                   "",                                        NIL, NIL, 0, NIL },
    { "Lo&w Pass (Mode 3)", options_audio_filters_menu_low_pass_mode_3, NIL, 0, NIL },
    {                   "",                                        NIL, NIL, 0, NIL },
    {         "&High Pass",       options_audio_filters_menu_high_pass, NIL, 0, NIL },
    {                  NIL,                                        NIL, NIL, 0, NIL }
};


static int options_audio_channels_menu_square_wave_a (void);

static int options_audio_channels_menu_square_wave_b (void);

static int options_audio_channels_menu_triangle_wave (void);

static int options_audio_channels_menu_white_noise (void);

static int options_audio_channels_menu_digital (void);

static int options_audio_channels_menu_extended (void);


static MENU options_audio_channels_menu [] =
{
    { "&Square Wave A", options_audio_channels_menu_square_wave_a, NIL, 0, NIL },
    {               "",                                       NIL, NIL, 0, NIL },
    { "S&quare Wave B", options_audio_channels_menu_square_wave_b, NIL, 0, NIL },
    {               "",                                       NIL, NIL, 0, NIL },
    { "&Triangle Wave", options_audio_channels_menu_triangle_wave, NIL, 0, NIL },
    {               "",                                       NIL, NIL, 0, NIL },
    {   "&White Noise",   options_audio_channels_menu_white_noise, NIL, 0, NIL },
    {               "",                                       NIL, NIL, 0, NIL },
    {       "&Digital",       options_audio_channels_menu_digital, NIL, 0, NIL },
    {               "",                                       NIL, NIL, 0, NIL },
    {      "&Extended",      options_audio_channels_menu_extended, NIL, 0, NIL },
    {              NIL,                                       NIL, NIL, 0, NIL }
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
    {  "&Filters",                        NIL,  options_audio_filters_menu, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    { "&Channels",                        NIL, options_audio_channels_menu, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    { "&Advanced",                        NIL, options_audio_advanced_menu, 0, NIL },
    {          "",                        NIL,                         NIL, 0, NIL },
    {   "&Record",                        NIL,   options_audio_record_menu, 0, NIL },
    {         NIL,                        NIL,                         NIL, 0, NIL }
};


#ifdef ALLEGRO_DOS

static int options_video_driver_dos_menu_vga (void);

static int options_video_driver_dos_menu_vga_mode_x (void);

static int options_video_driver_dos_menu_vesa (void);

static int options_video_driver_dos_menu_vesa_2_banked (void);

static int options_video_driver_dos_menu_vesa_2_linear (void);

static int options_video_driver_dos_menu_vesa_3 (void);

static int options_video_driver_dos_menu_vesa_vbe_af (void);


static MENU options_video_driver_dos_menu [] =
{
    {           "&VGA",           options_video_driver_dos_menu_vga, NIL, 0, NIL },
    {               "",                                         NIL, NIL, 0, NIL },
    {    "VGA Mode-&X",    options_video_driver_dos_menu_vga_mode_x, NIL, 0, NIL },
    {               "",                                         NIL, NIL, 0, NIL },
    {          "V&ESA",          options_video_driver_dos_menu_vesa, NIL, 0, NIL },
    {               "",                                         NIL, NIL, 0, NIL },
    { "VESA 2 &Banked", options_video_driver_dos_menu_vesa_2_banked, NIL, 0, NIL },
    {               "",                                         NIL, NIL, 0, NIL },
    { "VESA 2 &Linear", options_video_driver_dos_menu_vesa_2_linear, NIL, 0, NIL },
    {               "",                                         NIL, NIL, 0, NIL },
    {        "VE&SA 3",        options_video_driver_dos_menu_vesa_3, NIL, 0, NIL },
    {               "",                                         NIL, NIL, 0, NIL },
    {   "VESA VBE/&AF",   options_video_driver_dos_menu_vesa_vbe_af, NIL, 0, NIL },
    {              NIL,                                         NIL, NIL, 0, NIL }
};

#else

static MENU options_video_driver_dos_menu [] =
{
    { NIL, NIL, NIL, 0, NIL }
};

#endif


#ifdef ALLEGRO_WINDOWS

static int options_video_driver_windows_menu_directx (void);

static int options_video_driver_windows_menu_directx_window (void);

static int options_video_driver_windows_menu_directx_overlay (void);

static int options_video_driver_windows_menu_gdi (void);


static MENU options_video_driver_windows_menu [] =
{
    {         "&DirectX",         options_video_driver_windows_menu_directx, NIL, 0, NIL },
    {                 "",                                               NIL, NIL, 0, NIL },
    {  "DirectX &Window",  options_video_driver_windows_menu_directx_window, NIL, 0, NIL },
    {                 "",                                               NIL, NIL, 0, NIL },
    { "DirectX &Overlay", options_video_driver_windows_menu_directx_overlay, NIL, 0, NIL },
    {                 "",                                               NIL, NIL, 0, NIL },
    {             "&GDI",             options_video_driver_windows_menu_gdi, NIL, 0, NIL },
    {                NIL,                                               NIL, NIL, 0, NIL }
};

#else

static MENU options_video_driver_windows_menu [] =
{
    { NIL, NIL, NIL, 0, NIL }
};

#endif


#ifdef ALLEGRO_LINUX

static int options_video_driver_linux_menu_vga (void);

static int options_video_driver_linux_menu_vga_mode_x (void);

static int options_video_driver_linux_menu_vesa_vbe_af (void);

static int options_video_driver_linux_menu_framebuffer (void);

static int options_video_driver_linux_menu_svgalib (void);


static MENU options_video_driver_linux_menu [] =
{
    {         "&VGA",         options_video_driver_linux_menu_vga, NIL, 0, NIL },
    {             "",                                         NIL, NIL, 0, NIL },
    {  "VGA Mode-&X",  options_video_driver_linux_menu_vga_mode_x, NIL, 0, NIL },
    {             "",                                         NIL, NIL, 0, NIL },
    { "VESA VBE/&AF", options_video_driver_linux_menu_vesa_vbe_af, NIL, 0, NIL },
    {             "",                                         NIL, NIL, 0, NIL },
    { "&Framebuffer", options_video_driver_linux_menu_framebuffer, NIL, 0, NIL },
    {             "",                                         NIL, NIL, 0, NIL },
    {     "&SVGAlib",     options_video_driver_linux_menu_svgalib, NIL, 0, NIL },
    {            NIL,                                         NIL, NIL, 0, NIL }
};

#else

static MENU options_video_driver_linux_menu [] =
{
    { NIL, NIL, NIL, 0, NIL }
};

#endif


#ifdef ALLEGRO_UNIX

static int options_video_driver_unix_menu_x_windows (void);

static int options_video_driver_unix_menu_x_windows_full (void);

static int options_video_driver_unix_menu_x_dga (void);

static int options_video_driver_unix_menu_x_dga_full (void);

static int options_video_driver_unix_menu_x_dga_2 (void);


static MENU options_video_driver_unix_menu [] =
{
    {      "X &Windows",      options_video_driver_unix_menu_x_windows, NIL, 0, NIL },
    {                "",                                           NIL, NIL, 0, NIL },
    { "X &Windows Full", options_video_driver_unix_menu_x_windows_full, NIL, 0, NIL },
    {                "",                                           NIL, NIL, 0, NIL },
    {          "X/&DGA",          options_video_driver_unix_menu_x_dga, NIL, 0, NIL },
    {                "",                                           NIL, NIL, 0, NIL },
    {     "X/&DGA FULL",     options_video_driver_unix_menu_x_dga_full, NIL, 0, NIL },
    {                "",                                           NIL, NIL, 0, NIL },
    {        "X/D&GA 2",        options_video_driver_unix_menu_x_dga_2, NIL, 0, NIL },
    {               NIL,                                           NIL, NIL, 0, NIL }
};

#else

static MENU options_video_driver_unix_menu [] =
{
    { NIL, NIL, NIL, 0, NIL }
};

#endif


static int options_video_driver_menu_automatic (void);


static MENU options_video_driver_menu [] =
{
    { "&Automatic", options_video_driver_menu_automatic,                               NIL, 0, NIL },
    {           "",                                 NIL,                               NIL, 0, NIL },
    {       "&DOS",                                 NIL,     options_video_driver_dos_menu, 0, NIL },
    {           "",                                 NIL,                               NIL, 0, NIL },
    {   "&Windows",                                 NIL, options_video_driver_windows_menu, 0, NIL },
    {           "",                                 NIL,                               NIL, 0, NIL },
    {     "&Linux",                                 NIL,   options_video_driver_linux_menu, 0, NIL },
    {           "",                                 NIL,                               NIL, 0, NIL },
    {      "&Unix",                                 NIL,    options_video_driver_unix_menu, 0, NIL },
    {          NIL,                                 NIL,                               NIL, 0, NIL }
};


static int options_video_resolution_proportionate_menu_256_224 (void);

static int options_video_resolution_proportionate_menu_256_240 (void);

static int options_video_resolution_proportionate_menu_512_448 (void);

static int options_video_resolution_proportionate_menu_512_480 (void);

static int options_video_resolution_proportionate_menu_768_672 (void);

static int options_video_resolution_proportionate_menu_768_720 (void);

static int options_video_resolution_proportionate_menu_1024_896 (void);

static int options_video_resolution_proportionate_menu_1024_960 (void);

static int options_video_resolution_proportionate_menu_1280_1120 (void);

static int options_video_resolution_proportionate_menu_1280_1200 (void);


static MENU options_video_resolution_proportionate_menu [] =
{
    {   " &1: 256x224",   options_video_resolution_proportionate_menu_256_224, NIL, 0, NIL },
    {               "",                                                   NIL, NIL, 0, NIL },
    {   " &2: 256x240",   options_video_resolution_proportionate_menu_256_240, NIL, 0, NIL },
    {               "",                                                   NIL, NIL, 0, NIL },
    {   " &3: 512x448",   options_video_resolution_proportionate_menu_512_448, NIL, 0, NIL },
    {               "",                                                   NIL, NIL, 0, NIL },
    {   " &4: 512x480",   options_video_resolution_proportionate_menu_512_480, NIL, 0, NIL },
    {               "",                                                   NIL, NIL, 0, NIL },
    {   " &5: 768x672",   options_video_resolution_proportionate_menu_768_672, NIL, 0, NIL },
    {               "",                                                   NIL, NIL, 0, NIL },
    {   " &6: 768x720",   options_video_resolution_proportionate_menu_768_720, NIL, 0, NIL },
    {               "",                                                   NIL, NIL, 0, NIL },
    {  " &7: 1152x896",  options_video_resolution_proportionate_menu_1024_896, NIL, 0, NIL },
    {               "",                                                   NIL, NIL, 0, NIL },
    {  " &8: 1152x900",  options_video_resolution_proportionate_menu_1024_960, NIL, 0, NIL },
    {               "",                                                   NIL, NIL, 0, NIL },
    { " &9: 1280x1120", options_video_resolution_proportionate_menu_1280_1120, NIL, 0, NIL },
    {               "",                                                   NIL, NIL, 0, NIL },
    { "1&0: 1280x1200", options_video_resolution_proportionate_menu_1280_1200, NIL, 0, NIL },
    {              NIL,                                                   NIL, NIL, 0, NIL }
};


static int options_video_resolution_extended_menu_400_300 (void);

static int options_video_resolution_extended_menu_480_360 (void);

static int options_video_resolution_extended_menu_512_384 (void);

static int options_video_resolution_extended_menu_640_400 (void);

static int options_video_resolution_extended_menu_720_480 (void);

static int options_video_resolution_extended_menu_720_576 (void);

static int options_video_resolution_extended_menu_848_480 (void);

static int options_video_resolution_extended_menu_1280_720 (void);

static int options_video_resolution_extended_menu_1280_960 (void);

static int options_video_resolution_extended_menu_1360_768 (void);


static MENU options_video_resolution_extended_menu [] =
{
    {  " &1: 400x300",  options_video_resolution_extended_menu_400_300, NIL, 0, NIL },
    {              "",                                             NIL, NIL, 0, NIL },
    {  " &2: 480x360",  options_video_resolution_extended_menu_480_360, NIL, 0, NIL },
    {              "",                                             NIL, NIL, 0, NIL },
    {  " &3: 512x384",  options_video_resolution_extended_menu_512_384, NIL, 0, NIL },
    {              "",                                             NIL, NIL, 0, NIL },
    {  " &4: 640x400",  options_video_resolution_extended_menu_640_400, NIL, 0, NIL },
    {              "",                                             NIL, NIL, 0, NIL },
    {  " &5: 720x480",  options_video_resolution_extended_menu_720_480, NIL, 0, NIL },
    {              "",                                             NIL, NIL, 0, NIL },
    {  " &6: 720x576",  options_video_resolution_extended_menu_720_576, NIL, 0, NIL },
    {              "",                                             NIL, NIL, 0, NIL },
    {  " &7: 848x480",  options_video_resolution_extended_menu_848_480, NIL, 0, NIL },
    {              "",                                             NIL, NIL, 0, NIL },
    { " &8: 1280x720", options_video_resolution_extended_menu_1280_720, NIL, 0, NIL },
    {              "",                                             NIL, NIL, 0, NIL },
    { " &9: 1280x960", options_video_resolution_extended_menu_1280_960, NIL, 0, NIL },
    {              "",                                             NIL, NIL, 0, NIL },
    { "1&0: 1360x768", options_video_resolution_extended_menu_1360_768, NIL, 0, NIL },
    {             NIL,                                             NIL, NIL, 0, NIL }
};


static int options_video_resolution_menu_320_240 (void);

static int options_video_resolution_menu_640_480 (void);

static int options_video_resolution_menu_800_600 (void);

static int options_video_resolution_menu_1024_768 (void);

static int options_video_resolution_menu_1152_864 (void);

static int options_video_resolution_menu_1280_1024 (void);

static int options_video_resolution_menu_1600_1200 (void);


static MENU options_video_resolution_menu [] =
{
    { "&Proportionate",                                     NIL, options_video_resolution_proportionate_menu, 0, NIL },
    {               "",                                     NIL,                                         NIL, 0, NIL },
    {    "&1: 320x240",   options_video_resolution_menu_320_240,                                         NIL, 0, NIL },
    {               "",                                     NIL,                                         NIL, 0, NIL },
    {    "&2: 640x480",   options_video_resolution_menu_640_480,                                         NIL, 0, NIL },
    {               "",                                     NIL,                                         NIL, 0, NIL },
    {    "&3: 800x600",   options_video_resolution_menu_800_600,                                         NIL, 0, NIL },
    {               "",                                     NIL,                                         NIL, 0, NIL },
    {   "&4: 1024x768",  options_video_resolution_menu_1024_768,                                         NIL, 0, NIL },
    {               "",                                     NIL,                                         NIL, 0, NIL },
    {   "&5: 1152x864",  options_video_resolution_menu_1152_864,                                         NIL, 0, NIL },
    {               "",                                     NIL,                                         NIL, 0, NIL },
    {  "&6: 1280x1024", options_video_resolution_menu_1280_1024,                                         NIL, 0, NIL },
    {               "",                                     NIL,                                         NIL, 0, NIL },
    {  "&7: 1600x1200", options_video_resolution_menu_1600_1200,                                         NIL, 0, NIL },
    {               "",                                     NIL,                                         NIL, 0, NIL },
    {      "&Extended",                                     NIL,      options_video_resolution_extended_menu, 0, NIL },
    {              NIL,                                     NIL,                                         NIL, 0, NIL }
};


static int options_video_colors_menu_paletted_8_bit (void);

static int options_video_colors_menu_true_color_15_bit (void);

static int options_video_colors_menu_true_color_16_bit (void);

static int options_video_colors_menu_true_color_32_bit (void);


static MENU options_video_colors_menu [] =
{
    {    "&Paletted (8-bit)",    options_video_colors_menu_paletted_8_bit, NIL, 0, NIL },
    {                     "",                                         NIL, NIL, 0, NIL },
    { "&True Color (15-bit)", options_video_colors_menu_true_color_15_bit, NIL, 0, NIL },
    {                     "",                                         NIL, NIL, 0, NIL },
    { "T&rue Color (16-bit)", options_video_colors_menu_true_color_16_bit, NIL, 0, NIL },
    {                     "",                                         NIL, NIL, 0, NIL },
    { "Tr&ue Color (32-bit)", options_video_colors_menu_true_color_32_bit, NIL, 0, NIL },
    {                    NIL,                                         NIL, NIL, 0, NIL }
};


static int options_video_blitter_menu_automatic (void);

static int options_video_blitter_menu_normal (void);

static int options_video_blitter_menu_stretched (void);

static int options_video_blitter_menu_interpolated_2x (void);

static int options_video_blitter_menu_interpolated_3x (void);

static int options_video_blitter_menu_2xsoe (void);

static int options_video_blitter_menu_2xscl (void);

static int options_video_blitter_menu_super_2xsoe (void);

static int options_video_blitter_menu_super_2xscl (void);


static MENU options_video_blitter_menu [] =
{
    {          "&Automatic",       options_video_blitter_menu_automatic, NIL, 0, NIL },
    {                    "",                                        NIL, NIL, 0, NIL },
    {          "&1: Normal",          options_video_blitter_menu_normal, NIL, 0, NIL },
    {                    "",                                        NIL, NIL, 0, NIL },
    {       "&2: Stretched",       options_video_blitter_menu_stretched, NIL, 0, NIL },
    {                    "",                                        NIL, NIL, 0, NIL },
    { "&3: Interpolated 2X", options_video_blitter_menu_interpolated_2x, NIL, 0, NIL },
    {                    "",                                        NIL, NIL, 0, NIL },
    { "&4: Interpolated 3X", options_video_blitter_menu_interpolated_3x, NIL, 0, NIL },
    {                    "",                                        NIL, NIL, 0, NIL },
    {           "&5: 2xSOE",           options_video_blitter_menu_2xsoe, NIL, 0, NIL },
    {                    "",                                        NIL, NIL, 0, NIL },
    {           "&6: 2xSCL",           options_video_blitter_menu_2xscl, NIL, 0, NIL },
    {                    "",                                        NIL, NIL, 0, NIL },
    {     "&7: Super 2xSOE",     options_video_blitter_menu_super_2xsoe, NIL, 0, NIL },
    {                    "",                                        NIL, NIL, 0, NIL },
    {     "&8: Super 2xSCL",     options_video_blitter_menu_super_2xscl, NIL, 0, NIL },
    {                   NIL,                                        NIL, NIL, 0, NIL }
};


static int options_video_filters_menu_scanlines_25_percent (void);

static int options_video_filters_menu_scanlines_50_percent (void);

static int options_video_filters_menu_scanlines_100_percent (void);


static MENU options_video_filters_menu [] =
{
    {  "&Scanlines (25%)",  options_video_filters_menu_scanlines_25_percent, NIL, 0, NIL },
    {                  "",                                              NIL, NIL, 0, NIL },
    {  "S&canlines (50%)",  options_video_filters_menu_scanlines_50_percent, NIL, 0, NIL },
    {                  "",                                              NIL, NIL, 0, NIL },
    { "Sc&anlines (100%)", options_video_filters_menu_scanlines_100_percent, NIL, 0, NIL },
    {                 NIL,                                              NIL, NIL, 0, NIL }
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


static int options_video_palette_menu_ntsc_color (void);

static int options_video_palette_menu_ntsc_grayscale (void);

static int options_video_palette_menu_gnuboy (void);

static int options_video_palette_menu_nester (void);

static int options_video_palette_menu_nesticle (void);

static int options_video_palette_menu_custom (void);


static MENU options_video_palette_menu [] =
{
    {     "NTSC &Color",     options_video_palette_menu_ntsc_color, NIL, 0, NIL },
    {                "",                                       NIL, NIL, 0, NIL },
    { "NTSC &Grayscale", options_video_palette_menu_ntsc_grayscale, NIL, 0, NIL },
    {                "",                                       NIL, NIL, 0, NIL },
    {         "gn&uboy",         options_video_palette_menu_gnuboy, NIL, 0, NIL },
    {                "",                                       NIL, NIL, 0, NIL },
    {         "&NESter",         options_video_palette_menu_nester, NIL, 0, NIL },
    {                "",                                       NIL, NIL, 0, NIL },
    {       "N&ESticle",       options_video_palette_menu_nesticle, NIL, 0, NIL },
    {                "",                                       NIL, NIL, 0, NIL },
    {         "&Custom",         options_video_palette_menu_custom, NIL, 0, NIL },
    {               NIL,                                       NIL, NIL, 0, NIL }
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
    {     "&Driver",                        NIL,     options_video_driver_menu, 0, NIL },
    {            "",                        NIL,                           NIL, 0, NIL },
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


static int options_menu_status (void);

static int options_menu_input (void);

static int options_menu_patches (void);


static MENU options_menu [] =
{
    {     "&Status",  options_menu_status,                 NIL, 0, NIL },
    {            "",                  NIL,                 NIL, 0, NIL },
    {        "&GUI",                  NIL,    options_gui_menu, 0, NIL },
    {            "",                  NIL,                 NIL, 0, NIL },
    {     "S&ystem",                  NIL, options_system_menu, 0, NIL },
    {            "",                  NIL,                 NIL, 0, NIL },
    {      "&Audio",                  NIL,  options_audio_menu, 0, NIL },
    {            "",                  NIL,                 NIL, 0, NIL },
    {      "&Video",                  NIL,  options_video_menu, 0, NIL },
    {            "",                  NIL,                 NIL, 0, NIL },
    {   "&Input...",   options_menu_input,                 NIL, 0, NIL },
    {            "",                  NIL,                 NIL, 0, NIL },
    { "&Patches...", options_menu_patches,                 NIL, 0, NIL },
    {           NIL,                  NIL,                 NIL, 0, NIL }
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
    { "&Options", NIL, options_menu, 0, NIL },
    { "&NetPlay", NIL, netplay_menu, 0, NIL },
    {    "&Help", NIL,    help_menu, 0, NIL },
    {        NIL, NIL,          NIL, 0, NIL }
};
