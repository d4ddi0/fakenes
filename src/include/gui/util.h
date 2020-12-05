static INLINE void pack_color (GUI_COLOR *color)
{
    int r, g, b;

    RT_ASSERT(color);

    r = (color->r * 255);
    g = (color->g * 255);
    b = (color->b * 255);

    color->packed = makecol (r, g, b);
}

static INLINE void update_colors (void)
{
   /* This function simply re-sets the last (current) theme to make sure all
      colors are correctly packed. */

   if (last_theme)
      gui_set_theme (last_theme);
}

static INLINE void redraw (void)
{
   /* This function redraws the current dialog. */

   broadcast_dialog_message (MSG_DRAW, 0);
}

/* If the GUI is not being drawn directly to the screen, this function
   displays it via video_update_display(). */
static INLINE void refresh (void)
{
   BITMAP* bitmap, *display;
   BITMAP* cursor = NULL;

   bitmap = gui_get_screen();
   if(bitmap == screen)
      return;

   display = video_get_display_buffer();
   if(!display) {
      WARN_GENERIC();
      return;
   }

   blit(bitmap, display, 0, 0, 0, 0, bitmap->w, bitmap->h);

   if(gui_mouse_sprite) {
      const int width = gui_mouse_sprite->w;
      const int height = gui_mouse_sprite->h;

      cursor = create_bitmap(width, height);
      if(cursor)
         blit(gui_mouse_sprite, cursor, 0, 0, 0, 0, width, height);
   }

   if(cursor) {
      int x, y;
      const int cursorX = mouse_x - mouse_x_focus;
      const int cursorY = mouse_y - mouse_y_focus;

      set_trans_blender(255, 255, 255, 128);
      drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);

      for(y = 0; y < cursor->h; y++) {
         for(x = 0; x < cursor->w; x++) {
            const int pixel = getpixel(cursor, x, y);
            const int r = getr(pixel);
            const int b = getb(pixel);
            const int g = getg(pixel);

            if((g == 0) && (r > 240) && (b > 240))
               continue;

            putpixel(display, cursorX + 8 + x, cursorY + 8 + y, makecol(0, 0, 0));
         }
      }

      solid_mode();

      draw_sprite(display, cursor, cursorX, cursorY);
   }

   /* If double buffering is not enabled, the display buffer won't be shown. */
   video_set_profile_boolean(VIDEO_PROFILE_DISPLAY_DOUBLE_BUFFER, TRUE);
   video_update_display();

   video_set_profile_boolean(VIDEO_PROFILE_DISPLAY_DOUBLE_BUFFER, FALSE);

   if(cursor)
      destroy_bitmap(cursor);
}

static INLINE void draw_message (int color)
{
   /* This function draws the message currenty present in message_buffer
      in the GUI status bar area. */

   BITMAP *bmp;
   int x1, y1, x2, y2;

   bmp = gui_get_screen ();

   x1 = 8;
   y1 = (((bmp->h - 8) - text_height (font)) - 8);
   x2 = (bmp->w - 8);
   y2 = (bmp->h - 8);

   vline (bmp, (x2 + 1), (y1 + 1), (y2 + 1), GUI_SHADOW_COLOR);
   hline (bmp, (x1 + 1), (y2 + 1), (x2 + 1), GUI_SHADOW_COLOR);

   rectfill (bmp, x1, y1, x2, y2, GUI_FILL_COLOR);
   rect (bmp, x1, y1, x2, y2, GUI_BORDER_COLOR);

   textout_centre_ex (bmp, font, message_buffer, (bmp->w / 2), ((bmp->h
      - 11) - text_height (font)), 0, -1);
   textout_centre_ex (bmp, font, message_buffer, ((bmp->w / 2) - 1),
      (((bmp->h - 11) - text_height (font)) - 1), color, -1);

   refresh ();
}

static INLINE void message_local (const UCHAR *message, ...)
{
   /* This is identical to gui_message(), except that it always uses the
      GUI text color and does not check if the GUI is active or not. */

   va_list format;

   RT_ASSERT(message);

   va_start (format, message);
   uvszprintf (message_buffer, USTRING_SIZE, message, format);
   va_end (format);

   draw_message (GUI_TEXT_COLOR);

   video_message (message_buffer);

   log_printf ("GUI: %s", message_buffer);}

static INLINE void status_message (const UCHAR *message, ...)
{
   /* Like message_local(), but does not log anywhere. */

   va_list format;

   RT_ASSERT(message);

   va_start (format, message);
   uvszprintf (message_buffer, USTRING_SIZE, message, format);
   va_end (format);

   draw_message (GUI_TEXT_COLOR);
}

