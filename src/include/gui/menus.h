/* Define helper macros. */
#define DEFINE_MENU(name)           static MENU * name = NULL
#define DEFINE_MENU_CALLBACK(func)  static int func (void)
#define IMPORT_MENU(menu)           (MENU *) & menu
#define MENU_SPLITTER               { "",   NULL, NULL, 0, NULL }
#define MENU_ENDCAP                 { NULL, NULL, NULL, 0, NULL }

#define SET_FLAG(var, flag, value) \
   if (value)  \
      var |= flag;   \
   else  \
      var &= ~ flag; \

DEFINE_MENU(main_state_select_menu);
DEFINE_MENU(main_state_autosave_menu);
DEFINE_MENU(main_state_menu);
DEFINE_MENU(main_replay_select_menu);
DEFINE_MENU(main_replay_record_menu);
DEFINE_MENU(main_replay_play_menu);
DEFINE_MENU(main_replay_menu);
DEFINE_MENU(main_menu);
DEFINE_MENU(netplay_protocol_menu);
DEFINE_MENU(netplay_server_menu);
DEFINE_MENU(netplay_client_menu);
DEFINE_MENU(netplay_menu);
DEFINE_MENU(options_gui_theme_menu);
DEFINE_MENU(options_gui_menu);
DEFINE_MENU(options_system_menu);
DEFINE_MENU(options_audio_subsystem_menu);
DEFINE_MENU(options_audio_mixing_channels_menu);
DEFINE_MENU(options_audio_mixing_frequency_menu);
DEFINE_MENU(options_audio_mixing_quality_menu);
DEFINE_MENU(options_audio_mixing_anti_aliasing_menu);
DEFINE_MENU(options_audio_mixing_menu);
DEFINE_MENU(options_audio_effects_menu);
DEFINE_MENU(options_audio_filters_menu);
DEFINE_MENU(options_audio_channels_menu);
DEFINE_MENU(options_audio_advanced_menu);
DEFINE_MENU(options_audio_record_menu);
DEFINE_MENU(options_audio_menu);
DEFINE_MENU(options_video_driver_dos_menu);
DEFINE_MENU(options_video_driver_windows_menu);
DEFINE_MENU(options_video_driver_linux_menu);
DEFINE_MENU(options_video_driver_unix_menu);
DEFINE_MENU(options_video_driver_menu);
DEFINE_MENU(options_video_resolution_proportionate_menu);
DEFINE_MENU(options_video_resolution_extended_menu);
DEFINE_MENU(options_video_resolution_menu);
DEFINE_MENU(options_video_colors_menu);
DEFINE_MENU(options_video_blitter_menu);
DEFINE_MENU(options_video_filters_menu);
DEFINE_MENU(options_video_layers_menu);
DEFINE_MENU(options_video_palette_menu);
DEFINE_MENU(options_video_advanced_menu);
DEFINE_MENU(options_video_menu);
DEFINE_MENU(options_input_menu);
DEFINE_MENU(options_menu);
DEFINE_MENU(help_menu);
DEFINE_MENU(top_menu);

DEFINE_MENU_CALLBACK(main_state_select_menu_0);
DEFINE_MENU_CALLBACK(main_state_select_menu_1);
DEFINE_MENU_CALLBACK(main_state_select_menu_2);
DEFINE_MENU_CALLBACK(main_state_select_menu_3);
DEFINE_MENU_CALLBACK(main_state_select_menu_4);
DEFINE_MENU_CALLBACK(main_state_select_menu_5);
DEFINE_MENU_CALLBACK(main_state_select_menu_6);
DEFINE_MENU_CALLBACK(main_state_select_menu_7);
DEFINE_MENU_CALLBACK(main_state_select_menu_8);
DEFINE_MENU_CALLBACK(main_state_select_menu_9);

