

static int netplay_handler (int, DIALOG *, int);


static DIALOG main_dialog [] =
{
    { netplay_handler,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,      NIL, NIL, NIL },
    {     d_menu_proc, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, top_menu, NIL, NIL },
    {             NIL,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,      NIL, NIL, NIL }
};


static DIALOG machine_state_save_dialog [] =
{
   {          sl_frame,   0,  0, 156, 84, 0, 0,   0,      0,            0, 0,   NIL, "Save State", NIL },
   {     d_button_proc, 136,  4,  16, 12, 0, 0,   0, D_EXIT,            0, 0,   "X",          NIL, NIL },
   {           sl_text,   9, 36,   0,  0, 0, 0,   0,      0,            0, 0,   NIL,    "&Title:", NIL },
   { d_shadow_box_proc,  48, 32,  96, 16, 0, 0,   0,      0,            0, 0,   NIL,          NIL, NIL },
   {       d_edit_proc,  50, 36,  92, 12, 0, 0, 't',      0,            0, 0,   NIL,          NIL, NIL },
   {     d_button_proc, 112, 56,  32, 16, 0, 0, 'o', D_EXIT,            0, 0, "&OK",          NIL, NIL },
   {               NIL,   0,  0,   0,  0, 0, 0,   0,      0, SL_FRAME_END, 0,   NIL,          NIL, NIL }
};


static DIALOG machine_patches_add_dialog [] =
{
   {          sl_frame,   0,  0, 156, 100, 0, 0,   0,      0,            0, 0,   NIL, "Add Patch", NIL },
   {     d_button_proc, 136,  4,  16,  12, 0, 0,   0, D_EXIT,            0, 0,   "X",         NIL, NIL },
   {           sl_text,   9, 36,   0,   0, 0, 0, 't',      0,            0, 0,   NIL,   "&Title:", NIL },
   { d_shadow_box_proc,  48, 32,  96,  16, 0, 0,   0,      0,            0, 0,   NIL,         NIL, NIL },
   {       d_edit_proc,  50, 36,  92,  12, 0, 0,   0,      0,            0, 0,   NIL,         NIL, NIL },
   {           sl_text,  14, 56,   0,   0, 0, 0, 'c',      0,            0, 0,   NIL,    "&Code:", NIL },
   { d_shadow_box_proc,  48, 52,  61,  16, 0, 0,   0,      0,            0, 0,   NIL,         NIL, NIL },
   {       d_edit_proc,  50, 56,  57,  12, 0, 0,   0,      0,            0, 0,   NIL,         NIL, NIL },
   {     d_button_proc, 112, 76,  32,  16, 0, 0, 'o', D_EXIT,            0, 0, "&OK",         NIL, NIL },
   {               NIL,   0,  0,   0,   0, 0, 0,   0,      0, SL_FRAME_END, 0,   NIL,         NIL, NIL }
};


static int machine_patches_dialog_list (DIALOG *);

static int machine_patches_dialog_add (DIALOG *);

static int machine_patches_dialog_remove (DIALOG *);

static int machine_patches_dialog_enabled (DIALOG *);


static char * machine_patches_dialog_list_filler (int, int *);


static DIALOG machine_patches_dialog [] =
{
   {      sl_frame,   0,   0, 226, 160, 0, 0,   0,      0,            0, 0,                                NIL,                      "Patches",                         NIL },
   { d_button_proc, 206,   4,  16,  12, 0, 0,   0, D_EXIT,            0, 0,                                "X",                            NIL,                         NIL },
   {    sl_listbox,   9,  29, 207,  98, 0, 0,   0,      0,            0, 0, machine_patches_dialog_list_filler,                            NIL, machine_patches_dialog_list },
   {     sl_button,   8, 136,  32,  16, 0, 0, 'a',      0,            0, 0,                             "&Add",     machine_patches_dialog_add,                         NIL },
   {     sl_button,  48, 136,  53,  16, 0, 0, 'r',      0,            0, 0,                          "&Remove",  machine_patches_dialog_remove,                         NIL },
   {   sl_checkbox, 121, 140,  64,   8, 0, 0, 'e',      0,            0, 0,                         "&Enabled", machine_patches_dialog_enabled,                         NIL },
   { d_button_proc, 185, 136,  32,  16, 0, 0, 's', D_EXIT,            0, 0,                            "&Save",                            NIL,                         NIL },
   {           NIL,   0,   0,   0,   0, 0, 0,   0,      0, SL_FRAME_END, 0,                                NIL,                            NIL,                         NIL }
};


