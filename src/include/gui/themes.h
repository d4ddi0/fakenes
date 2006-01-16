enum
{
   GUI_THEME_CLASSIC = 0,
   GUI_THEME_STAINLESS_STEEL,
   GUI_THEME_ZERO_4,
   GUI_THEME_PANTA
};

static const GUI_THEME classic_theme =
{
   { 0.17f, 0.51f, 0.87f },   /* Gradients start. */
   { 0.17f, 0.51f, 0.87f },   /* Gradients end. */
   { 0,     0,     0     },   /* Background. */
   { 0.27f, 0.2f,  0.79f },   /* Fill. */
   { 0.17f, 0.51f, 0.87f },   /* Menu bar. */
   { 1.0f,  1.0f,  1.0f  },   /* Borders. */
   { 1.0f,  1.0f,  1.0f  },   /* Text. */
   { 1.0f,  1.0f,  1.0f  },   /* Light shadows. */
   { 0,     0,     0     },   /* Shadows. */
   { 0.27f, 0.2f,  0.79f },   /* Selected. */
   { 0.76f, 0.76f, 0.76f },   /* Disabled. */
   { 0.79f, 0.03f, 0.3f  }    /* Errors. */
};

static INLINE void set_classic_theme (void)
{
   gui_mouse_sprite = DATA_TO_BITMAP(GUI_CLASSIC_THEME_MOUSE_SPRITE);
   background_image = NULL;
   gui_image_palette = DATA_TO_RGB(GUI_CLASSIC_THEME_PALETTE);

   gui_theme_id = GUI_THEME_CLASSIC;
   gui_set_theme (&classic_theme);
}

static const GUI_THEME stainless_steel_theme =
{
   { 0.75f, 0.75f, 0.75f },   /* Gradients start. */
   { 0.25f, 0.25f, 0.25f },   /* Gradients end. */
   { 0,     0,     0     },   /* Background. */
   { 0.5f,  0.5f,  0.5f  },   /* Fill. */
   { 0.5f,  0.5f,  0.5f  },   /* Menu bar. */
   { 1.0f,  1.0f,  1.0f  },   /* Borders. */
   { 1.0f,  1.0f,  1.0f  },   /* Text. */
   { 0.5f,  0.5f,  0.5f  },   /* Light shadows. */
   { 0,     0,     0     },   /* Shadows. */
   { 0,     0,     0     },   /* Selected. */
   { 0.75f, 0.75f, 0.75f },   /* Disabled. */
   { 1.0f,  0.25f, 0.25f }    /* Errors. */
};

static INLINE void set_stainless_steel_theme (void)
{
   gui_mouse_sprite = DATA_TO_BITMAP(GUI_STAINLESS_STEEL_THEME_MOUSE_SPRITE);
   background_image = DATA_TO_BITMAP(GUI_STAINLESS_STEEL_THEME_BACKGROUND_IMAGE);
   gui_image_palette = DATA_TO_RGB(GUI_STAINLESS_STEEL_THEME_PALETTE);

   gui_theme_id = GUI_THEME_STAINLESS_STEEL;
   gui_set_theme (&stainless_steel_theme);
}

static const GUI_THEME zero_4_theme =
{
   { 0,    0.35f, 0.7f  },    /* Gradients start. */
   { 0,    0.1f,  0.2f  },    /* Gradients end. */
   { 0,    0.05f, 0.1f  },    /* Background. */
   { 0,    0.25f, 0.5f  },    /* Fill. */
   { 0,    0.25f, 0.5f  },    /* Menu bar. */
   { 0,    0.67f, 1.0f  },    /* Borders. */
   { 1.0f, 1.0f,  1.0f  },    /* Text. */
   { 0,    0.25f, 0.5f  },    /* Light shadows. */
   { 0,    0,     0     },    /* Shadows. */
   { 0,    0.1f,  0.2f  },    /* Selected. */
   { 0.5f, 0.67f, 0.75f },    /* Disabled. */
   { 1.0f, 0.25f, 0.25f }     /* Errors. */
};

static INLINE void set_zero_4_theme (void)
{
   gui_mouse_sprite = DATA_TO_BITMAP(GUI_ZERO_4_THEME_MOUSE_SPRITE);
   background_image = DATA_TO_BITMAP(GUI_ZERO_4_THEME_BACKGROUND_IMAGE);
   gui_image_palette = DATA_TO_RGB(GUI_ZERO_4_THEME_PALETTE);

   gui_theme_id = GUI_THEME_ZERO_4;
   gui_set_theme (&zero_4_theme);
}

static const GUI_THEME panta_theme =
{
   { 0,    0,     0    },  /* Gradients start. */
   { 0,    0.67f, 0    },  /* Gradients end. */
   { 0,    0.20f, 0    },  /* Background. */
   { 0,    0.33f, 0    },  /* Fill. */
   { 0,    0.33f, 0    },  /* Menu bar. */
   { 0,    0.85f, 0    },  /* Borders. */
   { 1.0f, 1.0f,  1.0f },  /* Text. */
   { 0,    0.25f, 0    },  /* Light shadows. */
   { 0,    0,     0    },  /* Shadows. */
   { 0,    0.5f,  0    },  /* Selected. */
   { 0,    0.4f,  0    },  /* Disabled. */
   { 1.0f, 1.0f,  0    }   /* Errors. */
};

static INLINE void set_panta_theme (void)
{
   gui_mouse_sprite = DATA_TO_BITMAP(GUI_ZERO_4_THEME_MOUSE_SPRITE);
   background_image = DATA_TO_BITMAP(GUI_ZERO_4_THEME_BACKGROUND_IMAGE);
   gui_image_palette = DATA_TO_RGB(GUI_ZERO_4_THEME_PALETTE);

   gui_theme_id = GUI_THEME_PANTA;
   gui_set_theme (&panta_theme);
}


static INLINE void set_theme (void)
{
   switch (gui_theme_id)
   {
      case GUI_THEME_CLASSIC:
      {
         set_classic_theme ();

         break;
      }

      case GUI_THEME_STAINLESS_STEEL:
      {
         set_stainless_steel_theme ();

         break;
      }

      case GUI_THEME_ZERO_4:
      {
         set_zero_4_theme ();

         break;
      }

      case GUI_THEME_PANTA:
      default:
      {
         set_panta_theme ();

         break;
      }
   }
}
