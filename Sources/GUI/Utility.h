static INLINE void draw_background (BITMAP* bmp, int w, int h);
static INLINE void draw_status_bar(BITMAP* bmp, int w, int h);

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

static INLINE void draw_game_border(BITMAP* bmp)
{
   const int x1 = gui_game_x - 1;
   const int y1 = gui_game_y - 1;
   const int x2 = x1 + (gui_game_width - 1) + 2;
   const int y2 = y1 + (gui_game_height - 1) + 2;
   const int black = makecol(0, 0, 0);
   BITMAP* pattern = NULL;

   pattern = create_bitmap(2, 2);
   if(pattern) {
      clear_to_color(pattern, bitmap_mask_color(pattern));
      putpixel(pattern, 0, 0, black);
      putpixel(pattern, 1, 1, black);

      drawing_mode(DRAW_MODE_MASKED_PATTERN, pattern, 0, 0);
   }

   /* Shadow. */
   rectfill(bmp, x1 + 8, y1 + 8, x2 + 8, y2 + 8, black);

   if(pattern) {
      solid_mode();
      destroy_bitmap(pattern);
   }

   /* Background. */
   rectfill(bmp, x1, y1, x2, y2, black);
   /* Frame. */
   rect(bmp, x1, y1, x2, y2, GUI_BORDER_COLOR);
}

/* If the GUI is not being drawn directly to the screen, this function
   displays it via video_update_display(). */
static INLINE void update_display (void) 
{
   BITMAP* bitmap, *display;
   BITMAP* cursor = NULL;
   int window_x, window_y, window_w, window_h;

   bitmap = gui_get_screen();

   display = video_get_display_buffer();
   if(!display) {
      WARN_GENERIC();
      return;
   }

   /* This isn't needed since draw_background() fills the display. */
   /* clear_bitmap(display); */

   window_w = bitmap->w;
   window_h = bitmap->h;
   window_x = (display->w / 2) - (window_w / 2);
   window_y = (display->h / 2) - (window_h / 2);
  
   gui_mouse_x_position = window_x + (mouse_x - mouse_x_focus);
   gui_mouse_y_position = window_y + (mouse_y - mouse_y_focus);

   draw_background(display, window_w, window_h);

   draw_status_bar(display, window_w, window_h);

   if(file_is_loaded) {
      video_update_game_display();

      BITMAP* render = video_get_render_buffer();

      if(render) {
         const int gameWidth = render->w;
         const int gameHeight = render->h;

         /* Only scale if neccesary. */
         if((window_w != 640) || (window_h != 480)) {
            /* draw_sprite() and masked_blit() don't support color conversion, pout. */

            BITMAP* game = create_bitmap(gameWidth, gameHeight);
            if(game) {
               const int width = (gameWidth * window_w) / 640;
               const int height = (gameHeight * window_h) / 480;

               const int x = window_x + ((window_w / 2) - (width / 2));
               const int y = window_y + ((window_h / 2) - (height / 2));

               gui_game_x = x;
               gui_game_y = y;
               gui_game_width = width;
               gui_game_height = height;

               if(!background_image)
                  draw_game_border(display);
               
               blit(render, game, 0, 0, 0, 0, gameWidth, gameHeight);
               stretch_blit(game, display, 0, 0, gameWidth, gameHeight, x, y, width, height);
  
               destroy_bitmap(game);
            }
         }
         else {
            const int x = window_x + ((window_w / 2) - (gameWidth / 2));
            const int y = window_y + ((window_h / 2) - (gameHeight / 2));

            gui_game_x = x;
            gui_game_y = y;
            gui_game_width = gameWidth;
            gui_game_height = gameHeight;
 
            if(!background_image)
               draw_game_border(display);

            blit(render, display, 0, 0, x, y, gameWidth, gameHeight);
         }
      }
   }

   masked_blit(bitmap, display, 0, 0, window_x, window_y, bitmap->w, bitmap->h);

   if(window_y >= 16) {
      FONT* font = video_get_font(VIDEO_FONT_LEGACY);
      textout_ex(display, font, "Note: This is not a 4:3 resolution, so the GUI will be letterboxed. "
         "Gameplay will not be affected.", 8, 8, makecol(255, 255, 255), -1);
   }

   if(gui_mouse_sprite) {
      const int width = gui_mouse_sprite->w;
      const int height = gui_mouse_sprite->h;

      cursor = create_bitmap(width, height);
      if(cursor)
         blit(gui_mouse_sprite, cursor, 0, 0, 0, 0, width, height);
   }

   if(cursor) {
      int sx, sy;
      BOOL truecolor = FALSE;

      if(bitmap_color_depth(display) > 8)
         truecolor = TRUE;

      for(sy = 0; sy < cursor->h; sy++) {
         for(sx = 0; sx < cursor->w; sx++) {
            int sub_x, sub_y;

            const int pixel = getpixel(cursor, sx, sy);
            if(pixel == bitmap_mask_color(cursor))
               continue;

            /* Patterned drawing. */
            sub_x = sx & 1;
            sub_y = sy & 1;
            if(((sub_x == 0) && (sub_y == 1)) ||
               ((sub_x == 1) && (sub_y == 0)))
               continue;

            putpixel(display, gui_mouse_x_position + 8 + sx, gui_mouse_y_position + 8 + sy, makecol(0, 0, 0));
         }
      }

      draw_sprite(display, cursor, gui_mouse_x_position, gui_mouse_y_position);
   }

   /* If double buffering is not enabled, the display buffer won't be shown. */
   video_set_profile_boolean(VIDEO_PROFILE_DISPLAY_DOUBLE_BUFFER, TRUE);
   video_update_display();

   video_set_profile_boolean(VIDEO_PROFILE_DISPLAY_DOUBLE_BUFFER, FALSE);

   if(cursor)
      destroy_bitmap(cursor);
}

