

#include "gui/dialogs.h"


static int file_menu_load_rom (void);

static int file_menu_exit (void);


static MENU file_menu [4] =
{
    { "&Load ROM...", file_menu_load_rom, NULL,          0, NULL },
    {             "",               NULL, NULL,          0, NULL },
    {        "E&xit",     file_menu_exit, NULL,          0, NULL },
    {           NULL,               NULL, NULL,          0, NULL }
};


static int machine_menu_reset (void);

static int machine_menu_status (void);


static MENU machine_menu [4] =
{
    {  "&Reset",  machine_menu_reset, NULL, 0, NULL },
    {        "",                NULL, NULL, 0, NULL },
    { "&Status", machine_menu_status, NULL, 0, NULL },
    {      NULL,                NULL, NULL, 0, NULL }
};


static int help_menu_about (void);


static MENU help_menu [2] =
{
    { "&About...", help_menu_about, NULL, 0, NULL },
    {        NULL,            NULL, NULL, 0, NULL }
};


static MENU menu_bar [5] =
{ 
    {    "&File", NULL,    file_menu, 0, NULL },
    { "&Machine", NULL, machine_menu, 0, NULL },
    {    "&Help", NULL,    help_menu, 0, NULL },
    {       NULL, NULL,         NULL, 0, NULL }
};


static DIALOG main_dialog [2] =
{
    { d_menu_proc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, menu_bar, NULL, NULL },
    {        NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     NULL, NULL, NULL }
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