static DIALOG netplay_client_connect_dialog [] =
{
   {          sl_frame,   0,  0, 156, 84, 0, 0,   0,      0,            0, 0,   NIL,  "Connect", NIL },
   {     d_button_proc, 136,  4,  16, 12, 0, 0,   0, D_EXIT,            0, 0,   "X",        NIL, NIL },
   {           sl_text,   9, 36,   0,  0, 0, 0, 's',      0,            0, 0,   NIL, "&Server:", NIL },
   { d_shadow_box_proc,  48, 32,  96, 16, 0, 0,   0,      0,            0, 0,   NIL,        NIL, NIL },
   {       d_edit_proc,  50, 36,  92, 12, 0, 0,   0,      0,            0, 0,   NIL,        NIL, NIL },
   {     d_button_proc, 112, 56,  32, 16, 0, 0, 'o', D_EXIT,            0, 0, "&OK",        NIL, NIL },
   {               NIL,   0,  0,   0,  0, 0, 0,   0,      0, SL_FRAME_END, 0,   NIL,        NIL, NIL }
};


static DIALOG help_shortcuts_dialog [] =
{
    {      sl_frame,   0,   0, 134, 165, 0, 0, 0,      0,            0, 0, NIL,               "Shortcuts", NIL },
    { d_button_proc, 114,   4,  16,  12, 0, 0, 0, D_EXIT,            0, 0, "X",                       NIL, NIL },
    {       sl_text,   9,  29,   0,   0, 0, 0, 0,      0,            0, 0, NIL,     "F1  - Save Snapshot", NIL },
    {       sl_text,   9,  41,   0,   0, 0, 0, 0,      0,            0, 0, NIL,     "F2  - Toggle Status", NIL },
    {       sl_text,   9,  53,   0,   0, 0, 0, 0,      0,            0, 0, NIL,        "F3  - Save State", NIL },
    {       sl_text,   9,  65,   0,   0, 0, 0, 0,      0,            0, 0, NIL,     "F4  - Restore State", NIL },
    {       sl_text,   9,  81,   0,   0, 0, 0, 0,      0,            0, 0, NIL,    "F5  - Flip Mirroring", NIL },
    {       sl_text,   9,  93,   0,   0, 0, 0, 0,      0,            0, 0, NIL,     "F6  - Toggle Zapper", NIL },
    {       sl_text,   9, 105,   0,   0, 0, 0, 0,      0,            0, 0, NIL,    "F7  - Toggle Sprites", NIL },
    {       sl_text,   9, 117,   0,   0, 0, 0, 0,      0,            0, 0, NIL, "F8  - Toggle Background", NIL },
    {       sl_text,   9, 133,   0,   0, 0, 0, 0,      0,            0, 0, NIL, "0-9 - Select State Slot", NIL },
    {       sl_text,   9, 149,   0,   0, 0, 0, 0,      0,            0, 0, NIL,    "ESC - Enter/Exit GUI", NIL },
    {           NIL,   0,   0,   0,   0, 0, 0, 0,      0, SL_FRAME_END, 0, NIL,                       NIL, NIL }
};


static DIALOG help_about_dialog [] =
{
    {      sl_frame,   0,   0, 120, 134, 0, 0, 0,      0,            0, 0, NIL,              "About", NIL },
    { d_button_proc, 100,   4,  16,  12, 0, 0, 0, D_EXIT,            0, 0, "X",                  NIL, NIL },
    {       sl_text,   9,  29,   0,   0, 0, 0, 0,      0,            0, 0, NIL,        "Developers:", NIL },
    {       sl_text,  17,  41,   0,   0, 0, 0, 0,      0,            0, 0, NIL,     "Randy McDowell", NIL },
    {       sl_text,  17,  53,   0,   0, 0, 0, 0,      0,            0, 0, NIL,          "Ian Smith", NIL },
    {       sl_text,  17,  65,   0,   0, 0, 0, 0,      0,            0, 0, NIL,    "Charles Bilyue'", NIL },
    {       sl_text,  17,  77,   0,   0, 0, 0, 0,      0,            0, 0, NIL, "Jonathan Gevaryahu", NIL },
    {       sl_text,   9,  93,   0,   0, 0, 0, 0,      0,            0, 0, NIL,      "Contributors:", NIL },
    {       sl_text,  17, 105,   0,   0, 0, 0, 0,      0,            0, 0, NIL,      "Chris Sheehan", NIL },
    {       sl_text,  17, 117,   0,   0, 0, 0, 0,      0,            0, 0, NIL,   "Amit Vainsencher", NIL },
    {           NIL,   0,   0,   0,   0, 0, 0, 0,      0, SL_FRAME_END, 0, NIL,                  NIL, NIL }
};