static const MENU main_state_select_menu_base[] =
{
   { NULL, main_state_select_menu_0, NULL, 0, NULL },
   { NULL, main_state_select_menu_1, NULL, 0, NULL },
   { NULL, main_state_select_menu_2, NULL, 0, NULL },
   { NULL, main_state_select_menu_3, NULL, 0, NULL },
   { NULL, main_state_select_menu_4, NULL, 0, NULL },
   { NULL, main_state_select_menu_5, NULL, 0, NULL },
   { NULL, main_state_select_menu_6, NULL, 0, NULL },
   { NULL, main_state_select_menu_7, NULL, 0, NULL },
   { NULL, main_state_select_menu_8, NULL, 0, NULL },
   { NULL, main_state_select_menu_9, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(main_state_autosave_menu_disabled);
DEFINE_MENU_CALLBACK(main_state_autosave_menu_10_seconds);
DEFINE_MENU_CALLBACK(main_state_autosave_menu_30_seconds);
DEFINE_MENU_CALLBACK(main_state_autosave_menu_60_seconds);

static const MENU main_state_autosave_menu_base[] =
{
   { "&Disabled",      main_state_autosave_menu_disabled,   NULL, 0, NULL },
   MENU_SPLITTER,
   { "&1: 10 Seconds", main_state_autosave_menu_10_seconds, NULL, 0, NULL },
   { "&2: 30 Seconds", main_state_autosave_menu_30_seconds, NULL, 0, NULL },
   { "&3: 60 Seconds", main_state_autosave_menu_60_seconds, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(main_state_menu_select);
DEFINE_MENU_CALLBACK(main_state_menu_save);
DEFINE_MENU_CALLBACK(main_state_menu_restore);

static const MENU main_state_menu_base[] =
{
   { "S&elect",   NULL,                    IMPORT_MENU(main_state_select_menu),   0, NULL },
   MENU_SPLITTER,
   { "&Save",     main_state_menu_save,    NULL,                                  0, NULL },
   { "&Restore",  main_state_menu_restore, NULL,                                  0, NULL },
   MENU_SPLITTER,
   { "&Autosave", NULL,                    IMPORT_MENU(main_state_autosave_menu), 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(main_replay_select_menu_0);
DEFINE_MENU_CALLBACK(main_replay_select_menu_1);
DEFINE_MENU_CALLBACK(main_replay_select_menu_2);
DEFINE_MENU_CALLBACK(main_replay_select_menu_3);
DEFINE_MENU_CALLBACK(main_replay_select_menu_4);

static const MENU main_replay_select_menu_base[] =
{
   { NULL, main_replay_select_menu_0, NULL, 0, NULL },
   { NULL, main_replay_select_menu_1, NULL, 0, NULL },
   { NULL, main_replay_select_menu_2, NULL, 0, NULL },
   { NULL, main_replay_select_menu_3, NULL, 0, NULL },
   { NULL, main_replay_select_menu_4, NULL, 0, NULL },
   MENU_ENDCAP
};


DEFINE_MENU_CALLBACK(main_replay_record_menu_start);
DEFINE_MENU_CALLBACK(main_replay_record_menu_stop);

static const MENU main_replay_record_menu_base[] =
{
   { "&Start", main_replay_record_menu_start, NULL, 0,          NULL },
   { "S&top",  main_replay_record_menu_stop,  NULL, D_DISABLED, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(main_replay_play_menu_start);
DEFINE_MENU_CALLBACK(main_replay_play_menu_stop);

static const MENU main_replay_play_menu_base[] =
{
   { "&Start", main_replay_play_menu_start, NULL,          0, NULL },
   { "S&top",  main_replay_play_menu_stop,  NULL, D_DISABLED, NULL },
   MENU_ENDCAP
};

static const MENU main_replay_menu_base[] =
{
   { "&Select", NULL, IMPORT_MENU(main_replay_select_menu), 0, NULL },
   MENU_SPLITTER,
   { "&Record", NULL, IMPORT_MENU(main_replay_record_menu), 0, NULL },
   { "&Play",   NULL, IMPORT_MENU(main_replay_play_menu),   0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(main_menu_load_rom);
DEFINE_MENU_CALLBACK(main_menu_resume);
DEFINE_MENU_CALLBACK(main_menu_reset);
DEFINE_MENU_CALLBACK(main_menu_snapshot);
DEFINE_MENU_CALLBACK(main_menu_messages);
DEFINE_MENU_CALLBACK(main_menu_exit);

static const MENU main_menu_base[] =
{
   { "&Load ROM...", main_menu_load_rom, NULL,                          0, NULL },
   MENU_SPLITTER,
   { "&Resume",      main_menu_resume,   NULL,                          0, NULL },
   { "R&eset",       main_menu_reset,    NULL,                          0, NULL },
   { "&State",       NULL,               IMPORT_MENU(main_state_menu),  0, NULL },
   MENU_SPLITTER,
   { "Re&play",      NULL,               IMPORT_MENU(main_replay_menu), 0, NULL },
   { "S&napshot",    main_menu_snapshot, NULL,                          0, NULL },
   { "&Messages...", main_menu_messages, NULL,                          0, NULL },
   MENU_SPLITTER,
   { "E&xit",        main_menu_exit,     NULL,                          0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(netplay_protocol_menu_tcpip);
DEFINE_MENU_CALLBACK(netplay_protocol_menu_spx);

static const MENU netplay_protocol_menu_base[] =
{
   { "&TCP/IP", netplay_protocol_menu_tcpip, NULL, 0, NULL },
   { "&SPX",    netplay_protocol_menu_spx,   NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(netplay_server_menu_start);
DEFINE_MENU_CALLBACK(netplay_server_menu_stop);

static const MENU netplay_server_menu_base[] =
{
   { "&Start", netplay_server_menu_start, NULL,          0, NULL },
   { "S&top",  netplay_server_menu_stop,  NULL, D_DISABLED, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(netplay_client_menu_connect);
DEFINE_MENU_CALLBACK(netplay_client_menu_disconnect);

static const MENU netplay_client_menu_base[] =
{
   { "&Connect...",    netplay_client_menu_connect, NULL,          0, NULL },
   { "&Disconnect", netplay_client_menu_disconnect, NULL, D_DISABLED, NULL },
   MENU_ENDCAP
};

static const MENU netplay_menu_base[] =
{
   { "&Protocol", NULL, IMPORT_MENU(netplay_protocol_menu), 0, NULL },
   MENU_SPLITTER,
   { "&Server",   NULL, IMPORT_MENU(netplay_server_menu),   0, NULL },
   { "&Client",   NULL, IMPORT_MENU(netplay_client_menu),   0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_gui_theme_menu_classic);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_stainless_steel);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_zero_4);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_panta);

static const MENU options_gui_theme_menu_base[] =
{
   { "&1: Classic",         options_gui_theme_menu_classic,         NULL, 0, NULL },
   { "&2: stainless Steel", options_gui_theme_menu_stainless_steel, NULL, 0, NULL },
   { "&3: Zero 4",          options_gui_theme_menu_zero_4,          NULL, 0, NULL },
   { "&4: Panta",           options_gui_theme_menu_panta,           NULL, 0, NULL },
   MENU_ENDCAP
};

static const MENU options_gui_menu_base [] =
{
   { "&Theme", NULL, IMPORT_MENU(options_gui_theme_menu), 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_system_menu_ntsc_60_hz);
DEFINE_MENU_CALLBACK(options_system_menu_pal_50_hz);

static const MENU options_system_menu_base[] =
{
   { "&NTSC (60 Hz)", options_system_menu_ntsc_60_hz, NULL, 0, NULL },
   { "&PAL (50 Hz)",  options_system_menu_pal_50_hz,  NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_audio_subsystem_menu_none);
DEFINE_MENU_CALLBACK(options_audio_subsystem_menu_allegro);
DEFINE_MENU_CALLBACK(options_audio_subsystem_menu_openal);

static const MENU options_audio_subsystem_menu_base[] =
{
   { "&None",    options_audio_subsystem_menu_none,    NULL, 0, NULL },
   MENU_SPLITTER,
   { "&Allegro", options_audio_subsystem_menu_allegro, NULL, 0, NULL },
   { "&OpenAL",  options_audio_subsystem_menu_openal,  NULL, 0, NULL },
   MENU_ENDCAP
};                                             

DEFINE_MENU_CALLBACK(options_audio_mixing_channels_menu_mono);
DEFINE_MENU_CALLBACK(options_audio_mixing_channels_menu_stereo_mix);
DEFINE_MENU_CALLBACK(options_audio_mixing_channels_menu_pseudo_stereo_mode_1);
DEFINE_MENU_CALLBACK(options_audio_mixing_channels_menu_pseudo_stereo_mode_2);
DEFINE_MENU_CALLBACK(options_audio_mixing_channels_menu_stereo);
DEFINE_MENU_CALLBACK(options_audio_mixing_channels_menu_swap_channels);

static const MENU options_audio_mixing_channels_menu_base[] =
{
   { "&Mono",                   options_audio_mixing_channels_menu_mono,                 NULL, 0, NULL },
   { "St&ereo Mix",             options_audio_mixing_channels_menu_stereo_mix,           NULL, 0, NULL },
   { "&Pseudo Stereo (Mode 1)", options_audio_mixing_channels_menu_pseudo_stereo_mode_1, NULL, 0, NULL },
   { "P&seudo Stereo (Mode 2)", options_audio_mixing_channels_menu_pseudo_stereo_mode_2, NULL, 0, NULL },
   { "S&tereo",                 options_audio_mixing_channels_menu_stereo,               NULL, 0, NULL },
   MENU_SPLITTER,
   { "S&wap Channels",          options_audio_mixing_channels_menu_swap_channels,        NULL, 0, NULL },
   MENU_ENDCAP
};                                             

DEFINE_MENU_CALLBACK(options_audio_mixing_frequency_menu_8000_hz);
DEFINE_MENU_CALLBACK(options_audio_mixing_frequency_menu_11025_hz);
DEFINE_MENU_CALLBACK(options_audio_mixing_frequency_menu_16000_hz);
DEFINE_MENU_CALLBACK(options_audio_mixing_frequency_menu_22050_hz);
DEFINE_MENU_CALLBACK(options_audio_mixing_frequency_menu_32000_hz);
DEFINE_MENU_CALLBACK(options_audio_mixing_frequency_menu_44100_hz);
DEFINE_MENU_CALLBACK(options_audio_mixing_frequency_menu_48000_hz);
DEFINE_MENU_CALLBACK(options_audio_mixing_frequency_menu_80200_hz);
DEFINE_MENU_CALLBACK(options_audio_mixing_frequency_menu_96000_hz);

static const MENU options_audio_mixing_frequency_menu_base[] =
{
   { "&1: 8000 Hz",  options_audio_mixing_frequency_menu_8000_hz,  NULL, 0, NULL },
   { "&2: 11025 Hz", options_audio_mixing_frequency_menu_11025_hz, NULL, 0, NULL },
   { "&3: 16000 Hz", options_audio_mixing_frequency_menu_16000_hz, NULL, 0, NULL },
   { "&4: 22050 Hz", options_audio_mixing_frequency_menu_22050_hz, NULL, 0, NULL },
   { "&5: 32000 Hz", options_audio_mixing_frequency_menu_32000_hz, NULL, 0, NULL },
   { "&6: 44100 Hz", options_audio_mixing_frequency_menu_44100_hz, NULL, 0, NULL },
   { "&7: 48000 Hz", options_audio_mixing_frequency_menu_48000_hz, NULL, 0, NULL },
   { "&8: 80200 Hz", options_audio_mixing_frequency_menu_80200_hz, NULL, 0, NULL },
   { "&9: 96000 Hz", options_audio_mixing_frequency_menu_96000_hz, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_audio_mixing_quality_menu_low_8_bit);
DEFINE_MENU_CALLBACK(options_audio_mixing_quality_menu_high_16_bit);
DEFINE_MENU_CALLBACK(options_audio_mixing_quality_menu_interpolation);
DEFINE_MENU_CALLBACK(options_audio_mixing_quality_menu_dithering);

static const MENU options_audio_mixing_quality_menu_base[] =
{
   { "&Low (8 bits)",   options_audio_mixing_quality_menu_low_8_bit,     NULL, 0, NULL },
   { "&High (16 bits)", options_audio_mixing_quality_menu_high_16_bit,   NULL, 0, NULL },
   MENU_SPLITTER,
   { "&Interpolation",  options_audio_mixing_quality_menu_interpolation, NULL, 0, NULL },
   { "&Dithering",      options_audio_mixing_quality_menu_dithering,     NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_audio_mixing_anti_aliasing_menu_disabled);
DEFINE_MENU_CALLBACK(options_audio_mixing_anti_aliasing_menu_bilinear_2x);
DEFINE_MENU_CALLBACK(options_audio_mixing_anti_aliasing_menu_bilinear_4x);
DEFINE_MENU_CALLBACK(options_audio_mixing_anti_aliasing_menu_bilinear_8x);
DEFINE_MENU_CALLBACK(options_audio_mixing_anti_aliasing_menu_bilinear_16x);

static const MENU options_audio_mixing_anti_aliasing_menu_base[] =
{
   { "&Disabled",         options_audio_mixing_anti_aliasing_menu_disabled,     NULL, 0, NULL },
   MENU_SPLITTER,
   { "&1: Bi-linear 2X",  options_audio_mixing_anti_aliasing_menu_bilinear_2x,  NULL, 0, NULL },
   { "&2: Bi-linear 4X",  options_audio_mixing_anti_aliasing_menu_bilinear_4x,  NULL, 0, NULL },
   { "&3: Bi-linear 8X",  options_audio_mixing_anti_aliasing_menu_bilinear_8x,  NULL, 0, NULL },
   { "&4: Bi-linear 16X", options_audio_mixing_anti_aliasing_menu_bilinear_16x, NULL, 0, NULL },
   MENU_ENDCAP
};

static const MENU options_audio_mixing_menu_base[] =
{
   { "&Frequency",     NULL, IMPORT_MENU(options_audio_mixing_frequency_menu),     0, NULL },
   { "&Channels",      NULL, IMPORT_MENU(options_audio_mixing_channels_menu),      0, NULL },
   { "&Quality",       NULL, IMPORT_MENU(options_audio_mixing_quality_menu),       0, NULL },
   { "&Anti-aliasing", NULL, IMPORT_MENU(options_audio_mixing_anti_aliasing_menu), 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_audio_effects_menu_linear_echo);
DEFINE_MENU_CALLBACK(options_audio_effects_menu_spatial_stereo_mode_1);
DEFINE_MENU_CALLBACK(options_audio_effects_menu_spatial_stereo_mode_2);
DEFINE_MENU_CALLBACK(options_audio_effects_menu_spatial_stereo_mode_3);

static const MENU options_audio_effects_menu_base[] =
{                          
   { "&Linear Echo",             options_audio_effects_menu_linear_echo,           NULL, 0, NULL },
   { "&Spatial Stereo (Mode 1)", options_audio_effects_menu_spatial_stereo_mode_1, NULL, 0, NULL },
   { "S&patial Stereo (Mode 2)", options_audio_effects_menu_spatial_stereo_mode_2, NULL, 0, NULL },
   { "Sp&atial Stereo (Mode 3)", options_audio_effects_menu_spatial_stereo_mode_3, NULL, 0, NULL },
   MENU_ENDCAP   
};

DEFINE_MENU_CALLBACK(options_audio_filters_menu_low_pass_mode_1);
DEFINE_MENU_CALLBACK(options_audio_filters_menu_low_pass_mode_2);
DEFINE_MENU_CALLBACK(options_audio_filters_menu_low_pass_mode_3);
DEFINE_MENU_CALLBACK(options_audio_filters_menu_high_pass);
DEFINE_MENU_CALLBACK(options_audio_filters_menu_delta_sigma_filter);

static const MENU options_audio_filters_menu_base[] =
{     
   { "&Low Pass (Mode 1)",  options_audio_filters_menu_low_pass_mode_1,    NULL, 0, NULL },
   { "L&ow Pass (Mode 2)",  options_audio_filters_menu_low_pass_mode_2,    NULL, 0, NULL },
   { "Lo&w Pass (Mode 3)",  options_audio_filters_menu_low_pass_mode_3,    NULL, 0, NULL },
   { "&High Pass",          options_audio_filters_menu_high_pass,          NULL, 0, NULL },
   { "&Delta-Sigma Filter", options_audio_filters_menu_delta_sigma_filter, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_audio_channels_menu_square_wave_a);
DEFINE_MENU_CALLBACK(options_audio_channels_menu_square_wave_b);
DEFINE_MENU_CALLBACK(options_audio_channels_menu_triangle_wave);
DEFINE_MENU_CALLBACK(options_audio_channels_menu_white_noise);
DEFINE_MENU_CALLBACK(options_audio_channels_menu_digital);
DEFINE_MENU_CALLBACK(options_audio_channels_menu_extended);

static const MENU options_audio_channels_menu_base[] =
{
   { "&Square Wave A", options_audio_channels_menu_square_wave_a, NULL, 0, NULL },
   { "S&quare Wave B", options_audio_channels_menu_square_wave_b, NULL, 0, NULL },
   { "&Triangle Wave", options_audio_channels_menu_triangle_wave, NULL, 0, NULL },
   { "&White Noise",   options_audio_channels_menu_white_noise,   NULL, 0, NULL },
   { "&Digital",       options_audio_channels_menu_digital,       NULL, 0, NULL },
   { "&Extended",      options_audio_channels_menu_extended,      NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_audio_advanced_menu_ideal_triangle);
DEFINE_MENU_CALLBACK(options_audio_advanced_menu_hard_sync);

static const MENU options_audio_advanced_menu_base[] =
{
   { "&Ideal Triangle", options_audio_advanced_menu_ideal_triangle, NULL, 0, NULL },
   { "&Hard Sync",      options_audio_advanced_menu_hard_sync,      NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_audio_record_menu_start);
DEFINE_MENU_CALLBACK(options_audio_record_menu_stop);

static const MENU options_audio_record_menu_base[] =
{
   { "&Start", options_audio_record_menu_start, NULL, 0,          NULL },
   { "S&top",  options_audio_record_menu_stop,  NULL, D_DISABLED, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_audio_menu_enabled);

static const MENU options_audio_menu_base[] =
{
   { "&Enabled",   options_audio_menu_enabled, NULL,                                      0, NULL },
   MENU_SPLITTER,
   { "&Subsystem", NULL,                       IMPORT_MENU(options_audio_subsystem_menu), 0, NULL },
   { "&Mixing",    NULL,                       IMPORT_MENU(options_audio_mixing_menu),    0, NULL },
   { "Effec&ts",   NULL,                       IMPORT_MENU(options_audio_effects_menu),   0, NULL },
   { "&Filters",   NULL,                       IMPORT_MENU(options_audio_filters_menu),   0, NULL },
   { "&Channels",  NULL,                       IMPORT_MENU(options_audio_channels_menu),  0, NULL },
   { "&Advanced",  NULL,                       IMPORT_MENU(options_audio_advanced_menu),  0, NULL },
   MENU_SPLITTER,
   { "&Record",    NULL,                       IMPORT_MENU(options_audio_record_menu),    0, NULL },
   MENU_ENDCAP
};

#ifdef ALLEGRO_DOS

DEFINE_MENU_CALLBACK(options_video_driver_dos_menu_vga);
DEFINE_MENU_CALLBACK(options_video_driver_dos_menu_vga_mode_x);
DEFINE_MENU_CALLBACK(options_video_driver_dos_menu_vesa);
DEFINE_MENU_CALLBACK(options_video_driver_dos_menu_vesa_2_banked);
DEFINE_MENU_CALLBACK(options_video_driver_dos_menu_vesa_2_linear);
DEFINE_MENU_CALLBACK(options_video_driver_dos_menu_vesa_3);
DEFINE_MENU_CALLBACK(options_video_driver_dos_menu_vesa_vbe_af);

static const MENU options_video_driver_dos_menu_base[] =
{
   { "&VGA",           options_video_driver_dos_menu_vga,           NULL, 0, NULL },
   { "VGA Mode-&X",    options_video_driver_dos_menu_vga_mode_x,    NULL, 0, NULL },
   MENU_SPLITTER,
   { "V&ESA",          options_video_driver_dos_menu_vesa,          NULL, 0, NULL },
   { "VESA 2 &Banked", options_video_driver_dos_menu_vesa_2_banked, NULL, 0, NULL },
   { "VESA 2 &Linear", options_video_driver_dos_menu_vesa_2_linear, NULL, 0, NULL },
   { "VE&SA 3",        options_video_driver_dos_menu_vesa_3,        NULL, 0, NULL },
   { "VESA VBE/&AF",   options_video_driver_dos_menu_vesa_vbe_af,   NULL, 0, NULL },
   MENU_ENDCAP
};

#else /* ALLEGRO_DOS */

static const MENU options_video_driver_dos_menu_base[] =
{
   MENU_ENDCAP
};

#endif   /* !ALLEGRO_DOS */

#ifdef ALLEGRO_WINDOWS

DEFINE_MENU_CALLBACK(options_video_driver_windows_menu_directx);
DEFINE_MENU_CALLBACK(options_video_driver_windows_menu_directx_window);
DEFINE_MENU_CALLBACK(options_video_driver_windows_menu_directx_overlay);
DEFINE_MENU_CALLBACK(options_video_driver_windows_menu_gdi);

static const MENU options_video_driver_windows_menu_base[] =
{
   { "&DirectX",         options_video_driver_windows_menu_directx,         NULL, 0, NULL },
   { "DirectX &Window",  options_video_driver_windows_menu_directx_window,  NULL, 0, NULL },
   { "DirectX &Overlay", options_video_driver_windows_menu_directx_overlay, NULL, 0, NULL },
   MENU_SPLITTER,
   { "&GDI",             options_video_driver_windows_menu_gdi,             NULL, 0, NULL },
   MENU_ENDCAP
};

#else /* ALLEGRO_WINDOWS */

static const MENU options_video_driver_windows_menu_base[] =
{
   MENU_ENDCAP
};

#endif   /* !ALLEGRO_WINDOWS */

#ifdef ALLEGRO_LINUX

DEFINE_MENU_CALLBACK(options_video_driver_linux_menu_vga);
DEFINE_MENU_CALLBACK(options_video_driver_linux_menu_vga_mode_x);
DEFINE_MENU_CALLBACK(options_video_driver_linux_menu_vesa_vbe_af);
DEFINE_MENU_CALLBACK(options_video_driver_linux_menu_framebuffer);
DEFINE_MENU_CALLBACK(options_video_driver_linux_menu_svgalib);

static const MENU options_video_driver_linux_menu_base[] =
{
   { "&VGA",         options_video_driver_linux_menu_vga,         NULL, 0, NULL },
   { "VGA Mode-&X",  options_video_driver_linux_menu_vga_mode_x,  NULL, 0, NULL },
   MENU_SPLITTER,
   { "VESA VBE/&AF", options_video_driver_linux_menu_vesa_vbe_af, NULL, 0, NULL },
   MENU_SPLITTER,
   { "&Framebuffer", options_video_driver_linux_menu_framebuffer, NULL, 0, NULL },
   { "&SVGAlib",     options_video_driver_linux_menu_svgalib,     NULL, 0, NULL },
   MENU_ENDCAP
};

#else /* ALLEGRO_LINUX */

static const MENU options_video_driver_linux_menu_base[] =
{
   MENU_ENDCAP
};

#endif   /* !ALLEGRO_LINUX */

#ifdef ALLEGRO_UNIX

DEFINE_MENU_CALLBACK(options_video_driver_unix_menu_x_windows);
DEFINE_MENU_CALLBACK(options_video_driver_unix_menu_x_windows_full);
DEFINE_MENU_CALLBACK(options_video_driver_unix_menu_x_dga);
DEFINE_MENU_CALLBACK(options_video_driver_unix_menu_x_dga_full);
DEFINE_MENU_CALLBACK(options_video_driver_unix_menu_x_dga_2);

static const MENU options_video_driver_unix_menu_base[] =
{
   { "X &Windows",      options_video_driver_unix_menu_x_windows,      NULL, 0, NULL },
   { "X &Windows Full", options_video_driver_unix_menu_x_windows_full, NULL, 0, NULL },
   MENU_SPLITTER,
   { "X/&DGA",          options_video_driver_unix_menu_x_dga,          NULL, 0, NULL },
   { "X/&DGA FULL",     options_video_driver_unix_menu_x_dga_full,     NULL, 0, NULL },
   { "X/D&GA 2",        options_video_driver_unix_menu_x_dga_2,        NULL, 0, NULL },
   MENU_ENDCAP
};

#else /* ALLEGRO_UNIX */

static const MENU options_video_driver_unix_menu_base[] =
{
   MENU_ENDCAP
};

#endif   /* !ALLEGRO_UNIX */

DEFINE_MENU_CALLBACK(options_video_driver_menu_automatic);

static const MENU options_video_driver_menu_base[] =
{
   { "&Automatic", options_video_driver_menu_automatic, NULL,                                           0, NULL },
   MENU_SPLITTER,
   { "&DOS",       NULL,                                IMPORT_MENU(options_video_driver_dos_menu),     0, NULL },
   { "&Windows",   NULL,                                IMPORT_MENU(options_video_driver_windows_menu), 0, NULL },
   { "&Linux",     NULL,                                IMPORT_MENU(options_video_driver_linux_menu),   0, NULL },
   { "&Unix",      NULL,                                IMPORT_MENU(options_video_driver_unix_menu),    0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_video_resolution_proportionate_menu_256_224);
DEFINE_MENU_CALLBACK(options_video_resolution_proportionate_menu_256_240);
DEFINE_MENU_CALLBACK(options_video_resolution_proportionate_menu_512_448);
DEFINE_MENU_CALLBACK(options_video_resolution_proportionate_menu_512_480);
DEFINE_MENU_CALLBACK(options_video_resolution_proportionate_menu_768_672);
DEFINE_MENU_CALLBACK(options_video_resolution_proportionate_menu_768_720);
DEFINE_MENU_CALLBACK(options_video_resolution_proportionate_menu_1024_896);
DEFINE_MENU_CALLBACK(options_video_resolution_proportionate_menu_1024_960);
DEFINE_MENU_CALLBACK(options_video_resolution_proportionate_menu_1280_1120);
DEFINE_MENU_CALLBACK(options_video_resolution_proportionate_menu_1280_1200);

static const MENU options_video_resolution_proportionate_menu_base[] =
{
   { " &1: 256x224",   options_video_resolution_proportionate_menu_256_224,   NULL, 0, NULL },
   { " &2: 256x240",   options_video_resolution_proportionate_menu_256_240,   NULL, 0, NULL },
   { " &3: 512x448",   options_video_resolution_proportionate_menu_512_448,   NULL, 0, NULL },
   { " &4: 512x480",   options_video_resolution_proportionate_menu_512_480,   NULL, 0, NULL },
   { " &5: 768x672",   options_video_resolution_proportionate_menu_768_672,   NULL, 0, NULL },
   { " &6: 768x720",   options_video_resolution_proportionate_menu_768_720,   NULL, 0, NULL },
   { " &7: 1152x896",  options_video_resolution_proportionate_menu_1024_896,  NULL, 0, NULL },
   { " &8: 1152x900",  options_video_resolution_proportionate_menu_1024_960,  NULL, 0, NULL },
   { " &9: 1280x1120", options_video_resolution_proportionate_menu_1280_1120, NULL, 0, NULL },
   { "1&0: 1280x1200", options_video_resolution_proportionate_menu_1280_1200, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_video_resolution_extended_menu_400_300);
DEFINE_MENU_CALLBACK(options_video_resolution_extended_menu_480_360);
DEFINE_MENU_CALLBACK(options_video_resolution_extended_menu_512_384);
DEFINE_MENU_CALLBACK(options_video_resolution_extended_menu_640_400);
DEFINE_MENU_CALLBACK(options_video_resolution_extended_menu_720_480);
DEFINE_MENU_CALLBACK(options_video_resolution_extended_menu_720_576);
DEFINE_MENU_CALLBACK(options_video_resolution_extended_menu_848_480);
DEFINE_MENU_CALLBACK(options_video_resolution_extended_menu_1280_720);
DEFINE_MENU_CALLBACK(options_video_resolution_extended_menu_1280_960);
DEFINE_MENU_CALLBACK(options_video_resolution_extended_menu_1360_768);

static const MENU options_video_resolution_extended_menu_base[] =
{
   { " &1: 400x300",  options_video_resolution_extended_menu_400_300,  NULL, 0, NULL },
   { " &2: 480x360",  options_video_resolution_extended_menu_480_360,  NULL, 0, NULL },
   { " &3: 512x384",  options_video_resolution_extended_menu_512_384,  NULL, 0, NULL },
   { " &4: 640x400",  options_video_resolution_extended_menu_640_400,  NULL, 0, NULL },
   { " &5: 720x480",  options_video_resolution_extended_menu_720_480,  NULL, 0, NULL },
   { " &6: 720x576",  options_video_resolution_extended_menu_720_576,  NULL, 0, NULL },
   { " &7: 848x480",  options_video_resolution_extended_menu_848_480,  NULL, 0, NULL },
   { " &8: 1280x720", options_video_resolution_extended_menu_1280_720, NULL, 0, NULL },
   { " &9: 1280x960", options_video_resolution_extended_menu_1280_960, NULL, 0, NULL },
   { "1&0: 1360x768", options_video_resolution_extended_menu_1360_768, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_video_resolution_menu_320_240);
DEFINE_MENU_CALLBACK(options_video_resolution_menu_640_480);
DEFINE_MENU_CALLBACK(options_video_resolution_menu_800_600);
DEFINE_MENU_CALLBACK(options_video_resolution_menu_1024_768);
DEFINE_MENU_CALLBACK(options_video_resolution_menu_1152_864);
DEFINE_MENU_CALLBACK(options_video_resolution_menu_1280_1024);
DEFINE_MENU_CALLBACK(options_video_resolution_menu_1600_1200);

static const MENU options_video_resolution_menu_base[] =
{
   { "&Proportionate", NULL,                                    IMPORT_MENU(options_video_resolution_proportionate_menu), 0, NULL },
   MENU_SPLITTER,
   { "&1: 320x240",    options_video_resolution_menu_320_240,   NULL,                                                     0, NULL },
   { "&2: 640x480",    options_video_resolution_menu_640_480,   NULL,                                                     0, NULL },
   { "&3: 800x600",    options_video_resolution_menu_800_600,   NULL,                                                     0, NULL },
   { "&4: 1024x768",   options_video_resolution_menu_1024_768,  NULL,                                                     0, NULL },
   { "&5: 1152x864",   options_video_resolution_menu_1152_864,  NULL,                                                     0, NULL },
   { "&6: 1280x1024",  options_video_resolution_menu_1280_1024, NULL,                                                     0, NULL },
   { "&7: 1600x1200",  options_video_resolution_menu_1600_1200, NULL,                                                     0, NULL },
   MENU_SPLITTER,
   { "&Extended",      NULL,                                    IMPORT_MENU(options_video_resolution_extended_menu),      0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_video_colors_menu_paletted_8_bit);
DEFINE_MENU_CALLBACK(options_video_colors_menu_true_color_15_bit);
DEFINE_MENU_CALLBACK(options_video_colors_menu_true_color_16_bit);
DEFINE_MENU_CALLBACK(options_video_colors_menu_true_color_24_bit);
DEFINE_MENU_CALLBACK(options_video_colors_menu_true_color_32_bit);

static const MENU options_video_colors_menu_base[] =
{
   { "&Paletted (8-bit)",    options_video_colors_menu_paletted_8_bit,    NULL, 0, NULL },
   { "&True Color (15-bit)", options_video_colors_menu_true_color_15_bit, NULL, 0, NULL },
   { "T&rue Color (16-bit)", options_video_colors_menu_true_color_16_bit, NULL, 0, NULL },
   { "Tr&ue Color (24-bit)", options_video_colors_menu_true_color_24_bit, NULL, 0, NULL },
   { "Tru&e Color (32-bit)", options_video_colors_menu_true_color_32_bit, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_video_blitter_menu_automatic);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_normal);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_des);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_interpolated_2x);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_2xscl);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_desii);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_super_2xscl);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_ultra_2xscl);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_hq2x);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_interpolated_3x);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_hq3x);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_hq4x);
DEFINE_MENU_CALLBACK(options_video_blitter_menu_stretched);

static const MENU options_video_blitter_menu_base[] =
{
   { "&Automatic",           options_video_blitter_menu_automatic,       NULL, 0, NULL },
   MENU_SPLITTER,
   { " &1: Normal",          options_video_blitter_menu_normal,          NULL, 0, NULL },
   { " &2: DES",             options_video_blitter_menu_des,             NULL, 0, NULL },
   { " &3: Interpolated 2X", options_video_blitter_menu_interpolated_2x, NULL, 0, NULL },
   { " &4: 2xSCL",           options_video_blitter_menu_2xscl,           NULL, 0, NULL },
   { " &5: DESii",           options_video_blitter_menu_desii,           NULL, 0, NULL },
   { " &6: Super 2xSCL",     options_video_blitter_menu_super_2xscl,     NULL, 0, NULL },
   { " &7: Ultra 2xSCL",     options_video_blitter_menu_ultra_2xscl,     NULL, 0, NULL },
   { " &8: HQ2X",            options_video_blitter_menu_hq2x,            NULL, 0, NULL },
   { " &9: Interpolated 3X", options_video_blitter_menu_interpolated_3x, NULL, 0, NULL },
   { "1&0: HQ3X",            options_video_blitter_menu_hq3x,            NULL, 0, NULL },
   { "11: &HQ4X",            options_video_blitter_menu_hq4x,            NULL, 0, NULL },
   { "12: &Stretched",       options_video_blitter_menu_stretched,       NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_video_filters_menu_scanlines_25_percent);
DEFINE_MENU_CALLBACK(options_video_filters_menu_scanlines_50_percent);
DEFINE_MENU_CALLBACK(options_video_filters_menu_scanlines_100_percent);

static const MENU options_video_filters_menu_base[] =
{
   { "&Scanlines (25%)",  options_video_filters_menu_scanlines_25_percent,  NULL, 0, NULL },
   { "S&canlines (50%)",  options_video_filters_menu_scanlines_50_percent,  NULL, 0, NULL },
   { "Sc&anlines (100%)", options_video_filters_menu_scanlines_100_percent, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_video_layers_menu_sprites_a);
DEFINE_MENU_CALLBACK(options_video_layers_menu_sprites_b);
DEFINE_MENU_CALLBACK(options_video_layers_menu_background);

static const MENU options_video_layers_menu_base[] =
{
   { "&Sprites A",  options_video_layers_menu_sprites_a,  NULL, 0, NULL },
   { "S&prites B",  options_video_layers_menu_sprites_b,  NULL, 0, NULL },
   { "&Background", options_video_layers_menu_background, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_video_palette_menu_ntsc_color);
DEFINE_MENU_CALLBACK(options_video_palette_menu_ntsc_grayscale);
DEFINE_MENU_CALLBACK(options_video_palette_menu_gnuboy);
DEFINE_MENU_CALLBACK(options_video_palette_menu_nester);
DEFINE_MENU_CALLBACK(options_video_palette_menu_nesticle);
DEFINE_MENU_CALLBACK(options_video_palette_menu_modern_ntsc);
DEFINE_MENU_CALLBACK(options_video_palette_menu_modern_pal);
DEFINE_MENU_CALLBACK(options_video_palette_menu_ega_mode_1);
DEFINE_MENU_CALLBACK(options_video_palette_menu_ega_mode_2);
DEFINE_MENU_CALLBACK(options_video_palette_menu_custom);

static const MENU options_video_palette_menu_base[] =
{
   { "NTSC &Color",     options_video_palette_menu_ntsc_color,     NULL, 0, NULL },
   { "NTSC &Grayscale", options_video_palette_menu_ntsc_grayscale, NULL, 0, NULL },
   { "gn&uboy",         options_video_palette_menu_gnuboy,         NULL, 0, NULL },
   { "&NESter",         options_video_palette_menu_nester,         NULL, 0, NULL },
   { "N&ESticle",       options_video_palette_menu_nesticle,       NULL, 0, NULL },
   { "&Modern NTSC",    options_video_palette_menu_modern_ntsc,    NULL, 0, NULL },
   { "Modern &PAL",     options_video_palette_menu_modern_pal,     NULL, 0, NULL },
   { "&EGA (Mode 1)",   options_video_palette_menu_ega_mode_1,     NULL, 0, NULL },
   { "EG&A (Mode 2)",   options_video_palette_menu_ega_mode_2,     NULL, 0, NULL },
   MENU_SPLITTER,
   { "Cu&stom",         options_video_palette_menu_custom,         NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_video_advanced_menu_force_window);

static const MENU options_video_advanced_menu_base[] =
{
   { "&Force Window", options_video_advanced_menu_force_window, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_video_menu_vsync);

static const MENU options_video_menu_base[] =
{
   { "&Driver",     NULL,                     IMPORT_MENU(options_video_driver_menu),     0, NULL },
   { "&Resolution", NULL,                     IMPORT_MENU(options_video_resolution_menu), 0, NULL },
   { "&Colors",     NULL,                     IMPORT_MENU(options_video_colors_menu),     0, NULL },
   { "&Blitter",    NULL,                     IMPORT_MENU(options_video_blitter_menu),    0, NULL },
   { "&Filters",    NULL,                     IMPORT_MENU(options_video_filters_menu),    0, NULL },
   MENU_SPLITTER,
   { "&VSync",      options_video_menu_vsync, NULL,                                       0, NULL },
   MENU_SPLITTER,
   { "&Layers",     NULL,                     IMPORT_MENU(options_video_layers_menu),     0, NULL },
   { "&Palette",    NULL,                     IMPORT_MENU(options_video_palette_menu),    0, NULL },
   { "&Advanced",   NULL,                     IMPORT_MENU(options_video_advanced_menu),   0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_input_menu_enable_zapper);
DEFINE_MENU_CALLBACK(options_input_menu_configure);

static const MENU options_input_menu_base[] =
{
   { "Enable &Zapper", options_input_menu_enable_zapper, NULL, 0, NULL },
   MENU_SPLITTER,
   { "&Configure...",  options_input_menu_configure,     NULL, 0, NULL },
   MENU_ENDCAP
};


DEFINE_MENU_CALLBACK(options_menu_status);
DEFINE_MENU_CALLBACK(options_menu_patches);

static const MENU options_menu_base[] =
{
   { "&Status",     options_menu_status,  NULL,                             0, NULL },
   MENU_SPLITTER,
   { "&GUI",        NULL,                 IMPORT_MENU(options_gui_menu),    0, NULL },
   { "S&ystem",     NULL,                 IMPORT_MENU(options_system_menu), 0, NULL },
   { "&Audio",      NULL,                 IMPORT_MENU(options_audio_menu),  0, NULL },
   { "&Video",      NULL,                 IMPORT_MENU(options_video_menu),  0, NULL },
   { "&Input",      NULL,                 IMPORT_MENU(options_input_menu),  0, NULL },
   MENU_SPLITTER,
   { "&Patches...", options_menu_patches, NULL,                             0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(help_menu_shortcuts);
DEFINE_MENU_CALLBACK(help_menu_about);

static const MENU help_menu_base[] =
{
   { "&Shortcuts...", help_menu_shortcuts, NULL, 0, NULL },
   MENU_SPLITTER,
   { "&About...",     help_menu_about,     NULL, 0, NULL },
   MENU_ENDCAP
};

static const MENU top_menu_base[] =
{ 
   { "&Main",    NULL, IMPORT_MENU(main_menu),    0, NULL },
   { "&Options", NULL, IMPORT_MENU(options_menu), 0, NULL },
   { "&NetPlay", NULL, IMPORT_MENU(netplay_menu), 0, NULL },
   { "&Help",    NULL, IMPORT_MENU(help_menu),    0, NULL },
   MENU_ENDCAP
};

enum
{
   MENU_PROPERTY_CHECKED,
   MENU_PROPERTY_ENABLED
};

static INLINE void set_menu_object_property (MENU *object, ENUM property,
   BOOL value)
{
   /* This function sets the boolean property 'property' of the menu or menu
      item specified by 'menu' to 'value'. */

   RT_ASSERT(object);

   switch (property)
   {
      case MENU_PROPERTY_CHECKED:
      {
         SET_FLAG(object->flags, D_SELECTED, value);

         break;
      }

      case MENU_PROPERTY_ENABLED:
      {
         SET_FLAG(object->flags, D_DISABLED, !value);

         break;
      }

      default:
         WARN_GENERIC();
   }
}

static INLINE BOOL set_menu_property (MENU *menu, void *callback, ENUM
   property, BOOL value)
{
   /* Recursive function that searches through the menu specified in 'menu'
      to find a menu item with a callback function matching 'callback', then
      sets it's boolean property specified by 'property' to 'value'.
      Returns TRUE if the item was found and all recursion levels should
      break out of the search, or FALSE if the item was not found and the
      search should continue. */

   int index = 0;

   RT_ASSERT(menu);
   RT_ASSERT(callback);

   while (menu[index].text || menu[index].proc)
   {
      MENU *item = &menu[index];

      if (item->child)
      {
         /* Recurse to the next sublevel. */
         if (set_menu_property (item->child, callback, property, value))
            return (TRUE);
      }
      else
      {
         if (item->proc == callback)
         {
            set_menu_object_property (item, property, value);

            return (TRUE);
         }
      }

      index++;
   }

   return (FALSE);
}

static INLINE BOOL set_submenu_property (MENU *menu, const MENU *submenu,
   ENUM property, BOOL value)
{
   /* Recursive function that searches through the menu specified in 'menu'
      to find the first reference to the submenu specified in 'submenu' and
      sets it's boolean property specified by 'property' to 'value'.
      Returns TRUE if the item was found and all recursion levels should
      break out of the search, or FALSE if the item was not found and the
      search should continue. */

   int index = 0;

   RT_ASSERT(menu);
   RT_ASSERT(submenu);

   while (menu[index].text || menu[index].proc)
   {
      MENU *item = &menu[index];

      if (item->child)
      {
         if (item->child == submenu)
         {
            set_menu_object_property (item, property, value);
   
            return (TRUE);
         }
         else
         {
            /* Recurse to the next sublevel. */
            if (set_submenu_property (item->child, submenu, property,
               value))
            {
               return (TRUE);
            }
         }
      }

      index++;
   }

   return (FALSE);
}

#define SET_MENU_ITEM_CHECKED(item, checked) \
   set_menu_property (top_menu, item, MENU_PROPERTY_CHECKED, checked)
#define SET_MENU_ITEM_ENABLED(item, enabled) \
   set_menu_property (top_menu, item, MENU_PROPERTY_ENABLED, enabled)
#define SET_SUBMENU_ENABLED(menu, submenu, enabled)   \
   set_submenu_property (menu, submenu, MENU_PROPERTY_ENABLED, enabled)

#define CHECK_MENU_ITEM(item)          SET_MENU_ITEM_CHECKED(item, TRUE)
#define UNCHECK_MENU_ITEM(item)        SET_MENU_ITEM_CHECKED(item, FALSE)
#define TOGGLE_MENU_ITEM               SET_MENU_ITEM_CHECKED
#define ENABLE_MENU_ITEM(item)         SET_MENU_ITEM_ENABLED(item, TRUE)
#define DISABLE_MENU_ITEM(item)        SET_MENU_ITEM_ENABLED(item, FALSE)
#define ENABLE_SUBMENU(submenu)  \
   SET_SUBMENU_ENABLED(top_menu, submenu, TRUE)
#define DISABLE_SUBMENU(submenu) \
   SET_SUBMENU_ENABLED(top_menu, submenu, FALSE)

#define CHECK_MENU(n,i)
#define UNCHECK_MENU(n,i)
#define ENABLE_MENU(n,i)
#define DISABLE_MENU(n,i)
#define TOGGLE_MENU(n,i,x)

/* Undefine helper macros. */
#undef DEFINE_MENU
#undef DEFINE_MENU_CALLBACK
#undef IMPORT_MENU
#undef MENU_SPLITTER
#undef MENU_ENDCAP
#undef SET_FLAG
