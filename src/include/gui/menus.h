

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


static MENU options_audio_filter_menu [] =
{     
    {      "&None", options_audio_filter_menu_none,                               NULL, 0, NULL },
    {           "",                           NULL,                               NULL, 0, NULL },
    {  "&Low Pass",                           NULL, options_audio_filter_low_pass_menu, 0, NULL },
    {         NULL,                           NULL,                               NULL, 0, NULL }
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


static MENU options_audio_menu [] =
{
    {   "&Filter", NULL,   options_audio_filter_menu, 0, NULL },
    {          "", NULL,                        NULL, 0, NULL },
    { "&Channels", NULL, options_audio_channels_menu, 0, NULL },
    {        NULL, NULL,                        NULL, 0, NULL }
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

static int options_video_palette_menu_gnuboy (void);

static int options_video_palette_menu_nester (void);

static int options_video_palette_menu_nesticle (void);

static int options_video_palette_menu_custom (void);


static MENU options_video_palette_menu [] =
{
    {  "&Default",  options_video_palette_menu_default, NULL, 0, NULL },
    {          "",                                NULL, NULL, 0, NULL },
    {   "&gnuboy",   options_video_palette_menu_gnuboy, NULL, 0, NULL },
    {          "",                                NULL, NULL, 0, NULL },
    {   "&NESter",   options_video_palette_menu_nester, NULL, 0, NULL },
    {          "",                                NULL, NULL, 0, NULL },
    { "&NESticle", options_video_palette_menu_nesticle, NULL, 0, NULL },
    {          "",                                NULL, NULL, 0, NULL },
    {   "&Custom",   options_video_palette_menu_custom, NULL, 0, NULL },
    {        NULL,                                NULL, NULL, 0, NULL }
};


static int options_video_menu_vsync (void);


static MENU options_video_menu [] =
{
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