static INLINE void refresh (void)
{
   if(file_is_loaded) {
      /* Emulation loop. */
      machine_main();
   }
  
   if(gui_theme_callback)
      gui_theme_callback();

   /* When a game is loaded, we let it take care of updating the display by 
      calling gui_update_display() instead. We also avoid resting as it will
      mess up main timing (which has its own rests). */
   if(!file_is_loaded) {
      if (cpu_usage == CPU_USAGE_PASSIVE)
         rest (1);
      else if (cpu_usage == CPU_USAGE_NORMAL)
         rest (0);

      update_display();
  }
}

static INLINE void add_message(const UDATA* message)
{
   int i;

   RT_ASSERT(message);

   for(i = 0; i < GUI_MESSAGE_HISTORY_SIZE; i++)
      ustrzcpy(&gui_message_history[i][0], USTRING_SIZE, &gui_message_history[i + 1][0]);

   ustrzcpy(&gui_message_history[GUI_MESSAGE_HISTORY_SIZE - 1][0], USTRING_SIZE, message);

   gui_last_message++;
   if(gui_last_message > GUI_MESSAGE_HISTORY_SIZE)
      gui_last_message = GUI_MESSAGE_HISTORY_SIZE;
}

static INLINE void draw_status_bar(BITMAP* bmp, int w, int h)
{
   int window_x, window_y, window_w, window_h;
   int x1, y1, x2, y2;
   const UDATA* message;
   int color;

   /* bmp = gui_get_screen (); */
   RT_ASSERT(bmp);

   window_x = (bmp->w / 2) - (w / 2);
   window_y = (bmp->h / 2) - (h / 2);
   window_w = w;
   window_h = h;

   x1 = window_x + 8;
   y1 = (window_y + (window_h - 1)) - (text_height(font) + 16);
   x2 = (x1 +( window_w - 1)) - 16;
   y2 = y1 + text_height(font) + 8;

   message = gui_status_text;
   color = gui_status_color;

   vline (bmp, (x2 + 1), (y1 + 1), (y2 + 1), GUI_SHADOW_COLOR);
   hline (bmp, (x1 + 1), (y2 + 1), (x2 + 1), GUI_SHADOW_COLOR);

   rectfill (bmp, x1, y1, x2, y2, GUI_FILL_COLOR);
   rect (bmp, x1, y1, x2, y2, GUI_BORDER_COLOR);

   textout_centre_ex (bmp, font, message, window_x + (window_w / 2), window_y + ((window_h
      - 11) - text_height (font)), 0, -1);
   textout_centre_ex (bmp, font, message, window_x + ((window_w / 2) - 1),
      window_y + (((window_h - 11) - text_height (font)) - 1), color, -1);
}

