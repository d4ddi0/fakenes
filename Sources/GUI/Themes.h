enum
{
   /* Complete themes. */
   GUI_THEME_CLASSIC = 0,
   GUI_THEME_STAINLESS_STEEL,
   GUI_THEME_ZERO_4,
   GUI_THEME_PANTA,
   GUI_THEME_FIREFLOWER,

   /* Generic themes. */
   GUI_THEME_XODIAC,
   GUI_THEME_MONOCHROME,
   GUI_THEME_ESSENCE,
   GUI_THEME_VOODOO,
   GUI_THEME_HUGS_AND_KISSES,
};

static const GUI_THEME classic_theme =
{
   { 0.17f, 0.51f, 0.87f, 0 },   /* Gradients start. */
   { 0.17f, 0.51f, 0.87f, 0 },   /* Gradients end. */
   { 0,     0,     0,     0 },   /* Background. */
   { 0.27f, 0.2f,  0.79f, 0 },   /* Fill. */
   { 0.17f, 0.51f, 0.87f, 0 },   /* Menu bar. */
   { 1.0f,  1.0f,  1.0f,  0 },   /* Borders. */
   { 1.0f,  1.0f,  1.0f,  0 },   /* Text. */
   { 1.0f,  1.0f,  1.0f,  0 },   /* Light shadows. */
   { 0,     0,     0,     0 },   /* Shadows. */
   { 0.27f, 0.2f,  0.79f, 0 },   /* Selected. */
   { 0.76f, 0.76f, 0.76f, 0 },   /* Disabled. */
   { 0.79f, 0.03f, 0.3f,  0 }    /* Errors. */
};

static INLINE void set_classic_theme (void)
{
   gui_mouse_sprite = DATA_TO_BITMAP(CURSOR_THEME_CLASSIC);
   background_image = NULL;

   gui_theme_id = GUI_THEME_CLASSIC;
   gui_set_theme (&classic_theme);

   gui_theme_callback = NULL;
}

static const GUI_THEME stainless_steel_theme =
{
   { 0.25f, 0.25f, 0.25f, 0 },   /* Gradients start. */
   { 0.75f, 0.75f, 0.75f, 0 },   /* Gradients end. */
   { 0,     0,     0,     0 },   /* Background. */
   { 0.5f,  0.5f,  0.5f,  0 },   /* Fill. */
   { 0.5f,  0.5f,  0.5f,  0 },   /* Menu bar. */
   { 1.0f,  1.0f,  1.0f,  0 },   /* Borders. */
   { 1.0f,  1.0f,  1.0f,  0 },   /* Text. */
   { 0.5f,  0.5f,  0.5f,  0 },   /* Light shadows. */
   { 0,     0,     0,     0 },   /* Shadows. */
   { 0,     0,     0,     0 },   /* Selected. */
   { 0.75f, 0.75f, 0.75f, 0 },   /* Disabled. */
   { 1.0f,  0.25f, 0.25f, 0 }    /* Errors. */
};

static INLINE void set_stainless_steel_theme (void)
{
   gui_mouse_sprite = DATA_TO_BITMAP(CURSOR_THEME_STAINLESS_STEEL);
   background_image = DATA_TO_BITMAP(IMAGE_THEME_BACKGROUND_STAINLESS_STEEL);

   gui_theme_id = GUI_THEME_STAINLESS_STEEL;
   gui_set_theme (&stainless_steel_theme);

   gui_theme_callback = NULL;
}

static const GUI_THEME zero_4_theme =
{
   { 0,    0.1f,  0.2f,  0 },    /* Gradients start. */
   { 0,    0.35f, 0.7f,  0 },    /* Gradients end. */
   { 0,    0.05f, 0.1f,  0 },    /* Background. */
   { 0,    0.25f, 0.5f,  0 },    /* Fill. */
   { 0,    0.25f, 0.5f,  0 },    /* Menu bar. */
   { 0,    0.67f, 1.0f,  0 },    /* Borders. */
   { 1.0f, 1.0f,  1.0f,  0 },    /* Text. */
   { 0,    0.25f, 0.5f,  0 },    /* Light shadows. */
   { 0,    0,     0,     0 },    /* Shadows. */
   { 0,    0.1f,  0.2f,  0 },    /* Selected. */
   { 0.5f, 0.67f, 0.75f, 0 },    /* Disabled. */
   { 1.0f, 0.25f, 0.25f, 0 }     /* Errors. */
};

static INLINE void set_zero_4_theme (void)
{
   gui_mouse_sprite = DATA_TO_BITMAP(CURSOR_THEME_ZERO_4);
   background_image = DATA_TO_BITMAP(IMAGE_THEME_BACKGROUND_ZERO_4);

   gui_theme_id = GUI_THEME_ZERO_4;
   gui_set_theme (&zero_4_theme);

   gui_theme_callback = NULL;
}

