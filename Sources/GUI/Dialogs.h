/* Define helper macros. */
#define DEFINE_DIALOG(name)            static DIALOG * name = NULL
#define DEFINE_DIALOG_CALLBACK(func)   static int func (DIALOG *)
#define DIALOG_ENDCAP                  { sl_idle, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }, { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
#define DIALOG_FRAME_ENDCAP            { sl_idle, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }, { NULL, 0, 0, 0, 0, 0, 0, 0, 0, SL_FRAME_END, 0, NULL, NULL, NULL }
#define IMPORT_MENU(menu)              (MENU *) & menu
  
DEFINE_DIALOG(main_dialog);
DEFINE_DIALOG(main_replay_record_start_dialog);
DEFINE_DIALOG(main_cheat_manager_add_dialog);
DEFINE_DIALOG(main_cheat_manager_dialog);
DEFINE_DIALOG(system_save_state_save_dialog);
DEFINE_DIALOG(video_color_dialog);
DEFINE_DIALOG(input_configure_dialog);
DEFINE_DIALOG(options_paths_dialog);
DEFINE_DIALOG(netplay_dialog);
DEFINE_DIALOG(lobby_dialog);
DEFINE_DIALOG(help_keyboard_shortcuts_dialog);
DEFINE_DIALOG(help_fakenes_team_dialog);

