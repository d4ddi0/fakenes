/* Define helper macros. */
#define DEFINE_DIALOG(name)            static DIALOG * name = NULL
#define DEFINE_DIALOG_CALLBACK(func)   static int func (DIALOG *)
#define DIALOG_ENDCAP                  { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
#define DIALOG_FRAME_ENDCAP            { NULL, 0, 0, 0, 0, 0, 0, 0, 0, SL_FRAME_END, 0, NULL, NULL, NULL }
#define IMPORT_MENU(menu)              (MENU *) & menu
  
DEFINE_DIALOG(main_dialog);
DEFINE_DIALOG(main_state_save_dialog);
DEFINE_DIALOG(main_replay_record_start_dialog);
DEFINE_DIALOG(main_messages_dialog);
DEFINE_DIALOG(options_input_dialog);
DEFINE_DIALOG(options_patches_add_dialog);
DEFINE_DIALOG(options_patches_dialog);
DEFINE_DIALOG(netplay_client_connect_dialog);
DEFINE_DIALOG(help_shortcuts_dialog);
DEFINE_DIALOG(help_about_dialog);

static int netplay_handler (int, DIALOG *, int);

static const DIALOG main_dialog_base[] =
{
   { netplay_handler, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, NULL,                  NULL, NULL },
   { d_menu_proc,     16, 16, 0, 0, 0, 0, 0, 0, 0, 0, IMPORT_MENU(top_menu), NULL, NULL },
   DIALOG_ENDCAP
};

static const DIALOG main_state_save_dialog_base[] =
{
   { sl_frame,          0,   0,  156, 84, 0, 0, 0,   0,      0, 0, NULL,  "Save State", NULL },
   { sl_x_button,       136, 4,  16,  12, 0, 0, 0,   D_EXIT, 0, 0, "X",   NULL,         NULL },
   { sl_text,           9,   36, 0,   0,  0, 0, 0,   0,      0, 0, NULL,  "&Title:",    NULL },
   { d_shadow_box_proc, 48,  32, 96,  16, 0, 0, 0,   0,      0, 0, NULL,  NULL,         NULL },
   { d_edit_proc,       50,  36, 92,  12, 0, 0, 't', 0,      0, 0, NULL,  NULL,         NULL },
   { d_button_proc,     112, 56, 32,  16, 0, 0, 'o', D_EXIT, 0, 0, "&OK", NULL,         NULL },
   DIALOG_FRAME_ENDCAP
};

static const DIALOG main_replay_record_start_dialog_base[] =
{
   { sl_frame,          0,   0,  156, 84, 0, 0, 0,   0,      0, 0, NULL,  "Record Replay", NULL },
   { sl_x_button,       136, 4,  16,  12, 0, 0, 0,   D_EXIT, 0, 0, "X",   NULL,            NULL },
   { sl_text,           9,   36, 0,   0,  0, 0, 0,   0,      0, 0, NULL,  "&Title:",       NULL },
   { d_shadow_box_proc, 48,  32, 96,  16, 0, 0, 0,   0,      0, 0, NULL,  NULL,            NULL },
   { d_edit_proc,       50,  36, 92,  12, 0, 0, 't', 0,      0, 0, NULL,  NULL,            NULL },
   { d_button_proc,     112, 56, 32,  16, 0, 0, 'o', D_EXIT, 0, 0, "&OK", NULL,            NULL },
   DIALOG_FRAME_ENDCAP
};  

static const DIALOG main_messages_dialog_base[] =
{
   { sl_frame,    0,   0,  276, 136, 0, 0, 0, 0,      0, 0, NULL, "Messages", NULL },
   { sl_x_button, 256, 4,  16,  12,  0, 0, 0, D_EXIT, 0, 0, "X",  NULL,       NULL },
   { sl_viewer,   9,   29, 257, 98,  0, 0, 0, 0,      0, 0, NULL, NULL,       NULL },
   DIALOG_FRAME_ENDCAP
};