static const GUI_THEME panta_theme =
{
   { 0,    0,     0,    0 },  /* Gradients start. */
   { 0,    0.67f, 0,    0 },  /* Gradients end. */
   { 0,    0.2f,  0,    0 },  /* Background. */
   { 0,    0.33f, 0,    0 },  /* Fill. */
   { 0,    0.33f, 0,    0 },  /* Menu bar. */
   { 0,    0.85f, 0,    0 },  /* Borders. */
   { 1.0f, 1.0f,  1.0f, 0 },  /* Text. */
   { 0,    0.25f, 0,    0 },  /* Light shadows. */
   { 0,    0,     0,    0 },  /* Shadows. */
   { 0,    0.5f,  0,    0 },  /* Selected. */
   { 0,    0.4f,  0,    0 },  /* Disabled. */
   { 1.0f, 1.0f,  0,    0 }   /* Errors. */
};

static INLINE void set_panta_theme (void)
{
   gui_mouse_sprite = DATA_TO_BITMAP(CURSOR_THEME_PANTA);
   background_image = DATA_TO_BITMAP(IMAGE_THEME_BACKGROUND_PANTA);

   gui_theme_id = GUI_THEME_PANTA;
   gui_set_theme (&panta_theme);

   gui_theme_callback = NULL;
}

static const GUI_THEME fireflower_theme =
{
   { 0,     0,       0,       0 },  /* Gradients start. */
   { 0.75f, 0.1875f, 0,       0 },  /* Gradients end. */
   { 0,     0,       0,       0 },  /* Background. */
   { 0,     0,       0,       0 },  /* Fill. */
   { 0.5f , 0.125f,  0,       0 },  /* Menu bar. */
   { 1.0f,  0.9f,    0.6f,    0 },  /* Borders. */
   { 1.0f,  0.9f,    0.6f,    0 },  /* Text. */
   { 0.25f, 0.0625f, 0,       0 },  /* Light shadows. */
   { 0,     0,       0,       0 },  /* Shadows. */
   { 0.75f, 0.1875f, 0,       0 },  /* Selected. */
   { 0.75f, 0.675f,  0.4f,    0 },  /* Disabled. */
   { 1.0f,  0,       0,       0 }   /* Errors. */
};

static int fireflower_cursor = 0;

static void fireflower_theme_callback(void)
{
   fireflower_cursor ^= 1;
   if(fireflower_cursor == 0)
      gui_mouse_sprite = DATA_TO_BITMAP(CURSOR_THEME_FIREFLOWER_FRAME_1);
   else
      gui_mouse_sprite = DATA_TO_BITMAP(CURSOR_THEME_FIREFLOWER_FRAME_2);
}

static INLINE void set_fireflower_theme (void)
{
   gui_mouse_sprite = DATA_TO_BITMAP(CURSOR_THEME_FIREFLOWER_FRAME_1);
   background_image = DATA_TO_BITMAP(IMAGE_THEME_BACKGROUND_FIREFLOWER);

   gui_theme_id = GUI_THEME_FIREFLOWER;
   gui_set_theme (&fireflower_theme);

   fireflower_cursor = 0;
   gui_theme_callback = fireflower_theme_callback;
}

#define set_default_theme()   set_panta_theme ()

/* Generic themes. */

static const GUI_THEME xodiac_theme =
{
   { 0.1f,  0.1f,  0.1f,  0 },   /* Gradients start. */
   { 0.67f, 0,     0,     0 },   /* Gradients end. */
   { 0,     0,     0,     0 },   /* Background. */
   { 0.1f,  0.1f,  0.1f,  0 },   /* Fill. */
   { 0.33f, 0,     0,     0 },   /* Menu bar. */
   { 0.45f, 0.45f, 0.45f, 0 },   /* Borders. */
   { 0.8f,  0.8f,  0.8f,  0 },   /* Text. */
   { 0.33f, 0,     0,     0 },   /* Light shadows. */
   { 0,     0,     0,     0 },   /* Shadows. */
   { 0,     0,     0,     0 },   /* Selected. */
   { 0.33f, 0,     0,     0 },   /* Disabled. */
   { 0.8f,  1.0f,  0,     0 }    /* Errors. */
};

static const GUI_THEME monochrome_theme =
{
   { 0,     0,     0,     0 },   /* Gradients start. */
   { 0,     0,     0,     0 },   /* Gradients end. */
   { 0,     0,     0,     0 },   /* Background. */
   { 0,     0,     0,     0 },   /* Fill. */
   { 0,     0,     0,     0 },   /* Menu bar. */
   { 1.0f,  1.0f,  1.0f,  0 },   /* Borders. */
   { 1.0f,  1.0f,  1.0f,  0 },   /* Text. */
   { 0,     0,     0,     0 },   /* Light shadows. */
   { 0,     0,     0,     0 },   /* Shadows. */
   { 0.25f, 0.25f, 0.25f, 0 },   /* Selected. */
   { 0.5f,  0.5f,  0.5f,  0 },   /* Disabled. */
   { 1.0f,  0,     0,     0 }    /* Errors. */
};

