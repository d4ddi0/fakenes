

static int file_menu_load_rom (void);

static int file_menu_snapshot (void);

static int file_menu_exit (void);


static MENU file_menu [] =
{
    { "&Load ROM...", file_menu_load_rom, NULL, 0, NULL },
    {             "",               NULL, NULL, 0, NULL },
    {    "&Snapshot", file_menu_snapshot, NULL, 0, NULL },
    {             "",               NULL, NULL, 0, NULL },
    {        "E&xit",     file_menu_exit, NULL, 0, NULL },
    {           NULL,               NULL, NULL, 0, NULL }
};


static int machine_type_menu_ntsc (void);

static int machine_type_menu_pal (void);


static MENU machine_type_menu [] =
{
    { "&NTSC", machine_type_menu_ntsc, NULL, 0, NULL },
    {      "",                   NULL, NULL, 0, NULL },
    {  "&PAL",  machine_type_menu_pal, NULL, 0, NULL },
    {    NULL,                   NULL, NULL, 0, NULL }
};


static int machine_state_menu_select (void);

static int machine_state_menu_save (void);

static int machine_state_menu_restore (void);


static MENU machine_state_menu [] =
{
    { "S&elect...",  machine_state_menu_select, NULL, D_DISABLED, NULL },
    {           "",                       NULL, NULL,          0, NULL },
    {      "&Save",    machine_state_menu_save, NULL,          0, NULL },
    {           "",                       NULL, NULL,          0, NULL },
    {   "&Restore", machine_state_menu_restore, NULL,          0, NULL },
    {         NULL,                       NULL, NULL,          0, NULL }
};


static int machine_menu_reset (void);

static int machine_menu_status (void);


static MENU machine_menu [] =
{
    {  "&Reset",  machine_menu_reset,               NULL, 0, NULL },
    {        "",                NULL,               NULL, 0, NULL },
    { "&Status", machine_menu_status,               NULL, 0, NULL },
    {        "",                NULL,               NULL, 0, NULL },
    {   "&Type",                NULL,  machine_type_menu, 0, NULL },
    {        "",                NULL,               NULL, 0, NULL },
    {  "&State",                NULL, machine_state_menu, 0, NULL },
    {      NULL,                NULL,               NULL, 0, NULL }
};


static int options_audio_mixing_quality_menu_low_8_bit (void);

static int options_audio_mixing_quality_menu_high_16_bit (void);


static MENU options_audio_mixing_quality_menu [] =
{
    {   "&Low (8 bits)",  options_audio_mixing_quality_menu_low_8_bit, NULL, 0, NULL },
    {                "",                                     NULL, NULL, 0, NULL },
    { "&High (16 bits)", options_audio_mixing_quality_menu_high_16_bit, NULL, 0, NULL },
    {              NULL,                                     NULL, NULL, 0, NULL }
};


static int options_audio_mixing_stereo_menu_classic (void);

static int options_audio_mixing_stereo_menu_enhanced (void);

static int options_audio_mixing_stereo_menu_accurate (void);


static MENU options_audio_mixing_stereo_menu [] =
{
    {  "&Classic",  options_audio_mixing_stereo_menu_classic, NULL, 0, NULL },
    {          "",                                      NULL, NULL, 0, NULL },
    { "&Enhanced", options_audio_mixing_stereo_menu_enhanced, NULL, 0, NULL },
    {          "",                                      NULL, NULL, 0, NULL },
    { "&Accurate", options_audio_mixing_stereo_menu_accurate, NULL, 0, NULL },
    {        NULL,                                      NULL, NULL, 0, NULL }
};


static int options_audio_mixing_menu_normal (void);


static MENU options_audio_mixing_menu [] =
{
    {  "&Normal",  options_audio_mixing_menu_normal,                              NULL, 0, NULL },
    {         "",                              NULL,                              NULL, 0, NULL },
    {  "&Stereo",                              NULL,  options_audio_mixing_stereo_menu, 0, NULL },
    {         "",                              NULL,                              NULL, 0, NULL },
    { "&Quality",                              NULL, options_audio_mixing_quality_menu, 0, NULL },
    {       NULL,                              NULL,                              NULL, 0, NULL }
};


static int options_audio_effects_menu_linear_echo (void);

static int options_audio_effects_menu_surround_sound (void);


static MENU options_audio_effects_menu [] =
{
    {    "&Linear Echo",    options_audio_effects_menu_linear_echo, NULL, 0, NULL },
    {                "",                                      NULL, NULL, 0, NULL },
    { "&Surround Sound", options_audio_effects_menu_surround_sound, NULL, 0, NULL },
    {              NULL,                                      NULL, NULL, 0, NULL }
};