static INLINE void draw_background (void)
{
   BITMAP *bmp;

   bmp = gui_get_screen ();

   rectfill (bmp, 0, 0, bmp->w, bmp->h, GUI_BACKGROUND_COLOR);

   if (background_image)
   {
      if (background_image->h < 200)
      {
         blit (background_image, bmp, 0, 0, ((bmp->w / 2) -
            (background_image->w / 2)), ((bmp->h / 2) - (background_image->h
               / 2)), background_image->w, background_image->h);
      }
      else
      {
         BITMAP *buffer;
   
         /* Hack to handle color conversion. */
   
         buffer = create_bitmap (background_image->w, background_image->h);
         if (!buffer)
         {
            WARN("Failed to create background buffer");
            return;
         }
   
         blit (background_image, buffer, 0, 0, 0, 0, background_image->w,
            background_image->h);
         stretch_blit (buffer, bmp, 0, 0, buffer->w, buffer->h, 0, 0,
            bmp->w, bmp->h);
   
         destroy_bitmap (buffer);
      }
   }

   if(rom_is_loaded) {
      /* draw_sprite() and masked_blit() don't support color conversion, pout. */
      BITMAP* render = video_get_render_buffer();
      if(render) {
         int gameWidth = render->w;
         int gameHeight = render->h;

         BITMAP* game = create_bitmap(gameWidth, gameHeight);
         if(game) {
            int width = (gameWidth * bmp->w) / 640;
            int height = (gameHeight * bmp->h) / 480;

            int x = (bmp->w / 2) - (width / 2);
            int y = (bmp->h / 2) - (height / 2);

            blit(render, game, 0, 0, 0, 0, gameWidth, gameHeight);
            stretch_blit(game, bmp, 0, 0, gameWidth, gameHeight, x, y, width, height);

            destroy_bitmap(game);
         }
      }
   }
}

static INLINE void cycle_audio (void)
{
   /* This function cycles (removes and reinstalls) the audio subsystem so
      that any major parameter changes can take effect. */

   audio_exit();

   if(audio_init() != 0) {
      gui_alert("Error initializing sound",
                "It looks like sound failed to initialize, so I'm disabling it for now.",
                NULL,
                NULL,
                "&OK", NULL, 0, 0);
      audio_options.enable_output = FALSE;
      audio_init();
   }

   apu_update();
}

static INLINE void cycle_video (void)
{
   BITMAP *bmp;

   /* This function fixes any problems with the GUI after a video change
      has taken effect (such as changing the screen color depth).  It also
      gets called each time you enter and exit the GUI. */

   bmp = gui_get_screen ();

   clear_bitmap (bmp);

   if (gui_is_active)
   {
      update_colors ();

      draw_background ();

      redraw ();

      if (gui_mouse_sprite)
         set_mouse_sprite (gui_mouse_sprite);
      set_mouse_sprite_focus (8, 8);

      /* Note that we only show the mouse here if the drawing bitmap is the
         screen; otherwise we let refresh() handle it. */
      if (bmp == screen)
         show_mouse (bmp);

      message_local ("%dx%d %d-bit, %s.", bmp->w, bmp->h, bitmap_color_depth
         (bmp), gfx_driver->name);
   }
   else
   {
      show_mouse (NULL);
      set_mouse_sprite_focus (0, 0);
   }

   refresh ();
}

static INLINE const UCHAR *get_enabled_text (BOOL value)
{
   /* This simple function returns either "enabled" or "disabled", depending
      on the value of the boolean parameter 'value'. */

   return ((value ? "enabled" : "disabled"));
}

static INLINE const UCHAR *get_enabled_text_ex (BOOL value, const UCHAR
   *enabled_text)
{
   /* Identical to the above function, except that it returns 'enabled_text'
      instead of "enabled". */

   RT_ASSERT(enabled_text);

   return ((value ? enabled_text : "disabled"));
}

static INLINE int gui_open (void)
{
   /* Helper function for show_gui() and gui_alert().  Enters the GUI
      (e.g, sets up display buffer, etc.) but doesn't do anything else. */

   /* Pause audio. */
   audio_suspend ();

   /* Suspend timers. */
   suspend_timing ();

   gui_is_active = TRUE;

   /* In the past we only double buffered in OpenGL mode, but now we
      always do it to fix some issues related to the horrible
      performance of screen reads/writes. */

   /* Create drawing buffer. */
   gui_buffer = create_bitmap(SCREEN_W, SCREEN_H);
   if (!gui_buffer)
   {
      WARN("Couldn't create GUI drawing buffer");
      return (1);
   }
   
   /* Make Allegro use it. */
   gui_set_screen (gui_buffer);

   cycle_video ();

   /* Return success. */
   return (0);
}

