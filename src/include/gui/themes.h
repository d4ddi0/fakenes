

static GUI_THEME classic_theme =
{
    { 0.17, 0.51, 0.87 }, /* Gradients start. */
    { 0.17, 0.51, 0.87 }, /* Gradients end. */
    {    0,    0,    0 }, /* Background. */
    { 0.27,  0.2, 0.79 }, /* Fill. */
    { 0.17, 0.51, 0.87 }, /* Menu bar. */
    {  1.0,  1.0,  1.0 }, /* Borders. */
    {  1.0,  1.0,  1.0 }, /* Text. */
    {  1.0,  1.0,  1.0 }, /* Light shadows. */
    {    0,    0,    0 }, /* Shadows. */
    { 0.27,  0.2, 0.79 }, /* Selected. */
    { 0.76, 0.76, 0.76 }, /* Disabled. */
    { 0.79, 0.03,  0.3 }  /* Errors. */
};


static INLINE void set_classic_theme (void)
{
    mouse_sprite = DATA_TO_BITMAP (GUI_CLASSIC_THEME_MOUSE_SPRITE);


    background_image = NIL;


    gui_image_palette = DATA_TO_RGB (GUI_CLASSIC_THEME_PALETTE);


    gui_set_theme (&classic_theme);
}


static GUI_THEME stainless_steel_theme =
{
    { 0.75, 0.75, 0.75 }, /* Gradients start. */
    { 0.25, 0.25, 0.25 }, /* Gradients end. */
    {    0,    0,    0 }, /* Background. */
    {  0.5,  0.5,  0.5 }, /* Fill. */
    {  0.5,  0.5,  0.5 }, /* Menu bar. */
    {  1.0,  1.0,  1.0 }, /* Borders. */
    {  1.0,  1.0,  1.0 }, /* Text. */
    {  0.5,  0.5,  0.5 }, /* Light shadows. */
    {    0,    0,    0 }, /* Shadows. */
    {    0,    0,    0 }, /* Selected. */
    { 0.75, 0.75, 0.75 }, /* Disabled. */
    {  1.0, 0.25, 0.25 }  /* Errors. */
};


static INLINE void set_stainless_steel_theme (void)
{
    mouse_sprite = DATA_TO_BITMAP (GUI_STAINLESS_STEEL_THEME_MOUSE_SPRITE);


    background_image = DATA_TO_BITMAP (GUI_STAINLESS_STEEL_THEME_BACKGROUND_IMAGE);


    gui_image_palette = DATA_TO_RGB (GUI_STAINLESS_STEEL_THEME_PALETTE);


    gui_set_theme (&stainless_steel_theme);
}


static GUI_THEME zero_4_theme =
{
    {    0, 0.35,  0.7 }, /* Gradients start. */
    {    0,  0.1,  0.2 }, /* Gradients end. */
    {    0, 0.05,  0.1 }, /* Background. */
    {    0, 0.25,  0.5 }, /* Fill. */
    {    0, 0.25,  0.5 }, /* Menu bar. */
    {    0, 0.67,  1.0 }, /* Borders. */
    {  1.0,  1.0,  1.0 }, /* Text. */
    {    0, 0.25,  0.5 }, /* Light shadows. */
    {    0,    0,    0 }, /* Shadows. */
    {    0,  0.1,  0.2 }, /* Selected. */
    {  0.5, 0.67, 0.75 }, /* Disabled. */
    {  1.0, 0.25, 0.25 }  /* Errors. */
};


static INLINE void set_zero_4_theme (void)
{
    mouse_sprite = DATA_TO_BITMAP (GUI_ZERO_4_THEME_MOUSE_SPRITE);


    background_image = DATA_TO_BITMAP (GUI_ZERO_4_THEME_BACKGROUND_IMAGE);


    gui_image_palette = DATA_TO_RGB (GUI_ZERO_4_THEME_PALETTE);


    gui_set_theme (&zero_4_theme);
}