static int options_audio_filter_low_pass_menu_simple (void);

static int options_audio_filter_low_pass_menu_weighted (void);

static int options_audio_filter_low_pass_menu_dynamic (void);


static MENU options_audio_filter_low_pass_menu [] =
{
    {   "&Simple",   options_audio_filter_low_pass_menu_simple, NULL, 0, NULL },
    {          "",                                        NULL, NULL, 0, NULL },
    { "&Weighted", options_audio_filter_low_pass_menu_weighted, NULL, 0, NULL },
    {          "",                                        NULL, NULL, 0, NULL },
    {  "&Dynamic",  options_audio_filter_low_pass_menu_dynamic, NULL, 0, NULL },
    {        NULL,                                        NULL, NULL, 0, NULL }
};


static int options_audio_filter_menu_none (void);

static int options_audio_filter_menu_high_pass (void);


static MENU options_audio_filter_menu [] =
{     
    {      "&None",      options_audio_filter_menu_none,                               NULL, 0, NULL },
    {           "",                                NULL,                               NULL, 0, NULL },
    {  "&Low Pass",                                NULL, options_audio_filter_low_pass_menu, 0, NULL },
    {           "",                                NULL,                               NULL, 0, NULL },
    { "&High Pass", options_audio_filter_menu_high_pass,                               NULL, 0, NULL },
    {         NULL,                                NULL,                               NULL, 0, NULL }
};


static int options_audio_channels_menu_square_1 (void);

static int options_audio_channels_menu_square_2 (void);

static int options_audio_channels_menu_triangle (void);

static int options_audio_channels_menu_noise (void);

static int options_audio_channels_menu_dmc (void);


static MENU options_audio_channels_menu [] =
{
    { "&Square 1", options_audio_channels_menu_square_1, NULL, 0, NULL },
    {          "",                                 NULL, NULL, 0, NULL },
    { "S&quare 2", options_audio_channels_menu_square_2, NULL, 0, NULL },
    {          "",                                 NULL, NULL, 0, NULL },
    { "&Triangle", options_audio_channels_menu_triangle, NULL, 0, NULL },
    {          "",                                 NULL, NULL, 0, NULL },
    {    "&Noise",    options_audio_channels_menu_noise, NULL, 0, NULL },
    {          "",                                 NULL, NULL, 0, NULL },
    {      "&DMC",      options_audio_channels_menu_dmc, NULL, 0, NULL },
    {        NULL,                                 NULL, NULL, 0, NULL }
};


static int options_audio_advanced_menu_ideal_triangle (void);

static int options_audio_advanced_menu_smooth_envelope (void);

static int options_audio_advanced_menu_smooth_sweep (void);


static MENU options_audio_advanced_menu [] =
{
    {  "&Ideal Triangle",  options_audio_advanced_menu_ideal_triangle, NULL, 0, NULL },
    {                 "",                                        NULL, NULL, 0, NULL },
    { "&Smooth Envelope", options_audio_advanced_menu_smooth_envelope, NULL, 0, NULL },
    {                 "",                                        NULL, NULL, 0, NULL },
    {    "S&mooth Sweep",    options_audio_advanced_menu_smooth_sweep, NULL, 0, NULL },
    {               NULL,                                        NULL, NULL, 0, NULL }
};


static int options_audio_record_menu_start (void);

static int options_audio_record_menu_stop (void);


static MENU options_audio_record_menu [] =
{
    { "&Start",  options_audio_record_menu_start, NULL,          0, NULL },
    {       "",                             NULL, NULL,          0, NULL },
    {  "S&top",   options_audio_record_menu_stop, NULL, D_DISABLED, NULL },
    {     NULL,                             NULL, NULL,          0, NULL }
};


static int options_audio_menu_enabled (void);


static MENU options_audio_menu [] =
{
    {  "&Enabled", options_audio_menu_enabled,                        NULL, 0, NULL },
    {          "",                       NULL,                        NULL, 0, NULL },
    {   "&Mixing",                       NULL,   options_audio_mixing_menu, 0, NULL },
    {          "",                       NULL,                        NULL, 0, NULL },
    {  "&Effects",                       NULL,  options_audio_effects_menu, 0, NULL },
    {          "",                       NULL,                        NULL, 0, NULL },
    {   "&Filter",                       NULL,   options_audio_filter_menu, 0, NULL },
    {          "",                       NULL,                        NULL, 0, NULL },
    { "&Channels",                       NULL, options_audio_channels_menu, 0, NULL },
    {          "",                       NULL,                        NULL, 0, NULL },
    { "&Advanced",                       NULL, options_audio_advanced_menu, 0, NULL },
    {          "",                       NULL,                        NULL, 0, NULL },
    {   "&Record",                       NULL,   options_audio_record_menu, 0, NULL },
    {        NULL,                       NULL,                        NULL, 0, NULL }
};