static const GUI_THEME essence_theme =
{
   { 0,     0.75f, 0.75f, 0 },   /* Gradients start. */
   { 0.25f, 0,     0.25f, 0 },   /* Gradients end. */
   { 0,     0,     0,     0 },   /* Background. */
   { 0,     0.5f,  0.5f,  0 },   /* Fill. */
   { 0.5f,  0,     0.5f,  0 },   /* Menu bar. */
   { 1.0f,  1.0f,  0.8f,  0 },   /* Borders. */
   { 1.0f,  1.0f,  1.0f,  0 },   /* Text. */
   { 0.25f, 0.25f, 0.1f,  0 },   /* Light shadows. */
   { 0,     0,     0,     0 },   /* Shadows. */
   { 0,     0,     0.1f,  0 },   /* Selected. */
   { 0.75f, 0.5f,  0.75f, 0 },   /* Disabled. */
   { 1.0f,  0.25f, 0,     0 }    /* Errors. */
};

static const GUI_THEME voodoo_theme =
{
   { 0,     0,     0,     0 },   /* Gradients start. */
   { 0.67f, 0.1f,  0.67f, 0 },   /* Gradients end. */
   { 0.05f, 0.01f, 0.05f, 0 },   /* Background. */
   { 0.33f, 0,     0.33f, 0 },   /* Fill. */
   { 0.33f, 0,     0.33f, 0 },   /* Menu bar. */
   { 0.85f, 0.2f,  0.85f, 0 },   /* Borders. */
   { 1.0f,  1.0f,  1.0f,  0 },   /* Text. */
   { 0.25f, 0,     0.25f, 0 },   /* Light shadows. */
   { 0,     0.1f,  0,     0 },   /* Shadows. */
   { 0.5f,  0.05f, 0.5f,  0 },   /* Selected. */
   { 0.67f, 0.33f, 0.67f, 0 },   /* Disabled. */
   { 1.0f,  0.2f,  0.67f, 0 }    /* Errors. */
};

static const GUI_THEME hugs_and_kisses_theme =
{
   { 0.25f, 0.47f, 0.47f, 0 },   /* Gradients start. */
   { 0.69f, 0.47f, 0.47f, 0 },   /* Gradients end. */
   { 0,     0,     0,     0 },   /* Background. */
   { 0.69f, 0.47f, 0.47f, 0 },   /* Fill. */
   { 0.69f, 0.47f, 0.47f, 0 },   /* Menu bar. */
   { 1.0f,  0.9f,  0.9f,  0 },   /* Borders. */
   { 1.0f,  1.0f,  1.0f,  0 },   /* Text. */
   { 1.0f,  0.9f,  0.9f,  0 },   /* Light shadows. */
   { 0,     0,     0,     0 },   /* Shadows. */
   { 0.47f, 0.69f, 0.69f, 0 },   /* Selected. */
   { 0.8f,  0.8f,  0.8f,  0 },   /* Disabled. */
   { 0.79f, 0,     0.2f,  0 }    /* Errors. */
};

#define GENERIC_THEME_SETTER(name, id) \
   static INLINE void set_##name##_theme (void) \
   {  \
      gui_mouse_sprite = DATA_TO_BITMAP(CURSOR_ARROW);   \
      background_image = NULL;   \
      gui_theme_id = id ;  \
      gui_set_theme (& name##_theme );  \
      gui_theme_callback = NULL; \
   }

GENERIC_THEME_SETTER(xodiac,          GUI_THEME_XODIAC)
GENERIC_THEME_SETTER(monochrome,      GUI_THEME_MONOCHROME)
GENERIC_THEME_SETTER(essence,         GUI_THEME_ESSENCE)
GENERIC_THEME_SETTER(voodoo,          GUI_THEME_VOODOO)
GENERIC_THEME_SETTER(hugs_and_kisses, GUI_THEME_HUGS_AND_KISSES)

#undef GENERIC_THEME_SETTER

/* --- */

#define THEME_SWITCH(id, name)   \
   case id :   \
   {  \
      set_##name##_theme ();  \
      break;   \
   }

static INLINE void set_theme (void)
{
   switch (gui_theme_id)
   {
      THEME_SWITCH(GUI_THEME_CLASSIC,         classic)
      THEME_SWITCH(GUI_THEME_STAINLESS_STEEL, stainless_steel)
      THEME_SWITCH(GUI_THEME_ZERO_4,          zero_4)
      THEME_SWITCH(GUI_THEME_PANTA,           panta)
      THEME_SWITCH(GUI_THEME_FIREFLOWER,      fireflower)
      THEME_SWITCH(GUI_THEME_XODIAC,          xodiac)
      THEME_SWITCH(GUI_THEME_MONOCHROME,      monochrome)
      THEME_SWITCH(GUI_THEME_ESSENCE,         essence)
      THEME_SWITCH(GUI_THEME_VOODOO,          voodoo)
      THEME_SWITCH(GUI_THEME_HUGS_AND_KISSES, hugs_and_kisses)

      default:
      {
         set_default_theme ();

         break;
      }
   }
}

#undef THEME_SWITCH
