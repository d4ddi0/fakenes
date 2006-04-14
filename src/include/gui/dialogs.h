/* Define helper macros. */
#define DEFINE_DIALOG(name)            static DIALOG * name = NULL
#define DEFINE_DIALOG_CALLBACK(func)   static int func (DIALOG *)
#define DIALOG_ENDCAP                  { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
#define DIALOG_FRAME_ENDCAP            { NULL, 0, 0, 0, 0, 0, 0, 0, 0, SL_FRAME_END, 0, NULL, NULL, NULL }
#define IMPORT_MENU(menu)              (MENU *) & menu
  
DEFINE_DIALOG(main_dialog);
DEFINE_DIALOG(main_replay_record_start_dialog);
DEFINE_DIALOG(machine_save_state_save_dialog);
DEFINE_DIALOG(machine_cheat_manager_add_dialog);
DEFINE_DIALOG(machine_cheat_manager_dialog);
DEFINE_DIALOG(options_input_configure_dialog);
DEFINE_DIALOG(netplay_dialog);
DEFINE_DIALOG(lobby_dialog);
DEFINE_DIALOG(help_shortcuts_dialog);
DEFINE_DIALOG(help_about_dialog);

/* alert() replacement dialog, loaded and built by gui_alert(). */
static const DIALOG alert_dialog_base[] =
{
   { sl_frame,      0, 0,  0,  83, 0, 0, 0, 0,      0, 0, NULL, NULL, NULL },
   { sl_x_button,   0, 4,  16, 12, 0, 0, 0, D_EXIT, 0, 0, "X",  NULL, NULL },
   { sl_text,       0, 28, 0,  0,  0, 0, 0, 0,      0, 0, NULL, NULL, NULL },
   { sl_text,       0, 37, 0,  0,  0, 0, 0, 0,      0, 0, NULL, NULL, NULL },
   { sl_text,       0, 46, 0,  0,  0, 0, 0, 0,      0, 0, NULL, NULL, NULL },
   { d_button_proc, 0, 61, 44, 16, 0, 0, 0, D_EXIT, 0, 0, NULL, NULL, NULL },
   { d_button_proc, 0, 61, 44, 16, 0, 0, 0, D_EXIT, 0, 0, NULL, NULL, NULL },
   DIALOG_FRAME_ENDCAP
};  

enum
{
   ALERT_DIALOG_FRAME = 0,
   ALERT_DIALOG_CLOSE_BUTTON,
   ALERT_DIALOG_STRING_1,
   ALERT_DIALOG_STRING_2,
   ALERT_DIALOG_STRING_3,
   ALERT_DIALOG_BUTTON_1,
   ALERT_DIALOG_BUTTON_2
};