static int options_video_blitter_menu_normal (void);

static int options_video_blitter_menu_stretched (void);


static MENU options_video_blitter_menu [] =
{
    {    "&Normal",    options_video_blitter_menu_normal, NULL, 0, NULL },
    {           "",                                 NULL, NULL, 0, NULL },
    { "&Stretched", options_video_blitter_menu_stretched, NULL, 0, NULL },
    {         NULL,                                 NULL, NULL, 0, NULL }
};


static int options_video_layers_menu_sprites_a (void);

static int options_video_layers_menu_sprites_b (void);

static int options_video_layers_menu_background (void);


static MENU options_video_layers_menu [] =
{
    {  "&Sprites A",  options_video_layers_menu_sprites_a, NULL, 0, NULL },
    {            "",                                 NULL, NULL, 0, NULL },
    {  "&Sprites B",  options_video_layers_menu_sprites_b, NULL, 0, NULL },
    {            "",                                 NULL, NULL, 0, NULL },
    { "&Background", options_video_layers_menu_background, NULL, 0, NULL },
    {          NULL,                                 NULL, NULL, 0, NULL }
};


static int options_video_palette_menu_default (void);

static int options_video_palette_menu_grayscale (void);

static int options_video_palette_menu_gnuboy (void);

static int options_video_palette_menu_nester (void);

static int options_video_palette_menu_nesticle (void);

static int options_video_palette_menu_custom (void);


static MENU options_video_palette_menu [] =
{
    {   "&Default",   options_video_palette_menu_default, NULL, 0, NULL },
    {           "",                                 NULL, NULL, 0, NULL },
    { "&Grayscale", options_video_palette_menu_grayscale, NULL, 0, NULL },
    {           "",                                 NULL, NULL, 0, NULL },
    {    "&gnuboy",    options_video_palette_menu_gnuboy, NULL, 0, NULL },
    {           "",                                 NULL, NULL, 0, NULL },
    {    "&NESter",    options_video_palette_menu_nester, NULL, 0, NULL },
    {           "",                                 NULL, NULL, 0, NULL },
    {  "&NESticle",  options_video_palette_menu_nesticle, NULL, 0, NULL },
    {           "",                                 NULL, NULL, 0, NULL },
    {    "&Custom",    options_video_palette_menu_custom, NULL, 0, NULL },
    {         NULL,                                 NULL, NULL, 0, NULL }
};


static int options_video_menu_vsync (void);


static MENU options_video_menu [] =
{
    { "&Blitter",                       NULL, options_video_blitter_menu, 0, NULL },
    {         "",                       NULL,                       NULL, 0, NULL },
    {   "&VSync",   options_video_menu_vsync,                       NULL, 0, NULL },
    {         "",                       NULL,                       NULL, 0, NULL },
    {  "&Layers",                       NULL,  options_video_layers_menu, 0, NULL },
    {         "",                       NULL,                       NULL, 0, NULL },
    { "&Palette",                       NULL, options_video_palette_menu, 0, NULL },
    {       NULL,                       NULL,                       NULL, 0, NULL }
};


static MENU options_menu [] =
{
    { "&Audio", NULL, options_audio_menu, 0, NULL },
    {       "", NULL,               NULL, 0, NULL },
    { "&Video", NULL, options_video_menu, 0, NULL },
    {     NULL, NULL,               NULL, 0, NULL }
};


static int help_menu_shortcuts (void);

static int help_menu_about (void);


static MENU help_menu [] =
{
    { "&Shortcuts...", help_menu_shortcuts, NULL, 0, NULL },
    {              "",                NULL, NULL, 0, NULL },
    {     "&About...",     help_menu_about, NULL, 0, NULL },
    {            NULL,                NULL, NULL, 0, NULL }
};


static MENU menu_bar [] =
{ 
    {    "&File", NULL,    file_menu,          0, NULL },
    { "&Machine", NULL, machine_menu,          0, NULL },
    { "&NetPlay", NULL,         NULL, D_DISABLED, NULL },
    { "&Options", NULL, options_menu,          0, NULL },
    {    "&Help", NULL,    help_menu,          0, NULL },
    {       NULL, NULL,         NULL,          0, NULL }
};