static INLINE void gui_close (BOOL exiting)
{
   if (gui_buffer)
   {
      /* Destroy and nullify drawing buffer. */
      destroy_bitmap(gui_buffer);
      gui_buffer = NULL;

      /* Restore screen. */
      gui_set_screen (screen);

      /* Disable double buffering. */
      video_set_profile_boolean(VIDEO_PROFILE_DISPLAY_DOUBLE_BUFFER, FALSE);
   }

   /* Deactivate. */
   gui_is_active = FALSE;

   if (!exiting)
   {
      cycle_video ();

      /* Restart timers. */
      resume_timing ();
      
      /* Unpause audio. */
      audio_resume ();
   }
}

static INLINE DIALOG *create_dialog (const DIALOG *base, const UCHAR *title)
{
   /* Abstract function to create a new dialog of class 'base', and set it's
      title to 'title'.  The resulting dialog must be later destroyed by a
      call to unload_dialog(). */

   DIALOG *dialog;

   RT_ASSERT(base);
   RT_ASSERT(title);

   /* Create dialog. */
   dialog = load_dialog (base);
   if (!dialog)
   {
      WARN("Failed to create dialog structure");
      return (NULL);
   }

   dialog[0].dp2 = (char *)title;

   return (dialog);
}

static INLINE BOOL get_resolution_input (const UCHAR *title, int *width, int
   *height)
{
   /* Pops up an abstract input dialog that allows the user to enter a
      custom resolution (in pixels).

      Returns FALSE if the dialog was cancelled, otherwise TRUE. */

   DIALOG *dialog;
   DIALOG *objwidth;
   DIALOG *objheight;
   USTRING widthstr, heightstr;
   int result;

   RT_ASSERT(title);
   RT_ASSERT(width);
   RT_ASSERT(height);

   /* Create dialog. */
   dialog = create_dialog (resolution_dialog_base, title);
   if (!dialog)
      return (FALSE);

   /* Get objects. */

   objwidth = &dialog[RESOLUTION_DIALOG_WIDTH];
   objheight = &dialog[RESOLUTION_DIALOG_HEIGHT];

   /* Set up objects. */

   uszprintf (widthstr, sizeof (widthstr), "%d", *width);
   objwidth->d1 = ((sizeof (widthstr) / MAX_UCHAR_LENGTH) - 1);
   objwidth->dp = widthstr;

   uszprintf (heightstr, sizeof (heightstr), "%d", *height);
   objheight->d1 = ((sizeof (heightstr) / MAX_UCHAR_LENGTH) - 1);
   objheight->dp = heightstr;

   /* Show dialog. */
   result = show_dialog (dialog, -1);

   /* Destroy dialog. */
   unload_dialog (dialog);

   if (result != RESOLUTION_DIALOG_OK_BUTTON)
      return (FALSE);

   *width  = ROUND(uatof (widthstr));
   *height = ROUND(uatof (heightstr));

   /* Return success. */
   return (TRUE);
}

static INLINE BOOL get_float_input (const UCHAR *title, REAL *value, const
   UCHAR *units)
{
   /* Pops up an abstract input dialog that allows the user to enter a
      floating point number specified in 'units' (e.g, frames, dots, etc.).

      Returns FALSE if the dialog was cancelled, otherwise TRUE. */

   DIALOG *dialog;
   DIALOG *objvalue;
   DIALOG *objunits;
   USTRING valuestr;
   int result;

   /* Create dialog. */
   dialog = create_dialog (amount_dialog_base, title);
   if (!dialog)
      return (FALSE);

   /* Get objects. */

   objvalue = &dialog[AMOUNT_DIALOG_VALUE];
   objunits = &dialog[AMOUNT_DIALOG_UNITS_LABEL];

   /* Set up objects. */

   uszprintf (valuestr, sizeof (valuestr), "%g", *value);
   objvalue->d1 = ((sizeof (valuestr) / MAX_UCHAR_LENGTH) - 1);
   objvalue->dp = valuestr;

   objunits->dp2 = (char *)units;

   /* Show dialog. */
   result = show_dialog (dialog, -1);

   /* Destroy dialog. */
   unload_dialog (dialog);

   if (result != AMOUNT_DIALOG_OK_BUTTON)
      return (FALSE);

   *value = uatof (valuestr);

   /* Return success. */
   return (TRUE);
}

static INLINE BOOL get_integer_input (const UCHAR *title, int *value, const
   UCHAR *units)
{
   /* Same as above, but rounds the value off to an integer. */

   REAL fvalue;

   fvalue = *value;

   if (get_float_input (title, &fvalue, units))
   {
      *value = ROUND(fvalue);

      return (TRUE);
   }

   return (FALSE);
}
