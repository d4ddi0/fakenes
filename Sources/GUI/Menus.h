/* Define helper macros. */
#define DEFINE_MENU(name)           static MENU * name = NULL
#define DEFINE_MENU_CALLBACK(func)  static int func (void)
#define IMPORT_MENU(menu)           (MENU *) & menu
#define MENU_SPLITTER               { "",   NULL, NULL, 0, NULL }
#define MENU_ENDCAP                 { NULL, NULL, NULL, 0, NULL }

DEFINE_MENU(main_open_recent_menu);
DEFINE_MENU(main_replay_select_menu);
DEFINE_MENU(main_replay_menu);
DEFINE_MENU(main_record_audio_menu);
DEFINE_MENU(main_menu);
DEFINE_MENU(system_save_state_select_menu);
DEFINE_MENU(system_save_state_autosave_menu);
DEFINE_MENU(system_save_state_menu);
DEFINE_MENU(system_region_menu);
DEFINE_MENU(system_speed_up_down_menu);
DEFINE_MENU(system_frame_skip_menu);
DEFINE_MENU(system_menu);
DEFINE_MENU(audio_channels_menu);
DEFINE_MENU(audio_output_buffer_size_menu);
DEFINE_MENU(audio_output_menu);
DEFINE_MENU(audio_menu);
DEFINE_MENU(video_driver_menu);
DEFINE_MENU(video_resolution_proportionate_menu);
DEFINE_MENU(video_resolution_menu);
DEFINE_MENU(video_color_depth_menu);
DEFINE_MENU(video_blitter_menu);
DEFINE_MENU(video_layers_menu);
DEFINE_MENU(video_palette_menu);
DEFINE_MENU(video_menu);
DEFINE_MENU(input_menu);
DEFINE_MENU(options_cpu_usage_menu);
DEFINE_MENU(options_gui_theme_menu);
DEFINE_MENU(options_menu);
DEFINE_MENU(netplay_menu);
DEFINE_MENU(help_menu);
DEFINE_MENU(top_menu);

DEFINE_MENU_CALLBACK(main_open_recent_menu_0);
DEFINE_MENU_CALLBACK(main_open_recent_menu_1);
DEFINE_MENU_CALLBACK(main_open_recent_menu_2);
DEFINE_MENU_CALLBACK(main_open_recent_menu_3);
DEFINE_MENU_CALLBACK(main_open_recent_menu_4);
DEFINE_MENU_CALLBACK(main_open_recent_menu_5);
DEFINE_MENU_CALLBACK(main_open_recent_menu_6);
DEFINE_MENU_CALLBACK(main_open_recent_menu_7);
DEFINE_MENU_CALLBACK(main_open_recent_menu_8);
DEFINE_MENU_CALLBACK(main_open_recent_menu_9);
DEFINE_MENU_CALLBACK(main_open_recent_menu_lock);
DEFINE_MENU_CALLBACK(main_open_recent_menu_clear);

