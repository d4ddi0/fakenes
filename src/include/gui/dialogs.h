

static int netplay_handler (int, DIALOG *, int);


static DIALOG main_dialog [] =
{
    { netplay_handler,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,      NIL, NIL, NIL },
    {     d_menu_proc, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, top_menu, NIL, NIL },
    {             NIL,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,      NIL, NIL, NIL }
};


static DIALOG main_state_save_dialog [] =
{
    {          sl_frame,   0,  0, 156, 84, 0, 0,   0,      0,            0, 0,   NIL, "Save State", NIL },
    {     d_button_proc, 136,  4,  16, 12, 0, 0,   0, D_EXIT,            0, 0,   "X",          NIL, NIL },
    {           sl_text,   9, 36,   0,  0, 0, 0,   0,      0,            0, 0,   NIL,    "&Title:", NIL },
    { d_shadow_box_proc,  48, 32,  96, 16, 0, 0,   0,      0,            0, 0,   NIL,          NIL, NIL },
    {       d_edit_proc,  50, 36,  92, 12, 0, 0, 't',      0,            0, 0,   NIL,          NIL, NIL },
    {     d_button_proc, 112, 56,  32, 16, 0, 0, 'o', D_EXIT,            0, 0, "&OK",          NIL, NIL },
    {               NIL,   0,  0,   0,  0, 0, 0,   0,      0, SL_FRAME_END, 0,   NIL,          NIL, NIL }
};  


static DIALOG main_replay_record_start_dialog [] =
{
    {          sl_frame,   0,  0, 156, 84, 0, 0,   0,      0,            0, 0,   NIL, "Record Replay", NIL },
    {     d_button_proc, 136,  4,  16, 12, 0, 0,   0, D_EXIT,            0, 0,   "X",             NIL, NIL },
    {           sl_text,   9, 36,   0,  0, 0, 0,   0,      0,            0, 0,   NIL,       "&Title:", NIL },
    { d_shadow_box_proc,  48, 32,  96, 16, 0, 0,   0,      0,            0, 0,   NIL,             NIL, NIL },
    {       d_edit_proc,  50, 36,  92, 12, 0, 0, 't',      0,            0, 0,   NIL,             NIL, NIL },
    {     d_button_proc, 112, 56,  32, 16, 0, 0, 'o', D_EXIT,            0, 0, "&OK",             NIL, NIL },
    {               NIL,   0,  0,   0,  0, 0, 0,   0,      0, SL_FRAME_END, 0,   NIL,             NIL, NIL }
};  


static DIALOG main_messages_dialog [] =
{
    {      sl_frame,   0,  0, 276, 136, 0, 0, 0,      0,            0, 0, NIL, "Messages", NIL },
    { d_button_proc, 256,  4,  16,  12, 0, 0, 0, D_EXIT,            0, 0, "X",        NIL, NIL },
    {     sl_viewer,   9, 29, 257,  98, 0, 0, 0,      0,            0, 0, NIL,        NIL, NIL },
    {           NIL,   0,  0,   0,   0, 0, 0, 0,      0, SL_FRAME_END, 0, NIL,        NIL, NIL }
};


static int options_input_dialog_player_select (DIALOG *);


static int options_input_dialog_device_select (DIALOG *);


static int options_input_dialog_set_buttons (DIALOG *);


