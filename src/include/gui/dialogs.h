

static int netplay_handler (int, DIALOG *, int);


static DIALOG main_dialog [] =
{
    { netplay_handler,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,     NULL, NULL, NULL },
    {     d_menu_proc, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, top_menu, NULL, NULL },
    {            NULL,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,     NULL, NULL, NULL }
};


static DIALOG machine_state_save_dialog [] =
{
   {          sl_frame,   0,  0, 156, 84, 0, 0,   0,      0,            0, 0,  NULL, "Save State", NULL },
   {     d_button_proc, 136,  4,  16, 12, 0, 0,   0, D_EXIT,            0, 0,   "X",         NULL, NULL },
   {           sl_text,   9, 36,   0,  0, 0, 0,   0,      0,            0, 0,  NULL,    "&Title:", NULL },
   { d_shadow_box_proc,  48, 32,  96, 16, 0, 0,   0,      0,            0, 0,  NULL,         NULL, NULL },
   {       d_edit_proc,  50, 36,  92, 12, 0, 0, 't',      0,            0, 0,  NULL,         NULL, NULL },
   {     d_button_proc, 112, 56,  32, 16, 0, 0, 'o', D_EXIT,            0, 0, "&OK",         NULL, NULL },
   {              NULL,   0,  0,   0,  0, 0, 0,   0,      0, SL_FRAME_END, 0,  NULL,         NULL, NULL }
};


static DIALOG machine_patches_add_dialog [] =
{
   {          sl_frame,   0,  0, 156, 100, 0, 0,   0,      0,            0, 0,  NULL, "Add Patch", NULL },
   {     d_button_proc, 136,  4,  16,  12, 0, 0,   0, D_EXIT,            0, 0,   "X",        NULL, NULL },
   {           sl_text,   9, 36,   0,   0, 0, 0, 't',      0,            0, 0,  NULL,   "&Title:", NULL },
   { d_shadow_box_proc,  48, 32,  96,  16, 0, 0,   0,      0,            0, 0,  NULL,        NULL, NULL },
   {       d_edit_proc,  50, 36,  92,  12, 0, 0,   0,      0,            0, 0,  NULL,        NULL, NULL },
   {           sl_text,  14, 56,   0,   0, 0, 0, 'c',      0,            0, 0,  NULL,    "&Code:", NULL },
   { d_shadow_box_proc,  48, 52,  61,  16, 0, 0,   0,      0,            0, 0,  NULL,        NULL, NULL },
   {       d_edit_proc,  50, 56,  57,  12, 0, 0,   0,      0,            0, 0,  NULL,        NULL, NULL },
   {     d_button_proc, 112, 76,  32,  16, 0, 0, 'o', D_EXIT,            0, 0, "&OK",        NULL, NULL },
   {              NULL,   0,  0,   0,   0, 0, 0,   0,      0, SL_FRAME_END, 0,  NULL,        NULL, NULL }
};


static int machine_patches_dialog_list (DIALOG *);

static int machine_patches_dialog_add (DIALOG *);

static int machine_patches_dialog_remove (DIALOG *);

static int machine_patches_dialog_enabled (DIALOG *);


static char * machine_patches_dialog_list_filler (int, int *);


static DIALOG machine_patches_dialog [] =
{
   {      sl_frame,   0,   0, 226, 160, 0, 0,   0,      0,            0, 0,                               NULL,                      "Patches",                        NULL },
   { d_button_proc, 206,   4,  16,  12, 0, 0,   0, D_EXIT,            0, 0,                                "X",                           NULL,                        NULL },
   {    sl_listbox,   9,  29, 207,  98, 0, 0,   0,      0,            0, 0, machine_patches_dialog_list_filler,                           NULL, machine_patches_dialog_list },
   {     sl_button,   8, 136,  32,  16, 0, 0, 'a',      0,            0, 0,                             "&Add",     machine_patches_dialog_add,                        NULL },
   {     sl_button,  48, 136,  53,  16, 0, 0, 'r',      0,            0, 0,                          "&Remove",  machine_patches_dialog_remove,                        NULL },
   {   sl_checkbox, 121, 140,  64,   8, 0, 0, 'e',      0,            0, 0,                         "&Enabled", machine_patches_dialog_enabled,                        NULL },
   { d_button_proc, 185, 136,  32,  16, 0, 0, 's', D_EXIT,            0, 0,                            "&Save",                           NULL,                        NULL },
   {          NULL,   0,   0,   0,   0, 0, 0,   0,      0, SL_FRAME_END, 0,                               NULL,                           NULL,                        NULL }
};


static DIALOG netplay_client_connect_dialog [] =
{
   {          sl_frame,   0,  0, 156, 84, 0, 0,   0,      0,            0, 0,  NULL,  "Connect", NULL },
   {     d_button_proc, 136,  4,  16, 12, 0, 0,   0, D_EXIT,            0, 0,   "X",       NULL, NULL },
   {           sl_text,   9, 36,   0,  0, 0, 0, 's',      0,            0, 0,  NULL, "&Server:", NULL },
   { d_shadow_box_proc,  48, 32,  96, 16, 0, 0,   0,      0,            0, 0,  NULL,       NULL, NULL },
   {       d_edit_proc,  50, 36,  92, 12, 0, 0,   0,      0,            0, 0,  NULL,       NULL, NULL },
   {     d_button_proc, 112, 56,  32, 16, 0, 0, 'o', D_EXIT,            0, 0, "&OK",       NULL, NULL },
   {              NULL,   0,  0,   0,  0, 0, 0,   0,      0, SL_FRAME_END, 0,  NULL,       NULL, NULL }
};


static DIALOG help_shortcuts_dialog [] =
{
    {      sl_frame,   0,   0, 134, 165, 0, 0, 0,      0,            0, 0, NULL,               "Shortcuts", NULL },
    { d_button_proc, 114,   4,  16,  12, 0, 0, 0, D_EXIT,            0, 0,  "X",                      NULL, NULL },
    {       sl_text,   9,  29,   0,   0, 0, 0, 0,      0,            0, 0, NULL,     "F1  - Save Snapshot", NULL },
    {       sl_text,   9,  41,   0,   0, 0, 0, 0,      0,            0, 0, NULL,     "F2  - Toggle Status", NULL },
    {       sl_text,   9,  53,   0,   0, 0, 0, 0,      0,            0, 0, NULL,        "F3  - Save State", NULL },
    {       sl_text,   9,  65,   0,   0, 0, 0, 0,      0,            0, 0, NULL,     "F4  - Restore State", NULL },
    {       sl_text,   9,  81,   0,   0, 0, 0, 0,      0,            0, 0, NULL,    "F5  - Flip Mirroring", NULL },
    {       sl_text,   9,  93,   0,   0, 0, 0, 0,      0,            0, 0, NULL,     "F6  - Toggle Zapper", NULL },
    {       sl_text,   9, 105,   0,   0, 0, 0, 0,      0,            0, 0, NULL,    "F7  - Toggle Sprites", NULL },
    {       sl_text,   9, 117,   0,   0, 0, 0, 0,      0,            0, 0, NULL, "F8  - Toggle Background", NULL },
    {       sl_text,   9, 133,   0,   0, 0, 0, 0,      0,            0, 0, NULL, "0-9 - Select State Slot", NULL },
    {       sl_text,   9, 149,   0,   0, 0, 0, 0,      0,            0, 0, NULL,    "ESC - Enter/Exit GUI", NULL },
    {          NULL,   0,   0,   0,   0, 0, 0, 0,      0, SL_FRAME_END, 0, NULL,                      NULL, NULL }
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