static const MENU main_open_recent_menu_base[] =
{
   { NULL,     main_open_recent_menu_0,     NULL, 0, NULL },
   { NULL,     main_open_recent_menu_1,     NULL, 0, NULL },
   { NULL,     main_open_recent_menu_2,     NULL, 0, NULL },
   { NULL,     main_open_recent_menu_3,     NULL, 0, NULL },
   { NULL,     main_open_recent_menu_4,     NULL, 0, NULL },
   { NULL,     main_open_recent_menu_5,     NULL, 0, NULL },
   { NULL,     main_open_recent_menu_6,     NULL, 0, NULL },
   { NULL,     main_open_recent_menu_7,     NULL, 0, NULL },
   { NULL,     main_open_recent_menu_8,     NULL, 0, NULL },
   { NULL,     main_open_recent_menu_9,     NULL, 0, NULL },
   MENU_SPLITTER,
   { "&Lock",  main_open_recent_menu_lock,  NULL, 0, NULL },
   { "&Clear", main_open_recent_menu_clear, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(main_replay_select_menu_0);
DEFINE_MENU_CALLBACK(main_replay_select_menu_1);
DEFINE_MENU_CALLBACK(main_replay_select_menu_2);
DEFINE_MENU_CALLBACK(main_replay_select_menu_3);
DEFINE_MENU_CALLBACK(main_replay_select_menu_4);
DEFINE_MENU_CALLBACK(main_replay_select_menu_5);
DEFINE_MENU_CALLBACK(main_replay_select_menu_6);
DEFINE_MENU_CALLBACK(main_replay_select_menu_7);
DEFINE_MENU_CALLBACK(main_replay_select_menu_8);
DEFINE_MENU_CALLBACK(main_replay_select_menu_9);

static const MENU main_replay_select_menu_base[] =
{
   { NULL, main_replay_select_menu_0, NULL, 0, NULL },
   { NULL, main_replay_select_menu_1, NULL, 0, NULL },
   { NULL, main_replay_select_menu_2, NULL, 0, NULL },
   { NULL, main_replay_select_menu_3, NULL, 0, NULL },
   { NULL, main_replay_select_menu_4, NULL, 0, NULL },
   { NULL, main_replay_select_menu_5, NULL, 0, NULL },
   { NULL, main_replay_select_menu_6, NULL, 0, NULL },
   { NULL, main_replay_select_menu_7, NULL, 0, NULL },
   { NULL, main_replay_select_menu_8, NULL, 0, NULL },
   { NULL, main_replay_select_menu_9, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(main_replay_menu_record_start);
DEFINE_MENU_CALLBACK(main_replay_menu_record_stop);
DEFINE_MENU_CALLBACK(main_replay_menu_play_start);
DEFINE_MENU_CALLBACK(main_replay_menu_play_stop);

static const MENU main_replay_menu_base[] =
{
   { "&Select",           NULL,                          IMPORT_MENU(main_replay_select_menu), 0,          NULL },
   MENU_SPLITTER,                                                                       
   { "Recording",         NULL,                          NULL,                                 0,          NULL },
   { "  S&tart... (F12)", main_replay_menu_record_start, NULL,                                 0,          NULL },
   { "  St&op     (F12)", main_replay_menu_record_stop,  NULL,                                 D_DISABLED, NULL },
   { "Playback",          NULL,                          NULL,                                 0,          NULL },
   { "  St&art",          main_replay_menu_play_start,   NULL,                                 0,          NULL },
   { "  Sto&p",           main_replay_menu_play_stop,    NULL,                                 D_DISABLED, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(main_record_audio_menu_start);
DEFINE_MENU_CALLBACK(main_record_audio_menu_stop);

static const MENU main_record_audio_menu_base[] =
{
   { "&Start", main_record_audio_menu_start, NULL, 0,          NULL },
   { "S&top",  main_record_audio_menu_stop,  NULL, D_DISABLED, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(main_menu_hide_gui);
DEFINE_MENU_CALLBACK(main_menu_open);
DEFINE_MENU_CALLBACK(main_menu_close);
DEFINE_MENU_CALLBACK(main_menu_cheat_manager);
DEFINE_MENU_CALLBACK(main_menu_save_snapshot);
DEFINE_MENU_CALLBACK(main_menu_advance_frame);
DEFINE_MENU_CALLBACK(main_menu_save_configuration);
DEFINE_MENU_CALLBACK(main_menu_exit);

static const MENU main_menu_base[] =
{
   { "&Hide GUI      (ESC)", main_menu_hide_gui,           NULL,                                0, NULL },
   MENU_SPLITTER,                                                                       
   { "&Open...",             main_menu_open,               NULL,                                0, NULL },
   { "Open R&ecent",         NULL,                         IMPORT_MENU(main_open_recent_menu),  0, NULL },
   { "&Close",               main_menu_close,              NULL,                                0, NULL },
   { "Re&play",              NULL,                         IMPORT_MENU(main_replay_menu),       0, NULL },
   { "Cheat &Manager...",    main_menu_cheat_manager,      NULL,                                0, NULL },
   MENU_SPLITTER,             
   { "&Save Snapshot (F1)",  main_menu_save_snapshot,      NULL,                                0, NULL },
   { "&Advance Frame",       main_menu_advance_frame,      NULL,                                0, NULL },
   { "Recor&d Audio",        NULL,                         IMPORT_MENU(main_record_audio_menu), 0, NULL },
   { "Save Co&nfiguration",  main_menu_save_configuration, NULL,                                0, NULL },
   { "E&xit",                main_menu_exit,               NULL,                                0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(system_save_state_select_menu_0);
DEFINE_MENU_CALLBACK(system_save_state_select_menu_1);
DEFINE_MENU_CALLBACK(system_save_state_select_menu_2);
DEFINE_MENU_CALLBACK(system_save_state_select_menu_3);
DEFINE_MENU_CALLBACK(system_save_state_select_menu_4);
DEFINE_MENU_CALLBACK(system_save_state_select_menu_5);
DEFINE_MENU_CALLBACK(system_save_state_select_menu_6);
DEFINE_MENU_CALLBACK(system_save_state_select_menu_7);
DEFINE_MENU_CALLBACK(system_save_state_select_menu_8);
DEFINE_MENU_CALLBACK(system_save_state_select_menu_9);

static const MENU system_save_state_select_menu_base[] =
{
   { NULL, system_save_state_select_menu_0, NULL, 0, NULL },
   { NULL, system_save_state_select_menu_1, NULL, 0, NULL },
   { NULL, system_save_state_select_menu_2, NULL, 0, NULL },
   { NULL, system_save_state_select_menu_3, NULL, 0, NULL },
   { NULL, system_save_state_select_menu_4, NULL, 0, NULL },
   { NULL, system_save_state_select_menu_5, NULL, 0, NULL },
   { NULL, system_save_state_select_menu_6, NULL, 0, NULL },
   { NULL, system_save_state_select_menu_7, NULL, 0, NULL },
   { NULL, system_save_state_select_menu_8, NULL, 0, NULL },
   { NULL, system_save_state_select_menu_9, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(system_save_state_autosave_menu_disabled);
DEFINE_MENU_CALLBACK(system_save_state_autosave_menu_10_seconds);
DEFINE_MENU_CALLBACK(system_save_state_autosave_menu_30_seconds);
DEFINE_MENU_CALLBACK(system_save_state_autosave_menu_60_seconds);
DEFINE_MENU_CALLBACK(system_save_state_autosave_menu_custom);

static const MENU system_save_state_autosave_menu_base[] =
{
   { "&Disabled",      system_save_state_autosave_menu_disabled,   NULL, 0, NULL },
   MENU_SPLITTER,
   { "&1: 10 Seconds", system_save_state_autosave_menu_10_seconds, NULL, 0, NULL },
   { "&2: 30 Seconds", system_save_state_autosave_menu_30_seconds, NULL, 0, NULL },
   { "&3: 60 Seconds", system_save_state_autosave_menu_60_seconds, NULL, 0, NULL },
   { "&Custom...",     system_save_state_autosave_menu_custom,     NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(system_save_state_menu_quick_save);
DEFINE_MENU_CALLBACK(system_save_state_menu_quick_load);
DEFINE_MENU_CALLBACK(system_save_state_menu_save);
DEFINE_MENU_CALLBACK(system_save_state_menu_restore);
DEFINE_MENU_CALLBACK(system_save_state_menu_select);

static const MENU system_save_state_menu_base[] =
{
   { "&Quick Save (F3)", system_save_state_menu_quick_save, NULL,                                         0, NULL },
   { "Quick &Load (F4)", system_save_state_menu_quick_load, NULL,                                         0, NULL },
   MENU_SPLITTER,
   { "&Select",          NULL,                              IMPORT_MENU(system_save_state_select_menu),   0, NULL },
   { "S&ave...    (F5)", system_save_state_menu_save,       NULL,                                         0, NULL },
   { "&Restore    (F6)", system_save_state_menu_restore,    NULL,                                         0, NULL },
   { "A&utosave",        NULL,                              IMPORT_MENU(system_save_state_autosave_menu), 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(system_region_menu_automatic);
DEFINE_MENU_CALLBACK(system_region_menu_ntsc);
DEFINE_MENU_CALLBACK(system_region_menu_pal);

static const MENU system_region_menu_base[] =
{
   { "&Automatic", system_region_menu_automatic, NULL, 0, NULL },
   { "&NTSC",      system_region_menu_ntsc,      NULL, 0, NULL },
   { "&PAL",       system_region_menu_pal,       NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(system_speed_up_down_menu_50_percent);
DEFINE_MENU_CALLBACK(system_speed_up_down_menu_100_percent);
DEFINE_MENU_CALLBACK(system_speed_up_down_menu_200_percent);
DEFINE_MENU_CALLBACK(system_speed_up_down_menu_custom);

static const MENU system_speed_up_down_menu_base[] =
{
   { "&1: 50%",    system_speed_up_down_menu_50_percent,  NULL, 0, NULL },
   { "&2: 100%",   system_speed_up_down_menu_100_percent, NULL, 0, NULL },
   { "&3: 200%",   system_speed_up_down_menu_200_percent, NULL, 0, NULL },
   { "&Custom...", system_speed_up_down_menu_custom,      NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(system_frame_skip_menu_automatic);
DEFINE_MENU_CALLBACK(system_frame_skip_menu_disabled);
DEFINE_MENU_CALLBACK(system_frame_skip_menu_skip_1_frames);
DEFINE_MENU_CALLBACK(system_frame_skip_menu_skip_2_frames);
DEFINE_MENU_CALLBACK(system_frame_skip_menu_skip_3_frames);
DEFINE_MENU_CALLBACK(system_frame_skip_menu_custom);

static const MENU system_frame_skip_menu_base[] =
{
   { "&Automatic",     system_frame_skip_menu_automatic,     NULL, 0, NULL },
   { "&Disabled",      system_frame_skip_menu_disabled,      NULL, 0, NULL },
   MENU_SPLITTER,
   { "Skip &1 Frames", system_frame_skip_menu_skip_1_frames, NULL, 0, NULL },
   { "Skip &2 Frames", system_frame_skip_menu_skip_2_frames, NULL, 0, NULL },
   { "Skip &3 Frames", system_frame_skip_menu_skip_3_frames, NULL, 0, NULL },
   { "&Custom...",     system_frame_skip_menu_custom,        NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(system_menu_show_status);
DEFINE_MENU_CALLBACK(system_menu_reset);
DEFINE_MENU_CALLBACK(system_menu_power_cycle);
DEFINE_MENU_CALLBACK(system_menu_timing_smoothest);
DEFINE_MENU_CALLBACK(system_menu_timing_most_accurate);
DEFINE_MENU_CALLBACK(system_menu_speed_cap);

static const MENU system_menu_base[] =
{
   { "Sa&ve State",       NULL,                             IMPORT_MENU(system_save_state_menu),    0, NULL },
   { "&Reset",            system_menu_reset,                NULL,                                   0, NULL },
   { "&Power Cycle",      system_menu_power_cycle,          NULL,                                   0, NULL },
   { "R&egion",           NULL,                             IMPORT_MENU(system_region_menu),        0, NULL },
   MENU_SPLITTER,
   { "Timing",            NULL,                             NULL,                                   0, NULL },
   { "  S&moothest",      system_menu_timing_smoothest,     NULL,                                   0, NULL },
   { "  Most &Accurate",  system_menu_timing_most_accurate, NULL,                                   0, NULL },
   { "Speed &Up/Down",    NULL,                             IMPORT_MENU(system_speed_up_down_menu), 0, NULL },
   { "Speed &Cap",        system_menu_speed_cap,            NULL,                                   0, NULL },
   { "&Frame Skip",       NULL,                             IMPORT_MENU(system_frame_skip_menu),    0, NULL },
   { "&Show Status (F2)", system_menu_show_status,          NULL,                                   0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(netplay_menu_start_as_server);
DEFINE_MENU_CALLBACK(netplay_menu_start_as_client);

static const MENU netplay_menu_base[] =
{
   { "&Start as Server...", netplay_menu_start_as_server, NULL, 0, NULL },
   { "Start as &Client...", netplay_menu_start_as_client, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(audio_channels_menu_square_1);
DEFINE_MENU_CALLBACK(audio_channels_menu_square_2);
DEFINE_MENU_CALLBACK(audio_channels_menu_triangle);
DEFINE_MENU_CALLBACK(audio_channels_menu_noise);
DEFINE_MENU_CALLBACK(audio_channels_menu_dmc);
DEFINE_MENU_CALLBACK(audio_channels_menu_extra_1);
DEFINE_MENU_CALLBACK(audio_channels_menu_extra_2);
DEFINE_MENU_CALLBACK(audio_channels_menu_extra_3);
DEFINE_MENU_CALLBACK(audio_channels_menu_enable_all);
DEFINE_MENU_CALLBACK(audio_channels_menu_disable_all);

static const MENU audio_channels_menu_base[] =
{
   { "&Square Wave 1",            audio_channels_menu_square_1,    NULL, 0, NULL },
   { "Square Wave &2",            audio_channels_menu_square_2,    NULL, 0, NULL },
   { "&Triangle Wave",            audio_channels_menu_triangle,    NULL, 0, NULL },
   { "&Noise",                    audio_channels_menu_noise,       NULL, 0, NULL },
   { "&Delta Modulation Channel", audio_channels_menu_dmc,         NULL, 0, NULL },
   { "&Expansion 1",              audio_channels_menu_extra_1,     NULL, 0, NULL },
   { "E&xpansion 2",              audio_channels_menu_extra_2,     NULL, 0, NULL },
   { "Expansion &3",              audio_channels_menu_extra_3,     NULL, 0, NULL },
   MENU_SPLITTER,
   { "En&able All",               audio_channels_menu_enable_all,  NULL, 0, NULL },
   { "D&isable All",              audio_channels_menu_disable_all, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(audio_output_buffer_size_menu_automatic);
DEFINE_MENU_CALLBACK(audio_output_buffer_size_menu_30ms);
DEFINE_MENU_CALLBACK(audio_output_buffer_size_menu_50ms);
DEFINE_MENU_CALLBACK(audio_output_buffer_size_menu_75ms);
DEFINE_MENU_CALLBACK(audio_output_buffer_size_menu_100ms);
DEFINE_MENU_CALLBACK(audio_output_buffer_size_menu_125ms);
DEFINE_MENU_CALLBACK(audio_output_buffer_size_menu_150ms);
DEFINE_MENU_CALLBACK(audio_output_buffer_size_menu_175ms);
DEFINE_MENU_CALLBACK(audio_output_buffer_size_menu_200ms);
DEFINE_MENU_CALLBACK(audio_output_buffer_size_menu_custom);

static const MENU audio_output_buffer_size_menu_base[] =
{
   { "&Automatic", audio_output_buffer_size_menu_automatic, NULL, 0, NULL },
   MENU_SPLITTER,        
   { "&1: 30ms",   audio_output_buffer_size_menu_30ms,      NULL, 0, NULL },
   { "&2: 50ms",   audio_output_buffer_size_menu_50ms,      NULL, 0, NULL },
   { "&3: 75ms",   audio_output_buffer_size_menu_75ms,      NULL, 0, NULL },
   { "&4: 100ms",  audio_output_buffer_size_menu_100ms,     NULL, 0, NULL },
   { "&5: 125ms",  audio_output_buffer_size_menu_125ms,     NULL, 0, NULL },
   { "&6: 150ms",  audio_output_buffer_size_menu_150ms,     NULL, 0, NULL },
   { "&7: 175ms",  audio_output_buffer_size_menu_175ms,     NULL, 0, NULL },
   { "&8: 200ms",  audio_output_buffer_size_menu_200ms,     NULL, 0, NULL },
   { "&Custom...", audio_output_buffer_size_menu_custom,    NULL, 0, NULL },
   MENU_ENDCAP
};                                             

DEFINE_MENU_CALLBACK(audio_output_menu_subsystem_automatic);
DEFINE_MENU_CALLBACK(audio_output_menu_subsystem_safe);
DEFINE_MENU_CALLBACK(audio_output_menu_subsystem_allegro);
DEFINE_MENU_CALLBACK(audio_output_menu_subsystem_openal);
DEFINE_MENU_CALLBACK(audio_output_menu_sampling_rate_automatic);
DEFINE_MENU_CALLBACK(audio_output_menu_sampling_rate_22050_hz);
DEFINE_MENU_CALLBACK(audio_output_menu_sampling_rate_44100_hz);
DEFINE_MENU_CALLBACK(audio_output_menu_sampling_rate_48000_hz);
DEFINE_MENU_CALLBACK(audio_output_menu_sampling_rate_custom);
DEFINE_MENU_CALLBACK(audio_output_menu_mixing_mono);
DEFINE_MENU_CALLBACK(audio_output_menu_mixing_stereo);
DEFINE_MENU_CALLBACK(audio_output_menu_mixing_reverse_stereo);
                                                  
static const MENU audio_output_menu_base[] =
{
   { "Subsystem",         NULL,                                      NULL,                                       0, NULL },
   { "  &Automatic",      audio_output_menu_subsystem_automatic,     NULL,                                       0, NULL },
   { "  &Safe",           audio_output_menu_subsystem_safe,          NULL,                                       0, NULL },
   { "  A&llegro",        audio_output_menu_subsystem_allegro,       NULL,                                       0, NULL },
   { "  &OpenAL",         audio_output_menu_subsystem_openal,        NULL,                                       0, NULL },
   { "Sampling Rate",     NULL,                                      NULL,                                       0, NULL },
   { "  A&utomatic",      audio_output_menu_sampling_rate_automatic, NULL,                                       0, NULL },
   { "  &1: 22050 Hz",    audio_output_menu_sampling_rate_22050_hz,  NULL,                                       0, NULL },
   { "  &2: 44100 Hz",    audio_output_menu_sampling_rate_44100_hz,  NULL,                                       0, NULL },
   { "  &3: 48000 Hz",    audio_output_menu_sampling_rate_48000_hz,  NULL,                                       0, NULL },
   { "  &Custom...",      audio_output_menu_sampling_rate_custom,    NULL,                                       0, NULL },
   { "Mixing",            NULL,                                      NULL,                                       0, NULL },
   { "  &Mono",           audio_output_menu_mixing_mono,             NULL,                                       0, NULL },
   { "  S&tereo",         audio_output_menu_mixing_stereo,           NULL,                                       0, NULL },
   { "  &Reverse Stereo", audio_output_menu_mixing_reverse_stereo,   NULL,                                       0, NULL },
   { "&Buffer Size",      NULL,                                      IMPORT_MENU(audio_output_buffer_size_menu), 0, NULL },
   MENU_ENDCAP          
};                                             
                          
DEFINE_MENU_CALLBACK(audio_menu_enable_apu);
DEFINE_MENU_CALLBACK(audio_menu_enable_output);
DEFINE_MENU_CALLBACK(audio_menu_emulation_fast);
DEFINE_MENU_CALLBACK(audio_menu_emulation_accurate);
DEFINE_MENU_CALLBACK(audio_menu_emulation_high_quality);
DEFINE_MENU_CALLBACK(audio_menu_volume_increase);
DEFINE_MENU_CALLBACK(audio_menu_volume_decrease);
DEFINE_MENU_CALLBACK(audio_menu_volume_custom);
DEFINE_MENU_CALLBACK(audio_menu_volume_reset);
DEFINE_MENU_CALLBACK(audio_menu_volume_logarithmic);
DEFINE_MENU_CALLBACK(audio_menu_volume_auto_gain);
DEFINE_MENU_CALLBACK(audio_menu_volume_auto_normalize);

/* Slot in which the "current volume" text is to placed. */
#define AUDIO_MENU_VOLUME_TEXT   10

static const MENU audio_menu_base[] =
{
   { "&Enable APU",       audio_menu_enable_apu,             NULL,                             0, NULL },
   { "Enable &Output",    audio_menu_enable_output,          NULL,                             0, NULL },
   MENU_SPLITTER,                                                                      
   { "Emulation",         NULL,                              NULL,                             0, NULL },
   { "  &Fast",           audio_menu_emulation_fast,         NULL,                             0, NULL },
   { "  &Accurate",       audio_menu_emulation_accurate,     NULL,                             0, NULL },
   { "  &High Quality",   audio_menu_emulation_high_quality, NULL,                             0, NULL },
   { "&Channels",         NULL,                              IMPORT_MENU(audio_channels_menu), 0, NULL },
   { "Out&put Options",   NULL,                              IMPORT_MENU(audio_output_menu),   0, NULL },
   MENU_SPLITTER,
   /* Kludge to make MENU_FROM_BASE() load the menu. */
   { "insert text here",  NULL,                              NULL,                             0, NULL },
   { "  &Increase (+)",   audio_menu_volume_increase,        NULL,                             0, NULL },
   { "  &Decrease (-)",   audio_menu_volume_decrease,        NULL,                             0, NULL },
   { "  Cu&stom...",      audio_menu_volume_custom,          NULL,                             0, NULL },
   { "  Rese&t",          audio_menu_volume_reset,           NULL,                             0, NULL },
   { "&Logarithmic",      audio_menu_volume_logarithmic,     NULL,                             0, NULL },
   { "Auto &Gain",        audio_menu_volume_auto_gain,       NULL,                             0, NULL },
   { "Auto &Normalize",   audio_menu_volume_auto_normalize,  NULL,                             0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(video_driver_menu_automatic);
DEFINE_MENU_CALLBACK(video_driver_menu_safe);

#ifdef USE_ALLEGROGL
DEFINE_MENU_CALLBACK(video_driver_menu_opengl);
DEFINE_MENU_CALLBACK(video_driver_menu_opengl_fullscreen);
DEFINE_MENU_CALLBACK(video_driver_menu_opengl_windowed);
#endif

#ifdef ALLEGRO_DOS
DEFINE_MENU_CALLBACK(video_driver_menu_vesa);
DEFINE_MENU_CALLBACK(video_driver_menu_vesa_2_banked);
DEFINE_MENU_CALLBACK(video_driver_menu_vesa_2_linear);
DEFINE_MENU_CALLBACK(video_driver_menu_vesa_3);
DEFINE_MENU_CALLBACK(video_driver_menu_vesa_vbe_af);
DEFINE_MENU_CALLBACK(video_driver_menu_vga);
DEFINE_MENU_CALLBACK(video_driver_menu_vga_mode_x);
#endif

#ifdef ALLEGRO_LINUX
DEFINE_MENU_CALLBACK(video_driver_menu_framebuffer);
DEFINE_MENU_CALLBACK(video_driver_menu_svgalib);
DEFINE_MENU_CALLBACK(video_driver_menu_vesa_vbe_af);
DEFINE_MENU_CALLBACK(video_driver_menu_vga);
DEFINE_MENU_CALLBACK(video_driver_menu_vga_mode_x);
#endif

#ifdef ALLEGRO_UNIX
DEFINE_MENU_CALLBACK(video_driver_menu_dga);
DEFINE_MENU_CALLBACK(video_driver_menu_dga_fullscreen);
DEFINE_MENU_CALLBACK(video_driver_menu_dga_2);
DEFINE_MENU_CALLBACK(video_driver_menu_x_windows);
DEFINE_MENU_CALLBACK(video_driver_menu_x_windows_fullscreen);
#endif

#ifdef ALLEGRO_WINDOWS
DEFINE_MENU_CALLBACK(video_driver_menu_directx);
DEFINE_MENU_CALLBACK(video_driver_menu_directx_overlay);
DEFINE_MENU_CALLBACK(video_driver_menu_directx_windowed);
DEFINE_MENU_CALLBACK(video_driver_menu_gdi);
#endif

static const MENU video_driver_menu_base[] =
{
   { "&Automatic",              video_driver_menu_automatic,                 NULL, 0, NULL },
   { "&Safe",                   video_driver_menu_safe,                      NULL, 0, NULL },
   MENU_SPLITTER,
   { "Accelerated",             NULL,                                   NULL, 0, NULL },
#ifdef USE_ALLEGROGL
   { "  &OpenGL",               video_driver_menu_opengl,               NULL, 0, NULL },
   { "  OpenGL &Fullscreen",    video_driver_menu_opengl_fullscreen,    NULL, 0, NULL },
   { "  OpenGL &Windowed",      video_driver_menu_opengl_windowed,      NULL, 0, NULL },
#endif
#ifdef ALLEGRO_DOS
   { "  V&ESA",                 video_driver_menu_vesa,                 NULL, 0, NULL },
   { "  VESA 2 &Banked",        video_driver_menu_vesa_2_banked,        NULL, 0, NULL },
   { "  VESA 2 &Linear",        video_driver_menu_vesa_2_linear,        NULL, 0, NULL },
   { "  VE&SA 3",               video_driver_menu_vesa_3,               NULL, 0, NULL },
   { "  VESA VBE/&AF",          video_driver_menu_vesa_vbe_af,          NULL, 0, NULL },
   { "  &VGA",                  video_driver_menu_vga,                  NULL, 0, NULL },
   { "  VGA Mode-&X",           video_driver_menu_vga_mode_x,           NULL, 0, NULL },
#endif
#ifdef ALLEGRO_LINUX
   { "  &SVGAlib",              video_driver_menu_svgalib,              NULL, 0, NULL },
   { "  VESA VBE/&AF",          video_driver_menu_vesa_vbe_af,          NULL, 0, NULL },
   { "  &VGA",                  video_driver_menu_vga,                  NULL, 0, NULL },
   { "  VGA Mode-&X",           video_driver_menu_vga_mode_x,           NULL, 0, NULL },
#endif
#ifdef ALLEGRO_UNIX
   { "  &DGA",                  video_driver_menu_dga,                  NULL, 0, NULL },
   { "  &DGA Fullscreen",       video_driver_menu_dga_fullscreen,       NULL, 0, NULL },
   { "  D&GA 2",                video_driver_menu_dga_2,                NULL, 0, NULL },
#endif
#ifdef ALLEGRO_WINDOWS
   { "  &DirectX",              video_driver_menu_directx,              NULL, 0, NULL },
   { "  DirectX &Overlay",      video_driver_menu_directx_overlay,      NULL, 0, NULL },
   { "  DirectX &Windowed",     video_driver_menu_directx_windowed,     NULL, 0, NULL },
#endif
   { "Software",                NULL,                                   NULL, 0, NULL },
#ifdef ALLEGRO_LINUX
   { "  &Framebuffer",          video_driver_menu_framebuffer ,         NULL, 0, NULL },
#endif
#ifdef ALLEGRO_UNIX
   { "  X &Windows",            video_driver_menu_x_windows,            NULL, 0, NULL },
   { "  X &Windows Fullscreen", video_driver_menu_x_windows_fullscreen, NULL, 0, NULL },
#endif
#ifdef ALLEGRO_WINDOWS
   { "  &GDI",                  video_driver_menu_gdi,                  NULL, 0, NULL },
#endif
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(video_resolution_proportionate_menu_256_224);
DEFINE_MENU_CALLBACK(video_resolution_proportionate_menu_256_240);
DEFINE_MENU_CALLBACK(video_resolution_proportionate_menu_512_448);
DEFINE_MENU_CALLBACK(video_resolution_proportionate_menu_512_480);
DEFINE_MENU_CALLBACK(video_resolution_proportionate_menu_768_672);
DEFINE_MENU_CALLBACK(video_resolution_proportionate_menu_768_720);
DEFINE_MENU_CALLBACK(video_resolution_proportionate_menu_1024_896);
DEFINE_MENU_CALLBACK(video_resolution_proportionate_menu_1024_960);
DEFINE_MENU_CALLBACK(video_resolution_proportionate_menu_1280_1120);
DEFINE_MENU_CALLBACK(video_resolution_proportionate_menu_1280_1200);

static const MENU video_resolution_proportionate_menu_base[] =
{
   { " &1: 256x224",   video_resolution_proportionate_menu_256_224,   NULL, 0, NULL },
   { " &2: 256x240",   video_resolution_proportionate_menu_256_240,   NULL, 0, NULL },
   { " &3: 512x448",   video_resolution_proportionate_menu_512_448,   NULL, 0, NULL },
   { " &4: 512x480",   video_resolution_proportionate_menu_512_480,   NULL, 0, NULL },
   { " &5: 768x672",   video_resolution_proportionate_menu_768_672,   NULL, 0, NULL },
   { " &6: 768x720",   video_resolution_proportionate_menu_768_720,   NULL, 0, NULL },
   { " &7: 1024x896",  video_resolution_proportionate_menu_1024_896,  NULL, 0, NULL },
   { " &8: 1024x960",  video_resolution_proportionate_menu_1024_960,  NULL, 0, NULL },
   { " &9: 1280x1120", video_resolution_proportionate_menu_1280_1120, NULL, 0, NULL },
   { "1&0: 1280x1200", video_resolution_proportionate_menu_1280_1200, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(video_resolution_menu_320_240);
DEFINE_MENU_CALLBACK(video_resolution_menu_400_300);
DEFINE_MENU_CALLBACK(video_resolution_menu_640_480);
DEFINE_MENU_CALLBACK(video_resolution_menu_800_600);
DEFINE_MENU_CALLBACK(video_resolution_menu_1024_768);
DEFINE_MENU_CALLBACK(video_resolution_menu_1152_864);
DEFINE_MENU_CALLBACK(video_resolution_menu_1280_960);
DEFINE_MENU_CALLBACK(video_resolution_menu_1280_1024);
DEFINE_MENU_CALLBACK(video_resolution_menu_1600_1200);
DEFINE_MENU_CALLBACK(video_resolution_menu_custom);

static const MENU video_resolution_menu_base[] =
{
   { "Standard",         NULL,                            NULL,                                             0, NULL },
   { "  &1: 320x240",    video_resolution_menu_320_240,   NULL,                                             0, NULL },
   { "  &2: 400x300",    video_resolution_menu_400_300,   NULL,                                             0, NULL },
   { "  &3: 640x480",    video_resolution_menu_640_480,   NULL,                                             0, NULL },
   { "  &4: 800x600",    video_resolution_menu_800_600,   NULL,                                             0, NULL },
   { "  &5: 1024x768",   video_resolution_menu_1024_768,  NULL,                                             0, NULL },
   { "  &6: 1152x864",   video_resolution_menu_1152_864,  NULL,                                             0, NULL },
   { "  &7: 1280x960",   video_resolution_menu_1280_960,  NULL,                                             0, NULL },
   { "  &8: 1280x1024",  video_resolution_menu_1280_1024, NULL,                                             0, NULL },
   { "  &9: 1600x1200",  video_resolution_menu_1600_1200, NULL,                                             0, NULL },
   { "&Proportionate",   NULL,                            IMPORT_MENU(video_resolution_proportionate_menu), 0, NULL },
   { "&Custom...",       video_resolution_menu_custom,    NULL,                                             0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(video_color_depth_menu_paletted_8_bit);
DEFINE_MENU_CALLBACK(video_color_depth_menu_true_color_15_bit);
DEFINE_MENU_CALLBACK(video_color_depth_menu_true_color_16_bit);
DEFINE_MENU_CALLBACK(video_color_depth_menu_true_color_24_bit);
DEFINE_MENU_CALLBACK(video_color_depth_menu_true_color_32_bit);

static const MENU video_color_depth_menu_base[] =
{
   { "&Paletted (8-bit)",    video_color_depth_menu_paletted_8_bit,    NULL, 0, NULL },
   { "&True Color (15-bit)", video_color_depth_menu_true_color_15_bit, NULL, 0, NULL },
   { "True &Color (16-bit)", video_color_depth_menu_true_color_16_bit, NULL, 0, NULL },
   { "True C&olor (24-bit)", video_color_depth_menu_true_color_24_bit, NULL, 0, NULL },
   { "True Co&lor (32-bit)", video_color_depth_menu_true_color_32_bit, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(video_blitter_menu_disabled);
DEFINE_MENU_CALLBACK(video_blitter_menu_hq2x);
DEFINE_MENU_CALLBACK(video_blitter_menu_hq3x);
DEFINE_MENU_CALLBACK(video_blitter_menu_hq4x);
DEFINE_MENU_CALLBACK(video_blitter_menu_interpolation);
DEFINE_MENU_CALLBACK(video_blitter_menu_interpolation_scanlines);
DEFINE_MENU_CALLBACK(video_blitter_menu_interpolation_tv_mode);
DEFINE_MENU_CALLBACK(video_blitter_menu_ntsc);
DEFINE_MENU_CALLBACK(video_blitter_menu_configure);

static const MENU video_blitter_menu_base[] =
{
   { "&Disabled",                        video_blitter_menu_disabled,                NULL, 0, NULL },
   MENU_SPLITTER,  
   { "Basic",                            NULL,                                       NULL, 0, NULL },
   { "   &1: HQ2X",                      video_blitter_menu_hq2x,                    NULL, 0, NULL },
   { "   &2: HQ3X",                      video_blitter_menu_hq3x,                    NULL, 0, NULL },
   { "   &3: HQ4X",                      video_blitter_menu_hq4x,                    NULL, 0, NULL },
   { "   &4: Interpolation",             video_blitter_menu_interpolation,           NULL, 0, NULL },
   { "   &5: Interpolation (Scanlines)", video_blitter_menu_interpolation_scanlines, NULL, 0, NULL },
   { "   &6: Interpolation (TV Mode)",   video_blitter_menu_interpolation_tv_mode,   NULL, 0, NULL },
   { "Advanced",                         NULL,                                       NULL, 0, NULL },
   { "  14: &NTSC",                      video_blitter_menu_ntsc,                    NULL, 0, NULL },
   { "&Configure...",                    video_blitter_menu_configure,               NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(video_layers_menu_show_back_sprites);
DEFINE_MENU_CALLBACK(video_layers_menu_show_front_sprites);
DEFINE_MENU_CALLBACK(video_layers_menu_show_background);

static const MENU video_layers_menu_base[] =
{
   { "&Show Back Sprites   (F7)", video_layers_menu_show_back_sprites,   NULL, 0, NULL },
   { "Show &Front Sprites  (F7)", video_layers_menu_show_front_sprites,  NULL, 0, NULL },
   { "Show &Background     (F8)", video_layers_menu_show_background,     NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(video_palette_menu_nester);
DEFINE_MENU_CALLBACK(video_palette_menu_nesticle);
DEFINE_MENU_CALLBACK(video_palette_menu_ntsc);
DEFINE_MENU_CALLBACK(video_palette_menu_pal);
DEFINE_MENU_CALLBACK(video_palette_menu_rgb);

static const MENU video_palette_menu_base[] =
{
   { "&Nester",   video_palette_menu_nester,   NULL, 0, NULL },
   { "N&esticle", video_palette_menu_nesticle, NULL, 0, NULL },
   { "N&TSC",     video_palette_menu_ntsc,     NULL, 0, NULL },
   { "&PAL",      video_palette_menu_pal,      NULL, 0, NULL },
   { "&RGB",      video_palette_menu_rgb,      NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(video_menu_color);

static const MENU video_menu_base[] =
{                    
   MENU_SPLITTER,          
   { "Basic Configuration",    NULL,              NULL,                                0, NULL },
   { "  &Screen/Window Size",  NULL,              IMPORT_MENU(video_resolution_menu),  0, NULL },
   { "  B&litter",             NULL,              IMPORT_MENU(video_blitter_menu),     0, NULL },
   { "  Pal&ette",             NULL,              IMPORT_MENU(video_palette_menu),     0, NULL },
   { "  Rendering Options",    NULL,              IMPORT_MENU(video_layers_menu),      0, NULL },
   { "Advanced Configuration", NULL,              NULL,                                0, NULL },
   { "  &Driver",              NULL,              IMPORT_MENU(video_driver_menu),      0, NULL },
   { "  &Color Depth",         NULL,              IMPORT_MENU(video_color_depth_menu), 0, NULL },
   { "  C&olor Correction...", video_menu_color,  NULL,                                0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(input_menu_configure);
DEFINE_MENU_CALLBACK(input_menu_enable_zapper);

static const MENU input_menu_base[] =
{
   { "&Configure...",  input_menu_configure,     NULL, 0, NULL },
   MENU_SPLITTER,
   { "&Enable Zapper", input_menu_enable_zapper, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_cpu_usage_menu_passive);
DEFINE_MENU_CALLBACK(options_cpu_usage_menu_normal);
DEFINE_MENU_CALLBACK(options_cpu_usage_menu_aggressive);

static const MENU options_cpu_usage_menu_base[] =
{
   { "&Passive/Laptop", options_cpu_usage_menu_passive,    NULL, 0, NULL },
   { "&Normal",         options_cpu_usage_menu_normal,     NULL, 0, NULL },
   { "&Aggressive",     options_cpu_usage_menu_aggressive, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_gui_theme_menu_classic);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_stainless_steel);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_zero_4);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_panta);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_fireflower);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_xodiac);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_monochrome);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_essence);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_voodoo);
DEFINE_MENU_CALLBACK(options_gui_theme_menu_hugs_and_kisses);

static const MENU options_gui_theme_menu_base[] =
{
   { "Complete Themes",       NULL,                                   NULL, 0, NULL },
   { "  &1: Classic",         options_gui_theme_menu_classic,         NULL, 0, NULL },
   { "  &2: stainless Steel", options_gui_theme_menu_stainless_steel, NULL, 0, NULL },
   { "  &3: Zero 4",          options_gui_theme_menu_zero_4,          NULL, 0, NULL },
   { "  &4: Panta",           options_gui_theme_menu_panta,           NULL, 0, NULL },
   { "  &5: Fireflower",      options_gui_theme_menu_fireflower,      NULL, 0, NULL },
   { "Color Themes",          NULL,                                   NULL, 0, NULL },
   { "  &6: Xodiac",          options_gui_theme_menu_xodiac,          NULL, 0, NULL },
   { "  &7: Monochrome",      options_gui_theme_menu_monochrome,      NULL, 0, NULL },
   { "  &8: Essence",         options_gui_theme_menu_essence,         NULL, 0, NULL },
   { "  &9: Voodoo",          options_gui_theme_menu_voodoo,          NULL, 0, NULL },
   { " 1&0: Hugs and Kisses", options_gui_theme_menu_hugs_and_kisses, NULL, 0, NULL },
   MENU_ENDCAP
};

DEFINE_MENU_CALLBACK(options_menu_paths);
DEFINE_MENU_CALLBACK(options_menu_reset_clock);

static const MENU options_menu_base[] =
{
   { "&Audio",            NULL,                      IMPORT_MENU(audio_menu),             0, NULL },
   { "&Video",            NULL,                      IMPORT_MENU(video_menu),             0, NULL },
   { "&Input",            NULL,                      IMPORT_MENU(input_menu),             0, NULL },
   { "&CPU Usage",        NULL,                      IMPORT_MENU(options_cpu_usage_menu), 0, NULL },
   { "&Paths...",         options_menu_paths,        NULL,                                0, NULL },
   { "&GUI Theme",        NULL,                      IMPORT_MENU(options_gui_theme_menu), 0, NULL },
   MENU_SPLITTER,
   { "&Reset Clock",      options_menu_reset_clock,  NULL,                                0, NULL },
   MENU_ENDCAP      
};

DEFINE_MENU_CALLBACK(help_menu_keyboard_shortcuts);
DEFINE_MENU_CALLBACK(help_menu_view_license);
DEFINE_MENU_CALLBACK(help_menu_view_log);
DEFINE_MENU_CALLBACK(help_menu_fakenes_team);
DEFINE_MENU_CALLBACK(help_menu_about);

static const MENU help_menu_base[] =
{
   { "&Keyboard Shortcuts...", help_menu_keyboard_shortcuts, NULL, 0,          NULL },
   { "&View Readme...",        NULL,                         NULL, D_DISABLED, NULL },
   { "&View License...",       help_menu_view_license,       NULL, 0,          NULL },
   { "View &Log...",           help_menu_view_log,           NULL, 0,          NULL },
   { "&FakeNES Team...",       help_menu_fakenes_team,       NULL, 0,          NULL },
   { "&About...",              help_menu_about,              NULL, 0,          NULL },
   MENU_ENDCAP
};

/* Note: Audio, Video and Input menu entries moved to the Options menu. */
static const MENU top_menu_base[] =
{ 
   { "&Main",    NULL, IMPORT_MENU(main_menu),    0, NULL },
   { "&System",  NULL, IMPORT_MENU(system_menu),  0, NULL },
   { "&Options", NULL, IMPORT_MENU(options_menu), 0, NULL },
#ifdef USE_HAWKNL
   { "&NetPlay", NULL, IMPORT_MENU(netplay_menu), 0, NULL },
#endif
   { "&Help",    NULL, IMPORT_MENU(help_menu),    0, NULL },
   MENU_ENDCAP
};

/* Undefine helper macros. */
#undef DEFINE_MENU
#undef DEFINE_MENU_CALLBACK
#undef IMPORT_MENU
#undef MENU_SPLITTER
#undef MENU_ENDCAP

#define SET_FLAG(var, flag, value) \
   if (value)  \
      var |= flag;   \
   else  \
      var &= ~ flag; \

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

#undef SET_FLAG

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

static INLINE MENU *load_menu (const MENU *menu)
{
   MENU *new_menu;
   int size = 0;
   int index = 0;

   RT_ASSERT(menu);

   while (menu[index].text || menu[index].proc)
   {
       size += sizeof (MENU);

       index++;
   }

   /* Once more for the end marker. */
   size += sizeof (MENU);

   if (!(new_menu = malloc (size)))
   {
      WARN("Failed to allocate menu structure");

      return (NULL);
   }

   memcpy (new_menu, menu, size);

   /* Reset counter. */
   index = 0;

   while (new_menu[index].text || new_menu[index].proc)
   {
      MENU *item = &new_menu[index];

      /* Import menu by reference. */
      if (item->child)
         item->child = *(MENU **)item->child;

      index++;
   }

   return (new_menu);
}

static INLINE void unload_menu (MENU *menu)
{
   RT_ASSERT(menu);

   free (menu);
}

#define MENU_FROM_BASE(name)  (name = load_menu (name ##_base))

static INLINE void load_menus (void)
{
   MENU_FROM_BASE(main_open_recent_menu);
   MENU_FROM_BASE(main_replay_select_menu);
   MENU_FROM_BASE(main_replay_menu);
   MENU_FROM_BASE(main_record_audio_menu);
   MENU_FROM_BASE(main_menu);
   MENU_FROM_BASE(system_save_state_select_menu);
   MENU_FROM_BASE(system_save_state_autosave_menu);
   MENU_FROM_BASE(system_save_state_menu);
   MENU_FROM_BASE(system_region_menu);
   MENU_FROM_BASE(system_speed_up_down_menu);
   MENU_FROM_BASE(system_frame_skip_menu);
   MENU_FROM_BASE(system_menu);
   MENU_FROM_BASE(audio_output_buffer_size_menu);
   MENU_FROM_BASE(audio_output_menu);
   MENU_FROM_BASE(audio_channels_menu);
   MENU_FROM_BASE(audio_menu);
   MENU_FROM_BASE(video_driver_menu);
   MENU_FROM_BASE(video_resolution_proportionate_menu);
   MENU_FROM_BASE(video_resolution_menu);
   MENU_FROM_BASE(video_color_depth_menu);
   MENU_FROM_BASE(video_blitter_menu);
   MENU_FROM_BASE(video_layers_menu);
   MENU_FROM_BASE(video_palette_menu);
   MENU_FROM_BASE(video_menu);
   MENU_FROM_BASE(input_menu);
   MENU_FROM_BASE(options_cpu_usage_menu);
   MENU_FROM_BASE(options_gui_theme_menu);
   MENU_FROM_BASE(options_menu);
   MENU_FROM_BASE(netplay_menu);
   MENU_FROM_BASE(help_menu);
   MENU_FROM_BASE(top_menu);
}

#undef MENU_FROM_BASE

static INLINE void unload_menus (void)
{
   unload_menu (main_open_recent_menu);
   unload_menu (main_replay_select_menu);
   unload_menu (main_replay_menu);
   unload_menu (main_record_audio_menu);
   unload_menu (main_menu);
   unload_menu (system_save_state_select_menu);
   unload_menu (system_save_state_autosave_menu);
   unload_menu (system_save_state_menu);
   unload_menu (system_region_menu);
   unload_menu (system_speed_up_down_menu);
   unload_menu (system_frame_skip_menu);
   unload_menu (system_menu);
   unload_menu (audio_output_buffer_size_menu);
   unload_menu (audio_output_menu);
   unload_menu (audio_channels_menu);
   unload_menu (audio_menu);
   unload_menu (video_driver_menu);
   unload_menu (video_resolution_proportionate_menu);
   unload_menu (video_resolution_menu);
   unload_menu (video_color_depth_menu);
   unload_menu (video_blitter_menu);
   unload_menu (video_layers_menu);
   unload_menu (video_palette_menu);
   unload_menu (video_menu);
   unload_menu (input_menu);
   unload_menu (options_cpu_usage_menu);
   unload_menu (options_gui_theme_menu);
   unload_menu (options_menu);
   unload_menu (netplay_menu);
   unload_menu (help_menu);
   unload_menu (top_menu);
}