static DIALOG options_input_dialog [] =
{
    {      sl_frame,   0,  0, 194, 157, 0, 0,   0,      0,            0,                          0,            NIL,                            "Input", NIL },
    { d_button_proc, 174,  4,  16,  12, 0, 0,   0, D_EXIT,            0,                          0,            "X",                                NIL, NIL },
    {       sl_text,   9, 29,   0,   0, 0, 0,   0,      0,            0,                          0,            NIL,                          "Player:", NIL },
    {   sl_radiobox,  52, 29,  24,   6, 0, 0, '1',      0,            1,                          1,           "&1", options_input_dialog_player_select, NIL },
    {   sl_radiobox,  76, 29,  24,   6, 0, 0, '2',      0,            1,                          2,           "&2", options_input_dialog_player_select, NIL },
    {   sl_radiobox, 100, 29,  24,   6, 0, 0, '3',      0,            1,                          3,           "&3", options_input_dialog_player_select, NIL },
    {   sl_radiobox, 124, 29,  24,   6, 0, 0, '4',      0,            1,                          4,           "&4", options_input_dialog_player_select, NIL },
    {       sl_text,   9, 45,   0,   0, 0, 0,   0,      0,            0,                          0,            NIL,                          "Device:", NIL },
    {   sl_radiobox,  52, 45,  64,   6, 0, 0, 'k',      0,            2,    INPUT_DEVICE_KEYBOARD_1,   "&Key Set 1", options_input_dialog_device_select, NIL },
    {   sl_radiobox,  52, 56,  64,   6, 0, 0, 'e',      0,            2,    INPUT_DEVICE_KEYBOARD_2,   "K&ey Set 2", options_input_dialog_device_select, NIL },
    {   sl_radiobox, 118, 45,  64,   6, 0, 0, 'p',      0,            2,    INPUT_DEVICE_JOYSTICK_1, "Stick/&Pad 1", options_input_dialog_device_select, NIL },
    {   sl_radiobox, 118, 56,  64,   6, 0, 0, 't',      0,            2,    INPUT_DEVICE_JOYSTICK_2, "S&tick/Pad 2", options_input_dialog_device_select, NIL },
    {       sl_text,   9, 72,   0,   0, 0, 0,   0,      0,            0,                          0,            NIL,                     "Set Buttons:", NIL },
    {     sl_button,  24, 88,  40,  12, 0, 0, 'u',      0,            0,     INPUT_DEVICE_BUTTON_UP,          "&Up",   options_input_dialog_set_buttons, NIL },
    {     sl_button,  24, 104, 40,  12, 0, 0, 'd',      0,            0,   INPUT_DEVICE_BUTTON_DOWN,        "&Down",   options_input_dialog_set_buttons, NIL },
    {     sl_button,  24, 120, 40,  12, 0, 0, 'l',      0,            0,   INPUT_DEVICE_BUTTON_LEFT,        "&Left",   options_input_dialog_set_buttons, NIL },
    {     sl_button,  24, 136, 40,  12, 0, 0, 'r',      0,            0,  INPUT_DEVICE_BUTTON_RIGHT,       "&Right",   options_input_dialog_set_buttons, NIL },
    {     sl_button,  80, 88,  48,  12, 0, 0, 's',      0,            0,  INPUT_DEVICE_BUTTON_START,       "&Start",   options_input_dialog_set_buttons, NIL },
    {     sl_button,  80, 104, 48,  12, 0, 0, 'c',      0,            0, INPUT_DEVICE_BUTTON_SELECT,      "Sele&ct",   options_input_dialog_set_buttons, NIL },
    {     sl_button,  80, 120, 32,  12, 0, 0, 'a',      0,            0,      INPUT_DEVICE_BUTTON_A,           "&A",   options_input_dialog_set_buttons, NIL },
    {     sl_button,  80, 136, 32,  12, 0, 0, 'b',      0,            0,      INPUT_DEVICE_BUTTON_B,           "&B",   options_input_dialog_set_buttons, NIL },
    {           NIL,   0,   0,  0,   0, 0, 0,   0,      0, SL_FRAME_END,                          0,            NIL,                                NIL, NIL }
};


static DIALOG options_patches_add_dialog [] =
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


static int options_patches_dialog_list (DIALOG *);

static int options_patches_dialog_add (DIALOG *);

static int options_patches_dialog_remove (DIALOG *);

static int options_patches_dialog_enabled (DIALOG *);


static char * options_patches_dialog_list_filler (int, int *);


static DIALOG options_patches_dialog [] =
{
    {      sl_frame,   0,   0, 226, 160, 0, 0,   0,      0,            0, 0,                                NIL,                      "Patches",                         NIL },
    { d_button_proc, 206,   4,  16,  12, 0, 0,   0, D_EXIT,            0, 0,                                "X",                            NIL,                         NIL },
    {    sl_listbox,   9,  29, 207,  98, 0, 0,   0,      0,            0, 0, options_patches_dialog_list_filler,                            NIL, options_patches_dialog_list },
    {     sl_button,   8, 136,  32,  16, 0, 0, 'a',      0,            0, 0,                             "&Add",     options_patches_dialog_add,                         NIL },
    {     sl_button,  48, 136,  53,  16, 0, 0, 'r',      0,            0, 0,                          "&Remove",  options_patches_dialog_remove,                         NIL },
    {   sl_checkbox, 121, 140,  64,   8, 0, 0, 'e',      0,            0, 0,                         "&Enabled", options_patches_dialog_enabled,                         NIL },
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
    {      sl_frame,   0,   0, 268, 165, 0, 0, 0,      0,            0, 0, NIL,               "Shortcuts", NIL },
    { d_button_proc, 248,   4,  16,  12, 0, 0, 0, D_EXIT,            0, 0, "X",                       NIL, NIL },
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
    {       sl_text, 143,  29,   0,   0, 0, 0, 0,      0,            0, 0, NIL, "~   - Fast Forward Mode", NIL },
    {       sl_text, 143,  41,   0,   0, 0, 0, 0,      0,            0, 0, NIL,  "F9  - Toggle Slow Mode", NIL },
    {       sl_text, 143,  57,   0,   0, 0, 0, 0,      0,            0, 0, NIL,    "F10 - Darken Palette", NIL },
    {       sl_text, 143,  69,   0,   0, 0, 0, 0,      0,            0, 0, NIL,  "F11 - Brighten Palette", NIL },
    {       sl_text, 143,  85,   0,   0, 0, 0, 0,      0,            0, 0, NIL, "F12 - Record Start/Stop", NIL },
    {       sl_text, 143,  99,   0,   0, 0, 0, 0,      0,            0, 0, NIL,     "BACKSPACE - Message", NIL },
    {       sl_text, 143, 149,   0,   0, 0, 0, 0,      0,            0, 0, NIL, "No other keys reserved.", NIL },
    {           NIL,   0,   0,   0,   0, 0, 0, 0,      0, SL_FRAME_END, 0, NIL,                       NIL, NIL }
};


