

static int gui_redraw_callback (int msg, DIALOG * d, int c);


static DIALOG main_dialog [4] =
{
    {         gui_menu_object, 4, 4, 0, 0,  0, 0, 0, 0, 0, 0, menu_bar, NULL, NULL },
    { gui_raised_label_object, 0, 0, 0, 0, 33, 0, 0, 0, 0, 0,     NULL,   "", NULL },
    {     gui_redraw_callback, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0,     NULL, NULL, NULL },
    {                    NULL, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0,     NULL, NULL, NULL }
};


static DIALOG help_about_dialog [10] =
{
    {       gui_window_object,  0,   0, 120, 134, 33, 18, 0, 0, 0, 3, NULL,              "About", NULL },
    { gui_raised_label_object,  9,  29,   0,   0, 17,  0, 0, 0, 0, 0, NULL,        "Developers:", NULL },
    { gui_raised_label_object, 17,  41,   0,   0, 33,  0, 0, 0, 0, 0, NULL,     "Randy McDowell", NULL },
    { gui_raised_label_object, 17,  53,   0,   0, 33,  0, 0, 0, 0, 0, NULL,          "Ian Smith", NULL },
    { gui_raised_label_object, 17,  65,   0,   0, 33,  0, 0, 0, 0, 0, NULL,    "Charles Bilyue'", NULL },
    { gui_raised_label_object, 17,  77,   0,   0, 33,  0, 0, 0, 0, 0, NULL, "Jonathan Gevaryahu", NULL },
    { gui_raised_label_object,  9,  93,   0,   0, 17,  0, 0, 0, 0, 0, NULL,      "Contributors:", NULL },
    { gui_raised_label_object, 17, 105,   0,   0, 33,  0, 0, 0, 0, 0, NULL,      "Chris Sheehon", NULL },
    { gui_raised_label_object, 17, 117,   0,   0, 33,  0, 0, 0, 0, 0, NULL,   "Amit Vainsencher", NULL },
    {                    NULL,  0,   0,   0,   0,  0,  0, 0, 0, 0, 0, NULL,                 NULL, NULL }
};