DEFINE_DIALOG_CALLBACK(options_input_dialog_player_select);
DEFINE_DIALOG_CALLBACK(options_input_dialog_device_select);
DEFINE_DIALOG_CALLBACK(options_input_dialog_set_buttons);

static const DIALOG options_input_dialog_base[] =
{
   { sl_frame,    0,   0,   194, 157, 0, 0, 0,   0,      0, 0,                         NULL,           "Input",                            NULL },
   { sl_x_button, 174, 4,   16,  12,  0, 0, 0,   D_EXIT, 0, 0,                         "X",            NULL,                               NULL },
   { sl_text,     9,   29,  0,   0,   0, 0, 0,   0,      0, 0,                         NULL,           "Player:",                          NULL },
   { sl_radiobox, 52,  29,  24,  6,   0, 0, '1', 0,      1, 1,                         "&1",           options_input_dialog_player_select, NULL },
   { sl_radiobox, 76,  29,  24,  6,   0, 0, '2', 0,      1, 2,                         "&2",           options_input_dialog_player_select, NULL },
   { sl_radiobox, 100, 29,  24,  6,   0, 0, '3', 0,      1, 3,                         "&3",           options_input_dialog_player_select, NULL },
   { sl_radiobox, 124, 29,  24,  6,   0, 0, '4', 0,      1, 4,                         "&4",           options_input_dialog_player_select, NULL },
   { sl_text,     9,   45,  0,   0,   0, 0, 0,   0,      0, 0,                         NULL,           "Device:",                          NULL },
   { sl_radiobox, 52,  45,  64,  6,   0, 0, 'k', 0,      2, INPUT_DEVICE_KEYBOARD_1,   "&Key Set 1",   options_input_dialog_device_select, NULL },
   { sl_radiobox, 52,  56,  64,  6,   0, 0, 'e', 0,      2, INPUT_DEVICE_KEYBOARD_2,   "K&ey Set 2",   options_input_dialog_device_select, NULL },
   { sl_radiobox, 118, 45,  64,  6,   0, 0, 'p', 0,      2, INPUT_DEVICE_JOYSTICK_1,   "Stick/&Pad 1", options_input_dialog_device_select, NULL },
   { sl_radiobox, 118, 56,  64,  6,   0, 0, 't', 0,      2, INPUT_DEVICE_JOYSTICK_2,   "S&tick/Pad 2", options_input_dialog_device_select, NULL },
   { sl_text,     9,   72,  0,   0,   0, 0, 0,   0,      0, 0,                          NULL,          "Set Buttons:",                     NULL },
   { sl_button,   24,  88,  40,  12,  0, 0, 'u', 0,      0, INPUT_DEVICE_BUTTON_UP,     "&Up",         options_input_dialog_set_buttons,   NULL },
   { sl_button,   24,  104, 40,  12,  0, 0, 'd', 0,      0, INPUT_DEVICE_BUTTON_DOWN,   "&Down",       options_input_dialog_set_buttons,   NULL },
   { sl_button,   24,  120, 40,  12,  0, 0, 'l', 0,      0, INPUT_DEVICE_BUTTON_LEFT,   "&Left",       options_input_dialog_set_buttons,   NULL },
   { sl_button,   24,  136, 40,  12,  0, 0, 'r', 0,      0, INPUT_DEVICE_BUTTON_RIGHT,  "&Right",      options_input_dialog_set_buttons,   NULL },
   { sl_button,   80,  88,  48,  12,  0, 0, 's', 0,      0, INPUT_DEVICE_BUTTON_START,  "&Start",      options_input_dialog_set_buttons,   NULL },
   { sl_button,   80,  104, 48,  12,  0, 0, 'c', 0,      0, INPUT_DEVICE_BUTTON_SELECT, "Sele&ct",     options_input_dialog_set_buttons,   NULL },
   { sl_button,   80,  120, 32,  12,  0, 0, 'a', 0,      0, INPUT_DEVICE_BUTTON_A,      "&A",          options_input_dialog_set_buttons,   NULL },
   { sl_button,   80,  136, 32,  12,  0, 0, 'b', 0,      0, INPUT_DEVICE_BUTTON_B,      "&B",          options_input_dialog_set_buttons,   NULL },
   DIALOG_FRAME_ENDCAP
};

