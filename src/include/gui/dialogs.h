

static int netplay_handler (int, DIALOG *, int);


static DIALOG main_dialog [] =
{
    { netplay_handler,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,     NULL, NULL, NULL },
    {     d_menu_proc, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, top_menu, NULL, NULL },
    {            NULL,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,     NULL, NULL, NULL }
};


static DIALOG netplay_client_connect_dialog [] =
{
   {          sl_frame,   0,  0, 156, 84, 0, 0, 0,      0,            0, 0,  NULL,  "Connect", NULL },
   {     d_button_proc, 136,  4,  16, 12, 0, 0, 0, D_EXIT,            0, 0,   "X",       NULL, NULL },
   {           sl_text,   9, 36,   0,  0, 0, 0, 0,      0,            0, 0,  NULL,  "Server:", NULL },
   { d_shadow_box_proc,  48, 32,  96, 16, 0, 0, 0,      0,            0, 0,  NULL,       NULL, NULL },
   {       d_edit_proc,  50, 36,  92, 12, 0, 0, 0,      0,            0, 0,  NULL,       NULL, NULL },
   {     d_button_proc, 112, 56,  32, 16, 0, 0, 0, D_EXIT,            0, 0, "&OK",       NULL, NULL },
   {              NULL,   0,  0,   0,  0, 0, 0, 0,      0, SL_FRAME_END, 0,  NULL,       NULL, NULL }
};


static DIALOG help_shortcuts_dialog [] =
{
    {      sl_frame,   0,   0, 134, 149, 0, 0, 0,          0,            0, 0, NULL,               "Shortcuts", NULL },
    { d_button_proc, 114,   4,  16,  12, 0, 0, 0,     D_EXIT,            0, 0,  "X",                      NULL, NULL },
    {       sl_text,   9,  29,   0,   0, 0, 0, 0,          0,            0, 0, NULL,     "F1  - Save Snapshot", NULL },
    {       sl_text,   9,  41,   0,   0, 0, 0, 0,          0,            0, 0, NULL,     "F2  - Toggle Status", NULL },
    {       sl_text,   9,  53,   0,   0, 0, 0, 0, D_DISABLED,            0, 0, NULL,        "F3  - Save State", NULL },
    {       sl_text,   9,  65,   0,   0, 0, 0, 0, D_DISABLED,            0, 0, NULL,     "F4  - Restore State", NULL },
    {       sl_text,   9,  81,   0,   0, 0, 0, 0,          0,            0, 0, NULL,    "F5  - Flip Mirroring", NULL },
    {       sl_text,   9,  93,   0,   0, 0, 0, 0,          0,            0, 0, NULL,     "F6  - Toggle Zapper", NULL },
    {       sl_text,   9, 105,   0,   0, 0, 0, 0,          0,            0, 0, NULL,    "F7  - Toggle Sprites", NULL },
    {       sl_text,   9, 117,   0,   0, 0, 0, 0,          0,            0, 0, NULL, "F8  - Toggle Background", NULL },
    {       sl_text,   9, 133,   0,   0, 0, 0, 0,          0,            0, 0, NULL,    "ESC - Enter/Exit GUI", NULL },
    {          NULL,   0,   0,   0,   0, 0, 0, 0,          0, SL_FRAME_END, 0, NULL,                      NULL, NULL }
};


static DIALOG help_about_dialog [] =
{
    {      sl_frame,   0,   0, 120, 134, 0, 0, 0,      0,            0, 0, NULL,              "About", NULL },
    { d_button_proc, 100,   4,  16,  12, 0, 0, 0, D_EXIT,            0, 0,  "X",                 NULL, NULL },
    {       sl_text,   9,  29,   0,   0, 0, 0, 0,      0,            0, 0, NULL,        "Developers:", NULL },
    {       sl_text,  17,  41,   0,   0, 0, 0, 0,      0,            0, 0, NULL,     "Randy McDowell", NULL },
    {       sl_text,  17,  53,   0,   0, 0, 0, 0,      0,            0, 0, NULL,          "Ian Smith", NULL },
    {       sl_text,  17,  65,   0,   0, 0, 0, 0,      0,            0, 0, NULL,    "Charles Bilyue'", NULL },
    {       sl_text,  17,  77,   0,   0, 0, 0, 0,      0,            0, 0, NULL, "Jonathan Gevaryahu", NULL },
    {       sl_text,   9,  93,   0,   0, 0, 0, 0,      0,            0, 0, NULL,      "Contributors:", NULL },
    {       sl_text,  17, 105,   0,   0, 0, 0, 0,      0,            0, 0, NULL,      "Chris Sheehan", NULL },
    {       sl_text,  17, 117,   0,   0, 0, 0, 0,      0,            0, 0, NULL,   "Amit Vainsencher", NULL },
    {          NULL,   0,   0,   0,   0, 0, 0, 0,      0, SL_FRAME_END, 0, NULL,                 NULL, NULL }
};