static INLINE void set_status_text (const UDATA* text, int color)
{
   /* This function places a message in the GUI status bar area.
      No actual drawing is done here, draw_status_bar() handles that. */

   USTRING_CLEAR(gui_status_text);
   ustrncat(gui_status_text, text, sizeof(gui_status_text) - 1);

   gui_status_color = color;

   if(!gui_is_active)
      video_message(-1, text);
}

static INLINE void status_text (const UDATA *text, ...)
{
   /* This is identical to gui_message(), except that it does not
      check if the GUI is active or not. */

   va_list format;
   USTRING buffer;

   RT_ASSERT(text);

   va_start (format, text);
   uvszprintf (buffer, USTRING_SIZE, text, format);
   va_end (format);

   set_status_text(buffer, GUI_TEXT_COLOR);
}

static INLINE void status_text_color (int color, const UDATA *text, ...)
{
   /* Like status_text(), but allows a color to be specified. */

   va_list format;
   USTRING buffer;

   RT_ASSERT(text);

   va_start (format, text);
   uvszprintf (buffer, USTRING_SIZE, text, format);
   va_end (format);

   set_status_text(buffer, color);
}

static INLINE void draw_background (BITMAP* bmp, int w, int h)
{
   int window_x, window_y, window_w, window_h;
   int count, start, offset_x, offset_y, i, line;
   BOOL cleared = FALSE;

   window_x = (bmp->w / 2) - (w / 2);
   window_y = (bmp->h / 2) - (h / 2);
   window_w = w;
   window_h = h;


   if((window_w != bmp->w) || (window_h != bmp->h)) {
      rectfill (bmp, 0, 0, bmp->w, bmp->h, GUI_BACKGROUND_COLOR);
      cleared = TRUE;
   }

   if(!background_image) {
      const int x1 = window_x;
      const int y1 = window_y;
      const int x2 = window_x + (window_w - 1);
      const int y2 = window_y + (window_h - 1);

      if (GUI_GRADIENT_START_COLOR == GUI_GRADIENT_END_COLOR) {
         /* Accelerated solid drawing. */
         rectfill (bmp, x1, y1, x2, y2, GUI_GRADIENT_START_COLOR);
      }
      else {
         /* Gradient fill. */
         int slice;

         /* We have to do this in big chunks to make it fast. */
         video_legacy_create_gradient (GUI_GRADIENT_START_COLOR,
            GUI_GRADIENT_END_COLOR, window_h / 24, 0, 0);

         for(slice = 0; slice < (window_h / 24); slice++) {
             int bottom = y1 + (slice * 24) + 24;
             if(bottom > y2)
                bottom = y2;

             const int color = video_legacy_create_gradient (0, 0, 0, slice, 0);
             rectfill(bmp, x1, y1 + (slice * 24), x2, bottom, color);
         }
      }
   }

   if (background_image)
   {
      if( (background_image->w == window_w) && (background_image->h == window_h) )
      { 
         blit(background_image, bmp, 0, 0, window_x, window_y, background_image->w, background_image->h);
      }
      else if (background_image->h < 240)
      {
         if(!cleared) {
            rectfill (bmp, 0, 0, bmp->w, bmp->h, GUI_BACKGROUND_COLOR);
            cleared = TRUE;
         }

         blit (background_image, bmp, 0, 0, window_x + ((window_w / 2) -
            (background_image->w / 2)), window_y + ((window_h / 2) - (background_image->h
               / 2)), background_image->w, background_image->h);
      }
      else
      {
         BITMAP *buffer;
   
         /* Hack to handle color conversion. */
   
         buffer = create_bitmap_ex(bitmap_color_depth(background_image), window_w, window_h);
         if (!buffer)
         {
            WARN("Failed to create background buffer");
            return;
         }
   
         stretch_blit (background_image, buffer,
            0, 0, background_image->w, background_image->h,
            0, 0, buffer->w, buffer->h);
         blit (buffer, bmp, 0, 0, window_x, window_y, buffer->w, buffer->h);
   
         destroy_bitmap (buffer);
      }
   }

   if(window_h >= 480) {
      /* Draw the message history in the smallest possible font. */
      FONT* font = video_get_font(VIDEO_FONT_SMALLEST);

      /* 7 lines is about right for 480 pixel height */
      count = (7 * window_h) / 480;
      if(count > (GUI_MESSAGE_HISTORY_SIZE - 1))
         count = GUI_MESSAGE_HISTORY_SIZE - 1;

      start = (GUI_MESSAGE_HISTORY_SIZE - 1) - count;

      offset_x = window_x + 16;
      offset_y = window_y + 40;

      line = 0;
      for(i = (start - 1); i < (start + count); i++) {
         textout_ex(bmp, font, gui_message_history[i], offset_x, offset_y + line, GUI_TEXT_COLOR, -1);
         line += text_height(font) + 3;
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

   clear_to_color (bmp, bitmap_mask_color(bmp));

   update_colors ();

   redraw ();

   if (gui_mouse_sprite) {
      set_mouse_sprite (gui_mouse_sprite);
      set_mouse_sprite_focus (8, 8);
   }

   if(gui_last_message == 0) {
      status_text ("%dx%d %d-bit, %s.", bmp->w, bmp->h, bitmap_color_depth
         (bmp), gfx_driver->name);
   }
   else
      status_text (gui_message_history[gui_last_message - 1]);

   refresh ();
}

static INLINE const UDATA *get_enabled_text (BOOL value)
{
   /* This simple function returns either "enabled" or "disabled", depending
      on the value of the boolean parameter 'value'. */

   return ((value ? "enabled" : "disabled"));
}

static INLINE const UDATA *get_enabled_text_ex (BOOL value, const UDATA
   *enabled_text)
{
   /* Identical to the above function, except that it returns 'enabled_text'
      instead of "enabled". */

   RT_ASSERT(enabled_text);

   return ((value ? enabled_text : "disabled"));
}

static INLINE int gui_open (void)
{
   int x, y, width, height;

   /* Clear keyboard buffer. This prevents some annoyances. */
   clear_keybuf();

   /* Helper function for show_gui() and gui_alert().  Enters the GUI
      (e.g, sets up display buffer, etc.) but doesn't do anything else. */
 
   gui_is_active = TRUE;

   /* In the past we only double buffered in OpenGL mode, but now we
      always do it to fix some issues related to the horrible
      performance of screen reads/writes. */

   /* Always use a 4:3 buffer. */
   width = SCREEN_W;
   height = (SCREEN_W * 3) / 4;

   if(height > SCREEN_H) {
      height = SCREEN_H;
      width = (SCREEN_H * 4) / 3;
   }

   /* Create drawing buffer. */
   gui_buffer = create_bitmap(width, height);
   if (!gui_buffer)
   {
      WARN("Couldn't create GUI drawing buffer");
      return (1);
   }
   
   /* Make Allegro use it. */
   gui_set_screen (gui_buffer);

   /* Do an initial video update. */
   cycle_video ();

   /* Return success. */
   return (0);
}

static INLINE void gui_close (void)
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

      /* Clear screen. */
      clear_bitmap(screen);
   }

   /* Deactivate. */
   gui_is_active = FALSE;

   /* Clear keyboard buffer. This prevents some annoyances. */
   clear_keybuf();
}

static INLINE DIALOG *create_dialog (const DIALOG *base, const UDATA *title)
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

static INLINE BOOL get_resolution_input (const UDATA *title, int *width, int
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
   objwidth->d1 = ((sizeof (widthstr) / MAX_UTF8_SEGMENTS) - 1);
   objwidth->dp = widthstr;

   uszprintf (heightstr, sizeof (heightstr), "%d", *height);
   objheight->d1 = ((sizeof (heightstr) / MAX_UTF8_SEGMENTS) - 1);
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

static INLINE BOOL get_float_input (const UDATA *title, REAL *value, const
   UDATA *units)
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
   objvalue->d1 = ((sizeof (valuestr) / MAX_UTF8_SEGMENTS) - 1);
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

static INLINE BOOL get_integer_input (const UDATA *title, int *value, const
   UDATA *units)
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