static const DIALOG main_dialog_base[] =
{
   { d_menu_proc, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, IMPORT_MENU(top_menu), NULL, NULL },
   DIALOG_ENDCAP
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

static const DIALOG machine_save_state_save_dialog_base[] =
{
   { sl_frame,          0,   0,  156, 84, 0, 0, 0,   0,      0, 0, NULL,  "Save State", NULL },
   { sl_x_button,       136, 4,  16,  12, 0, 0, 0,   D_EXIT, 0, 0, "X",   NULL,         NULL },
   { sl_text,           9,   36, 0,   0,  0, 0, 0,   0,      0, 0, NULL,  "&Title:",    NULL },
   { d_shadow_box_proc, 48,  32, 96,  16, 0, 0, 0,   0,      0, 0, NULL,  NULL,         NULL },
   { d_edit_proc,       50,  36, 92,  12, 0, 0, 't', 0,      0, 0, NULL,  NULL,         NULL },
   { d_button_proc,     112, 56, 32,  16, 0, 0, 'o', D_EXIT, 0, 0, "&OK", NULL,         NULL },
   DIALOG_FRAME_ENDCAP
};

static const DIALOG machine_cheat_manager_add_dialog_base[] =
{
   { sl_frame,          0,   0,  156, 100, 0, 0, 0,   0,      0, 0, NULL,  "Add Cheat", NULL },
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

enum
{
   MACHINE_CHEAT_MANAGER_ADD_DIALOG_FRAME = 0,
   MACHINE_CHEAT_MANAGER_ADD_DIALOG_CLOSE_BUTTON,
   MACHINE_CHEAT_MANAGER_ADD_DIALOG_TITLE_LABEL,
   MACHINE_CHEAT_MANAGER_ADD_DIALOG_TITLE_BOX,
   MACHINE_CHEAT_MANAGER_ADD_DIALOG_TITLE,
   MACHINE_CHEAT_MANAGER_ADD_DIALOG_CODE_LABEL,
   MACHINE_CHEAT_MANAGER_ADD_DIALOG_CODE_BOX,
   MACHINE_CHEAT_MANAGER_ADD_DIALOG_CODE,
   MACHINE_CHEAT_MANAGER_ADD_DIALOG_OK_BUTTON
};

DEFINE_DIALOG_CALLBACK(machine_cheat_manager_dialog_list);
DEFINE_DIALOG_CALLBACK(machine_cheat_manager_dialog_add);
DEFINE_DIALOG_CALLBACK(machine_cheat_manager_dialog_remove);
DEFINE_DIALOG_CALLBACK(machine_cheat_manager_dialog_enabled);

static char *machine_cheat_manager_dialog_list_filler (int, int *);

static const DIALOG machine_cheat_manager_dialog_base[] =
{
   { sl_frame,      0,   0,   226, 160, 0, 0, 0,   0,      0, 0, NULL,                                      "Cheat Manager",                      NULL                              },
   { sl_x_button,   206, 4,   16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",                                       NULL,                                 NULL                              },
   { sl_listbox,    9,   29,  207, 98,  0, 0, 0,   0,      0, 0, machine_cheat_manager_dialog_list_filler, NULL,                                  machine_cheat_manager_dialog_list },
   { sl_button,     8,   136, 32,  16,  0, 0, 'a', 0,      0, 0, "&Add",                                    machine_cheat_manager_dialog_add,     NULL                              },
   { sl_button,     48,  136, 53,  16,  0, 0, 'r', 0,      0, 0, "&Remove",                                 machine_cheat_manager_dialog_remove,  NULL                              },
   { sl_checkbox,   121, 140, 64,  8,   0, 0, 'e', 0,      0, 0, "&Enabled",                                machine_cheat_manager_dialog_enabled, NULL                              },
   { d_button_proc, 185, 136, 32,  16,  0, 0, 's', D_EXIT, 0, 0, "&Save",                                   NULL,                                 NULL                              },
   DIALOG_FRAME_ENDCAP
};

enum
{
   MACHINE_CHEAT_MANAGER_DIALOG_FRAME = 0,
   MACHINE_CHEAT_MANAGER_DIALOG_CLOSE_BUTTON,
   MACHINE_CHEAT_MANAGER_DIALOG_LIST,
   MACHINE_CHEAT_MANAGER_DIALOG_ADD_BUTTON,
   MACHINE_CHEAT_MANAGER_DIALOG_REMOVE_BUTTON,
   MACHINE_CHEAT_MANAGER_DIALOG_ENABLED_CHECKBOX,
   MACHINE_CHEAT_MANAGER_DIALOG_SAVE_BUTTON
};

DEFINE_DIALOG_CALLBACK(options_input_configure_dialog_player_select);
DEFINE_DIALOG_CALLBACK(options_input_configure_dialog_device_select);
DEFINE_DIALOG_CALLBACK(options_input_configure_dialog_set_buttons);
DEFINE_DIALOG_CALLBACK(options_input_configure_dialog_calibrate);

static const DIALOG options_input_configure_dialog_base[] =
{
   { sl_frame,    0,   0,   194, 151, 0, 0, 0,   0,      0, 0,                          NULL,           "Input",                                      NULL },
   { sl_x_button, 174, 4,   16,  12,  0, 0, 0,   D_EXIT, 0, 0,                          "X",            NULL,                                         NULL },
   { sl_text,     9,   28,  0,   0,   0, 0, 0,   0,      0, 0,                          NULL,           "Player:",                                    NULL },
   { sl_radiobox, 52,  28,  24,  6,   0, 0, '1', 0,      1, 1,                          "&1",           options_input_configure_dialog_player_select, NULL },
   { sl_radiobox, 76,  28,  24,  6,   0, 0, '2', 0,      1, 2,                          "&2",           options_input_configure_dialog_player_select, NULL },
   { sl_radiobox, 100, 28,  24,  6,   0, 0, '3', 0,      1, 3,                          "&3",           options_input_configure_dialog_player_select, NULL },
   { sl_radiobox, 124, 28,  24,  6,   0, 0, '4', 0,      1, 4,                          "&4",           options_input_configure_dialog_player_select, NULL },
   { sl_text,     9,   41,  0,   0,   0, 0, 0,   0,      0, 0,                          NULL,           "Device:",                                    NULL },
   { sl_radiobox, 52,  41,  64,  6,   0, 0, 'k', 0,      2, INPUT_DEVICE_KEYBOARD_1,    "&Key Set 1",   options_input_configure_dialog_device_select, NULL },
   { sl_radiobox, 52,  51,  64,  6,   0, 0, 'e', 0,      2, INPUT_DEVICE_KEYBOARD_2,    "K&ey Set 2",   options_input_configure_dialog_device_select, NULL },
   { sl_radiobox, 118, 41,  68,  6,   0, 0, 'p', 0,      2, INPUT_DEVICE_JOYSTICK_1,    "Stick/&Pad 1", options_input_configure_dialog_device_select, NULL },
   { sl_radiobox, 118, 51,  68,  6,   0, 0, 't', 0,      2, INPUT_DEVICE_JOYSTICK_2,    "S&tick/Pad 2", options_input_configure_dialog_device_select, NULL },
   { sl_radiobox, 52,  61,  64,  6,   0, 0, 'm', 0,      2, INPUT_DEVICE_MOUSE,         "&Mouse",       options_input_configure_dialog_device_select, NULL },
   { sl_text,     9,   74,  0,   0,   0, 0, 0,   0,      0, 0,                          NULL,           "Set Buttons:",                               NULL },
   { sl_button,   18,  86,  40,  12,  0, 0, 'u', 0,      0, INPUT_DEVICE_BUTTON_UP,     "&Up",          options_input_configure_dialog_set_buttons,   NULL },
   { sl_button,   18,  101, 40,  12,  0, 0, 'd', 0,      0, INPUT_DEVICE_BUTTON_DOWN,   "&Down",        options_input_configure_dialog_set_buttons,   NULL },
   { sl_button,   18,  116, 40,  12,  0, 0, 'l', 0,      0, INPUT_DEVICE_BUTTON_LEFT,   "&Left",        options_input_configure_dialog_set_buttons,   NULL },
   { sl_button,   18,  131, 40,  12,  0, 0, 'r', 0,      0, INPUT_DEVICE_BUTTON_RIGHT,  "&Right",       options_input_configure_dialog_set_buttons,   NULL },
   { sl_button,   66,  86,  48,  12,  0, 0, 's', 0,      0, INPUT_DEVICE_BUTTON_START,  "&Start",       options_input_configure_dialog_set_buttons,   NULL },
   { sl_button,   66,  101, 48,  12,  0, 0, 'c', 0,      0, INPUT_DEVICE_BUTTON_SELECT, "Sele&ct",      options_input_configure_dialog_set_buttons,   NULL },
   { sl_button,   66,  116, 32,  12,  0, 0, 'a', 0,      0, INPUT_DEVICE_BUTTON_A,      "&A",           options_input_configure_dialog_set_buttons,   NULL },
   { sl_button,   66,  131, 32,  12,  0, 0, 'b', 0,      0, INPUT_DEVICE_BUTTON_B,      "&B",           options_input_configure_dialog_set_buttons,   NULL },
   { sl_button,   122, 91,  64,  16,  0, 0, 'c', 0,      0, 0,                          "&Calibrate",   options_input_configure_dialog_calibrate,     NULL },
   DIALOG_FRAME_ENDCAP
};

enum
{
   OPTIONS_INPUT_CONFIGURE_DIALOG_FRAME = 0,
   OPTIONS_INPUT_CONFIGURE_DIALOG_CLOSE_BUTTON,
   OPTIONS_INPUT_CONFIGURE_DIALOG_PLAYER_LABEL,
   OPTIONS_INPUT_CONFIGURE_DIALOG_PLAYER_1_SELECT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_PLAYER_2_SELECT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_PLAYER_3_SELECT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_PLAYER_4_SELECT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_DEVICE_LABEL,
   OPTIONS_INPUT_CONFIGURE_DIALOG_DEVICE_1_SELECT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_DEVICE_2_SELECT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_DEVICE_3_SELECT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_DEVICE_4_SELECT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_DEVICE_5_SELECT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_SET_BUTTONS_LABEL,
   OPTIONS_INPUT_CONFIGURE_DIALOG_SET_BUTTON_UP,
   OPTIONS_INPUT_CONFIGURE_DIALOG_SET_BUTTON_DOWN,
   OPTIONS_INPUT_CONFIGURE_DIALOG_SET_BUTTON_LEFT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_SET_BUTTON_RIGHT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_SET_BUTTON_START,
   OPTIONS_INPUT_CONFIGURE_DIALOG_SET_BUTTON_SELECT,
   OPTIONS_INPUT_CONFIGURE_DIALOG_SET_BUTTON_A,
   OPTIONS_INPUT_CONFIGURE_DIALOG_SET_BUTTON_B,
   OPTIONS_INPUT_CONFIGURE_DIALOG_CALIBRATE_BUTTON
};

static const DIALOG netplay_dialog_base[] =
{
   { sl_frame,          0,   0,  153, 108, 0, 0, 0,   0,      0, 0, NULL,      "NetPlay", NULL },
   { sl_x_button,       133, 4,  16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",       NULL,      NULL },
   { sl_text,           9,   31, 0,   0,   0, 0, 'h', 0,      0, 0, NULL,      "&Host:",  NULL },
   { d_shadow_box_proc, 40,  27, 104, 14,  0, 0, 0,   0,      0, 0, NULL,      NULL,      NULL },
   { d_edit_proc,       42,  31, 100, 12,  0, 0, 0,   0,      0, 0, NULL,      NULL,      NULL },
   { sl_text,           9,   49, 0,   0,   0, 0, 'p', 0,      0, 0, NULL,      "&Port:",  NULL },
   { d_shadow_box_proc, 40,  45, 40,  14,  0, 0, 0,   0,      0, 0, NULL,      NULL,      NULL },
   { d_edit_proc,       42,  49, 36,  12,  0, 0, 0,   0,      0, 0, NULL,      NULL,      NULL },
   { sl_text,           84,  49, 0,   0,   0, 0, 0,   0,      0, 0, NULL,      "TCP",     NULL },
   { sl_text,           9,   69, 0,   0,   0, 0, 'n', 0,      0, 0, NULL,      "&Nick:",  NULL },
   { d_shadow_box_proc, 40,  65, 88,  14,  0, 0, 0,   0,      0, 0, NULL,      NULL,      NULL },
   { d_edit_proc,       42,  69, 84,  12,  0, 0, 0,   0,      0, 0, NULL,      NULL,      NULL },
   { d_button_proc,     61,  86, 32,  16,  0, 0, 'o', D_EXIT, 0, 0, "&OK",     NULL,      NULL },
   { d_button_proc,     97,  86, 48,  16,  0, 0, 'c', D_EXIT, 0, 0, "&Cancel", NULL,      NULL },
   DIALOG_FRAME_ENDCAP
};

enum
{
   NETPLAY_DIALOG_FRAME = 0,
   NETPLAY_DIALOG_CLOSE_BUTTON,
   NETPLAY_DIALOG_HOST_LABEL,
   NETPLAY_DIALOG_HOST_BOX,
   NETPLAY_DIALOG_HOST,
   NETPLAY_DIALOG_PORT_LABEL,
   NETPLAY_DIALOG_PORT_BOX,
   NETPLAY_DIALOG_PORT,
   NETPLAY_DIALOG_TCP_TEXT,
   NETPLAY_DIALOG_NICK_LABEL,
   NETPLAY_DIALOG_NICK_BOX,
   NETPLAY_DIALOG_NICK,
   NETPLAY_DIALOG_OK_BUTTON,
   NETPLAY_DIALOG_CANCEL_BUTTON
};

static const DIALOG lobby_dialog_base[] =
{
   { sl_frame,          0,   0,   320, 240, 0, 0, 0,   D_DISABLED, 0, 0, NULL,         "Lobby",   NULL },
   { sl_x_button,       300, 4,   16,  12,  0, 0, 0,   D_EXIT,     0, 0, "X",          NULL,      NULL },
   { sl_viewer,         10,  27,  207, 168, 0, 0, 0,   0,          0, 0, NULL,         NULL,      NULL },
   { sl_viewer,         223, 27,  89,  89,  0, 0, 0,   0,          0, 0, NULL,         NULL,      NULL },
   { sl_text,           227, 124, 0,   0,   0, 0, 0,   0,          0, 0, NULL,         "Inputs:", NULL },
   { sl_checkbox,       235, 133, 60,  8,   0, 0, '1', 0,          0, 0, "Player &1 ", NULL,      NULL },
   { sl_checkbox,       235, 142, 60,  8,   0, 0, '2', 0,          0, 0, "Player &2 ", NULL,      NULL },
   { sl_checkbox,       235, 151, 60,  8,   0, 0, '3', 0,          0, 0, "Player &3 ", NULL,      NULL },
   { sl_checkbox,       235, 160, 60,  8,   0, 0, '4', 0,          0, 0, "Player &4 ", NULL,      NULL },
   { d_shadow_box_proc, 10,  200, 302, 14,  0, 0, 0,   0,          0, 0, NULL,         NULL,      NULL },
   { sl_lobby_msgbox,   12,  204, 298, 12,  0, 0, 0,   0,          0, 0, NULL,         NULL,      NULL },
   { d_button_proc,     131, 219, 64,  16,  0, 0, 'l', 0,          0, 0, "&Load...",   NULL,      NULL },
   { d_button_proc,     203, 219, 52,  16,  0, 0, 'o', D_EXIT,     0, 0, "&OK",        NULL,      NULL },
   { d_button_proc,     259, 219, 52,  16,  0, 0, 'c', D_EXIT,     0, 0, "&Cancel",    NULL,      NULL },
   DIALOG_FRAME_ENDCAP                                                               
};

enum
{
   LOBBY_DIALOG_FRAME = 0,
   LOBBY_DIALOG_CLOSE_BUTTON,
   LOBBY_DIALOG_CHAT,
   LOBBY_DIALOG_LIST,
   LOBBY_DIALOG_INPUTS_TEXT,
   LOBBY_DIALOG_PLAYER_1_CHECKBOX,
   LOBBY_DIALOG_PLAYER_2_CHECKBOX,
   LOBBY_DIALOG_PLAYER_3_CHECKBOX,
   LOBBY_DIALOG_PLAYER_4_CHECKBOX,
   LOBBY_DIALOG_MESSAGE_BOX,
   LOBBY_DIALOG_MESSAGE,
   LOBBY_DIALOG_LOAD_BUTTON,
   LOBBY_DIALOG_OK_BUTTON,
   LOBBY_DIALOG_CANCEL_BUTTON
};

static const DIALOG help_shortcuts_dialog_base[] =
{
    { sl_frame,    0,   0,   278, 166, 0, 0, 0,   0,      0, 0, NULL,    "Shortcuts",                 NULL },
    { sl_x_button, 258, 4,   16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",     NULL,                        NULL },
    { sl_text,     9,   28,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F1  - Save Snapshot",       NULL },
    { sl_text,     9,   41,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F2  - Toggle Status",       NULL },
    { sl_text,     9,   54,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F3  - Quick Save State",    NULL },
    { sl_text,     9,   63,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F4  - Quick Load State",    NULL },
    { sl_text,     9,   72,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F5  - Save State",          NULL },
    { sl_text,     9,   81,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F6  - Restore State",       NULL },
    { sl_text,     9,   94,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F7  - Toggle Sprites",      NULL },
    { sl_text,     9,   103, 0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F8  - Toggle Background",   NULL },
    { sl_text,     9,   116, 0,   0,   0, 0, 0,   0,      0, 0, NULL,    "0-9 - Select State Slot",   NULL },
    { sl_text,     9,   129, 0,   0,   0, 0, 0,   0,      0, 0, NULL,    "ESC - Enter/Exit GUI",      NULL },
    { sl_text,     143, 28,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "~   - Fast Forward Game",   NULL },
    { sl_text,     143, 37,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "\\   - Rewind Game",        NULL },
    { sl_text,     143, 46,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F9  - Toggle Slow Mode",    NULL },
    { sl_text,     143, 59,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F10 - Decrease Brightness", NULL },
    { sl_text,     143, 68,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F11 - Increase Brightness", NULL },
    { sl_text,     143, 81,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "F12 - Record Start/Stop",   NULL },
    { sl_text,     143, 94,  0,   0,   0, 0, 0,   0,      0, 0, NULL,    "BACKSPACE - Message",       NULL },
    { sl_text,     143, 107, 0,   0,   0, 0, 0,   0,      0, 0, NULL,    "(-) - Decrease Volume",     NULL },
    { sl_text,     143, 116, 0,   0,   0, 0, 0,   0,      0, 0, NULL,    "(+) - Increase Volume",     NULL },
    { sl_text,     143, 129, 0,   0,   0, 0, 0,   0,      0, 0, NULL,    "No other keys reserved.",   NULL },
    { sl_button,   107, 143, 64,  16,  0, 0, 'c', D_EXIT, 0, 0, "&Close", NULL,                       NULL },
    DIALOG_FRAME_ENDCAP
};

static const DIALOG help_about_dialog_base[] =
{
   { sl_frame,    0,   0,   227, 159, 0, 0, 0, 0,      0, 0, NULL, "About",              NULL },
   { sl_x_button, 207, 4,   16,  12,  0, 0, 0, D_EXIT, 0, 0, "X",  NULL,                 NULL },
   { sl_text,     9,   28,  0 ,  0,   0, 0, 0, 0,      0, 0, NULL, "Developers:",        NULL },
   { sl_text,     18,  37,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Randy McDowell",     NULL },
   { sl_text,     18,  46,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Charles Bilyue'",    NULL },
   { sl_text,     18,  55,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Jonathan Gevaryahu", NULL },
   { sl_text,     18,  64,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Chris Robinson",     NULL },
   { sl_text,     9,   76,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Contributors:",      NULL },
   { sl_text,     18,  85,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Chris Sheehan",      NULL },
   { sl_text,     18,  94,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Amit Vainsencher",   NULL },
   { sl_text,     18,  103, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Astxist",            NULL },
   { sl_text,     18,  112, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Mexandrew",          NULL },
   { sl_text,     9,   124, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Remnants:",          NULL },
   { sl_text,     18,  133, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Ian Smith",          NULL },
   { sl_text,     29,  145, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Thank you all!",     NULL },
   { sl_text,     119, 28,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Special thanks to:", NULL },
   { sl_text,     128, 37,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "xodnizel",           NULL },
   { sl_text,     128, 46,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Matthew Conte",      NULL },
   { sl_text,     128, 55,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "TAKEDA, toshiya",    NULL },
   { sl_text,     128, 64,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "YANO, takashi",      NULL },
   { sl_text,     128, 73,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "kode54",             NULL },
   { sl_text,     128, 82,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "pagefault",          NULL },
   { sl_text,     128, 91,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "piinyouri",          NULL },
   { sl_text,     128, 100, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "aprentice",          NULL },
   { sl_text,     128, 109, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "sarencele",          NULL },
   { sl_text,     128, 118, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "Tim Inman",          NULL },
   { sl_text,     128, 127, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "loomsoft",           NULL },
   { sl_text,     128, 139, 0,   0,   0, 0, 0, 0,      0, 0, NULL, "... and many more!", NULL },
   DIALOG_FRAME_ENDCAP
};

/* Undefine helper macros. */
#undef DEFINE_DIALOG
#undef DEFINE_DIALOG_CALLBACK
#undef DIALOG_ENDCAP
#undef DIALOG_FRAME_ENDCAP
#undef IMPORT_MENU

static INLINE DIALOG *load_dialog (const DIALOG *dialog)
{
   DIALOG *new_dialog;
   int size = 0;
   int index = 0;
   int width, height;

   RT_ASSERT(dialog);

   while (dialog[index].proc)
   {
      size += sizeof (DIALOG);

      index++;
   }

   /* Once more for the end marker. */
   size += sizeof (DIALOG);

   if (!(new_dialog = malloc (size)))
   {
       WARN("Failed to allocate dialog structure");

       return (NULL);
   }

   memcpy (new_dialog, dialog, size);

   /* Font scaling parameters. */
   width = text_length (font, "X");
   height = text_height (font);

   /* Reset counter. */
   index = 0;

   while (new_dialog[index].proc)
   {
      DIALOG *object = &new_dialog[index];

      /* Import menu by reference. */
      if (object->proc == d_menu_proc)
         object->dp = *(MENU **)object->dp;

      /* Dialog to font scaling. */
      if (font != small_font)
      {
         switch (index)
         {
            case 0: /* sl_frame. */
            {
               object->w = ROUND(((object->w / 5.0f) * width));
               object->h = (ROUND(((object->h / 6.0f) * height)) - height);

               break;
            }
         
            case 1: /* sl_x_button. */
            {
               object->x = ROUND(((object->x / 5.0f) * width));
     
               break;
            } 
    
            default:
            {
               object->x = ROUND(((object->x / 5.0f) * width));
               object->y = (ROUND(((object->y / 6.0f) * height)) - height);
               object->w = ROUND(((object->w / 5.0f) * width));
               object->h = ROUND(((object->h / 6.0f) * height));
    
               break;
            }
         }

      }

      index++;
    }

    return (new_dialog);
}

static INLINE void unload_dialog (DIALOG *dialog)
{
   RT_ASSERT(dialog);

   free (dialog);
}

#define DIALOG_FROM_BASE(name)  (name = load_dialog (name ##_base))

static INLINE void load_dialogs (void)
{
   DIALOG_FROM_BASE(main_dialog);
   DIALOG_FROM_BASE(main_replay_record_start_dialog);
   DIALOG_FROM_BASE(machine_save_state_save_dialog);
   DIALOG_FROM_BASE(machine_cheat_manager_add_dialog);
   DIALOG_FROM_BASE(machine_cheat_manager_dialog);
   DIALOG_FROM_BASE(options_input_configure_dialog);
   DIALOG_FROM_BASE(netplay_dialog);
   DIALOG_FROM_BASE(lobby_dialog);
   DIALOG_FROM_BASE(help_shortcuts_dialog);
   DIALOG_FROM_BASE(help_about_dialog);
}

#undef DIALOG_FROM_BASE

static INLINE void unload_dialogs (void)
{
   unload_dialog (main_dialog);
   unload_dialog (main_replay_record_start_dialog);
   unload_dialog (machine_save_state_save_dialog);
   unload_dialog (machine_cheat_manager_add_dialog);
   unload_dialog (machine_cheat_manager_dialog);
   unload_dialog (options_input_configure_dialog);
   unload_dialog (netplay_dialog);
   unload_dialog (lobby_dialog);
   unload_dialog (help_shortcuts_dialog);
   unload_dialog (help_about_dialog);
}

static INLINE int run_dialog (DIALOG *dialog, int focus)
{
   DIALOG_PLAYER *player;
   int index;

   /* Similar to Allegro's do_dialog(), but is built to be non-blocking with
      minimal CPU usage and automatic screen refresh for when the GUI is not
      being drawn directly to the screen. */

   RT_ASSERT(dialog);

   player = init_dialog (dialog, focus);

   while (update_dialog (player))
      gui_heartbeat ();

   index = player->obj;

   shutdown_dialog (player);

   return (index);
}

static INLINE int show_dialog (DIALOG *dialog, int focus)
{
   BITMAP *bmp;
   BITMAP *saved;
   int position;
   UINT16 x = x, y = y; /* Kill warnings. */
   BOOL moved = FALSE;
   int index = 0;

   RT_ASSERT(dialog);

   bmp = gui_get_screen ();

   saved = create_bitmap (bmp->w, bmp->h);
   if (!saved)
      WARN("Failed to create temporary background buffer; crash imminent");

   scare_mouse ();
   blit (bmp, saved, 0, 0, 0, 0, bmp->w, bmp->h);
   unscare_mouse ();

   position = get_config_hex ("dialogs", dialog[0].dp2, -1);

   if (position == -1)
   {
      centre_dialog (dialog);
   }
   else
   {
      x = (position >> 16);
      y = (position & 0x0000ffff);

      position_dialog (dialog, x, y);
   }

   dialog[0].dp3 = DATA_TO_FONT(LARGE_FONT);

   while (dialog[index].d1 != SL_FRAME_END)
   {
      /* Update colors. */

      DIALOG *object = &dialog[index];

      object->fg = GUI_TEXT_COLOR;
      object->bg = gui_bg_color;

      index++;
   }

   next:
   {
      index = run_dialog (dialog, focus);
   
      scare_mouse ();
      blit (saved, bmp, 0, 0, 0, 0, saved->w, saved->h);
      unscare_mouse ();
   
      if (restart_dialog)
      {
         restart_dialog = FALSE;

         x = dialog_x;
         y = dialog_y;
   
         position_dialog (dialog, x, y);
   
         moved = TRUE;
   
         goto next;
      }
   }

   if (moved)
      set_config_hex ("dialogs", dialog[0].dp2, ((x << 16) | y));

   destroy_bitmap (saved);

   return (index);
}