static DIALOG help_about_dialog [] =
{
    {      sl_frame,   0,   0, 240, 180, 0, 0, 0,      0,            0, 0, NIL,              "About", NIL },
    { d_button_proc, 220,   4,  16,  12, 0, 0, 0, D_EXIT,            0, 0, "X",                  NIL, NIL },
    {       sl_text,   9,  29,   0,   0, 0, 0, 0,      0,            0, 0, NIL,        "Developers:", NIL },
    {       sl_text,  17,  41,   0,   0, 0, 0, 0,      0,            0, 0, NIL,     "Randy McDowell", NIL },
    {       sl_text,  17,  53,   0,   0, 0, 0, 0,      0,            0, 0, NIL,    "Charles Bilyue'", NIL },
    {       sl_text,  17,  65,   0,   0, 0, 0, 0,      0,            0, 0, NIL, "Jonathan Gevaryahu", NIL },
    {       sl_text,  17,  77,   0,   0, 0, 0, 0,      0,            0, 0, NIL,     "Chris Robinson", NIL },
    {       sl_text,   9,  93,   0,   0, 0, 0, 0,      0,            0, 0, NIL,      "Contributors:", NIL },
    {       sl_text,  17, 105,   0,   0, 0, 0, 0,      0,            0, 0, NIL,      "Chris Sheehan", NIL },
    {       sl_text,  17, 117,   0,   0, 0, 0, 0,      0,            0, 0, NIL,   "Amit Vainsencher", NIL },
    {       sl_text,   9, 133,   0,   0, 0, 0, 0,      0,            0, 0, NIL,          "Remnants:", NIL },
    {       sl_text,  17, 145,   0,   0, 0, 0, 0,      0,            0, 0, NIL,          "Ian Smith", NIL },
    {       sl_text,  27, 161,   0,   0, 0, 0, 0,      0,            0, 0, NIL,     "Thank you all!", NIL },
    {       sl_text, 129,  29,   0,   0, 0, 0, 0,      0,            0, 0, NIL, "Special thanks to:", NIL },
    {       sl_text, 137,  41,   0,   0, 0, 0, 0,      0,            0, 0, NIL,           "xodnizel", NIL },
    {       sl_text, 137,  53,   0,   0, 0, 0, 0,      0,            0, 0, NIL,      "Matthew Conte", NIL },
    {       sl_text, 137,  65,   0,   0, 0, 0, 0,      0,            0, 0, NIL,    "TAKEDA, toshiya", NIL },
    {       sl_text, 137,  77,   0,   0, 0, 0, 0,      0,            0, 0, NIL,      "YANO, takashi", NIL },
    {       sl_text, 137,  89,   0,   0, 0, 0, 0,      0,            0, 0, NIL,             "kode54", NIL },
    {       sl_text, 137, 101,   0,   0, 0, 0, 0,      0,            0, 0, NIL,          "pagefault", NIL },
    {       sl_text, 137, 113,   0,   0, 0, 0, 0,      0,            0, 0, NIL,          "piinyouri", NIL },
    {       sl_text, 137, 125,   0,   0, 0, 0, 0,      0,            0, 0, NIL,          "aprentice", NIL },
    {       sl_text, 137, 137,   0,   0, 0, 0, 0,      0,            0, 0, NIL,            "Astxist", NIL },
    {       sl_text, 137, 149,   0,   0, 0, 0, 0,      0,            0, 0, NIL,          "Tim Inman", NIL },
    {       sl_text, 137, 165,   0,   0, 0, 0, 0,      0,            0, 0, NIL, "... and many more!", NIL },
    {           NIL,   0,   0,   0,   0, 0, 0, 0,      0, SL_FRAME_END, 0, NIL,                  NIL, NIL }
};
