

static int gui_redraw_callback (int msg, DIALOG * d, int c);


static DIALOG main_dialog [] =
{
    {     gui_menu_object, 4, 4, 0, 0,  0, 0, 0, 0, 0, 0, menu_bar, NULL, NULL },
    {    gui_label_object, 0, 0, 0, 0, 33, 0, 0, 0, 0, 0,     NULL,   "", NULL },
    { gui_redraw_callback, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0,     NULL, NULL, NULL },
    {                NULL, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0,     NULL, NULL, NULL }
};


static DIALOG help_shortcuts_dialog [] =
{
    { gui_window_object, 0,   0, 134, 149, 33, 18, 0, 0, 0, 3, NULL,               "Shortcuts", NULL },
    {  gui_label_object, 9,  29,   0,   0, 33,  0, 0, 0, 0, 0, NULL,     "F1  - Save Snapshot", NULL },
    {  gui_label_object, 9,  41,   0,   0, 33,  0, 0, 0, 0, 0, NULL,     "F2  - Toggle Status", NULL },
    {  gui_label_object, 9,  53,   0,   0, 17,  0, 0, 0, 0, 0, NULL,        "F3  - Save State", NULL },
    {  gui_label_object, 9,  65,   0,   0, 17,  0, 0, 0, 0, 0, NULL,     "F4  - Restore State", NULL },
    {  gui_label_object, 9,  81,   0,   0, 33,  0, 0, 0, 0, 0, NULL,    "F5  - Flip Mirroring", NULL },
    {  gui_label_object, 9,  93,   0,   0, 33,  0, 0, 0, 0, 0, NULL,     "F6  - Toggle Zapper", NULL },
    {  gui_label_object, 9, 105,   0,   0, 33,  0, 0, 0, 0, 0, NULL,    "F7  - Toggle Sprites", NULL },
    {  gui_label_object, 9, 117,   0,   0, 33,  0, 0, 0, 0, 0, NULL, "F8  - Toggle Background", NULL },
    {  gui_label_object, 9, 133,   0,   0, 33,  0, 0, 0, 0, 0, NULL,    "ESC - Enter/Exit GUI", NULL },
    {              NULL, 0,   0,   0,   0,  0,  0, 0, 0, 0, 0, NULL,                      NULL, NULL }
};


static DIALOG help_about_dialog [] =
{
    { gui_window_object,  0,   0, 120, 134, 33, 18, 0, 0, 0, 3, NULL,              "About", NULL },
    {  gui_label_object,  9,  29,   0,   0, 17,  0, 0, 0, 0, 0, NULL,        "Developers:", NULL },
    {  gui_label_object, 17,  41,   0,   0, 33,  0, 0, 0, 0, 0, NULL,     "Randy McDowell", NULL },
    {  gui_label_object, 17,  53,   0,   0, 33,  0, 0, 0, 0, 0, NULL,          "Ian Smith", NULL },
    {  gui_label_object, 17,  65,   0,   0, 33,  0, 0, 0, 0, 0, NULL,    "Charles Bilyue'", NULL },
    {  gui_label_object, 17,  77,   0,   0, 33,  0, 0, 0, 0, 0, NULL, "Jonathan Gevaryahu", NULL },
    {  gui_label_object,  9,  93,   0,   0, 17,  0, 0, 0, 0, 0, NULL,      "Contributors:", NULL },
    {  gui_label_object, 17, 105,   0,   0, 33,  0, 0, 0, 0, 0, NULL,      "Chris Sheehon", NULL },
    {  gui_label_object, 17, 117,   0,   0, 33,  0, 0, 0, 0, 0, NULL,   "Amit Vainsencher", NULL },
    {              NULL,  0,   0,   0,   0,  0,  0, 0, 0, 0, 0, NULL,                 NULL, NULL }
};
