

static int file_menu_load_rom (void);

static int file_menu_snapshot (void);

static int file_menu_exit (void);


static MENU file_menu [6] =
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


static MENU machine_type_menu [4] =
{
    { "&NTSC", machine_type_menu_ntsc, NULL, 0, NULL },
    {      "",                   NULL, NULL, 0, NULL },
    {  "&PAL",  machine_type_menu_pal, NULL, 0, NULL },
    {    NULL,                   NULL, NULL, 0, NULL }
};


static int machine_state_menu_select (void);

static int machine_state_menu_save (void);

static int machine_state_menu_restore (void);


static MENU machine_state_menu [6] =
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


static MENU machine_menu [8] =
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


static MENU options_audio_filter_low_pass_menu [6] =
{
    {   "&Simple",   options_audio_filter_low_pass_menu_simple, NULL, 0, NULL },
    {          "",                                        NULL, NULL, 0, NULL },
    { "&Weighted", options_audio_filter_low_pass_menu_weighted, NULL, 0, NULL },
    {          "",                                        NULL, NULL, 0, NULL },
    {  "&Dynamic",  options_audio_filter_low_pass_menu_dynamic, NULL, 0, NULL },
    {        NULL,                                        NULL, NULL, 0, NULL }
};


static int options_audio_filter_menu_none (void);


static MENU options_audio_filter_menu [4] =
{     
    {      "&None", options_audio_filter_menu_none,                               NULL, 0, NULL },
    {           "",                           NULL,                               NULL, 0, NULL },
    {  "&Low Pass",                           NULL, options_audio_filter_low_pass_menu, 0, NULL },
    {         NULL,                           NULL,                               NULL, 0, NULL }
};


static MENU options_audio_menu [2] =
{
    { "&Filter", NULL, options_audio_filter_menu, 0, NULL },
    {      NULL, NULL,                      NULL, 0, NULL }
};


static int options_video_palette_menu_default (void);

static int options_video_palette_menu_gnuboy (void);

static int options_video_palette_menu_nester (void);


static MENU options_video_palette_menu [6] =
{
    { "&Default", options_video_palette_menu_default, NULL, 0, NULL },
    {         "",                               NULL, NULL, 0, NULL },
    {  "&gnuboy",  options_video_palette_menu_gnuboy, NULL, 0, NULL },
    {         "",                               NULL, NULL, 0, NULL },
    {  "&NESter",  options_video_palette_menu_nester, NULL, 0, NULL },
    {       NULL,                               NULL, NULL, 0, NULL }
};


static int options_video_menu_vsync (void);


static MENU options_video_menu [4] =
{
    {   "&VSync", options_video_menu_vsync,                       NULL, 0, NULL },
    {         "",                     NULL,                       NULL, 0, NULL },
    { "&Palette",                     NULL, options_video_palette_menu, 0, NULL },
    {       NULL,                     NULL,                       NULL, 0, NULL }
};


static MENU options_menu [6] =
{
    { "&Audio", NULL, options_audio_menu,          0, NULL },
    {       "", NULL,               NULL,          0, NULL },
    { "&Video", NULL, options_video_menu,          0, NULL },
    {       "", NULL,               NULL,          0, NULL },
    { "&Input", NULL,               NULL, D_DISABLED, NULL },
    {     NULL, NULL,               NULL,          0, NULL }
};


static int help_menu_shortcuts (void);

static int help_menu_about (void);


static MENU help_menu [4] =
{
    { "&Shortcuts...", help_menu_shortcuts, NULL, 0, NULL },
    {              "",                NULL, NULL, 0, NULL },
    {     "&About...",     help_menu_about, NULL, 0, NULL },
    {            NULL,                NULL, NULL, 0, NULL }
};


static MENU menu_bar [6] =
{ 
    {    "&File", NULL,    file_menu,          0, NULL },
    { "&Machine", NULL, machine_menu,          0, NULL },
    { "&NetPlay", NULL,         NULL, D_DISABLED, NULL },
    { "&Options", NULL, options_menu,          0, NULL },
    {    "&Help", NULL,    help_menu,          0, NULL },
    {       NULL, NULL,         NULL,          0, NULL }
};