static const DIALOG options_patches_add_dialog_base[] =
{
   { sl_frame,          0,   0,  156, 100, 0, 0, 0,   0,      0, 0, NULL,  "Add Patch", NULL },
   { sl_x_button,       136, 4,  16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",   NULL,        NULL },
   { sl_text,           9,   36, 0,   0,   0, 0, 't', 0,      0, 0, NULL,  "&Title:",   NULL },
   { d_shadow_box_proc, 48,  32, 96,  16,  0, 0, 0,   0,      0, 0, NULL,  NULL,        NULL },
   { d_edit_proc,       50,  36, 92,  12,  0, 0, 0,   0,      0, 0, NULL,  NULL,        NULL },
   { sl_text,           14,  56, 0,   0,   0, 0, 'c', 0,      0, 0, NULL,  "&Code:",    NULL },
   { d_shadow_box_proc, 48,  52, 61,  16,  0, 0, 0,   0,      0, 0, NULL,  NULL,        NULL },
   { d_edit_proc,       50,  56, 57,  12,  0, 0, 0,   0,      0, 0, NULL,  NULL,        NULL },
   { d_button_proc,     112, 76, 32,  16,  0, 0, 'o', D_EXIT, 0, 0, "&OK", NULL,        NULL },
   DIALOG_FRAME_ENDCAP
};

DEFINE_DIALOG_CALLBACK(options_patches_dialog_list);
DEFINE_DIALOG_CALLBACK(options_patches_dialog_add);
DEFINE_DIALOG_CALLBACK(options_patches_dialog_remove);
DEFINE_DIALOG_CALLBACK(options_patches_dialog_enabled);

static char *options_patches_dialog_list_filler (int, int *);

static const DIALOG options_patches_dialog_base[] =
{
   { sl_frame,      0,   0,   226, 160, 0, 0, 0,   0,      0, 0, NULL,                               "Patches",                      NULL                        },
   { sl_x_button,   206, 4,   16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",                                NULL,                           NULL                        },
   { sl_listbox,    9,   29,  207, 98,  0, 0, 0,   0,      0, 0, options_patches_dialog_list_filler, NULL,                           options_patches_dialog_list },
   { sl_button,     8,   136, 32,  16,  0, 0, 'a', 0,      0, 0, "&Add",                             options_patches_dialog_add,     NULL                        },
   { sl_button,     48,  136, 53,  16,  0, 0, 'r', 0,      0, 0, "&Remove",                          options_patches_dialog_remove,  NULL                        },
   { sl_checkbox,   121, 140, 64,  8,   0, 0, 'e', 0,      0, 0, "&Enabled",                         options_patches_dialog_enabled, NULL                        },
   { d_button_proc, 185, 136, 32,  16,  0, 0, 's', D_EXIT, 0, 0, "&Save",                            NULL,                           NULL                        },
   DIALOG_FRAME_ENDCAP
};

static const DIALOG netplay_client_connect_dialog_base[] =
{
   { sl_frame,          0,   0,  156, 84, 0, 0, 0,   0,      0, 0, NULL,  "Connect",  NULL },
   { sl_x_button,       136, 4,  16,  12, 0, 0, 0,   D_EXIT, 0, 0, "X",   NULL,       NULL },
   { sl_text,           9,   36, 0,   0,  0, 0, 's', 0,      0, 0, NULL,  "&Server:", NULL },
   { d_shadow_box_proc, 48,  32, 96,  16, 0, 0, 0,   0,      0, 0, NULL,  NULL,       NULL },
   { d_edit_proc,       50,  36, 92,  12, 0, 0, 0,   0,      0, 0, NULL,  NULL,       NULL },
   { d_button_proc,     112, 56, 32,  16, 0, 0, 'o', D_EXIT, 0, 0, "&OK", NULL,       NULL },
   DIALOG_FRAME_ENDCAP
};

static const DIALOG help_shortcuts_dialog_base[] =
{
    { sl_frame,    0,   0,   268, 165, 0, 0, 0, 0,      0, 0, NULL, "Shortcuts",               NULL },
    { sl_x_button, 248, 4,   16,  12,  0, 0, 0, D_EXIT, 0, 0, "X",  NULL,                      NULL },
    { sl_text,     9,   29,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "F1  - Save Snapshot",     NULL },
    { sl_text,     9,   41,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "F2  - Toggle Status",     NULL },
    { sl_text,     9,   53,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "F3  - Save State",        NULL },
    { sl_text,     9,   65,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "F4  - Restore State",     NULL },
    { sl_text,     9,   81,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "F5  - Flip Mirroring",    NULL },
    { sl_text,     9,   93,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "F6  - Toggle Zapper",     NULL },
    { sl_text,     9,   105, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "F7  - Toggle Sprites",    NULL },
    { sl_text,     9,   117, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "F8  - Toggle Background", NULL },
    { sl_text,     9,   133, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "0-9 - Select State Slot", NULL },
    { sl_text,     9,   149, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "ESC - Enter/Exit GUI",    NULL },
    { sl_text,     143, 29,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "~   - Fast Forward Mode", NULL },
    { sl_text,     143, 41,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "F9  - Toggle Slow Mode",  NULL },
    { sl_text,     143, 57,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "F10 - Darken Palette",    NULL },
    { sl_text,     143, 69,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "F11 - Brighten Palette",  NULL },
    { sl_text,     143, 85,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "F12 - Record Start/Stop", NULL },
    { sl_text,     143, 99,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "BACKSPACE - Message",     NULL },
    { sl_text,     143, 149, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "No other keys reserved.", NULL },
    DIALOG_FRAME_ENDCAP
};

static const DIALOG help_about_dialog_base[] =
{
   { sl_frame,    0,   0,   240, 180, 0, 0, 0, 0,      0, 0, NULL, "About",              NULL },
   { sl_x_button, 220, 4,   16,  12,  0, 0, 0, D_EXIT, 0, 0, "X",  NULL,                 NULL },
   { sl_text,     9,   29,  0 ,  0,   0, 0, 0, 0,      0, 0, NULL, "Developers:",        NULL },
   { sl_text,     17,  41,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Randy McDowell",     NULL },
   { sl_text,     17,  53,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Charles Bilyue'",    NULL },
   { sl_text,     17,  65,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Jonathan Gevaryahu", NULL },
   { sl_text,     17,  77,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Chris Robinson",     NULL },
   { sl_text,     9,   93,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Contributors:",      NULL },
   { sl_text,     17,  105, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Chris Sheehan",      NULL },
   { sl_text,     17,  117, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Amit Vainsencher",   NULL },
   { sl_text,     9,   133, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Remnants:",          NULL },
   { sl_text,     17,  145, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Ian Smith",          NULL },
   { sl_text,     27,  161, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Thank you all!",     NULL },
   { sl_text,     129, 29,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Special thanks to:", NULL },
   { sl_text,     137, 41,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "xodnizel",           NULL },
   { sl_text,     137, 53,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Matthew Conte",      NULL },
   { sl_text,     137, 65,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "TAKEDA, toshiya",    NULL },
   { sl_text,     137, 77,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "YANO, takashi",      NULL },
   { sl_text,     137, 89,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "kode54",             NULL },
   { sl_text,     137, 101, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "pagefault",          NULL },
   { sl_text,     137, 113, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "piinyouri",          NULL },
   { sl_text,     137, 125, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "aprentice",          NULL },
   { sl_text,     137, 137, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Astxist",            NULL },
   { sl_text,     137, 149, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Tim Inman",          NULL },
   { sl_text,     137, 165, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "... and many more!", NULL },
   DIALOG_FRAME_ENDCAP
};

/* Undefine helper macros. */
#undef DEFINE_DIALOG
#undef DEFINE_DIALOG_CALLBACK
#undef DIALOG_ENDCAP
#undef DIALOG_FRAME_ENDCAP
#undef IMPORT_MENU