/* alert() replacement dialog, loaded and built by gui_alert(). */
static const DIALOG alert_dialog_base[] =
{
   { sl_frame,    0, 0,  0,  83, 0, 0, 0, 0,      0, 0, NULL, NULL, NULL },
   { sl_x_button, 0, 4,  16, 12, 0, 0, 0, D_EXIT, 0, 0, "X",  NULL, NULL },
   { sl_text,     0, 28, 0,  0,  0, 0, 0, 0,      0, 0, NULL, NULL, NULL },
   { sl_text,     0, 37, 0,  0,  0, 0, 0, 0,      0, 0, NULL, NULL, NULL },
   { sl_text,     0, 46, 0,  0,  0, 0, 0, 0,      0, 0, NULL, NULL, NULL },
   { sl_button,   0, 61, 44, 16, 0, 0, 0, D_EXIT, 0, 0, NULL, NULL, NULL },
   { sl_button,   0, 61, 44, 16, 0, 0, 0, D_EXIT, 0, 0, NULL, NULL, NULL },
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

static const DIALOG viewer_dialog_base[] =
{
   { sl_frame,    0,   0,   418, 232, 0, 0, 0,   0,      0, 0, NULL,     NULL, NULL },
   { sl_x_button, 399, 4,   16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",      NULL, NULL },
   { sl_viewer,   9,   28,  400, 175, 0, 0, 0,   0,      0, 0, NULL,     NULL, NULL },
   { sl_button,   177, 209, 64,  16,  0, 0, 'c', D_EXIT, 0, 0, "&Close", NULL, NULL },
   DIALOG_FRAME_ENDCAP
};

enum
{
   VIEWER_DIALOG_FRAME = 0,
   VIEWER_DIALOG_CLOSE_BUTTON,
   VIEWER_DIALOG_TEXT,
   VIEWER_DIALOG_CLOSE_BUTTON_2,
};

static const DIALOG resolution_dialog_base[] =
{
   { sl_frame,    0,   0,  146, 74, 0, 0, 0,   0,      0, 0, NULL,      NULL,     NULL },
   { sl_x_button, 127, 4,  16,  12, 0, 0, 0,   D_EXIT, 0, 0, "X",       NULL,     NULL },
   { sl_editbox,  9,   28, 32,  14, 0, 0, 0,   0,      0, 0, NULL,      NULL,     NULL },
   { sl_text,     50,  31, 0,   0,  0, 0, 0,   0,      0, 0, NULL,      "x",      NULL },
   { sl_editbox,  64,  28, 32,  14, 0, 0, 0,   0,      0, 0, NULL,      NULL,     NULL },
   { sl_text,     105, 31, 0,   0,  0, 0, 0,   0,      0, 0, NULL,      "pixels", NULL },
   { sl_button,   9,   51, 48,  16, 0, 0, 'o', D_EXIT, 0, 0, "&OK",     NULL,     NULL },
   { sl_button,   63,  51, 48,  16, 0, 0, 'c', D_EXIT, 0, 0, "&Cancel", NULL,     NULL },
   DIALOG_FRAME_ENDCAP
};

enum
{
   RESOLUTION_DIALOG_FRAME = 0,
   RESOLUTION_DIALOG_CLOSE_BUTTON,
   RESOLUTION_DIALOG_WIDTH,
   RESOLUTION_DIALOG_X_TEXT,
   RESOLUTION_DIALOG_HEIGHT,
   RESOLUTION_DIALOG_PIXELS_TEXT,
   RESOLUTION_DIALOG_OK_BUTTON,
   RESOLUTION_DIALOG_CANCEL_BUTTON
};

static const DIALOG amount_dialog_base[] =
{
   { sl_frame,    0,   0,  146, 74, 0, 0, 0,   0,      0, 0, NULL,      NULL,     NULL },
   { sl_x_button, 127, 4,  16,  12, 0, 0, 0,   D_EXIT, 0, 0, "X",       NULL,     NULL },
   { sl_editbox,  9,   28, 32,  14, 0, 0, 0,   0,      0, 0, NULL,      NULL,     NULL },
   { sl_text,     50,  31, 0,   0,  0, 0, 0,   0,      0, 0, NULL,      NULL,     NULL },
   { sl_button,   9,   51, 48,  16, 0, 0, 'o', D_EXIT, 0, 0, "&OK",     NULL,     NULL },
   { sl_button,   63,  51, 48,  16, 0, 0, 'c', D_EXIT, 0, 0, "&Cancel", NULL,     NULL },
   DIALOG_FRAME_ENDCAP
};

enum
{
   AMOUNT_DIALOG_FRAME = 0,
   AMOUNT_DIALOG_CLOSE_BUTTON,
   AMOUNT_DIALOG_VALUE,
   AMOUNT_DIALOG_UNITS_LABEL,
   AMOUNT_DIALOG_OK_BUTTON,
   AMOUNT_DIALOG_CANCEL_BUTTON
};

static const DIALOG file_select_dialog_base[] =
{
   { sl_frame,    0,   0,   384, 265, 0, 0, 0,   0,      0, 0, NULL,                 NULL,            NULL },
   { sl_x_button, 365, 4,   16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",                  NULL,            NULL },
   { sl_ctext,    0,   28,  384, 0,   0, 0, 0,   0,      0, 0, NULL,                 NULL,            NULL },
   { sl_text,     60,  40,  0,   0,   0, 0, 'd', 0,      0, 0, NULL,                 "&Directories:", NULL },
   { sl_listbox,  9,   52,  148, 148, 0, 0, 0,   0,      0, 0, NULL,                 NULL,            NULL },
   { sl_checkbox, 18,  204, 100, 8,   0, 0, 'h', 0,      0, 0, "Show &Hidden Files", NULL,            NULL },
   { sl_text,     257, 40,  0,   0,   0, 0, 'f', 0,      0, 0, NULL,                 "&Files:",       NULL },
   { sl_listbox,  166, 52,  209, 160, 0, 0, 0,   0,      0, 0, NULL,                 NULL,            NULL },
   { sl_text,     9,   225, 0,   0,   0, 0, 0,   0,      0, 0, NULL,                 "File&name:",    NULL },
   { sl_editbox2, 63,  222, 312, 12,  0, 0, 'n', 0,      0, 0, NULL,                 NULL,            NULL },
   { sl_button,   125, 240, 64,  16,  0, 0, 'o', D_EXIT, 0, 0, "&OK",                NULL,            NULL },
   { sl_button,   195, 240, 64,  16,  0, 0, 'c', D_EXIT, 0, 0, "&Cancel",            NULL,            NULL },
   DIALOG_FRAME_ENDCAP                                                               
};

enum
{
   FILE_SELECT_DIALOG_FRAME = 0,
   FILE_SELECT_DIALOG_CLOSE_BUTTON,
   FILE_SELECT_DIALOG_CAPTION_LABEL,
   FILE_SELECT_DIALOG_DIRECTORIES_LABEL,
   FILE_SELECT_DIALOG_DIRECTORY_LIST,
   FILE_SELECT_DIALOG_SHOW_HIDDEN_FILES_CHECKBOX,
   FILE_SELECT_DIALOG_FILES_LABEL,
   FILE_SELECT_DIALOG_FILE_LIST,
   FILE_SELECT_DIALOG_FILENAME_LABEL,
   FILE_SELECT_DIALOG_FILENAME,
   FILE_SELECT_DIALOG_OK_BUTTON,
   FILE_SELECT_DIALOG_CANCEL_BUTTON
};

static const DIALOG main_dialog_base[] =
{
   { d_menu_proc, 8, 8, 0, 0, 0,  0, 0, 0, 0, 0, IMPORT_MENU(top_menu), NULL, NULL },
   DIALOG_ENDCAP
};

static const DIALOG main_replay_record_start_dialog_base[] =
{
   { sl_frame,    0,   0,  220, 89, 0, 0, 0,   0,      0, 0, NULL,      "Record Replay", NULL },
   { sl_x_button, 200, 4,  16,  12, 0, 0, 0,   D_EXIT, 0, 0, "X",       NULL,            NULL },
   { sl_text,     9,   28, 0,   0,  0, 0, 0,   0,      0, 0, NULL,      "&Title:",       NULL },
   { sl_editbox,  9,   39, 202, 16, 0, 0, 't', 0,      0, 0, NULL,      NULL,            NULL },
   { sl_button,   59,  64, 48,  16, 0, 0, 'o', D_EXIT, 0, 0, "&OK",     NULL,            NULL },
   { sl_button,   113, 64, 48,  16, 0, 0, 'c', D_EXIT, 0, 0, "&Cancel", NULL,            NULL },
   DIALOG_FRAME_ENDCAP
};

enum
{
   MAIN_REPLAY_RECORD_START_DIALOG_FRAME = 0,
   MAIN_REPLAY_RECORD_START_DIALOG_CLOSE_BUTTON,
   MAIN_REPLAY_RECORD_START_DIALOG_TITLE_LABEL,
   MAIN_REPLAY_RECORD_START_DIALOG_TITLE,
   MAIN_REPLAY_RECORD_START_DIALOG_OK_BUTTON,
   MAIN_REPLAY_RECORD_START_DIALOG_CANCEL_BUTTON
};

static const DIALOG main_cheat_manager_add_dialog_base[] =
{
   { sl_frame,    0,   0,  156, 100, 0, 0, 0,   0,      0, 0, NULL,  "Add Cheat", NULL },
   { sl_x_button, 136, 4,  16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",   NULL,        NULL },
   { sl_text,     9,   36, 0,   0,   0, 0, 0,   0,      0, 0, NULL,  "&Title:",   NULL },
   { sl_editbox,  48,  32, 96,  16,  0, 0, 't', 0,      0, 0, NULL,  NULL,        NULL },
   { sl_text,     14,  56, 0,   0,   0, 0, 0,   0,      0, 0, NULL,  "&Code:",    NULL },
   { sl_editbox,  48,  52, 61,  16,  0, 0, 'c', 0,      0, 0, NULL,  NULL,        NULL },
   { sl_button,   112, 76, 32,  16,  0, 0, 'o', D_EXIT, 0, 0, "&OK", NULL,        NULL },
   DIALOG_FRAME_ENDCAP
};

enum
{
   MAIN_CHEAT_MANAGER_ADD_DIALOG_FRAME = 0,
   MAIN_CHEAT_MANAGER_ADD_DIALOG_CLOSE_BUTTON,
   MAIN_CHEAT_MANAGER_ADD_DIALOG_TITLE_LABEL,
   MAIN_CHEAT_MANAGER_ADD_DIALOG_TITLE,
   MAIN_CHEAT_MANAGER_ADD_DIALOG_CODE_LABEL,
   MAIN_CHEAT_MANAGER_ADD_DIALOG_CODE,
   MAIN_CHEAT_MANAGER_ADD_DIALOG_OK_BUTTON
};

DEFINE_DIALOG_CALLBACK(main_cheat_manager_dialog_list);
DEFINE_DIALOG_CALLBACK(main_cheat_manager_dialog_add);
DEFINE_DIALOG_CALLBACK(main_cheat_manager_dialog_remove);
DEFINE_DIALOG_CALLBACK(main_cheat_manager_dialog_enabled);

static char *main_cheat_manager_dialog_list_filler (int, int *);

static const DIALOG main_cheat_manager_dialog_base[] =
{
   { sl_frame,    0,   0,   226, 160, 0, 0, 0,   0,      0, 0, NULL,                                      "Cheat Manager",                      NULL                              },
   { sl_x_button, 206, 4,   16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",                                       NULL,                                 NULL                              },
   { sl_listbox,  9,   29,  207, 98,  0, 0, 0,   0,      0, 0, main_cheat_manager_dialog_list_filler, NULL,                                  main_cheat_manager_dialog_list },
   { sl_button,   8,   136, 32,  16,  0, 0, 'a', 0,      0, 0, "&Add",                                    main_cheat_manager_dialog_add,     NULL                              },
   { sl_button,   48,  136, 53,  16,  0, 0, 'r', 0,      0, 0, "&Remove",                                 main_cheat_manager_dialog_remove,  NULL                              },
   { sl_checkbox, 121, 140, 64,  8,   0, 0, 'e', 0,      0, 0, "&Enabled",                                main_cheat_manager_dialog_enabled, NULL                              },
   { sl_button,   185, 136, 32,  16,  0, 0, 's', D_EXIT, 0, 0, "&Save",                                   NULL,                                 NULL                              },
   DIALOG_FRAME_ENDCAP
};

enum
{
   MAIN_CHEAT_MANAGER_DIALOG_FRAME = 0,
   MAIN_CHEAT_MANAGER_DIALOG_CLOSE_BUTTON,
   MAIN_CHEAT_MANAGER_DIALOG_LIST,
   MAIN_CHEAT_MANAGER_DIALOG_ADD_BUTTON,
   MAIN_CHEAT_MANAGER_DIALOG_REMOVE_BUTTON,
   MAIN_CHEAT_MANAGER_DIALOG_ENABLED_CHECKBOX,
   MAIN_CHEAT_MANAGER_DIALOG_SAVE_BUTTON
};

static const DIALOG system_save_state_save_dialog_base[] =
{
   { sl_frame,    0,   0,  220, 89, 0, 0, 0,   0,      0, 0, NULL,      "Save State", NULL },
   { sl_x_button, 200, 4,  16,  12, 0, 0, 0,   D_EXIT, 0, 0, "X",       NULL,         NULL },
   { sl_text,     9,   28, 0,   0,  0, 0, 0,   0,      0, 0, NULL,      "&Title:",    NULL },
   { sl_editbox,  9,   39, 202, 16, 0, 0, 't', 0,      0, 0, NULL,      NULL,         NULL },
   { sl_button,   59,  64, 48,  16, 0, 0, 'o', D_EXIT, 0, 0, "&OK",     NULL,         NULL },
   { sl_button,   113, 64, 48,  16, 0, 0, 'c', D_EXIT, 0, 0, "&Cancel", NULL,         NULL },
   DIALOG_FRAME_ENDCAP
};

enum
{
   SYSTEM_SAVE_STATE_SAVE_DIALOG_FRAME = 0,
   SYSTEM_SAVE_STATE_SAVE_DIALOG_CLOSE_BUTTON,
   SYSTEM_SAVE_STATE_SAVE_DIALOG_TITLE_LABEL,
   SYSTEM_SAVE_STATE_SAVE_DIALOG_TITLE,
   SYSTEM_SAVE_STATE_SAVE_DIALOG_OK_BUTTON,
   SYSTEM_SAVE_STATE_SAVE_DIALOG_CANCEL_BUTTON
};

static const DIALOG video_color_dialog_base[] =
{                                                           
   { sl_frame,      0,   0,   144, 163, 0, 0, 0,   0,      0,   0, NULL,     "Color",          NULL },
   { sl_x_button,   125, 4,   16,  12,  0, 0, 0,   D_EXIT, 0,   0, "X",      NULL,             NULL },
   { sl_text,       9,   28,  0,   0,   0, 0, 0,   0,      0,   0, NULL,     "&Hue:",          NULL },
   { d_slider_proc, 18,  37,  117, 5,   0, 0, 'h', 0,      200, 0, NULL,     NULL,             NULL },
   { sl_text,       9,   49,  0,   0,   0, 0, 0,   0,      0,   0, NULL,     "&Saturation:",   NULL },
   { d_slider_proc, 18,  58,  117, 5,   0, 0, 's', 0,      200, 0, NULL,     NULL,             NULL },
   { sl_text,       9,   70,  0,   0,   0, 0, 0,   0,      0,   0, NULL,     "&Brightness:",   NULL },
   { d_slider_proc, 18,  79,  117, 5,   0, 0, 'b', 0,      200, 0, NULL,     NULL,             NULL },
   { sl_text,       9,   91,  0,   0,   0, 0, 0,   0,      0,   0, NULL,     "&Contrast:",     NULL },
   { d_slider_proc, 18,  100, 117, 5,   0, 0, 'c', 0,      200, 0, NULL,     NULL,             NULL },
   { sl_text,       9,   112, 0,   0,   0, 0, 0,   0,      0,   0, NULL,     "&Gamma:",        NULL },
   { d_slider_proc, 18,  121, 117, 5,   0, 0, 'g', 0,      200, 0, NULL,     NULL,             NULL },
   { sl_button,     21,  138, 48,  16,  0, 0, 'a', D_EXIT, 0,   0, "S&ave",  NULL,             NULL },
   { sl_button,     75,  138, 48,  16,  0, 0, 'r', D_EXIT, 0,   0, "&Reset", NULL,             NULL },
   DIALOG_FRAME_ENDCAP                                                               
};  

enum
{
   VIDEO_COLOR_DIALOG_FRAME = 0,
   VIDEO_COLOR_DIALOG_CLOSE_BUTTON,
   VIDEO_COLOR_DIALOG_HUE_LABEL,
   VIDEO_COLOR_DIALOG_HUE,
   VIDEO_COLOR_DIALOG_SATURATION_LABEL,
   VIDEO_COLOR_DIALOG_SATURATION,
   VIDEO_COLOR_DIALOG_BRIGHTNESS_LABEL,
   VIDEO_COLOR_DIALOG_BRIGHTNESS,
   VIDEO_COLOR_DIALOG_CONTRAST_LABEL,
   VIDEO_COLOR_DIALOG_CONTRAST,
   VIDEO_COLOR_DIALOG_GAMMA_LABEL,
   VIDEO_COLOR_DIALOG_GAMMA,
   VIDEO_COLOR_DIALOG_SAVE_BUTTON,
   VIDEO_COLOR_DIALOG_RESET_BUTTON
};

static const DIALOG options_paths_dialog_base[] =
{
   { sl_frame,    0,   0,   251, 135, 0, 0, 0,   0,      0, 0, NULL,      "Paths",       NULL },
   { sl_x_button, 232, 4,   16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",       NULL,          NULL },
   { sl_text,     9,   28,  0,   0,   0, 0, 0,   0,      0, 0, NULL,      "O&pen Path:", NULL },
   { sl_editbox,  9,   39,  233, 14,  0, 0, 'p', 0,      0, 0, NULL,      NULL,          NULL },
   { sl_checkbox, 18,  58,  64,  8,   0, 0, 'l', 0,      0, 0, "&Locked", NULL,          NULL },
   { sl_text,     9,   76,  0,   0,   0, 0, 0,   0,      0, 0, NULL,      "&Save Path:", NULL },
   { sl_editbox,  9,   87,  233, 14,  0, 0, 's', 0,      0, 0, NULL,      NULL,          NULL },
   { sl_button,   75,  110, 48,  16,  0, 0, 'o', D_EXIT, 0, 0, "&OK",     NULL,          NULL },
   { sl_button,   129, 110, 48,  16,  0, 0, 'c', D_EXIT, 0, 0, "&Cancel", NULL,          NULL },
   DIALOG_FRAME_ENDCAP
};

enum
{
   OPTIONS_PATHS_DIALOG_FRAME = 0,
   OPTIONS_PATHS_DIALOG_CLOSE_BUTTON,
   OPTIONS_PATHS_DIALOG_OPEN_PATH_LABEL,
   OPTIONS_PATHS_DIALOG_OPEN_PATH,
   OPTIONS_PATHS_DIALOG_LOCKED,
   OPTIONS_PATHS_DIALOG_SAVE_PATH_LABEL,
   OPTIONS_PATHS_DIALOG_SAVE_PATH,
   OPTIONS_PATHS_DIALOG_OK_BUTTON,
   OPTIONS_PATHS_DIALOG_CANCEL_BUTTON
};

DEFINE_DIALOG_CALLBACK(input_configure_dialog_player_select);
DEFINE_DIALOG_CALLBACK(input_configure_dialog_device_select);
DEFINE_DIALOG_CALLBACK(input_configure_dialog_calibrate);
DEFINE_DIALOG_CALLBACK(input_configure_dialog_set_buttons);

static const DIALOG input_configure_dialog_base[] =
{
   { sl_frame,      0,   0,   255, 260, 0, 0, 0,   0,      0,   0,                       NULL,                                       "Input",                                      NULL },
   { sl_x_button,   236, 4,   16,  12,  0, 0, 0,   D_EXIT, 0,   0,                       "X",                                        NULL,                                         NULL },
   { sl_text,       9,   28,  0,   0,   0, 0, 0,   0,      0,   0,                       NULL,                                       "Player:",                                    NULL },
   { sl_radiobox,   52,  28,  28,  6,   0, 0, '1', 0,      1,   INPUT_PLAYER_1,          "&1",                                       input_configure_dialog_player_select, NULL },
   { sl_radiobox,   76,  28,  28,  6,   0, 0, '2', 0,      1,   INPUT_PLAYER_2,          "&2",                                       input_configure_dialog_player_select, NULL },
   { sl_radiobox,   100, 28,  28,  6,   0, 0, '3', 0,      1,   INPUT_PLAYER_3,          "&3",                                       input_configure_dialog_player_select, NULL },
   { sl_radiobox,   124, 28,  28,  6,   0, 0, '4', 0,      1,   INPUT_PLAYER_4,          "&4",                                       input_configure_dialog_player_select, NULL },
   { sl_text,       9,   43,  0,   0,   0, 0, 0,   0,      0,   0,                       NULL,                                       "Device:",                                    NULL },
   { sl_radiobox,   18,  55,  80,  6,   0, 0, 0,   0,      2,   INPUT_DEVICE_NONE,       "Disabled",                                 input_configure_dialog_device_select, NULL },
   { sl_radiobox,   18,  64,  80,  6,   0, 0, 'k', 0,      2,   INPUT_DEVICE_KEYS_1,     "&Key Set 1",                               input_configure_dialog_device_select, NULL },
   { sl_radiobox,   18,  73,  80,  6,   0, 0, 0,   0,      2,   INPUT_DEVICE_KEYS_2,     "Key Set 2",                                input_configure_dialog_device_select, NULL },
   { sl_radiobox,   18,  82,  80,  6,   0, 0, 'p', 0,      2,   INPUT_DEVICE_JOYSTICK_1, "Stick/&Pad 1",                             input_configure_dialog_device_select, NULL },
   { sl_radiobox,   18,  91,  80,  6,   0, 0, 0,   0,      2,   INPUT_DEVICE_JOYSTICK_2, "Stick/Pad 2",                              input_configure_dialog_device_select, NULL },
   { sl_radiobox,   18,  100, 80,  6,   0, 0, 0,   0,      2,   INPUT_DEVICE_JOYSTICK_3, "Stick/Pad 3",                              input_configure_dialog_device_select, NULL },
   { sl_radiobox,   18,  109, 80,  6,   0, 0, 0,   0,      2,   INPUT_DEVICE_JOYSTICK_4, "Stick/Pad 4",                              input_configure_dialog_device_select, NULL },
   { sl_radiobox,   18,  118, 80,  6,   0, 0, 'm', 0,      2,   INPUT_DEVICE_MOUSE,      "&Mouse",                                   input_configure_dialog_device_select, NULL },
   { sl_button,     18,  132, 80,  16,  0, 0, 'c', 0,      0,   0,                       "&Calibrate",                               input_configure_dialog_calibrate,     NULL },
   { sl_text,       98,  43,  0,   0,   0, 0, 0,   0,      0,   0,                       NULL,                                       "Set Buttons:",                               NULL },
   { sl_button,     107, 55,  48,  12,  0, 0, 'u', 0,      0,   INPUT_BUTTON_UP,         "&Up",                                      input_configure_dialog_set_buttons,   NULL },
   { sl_button,     107, 69,  48,  12,  0, 0, 'd', 0,      0,   INPUT_BUTTON_DOWN,       "&Down",                                    input_configure_dialog_set_buttons,   NULL },
   { sl_button,     107, 83,  48,  12,  0, 0, 'l', 0,      0,   INPUT_BUTTON_LEFT,       "&Left",                                    input_configure_dialog_set_buttons,   NULL },
   { sl_button,     107, 97,  48,  12,  0, 0, 'r', 0,      0,   INPUT_BUTTON_RIGHT,      "&Right",                                   input_configure_dialog_set_buttons,   NULL },
   { sl_button,     107, 111, 48,  12,  0, 0, 's', 0,      0,   INPUT_BUTTON_START,      "&Start",                                   input_configure_dialog_set_buttons,   NULL },
   { sl_button,     107, 125, 48,  12,  0, 0, 't', 0,      0,   INPUT_BUTTON_SELECT,     "Selec&t",                                  input_configure_dialog_set_buttons,   NULL },
   { sl_button,     107, 139, 48,  12,  0, 0, 'a', 0,      0,   INPUT_BUTTON_A,          "&A",                                       input_configure_dialog_set_buttons,   NULL },
   { sl_button,     107, 153, 48,  12,  0, 0, 'b', 0,      0,   INPUT_BUTTON_B,          "&B",                                       input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   164, 57,  40,  7,   0, 0, 0,   0,      1,   INPUT_BUTTON_UP,         "Auto",                                     input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   164, 71,  40,  7,   0, 0, 0,   0,      1,   INPUT_BUTTON_DOWN,       "Auto",                                     input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   164, 85,  40,  7,   0, 0, 0,   0,      1,   INPUT_BUTTON_LEFT,       "Auto",                                     input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   164, 99,  40,  7,   0, 0, 0,   0,      1,   INPUT_BUTTON_RIGHT,      "Auto",                                     input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   164, 113, 40,  7,   0, 0, 0,   0,      1,   INPUT_BUTTON_START,      "Auto",                                     input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   164, 127, 40,  7,   0, 0, 0,   0,      1,   INPUT_BUTTON_SELECT,     "Auto",                                     input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   164, 141, 40,  7,   0, 0, 0,   0,      1,   INPUT_BUTTON_A,          "Auto",                                     input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   164, 155, 40,  7,   0, 0, 0,   0,      1,   INPUT_BUTTON_B,          "Auto",                                     input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   207, 57,  48,  7,   0, 0, 0,   0,      2,   INPUT_BUTTON_UP,         "Turbo",                                    input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   207, 71,  48,  7,   0, 0, 0,   0,      2,   INPUT_BUTTON_DOWN,       "Turbo",                                    input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   207, 85,  48,  7,   0, 0, 0,   0,      2,   INPUT_BUTTON_LEFT,       "Turbo",                                    input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   207, 99,  48,  7,   0, 0, 0,   0,      2,   INPUT_BUTTON_RIGHT,      "Turbo",                                    input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   207, 113, 48,  7,   0, 0, 0,   0,      2,   INPUT_BUTTON_START,      "Turbo",                                    input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   207, 127, 48,  7,   0, 0, 0,   0,      2,   INPUT_BUTTON_SELECT,     "Turbo",                                    input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   207, 141, 48,  7,   0, 0, 0,   0,      2,   INPUT_BUTTON_A,          "Turbo",                                    input_configure_dialog_set_buttons,   NULL },
   { sl_checkbox,   207, 155, 48,  7,   0, 0, 0,   0,      2,   INPUT_BUTTON_B,          "Turbo",                                    input_configure_dialog_set_buttons,   NULL },
   { sl_hr,         0,   173, 255, 3,   0, 0, 0,   0,      0,   0,                       NULL,                                       NULL,                                         NULL },
   { sl_checkbox,   9,   182, 216, 7,   0, 0, 0,   0,      0,   0,                       "Allow conflicting directional controls",   NULL,                                         NULL },
   { sl_checkbox,   9,   193, 80,  7,   0, 0, 0,   0,      0,   0,                       "Toggled auto",                             NULL,                                         NULL },
   { sl_checkbox,   9,   203, 216, 7,   0, 0, 0,   0,      0,   0,                       "Merge players 3 && 4 with players 1 && 2", NULL,                                         NULL },
   { sl_text,       9,   219, 0,   0,   0, 0, 0,   0,      0,   0,                       NULL,                                       "Turbo:",                                     NULL },
   { d_slider_proc, 48,  219, 192, 7,   0, 0, 0,   0,      100, 0,                       NULL,                                       NULL,                                         NULL },
   { sl_button,     104, 235, 48,  16,  0, 0, 0,   D_EXIT, 0,   0,                       "Save",                                     NULL,                                         NULL },
   DIALOG_FRAME_ENDCAP                                                                                                              
};

enum
{
   INPUT_CONFIGURE_DIALOG_FRAME = 0,
   INPUT_CONFIGURE_DIALOG_CLOSE_BUTTON,
   INPUT_CONFIGURE_DIALOG_PLAYER_LABEL,
   INPUT_CONFIGURE_DIALOG_PLAYER_1_SELECT,
   INPUT_CONFIGURE_DIALOG_PLAYER_2_SELECT,
   INPUT_CONFIGURE_DIALOG_PLAYER_3_SELECT,
   INPUT_CONFIGURE_DIALOG_PLAYER_4_SELECT,
   INPUT_CONFIGURE_DIALOG_DEVICE_LABEL,
   INPUT_CONFIGURE_DIALOG_DEVICE_0_SELECT,
   INPUT_CONFIGURE_DIALOG_DEVICE_1_SELECT,
   INPUT_CONFIGURE_DIALOG_DEVICE_2_SELECT,
   INPUT_CONFIGURE_DIALOG_DEVICE_3_SELECT,
   INPUT_CONFIGURE_DIALOG_DEVICE_4_SELECT,
   INPUT_CONFIGURE_DIALOG_DEVICE_5_SELECT,
   INPUT_CONFIGURE_DIALOG_DEVICE_6_SELECT,
   INPUT_CONFIGURE_DIALOG_DEVICE_7_SELECT,
   INPUT_CONFIGURE_DIALOG_CALIBRATE_BUTTON,
   INPUT_CONFIGURE_DIALOG_SET_BUTTONS_LABEL,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_UP,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_DOWN,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_LEFT,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_RIGHT,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_START,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_SELECT,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_A,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_B,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_AUTO_1,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_AUTO_2,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_AUTO_3,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_AUTO_4,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_AUTO_5,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_AUTO_6,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_AUTO_7,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_AUTO_8,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_TURBO_1,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_TURBO_2,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_TURBO_3,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_TURBO_4,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_TURBO_5,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_TURBO_6,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_TURBO_7,
   INPUT_CONFIGURE_DIALOG_SET_BUTTON_TURBO_8,
   INPUT_CONFIGURE_DIALOG_SPLITTER,
   INPUT_CONFIGURE_DIALOG_ALLOW_CONFLICTS,
   INPUT_CONFIGURE_DIALOG_TOGGLED_AUTO,
   INPUT_CONFIGURE_DIALOG_MERGE_PLAYERS,
   INPUT_CONFIGURE_DIALOG_TURBO_LABEL,
   INPUT_CONFIGURE_DIALOG_TURBO,
   INPUT_CONFIGURE_DIALOG_SAVE_BUTTON
};

static const DIALOG netplay_dialog_base[] =
{
   { sl_frame,    0,   0,  153, 108, 0, 0, 0,   0,      0, 0, NULL,      "NetPlay", NULL },
   { sl_x_button, 133, 4,  16,  12,  0, 0, 0,   D_EXIT, 0, 0, "X",       NULL,      NULL },
   { sl_text,     9,   31, 0,   0,   0, 0, 0,   0,      0, 0, NULL,      "&Host:",  NULL },
   { sl_editbox,  40,  27, 104, 14,  0, 0, 'h', 0,      0, 0, NULL,      NULL,      NULL },
   { sl_text,     9,   49, 0,   0,   0, 0, 0,   0,      0, 0, NULL,      "&Port:",  NULL },
   { sl_editbox,  40,  45, 40,  14,  0, 0, 'p', 0,      0, 0, NULL,      NULL,      NULL },
   { sl_text,     84,  49, 0,   0,   0, 0, 0,   0,      0, 0, NULL,      "TCP",     NULL },
   { sl_text,     9,   69, 0,   0,   0, 0, 0,   0,      0, 0, NULL,      "&Nick:",  NULL },
   { sl_editbox,  40,  65, 88,  14,  0, 0, 'n', 0,      0, 0, NULL,      NULL,      NULL },
   { sl_button,   61,  86, 32,  16,  0, 0, 'o', D_EXIT, 0, 0, "&OK",     NULL,      NULL },
   { sl_button,   97,  86, 48,  16,  0, 0, 'c', D_EXIT, 0, 0, "&Cancel", NULL,      NULL },
   DIALOG_FRAME_ENDCAP
};

enum
{
   NETPLAY_DIALOG_FRAME = 0,
   NETPLAY_DIALOG_CLOSE_BUTTON,
   NETPLAY_DIALOG_HOST_LABEL,
   NETPLAY_DIALOG_HOST,
   NETPLAY_DIALOG_PORT_LABEL,
   NETPLAY_DIALOG_PORT,
   NETPLAY_DIALOG_TCP_TEXT,
   NETPLAY_DIALOG_NICK_LABEL,
   NETPLAY_DIALOG_NICK,
   NETPLAY_DIALOG_OK_BUTTON,
   NETPLAY_DIALOG_CANCEL_BUTTON
};

static int lobby_dialog_load (void);

static const DIALOG lobby_dialog_base[] =
{
   { sl_frame,          0,   0,   320, 240, 0, 0, 0,   D_DISABLED, 0, 0, NULL,         "Lobby",           NULL },
   { sl_x_button,       300, 4,   16,  12,  0, 0, 0,   D_EXIT,     0, 0, "X",          NULL,              NULL },
   { sl_viewer,         10,  27,  207, 168, 0, 0, 0,   0,          0, 0, NULL,         NULL,              NULL },
   { sl_viewer,         223, 27,  89,  89,  0, 0, 0,   0,          0, 0, NULL,         NULL,              NULL },
   { sl_text,           227, 124, 0,   0,   0, 0, 0,   0,          0, 0, NULL,         "Inputs:",         NULL },
   { sl_checkbox,       235, 133, 60,  8,   0, 0, '1', 0,          0, 0, "Player &1 ", NULL,              NULL },
   { sl_checkbox,       235, 142, 60,  8,   0, 0, '2', 0,          0, 0, "Player &2 ", NULL,              NULL },
   { sl_checkbox,       235, 151, 60,  8,   0, 0, '3', 0,          0, 0, "Player &3 ", NULL,              NULL },
   { sl_checkbox,       235, 160, 60,  8,   0, 0, '4', 0,          0, 0, "Player &4 ", NULL,              NULL },
   { d_shadow_box_proc, 10,  200, 302, 14,  0, 0, 0,   0,          0, 0, NULL,         NULL,              NULL },
   { sl_lobby_msgbox,   12,  204, 298, 12,  0, 0, 0,   0,          0, 0, NULL,         NULL,              NULL },
   { sl_button,         131, 219, 64,  16,  0, 0, 'l', 0,          0, 0, "&Load...",   lobby_dialog_load, NULL },
   { sl_button,         203, 219, 52,  16,  0, 0, 'o', D_EXIT,     0, 0, "&OK",        NULL,              NULL },
   { sl_button,         259, 219, 52,  16,  0, 0, 'c', D_EXIT,     0, 0, "&Cancel",    NULL,              NULL },
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

static const DIALOG help_keyboard_shortcuts_dialog_base[] =
{
    { sl_frame,    0,   0,   278, 166, 0, 0, 0,   0,      0, 0, NULL,    "Keyboard Shortcuts",        NULL },
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

static const DIALOG help_fakenes_team_dialog_base[] =
{
   { sl_frame,    0,   0,   157, 108, 0, 0, 0, 0,      0, 0, NULL, "FakeNES Team",                   NULL },
   { sl_x_button, 0,   0,   16,  12,  0, 0, 0, D_EXIT, 0, 0, "x",  NULL,                             NULL },
   { sl_text,     4,   4,   0 ,  0,   0, 0, 0, 0,      0, 0, NULL, "Current Developers:",            NULL },
   { sl_text,     20,  10,  0 ,  0,   0, 0, 0, 0,      0, 0, NULL, "Kaede, NishaKitty",              NULL },
   { sl_text,     4,   20,  0 ,  0,   0, 0, 0, 0,      0, 0, NULL, "Past Developers:",               NULL },
   { sl_text,     20,  26,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "randilyn, TRAC",                 NULL },
   { sl_text,     4,   36,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Contributors:",                  NULL },
   { sl_text,     20,  42,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "amit, Astxist, ipher, KittyCat", NULL },
   { sl_text,     20,  48,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Lord_Nightmare, RobotBebop",     NULL },
   { sl_text,     4,   58,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Special Thanks To:",             NULL },
   { sl_text,     20,  64,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "aprentice, kode54, loomsoft",    NULL },
   { sl_text,     20,  70,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Matthew Conte, Mexandrew",       NULL },
   { sl_text,     20,  76,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "pagefault, piinyouri, rOss",     NULL },
   { sl_text,     20,  82,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "sarencele, Takeda Toshiya",      NULL },
   { sl_text,     20,  88,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "Yano Takashi, xodnizel",         NULL },
   { sl_text,     4,   98,  0,   0,   0, 0, 0, 0,      0, 0, NULL, "... and many more!",             NULL },
   DIALOG_FRAME_ENDCAP
};

/* Dialog for configuration ntsc parameters. */
static const DIALOG ntsc_config_dialog_base[] =
{                                                           
   { sl_frame,      0,   0,   361, 234, 0, 0, 0,   0,      0,   0, NULL,              NULL,                 NULL },
   { sl_x_button,   342, 4,   16,  12,  0, 0, 0,   D_EXIT, 0,   0, "X",               NULL,                 NULL },
   { sl_text,       9,   28,  0,   0,   0, 0, 0,   0,      0,   0, NULL,              "&Hue:",              NULL },
   { d_slider_proc, 18,  37,  117, 5,   0, 0, 'h', 0,      200, 0, NULL,              NULL,                 NULL },
   { sl_text,       9,   49,  0,   0,   0, 0, 0,   0,      0,   0, NULL,              "Hue &Warping:",      NULL },
   { d_slider_proc, 18,  58,  117, 5,   0, 0, 'w', 0,      200, 0, NULL,              NULL,                 NULL },
   { sl_text,       9,   70,  0,   0,   0, 0, 0,   0,      0,   0, NULL,              "&Saturation:",       NULL },
   { d_slider_proc, 18,  79,  117, 5,   0, 0, 's', 0,      200, 0, NULL,               NULL,                NULL },
   { sl_text,       9,   91,  0,   0,   0, 0, 0,   0,      0,   0, NULL,              "&Brightness:",       NULL },
   { d_slider_proc, 18,  100, 117, 5,   0, 0, 'b', 0,      200, 0, NULL,              NULL,                 NULL },
   { sl_text,       9,   112, 0,   0,   0, 0, 0,   0,      0,   0, NULL,              "&Contrast:",         NULL },
   { d_slider_proc, 18,  121, 117, 5,   0, 0, 'c', 0,      200, 0, NULL,              NULL,                 NULL },
   { sl_text,       9,   133, 0,   0,   0, 0, 0,   0,      0,   0, NULL,              "&Gamma:",            NULL },
   { d_slider_proc, 18,  142, 117, 5,   0, 0, 'g', 0,      200, 0, NULL,              NULL,                 NULL },
   { sl_text,       183, 36,  0,   0,   0, 0, 0,   0,      0,   0, NULL,              "ADVANCED",           NULL },
   { sl_text,       144, 49,  0,   0,   0, 0, 0,   0,      0,   0, NULL,              "Sh&arpness:",        NULL },
   { d_slider_proc, 153, 58,  117, 5,   0, 0, 'a', 0,      200, 0, NULL,              NULL,                 NULL },
   { sl_text,       144, 70,  0,   0,   0, 0, 0,   0,      0,   0, NULL,              "&Resolution:",       NULL },
   { d_slider_proc, 153, 79,  117, 5,   0, 0, 'r', 0,      200, 0, NULL,              NULL,                 NULL },
   { sl_text,       144, 91,  0,   0,   0, 0, 0,   0,      0,   0, NULL,              "Ar&tifacts:",        NULL },
   { d_slider_proc, 153, 100, 117, 5,   0, 0, 't', 0,      200, 0, NULL,              NULL,                 NULL },
   { sl_text,       144, 112, 0,   0,   0, 0, 0,   0,      0,   0, NULL,              "C&olor Bleed:",      NULL },
   { d_slider_proc, 153, 121, 117, 5,   0, 0, 'o', 0,      200, 0, NULL,               NULL,                NULL },
   { sl_text,       144, 133, 0,   0,   0, 0, 0,   0,      0,   0, NULL,              "Color Fri&nging:",   NULL },
   { d_slider_proc, 153, 142, 117, 5,   0, 0, 'n', 0,      200, 0, NULL,              NULL,                 NULL },
   { sl_checkbox,   9,   157, 80,  6,   0, 0, 'e', 0,      0,   0, "R&educe Flicker", NULL,                 NULL },
   { sl_text,       9,   172, 0,   0,   0, 0, 0,   0,      0,   0, NULL,              "Scanline Doubling:", NULL },
   { sl_radiobox,   18,  181, 48,  6,   0, 0, 0,   0,      1,   0, "&Normal",         NULL,                 NULL },
   { sl_radiobox,   66,  181, 48,  6,   0, 0, 0,   0,      1,   0, "Brighten",        NULL,                 NULL },
   { sl_radiobox,   126, 181, 48,  6,   0, 0, 0,   0,      1,   0, "Darken",          NULL,                 NULL },
   { sl_checkbox,   18,  193, 64,  6,   0, 0, 'i', 0,      0,   0, "&Interpolated",   NULL,                 NULL },
   { sl_text,       279, 28,  0,   0,   0, 0, 0,   0,      0,   0, NULL,              "Presets:",           NULL },
   { sl_radiobox,   288, 40,  64,  8,   0, 0, 0,   0,      2,   0, "Default",         NULL,                 NULL },
   { sl_radiobox,   288, 52,  64,  8,   0, 0, 0,   0,      2,   0, "Composite",       NULL,                 NULL },
   { sl_radiobox,   288, 64,  64,  8,   0, 0, 0,   0,      2,   0, "S-Video",         NULL,                 NULL },
   { sl_radiobox,   288, 76,  64,  8,   0, 0, 0,   0,      2,   0, "RGB",             NULL,                 NULL },
   { sl_radiobox,   288, 88,  64,  8,   0, 0, 0,   0,      2,   0, "Monochrome",      NULL,                 NULL },
   { sl_button,     297, 103, 48,  16,  0, 0, 0,   D_EXIT, 0,   0, "Set",             NULL,                 NULL },
   { sl_button,     157, 211, 48,  16,  0, 0, 'v', D_EXIT, 0,   0, "Sa&ve",           NULL,                 NULL },
   DIALOG_FRAME_ENDCAP                                                               
};  

enum
{
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_1 = 0,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_2,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_3,
   NTSC_CONFIG_DIALOG_HUE,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_4,
   NTSC_CONFIG_DIALOG_HUE_WARPING,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_5,
   NTSC_CONFIG_DIALOG_SATURATION,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_6,
   NTSC_CONFIG_DIALOG_BRIGHTNESS,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_7,
   NTSC_CONFIG_DIALOG_CONTRAST,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_8,
   NTSC_CONFIG_DIALOG_GAMMA,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_9,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_10,
   NTSC_CONFIG_DIALOG_SHARPNESS,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_11,
   NTSC_CONFIG_DIALOG_RESOLUTION,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_12,
   NTSC_CONFIG_DIALOG_ARTIFACTS,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_13,
   NTSC_CONFIG_DIALOG_COLOR_BLEED,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_14,
   NTSC_CONFIG_DIALOG_COLOR_FRINGING,
   NTSC_CONFIG_DIALOG_REDUCE_FLICKER,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_15,
   NTSC_CONFIG_DIALOG_SCANLINE_DOUBLING_NORMAL,
   NTSC_CONFIG_DIALOG_SCANLINE_DOUBLING_BRIGHTEN,
   NTSC_CONFIG_DIALOG_SCANLINE_DOUBLING_DARKEN,
   NTSC_CONFIG_DIALOG_INTERPOLATED,
   NTSC_CONFIG_DIALOG_UNNAMED_OBJECT_16,
   NTSC_CONFIG_DIALOG_PRESETS_DEFAULT,
   NTSC_CONFIG_DIALOG_PRESETS_COMPOSITE,
   NTSC_CONFIG_DIALOG_PRESETS_SVIDEO,
   NTSC_CONFIG_DIALOG_PRESETS_RGB,
   NTSC_CONFIG_DIALOG_PRESETS_MONOCHROME,
   NTSC_CONFIG_DIALOG_SET_BUTTON,
   NTSC_CONFIG_DIALOG_SAVE_BUTTON
};

/* Undefine helper macros. */
#undef DEFINE_DIALOG
#undef DEFINE_DIALOG_CALLBACK
#undef DIALOG_REFRESH
#undef DIALOG_ENDCAP
#undef DIALOG_FRAME_ENDCAP
#undef IMPORT_MENU

static INLINE DIALOG *load_dialog (const DIALOG *dialog)
{
   DIALOG *new_dialog;
   int size = 0;
   int index = 0;
   int width, height;
   int title_height = 0, default_title_height = 0;

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
      // Width & height of reference font for normal text
      const float reference_width = 4.0f;
      const float reference_height = 5.0f;
      // Height of reference font for title bar text
      const float reference_title_height = 8.0f;

      /* Calculate the default titlebar height. */
      default_title_height = get_sl_frame_title_height() + reference_title_height;

      /* Import menu by reference. */
      if (object->proc == d_menu_proc)
         object->dp = *(MENU **)object->dp;

      /* Dialog to font scaling. */
      switch (index)
      {
         case 0: /* sl_frame. */
         {
            /* Set the titlebar font. */
            object->dp3 = video_get_font(VIDEO_FONT_LARGE);

            /* Calculate the scaled titlebar height. */
            title_height = get_sl_frame_title_height() + text_height(object->dp3);

            object->w = ROUND(((object->w / reference_width) * width));
            object->w += 2; /* borders */

            object->h = ROUND(((object->h / reference_height) * height));
            object->h += 1 + title_height; /* title_height already counts the top border */

            break;
         }
    
         case 1: /* sl_x_button. */
         {
            object->x = ROUND(((object->x / reference_width) * width));
            object->y = (ROUND(((object->y / reference_height) * height)) - height);
            object->w = ROUND(((object->w / reference_width) * width));
            object->h = ROUND(((object->h / reference_height) * height));

            break;
         }

         default:
         {
            object->x = ROUND(((object->x / reference_width) * width));
            object->y = (ROUND(((object->y / reference_height) * height)) - height);
            object->w = ROUND(((object->w / reference_width) * width));
            object->h = ROUND(((object->h / reference_height) * height));

            object->y += title_height;

            break;
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
   DIALOG_FROM_BASE(system_save_state_save_dialog);
   DIALOG_FROM_BASE(main_cheat_manager_add_dialog);
   DIALOG_FROM_BASE(main_cheat_manager_dialog);
   DIALOG_FROM_BASE(video_color_dialog);
   DIALOG_FROM_BASE(input_configure_dialog);
   DIALOG_FROM_BASE(options_paths_dialog);
   DIALOG_FROM_BASE(netplay_dialog);
   DIALOG_FROM_BASE(lobby_dialog);
   DIALOG_FROM_BASE(help_keyboard_shortcuts_dialog);
   DIALOG_FROM_BASE(help_fakenes_team_dialog);
}

#undef DIALOG_FROM_BASE

static INLINE void unload_dialogs (void)
{
   unload_dialog (main_dialog);
   unload_dialog (main_replay_record_start_dialog);
   unload_dialog (system_save_state_save_dialog);
   unload_dialog (main_cheat_manager_add_dialog);
   unload_dialog (main_cheat_manager_dialog);
   unload_dialog (video_color_dialog);
   unload_dialog (input_configure_dialog);
   unload_dialog (options_paths_dialog);
   unload_dialog (netplay_dialog);
   unload_dialog (lobby_dialog);
   unload_dialog (help_keyboard_shortcuts_dialog);
   unload_dialog (help_fakenes_team_dialog);
}

static INLINE int run_dialog (DIALOG *dialog, int focus)
{
   DIALOG_PLAYER *player;
   int index;

   /* Similar to Allegro's do_dialog().

      Although all custom dialogs handle the non-blocking setup of the GUI
      themselves (through sl_idle() from 'objects.h'), menus require
      init/update/shutdown_dialog() to be used in place of do_dialog().
      */

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
   INT16 x = x, y = y; /* Kill warnings. */
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

      if (((x <= (0 - dialog[0].w)) || (x >= bmp->w)) ||
          ((y <= (0 - dialog[0].h)) || (y >= bmp->h)))
      {
         /* Dialog is off the screen - center it instead. */
         centre_dialog (dialog);
      }
      else
      {
         /* Restore saved dialog position. */
         position_dialog (dialog, x, y);
      }
   }

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
