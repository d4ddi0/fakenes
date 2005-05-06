

int sl_text (int message, DIALOG * dialog, int key)
{
    int x;

    int y;


    FONT * old_font = NIL;


    /* dp3 = font. */

    if (dialog -> dp3)
    {
        old_font = font;


        font = dialog -> dp3;
    }


    switch (message)
    {
        case MSG_DRAW:

            /* dp = bitmap. */

            if (dialog -> dp)
            {
                x = dialog -> x;
        
                y = dialog -> y;
        

                /* d1 = shadow color. */

                /* gui_textout_ex (dialog -> dp, dialog -> dp2, (x + 1), (y + 1), dialog -> d1, -1, FALSE); */

                gui_textout_ex (dialog -> dp, dialog -> dp2, (x + 1), (y + 1), GUI_SHADOW_COLOR, -1, FALSE);


                if (dialog -> flags & D_DISABLED)
                {
                    gui_textout_ex (dialog -> dp, dialog -> dp2, x, y, GUI_DISABLED_COLOR, -1, FALSE);
                }
                else
                {
                    gui_textout_ex (dialog -> dp, dialog -> dp2, x, y, GUI_TEXT_COLOR, -1, FALSE);
                }
            }


            break;


        default:


            break;
    }


    return (D_O_K);
}


#define SL_FRAME_END    0xf0


int sl_frame (int message, DIALOG * dialog, int key)
{
    int x;

    int y;


    int x2;

    int y2;


    int text_x;

    int text_y;


    int move_x = 0;

    int move_y = 0;


    FONT * old_font = NIL;


    /* dp3 = font. */

    if (dialog -> dp3)
    {
        old_font = font;


        font = dialog -> dp3;
    }


    x = dialog -> x;

    y = dialog -> y;


    x2 = ((dialog -> x + dialog -> w) - 1);

    y2 = ((dialog -> y + dialog -> h) - 1);


    switch (message)
    {
        case MSG_DRAW:

            /* dp = bitmap. */

            if (dialog -> dp)
            {
                int pixel;


                text_x = (dialog -> x + 6);

                text_y = (dialog -> y + 6);


                /* d1 = shadow color. */

                /* rect (dialog -> dp, (x + 1), (y + 1), (x2 + 1), (y2 + 1), dialog -> d1); */

                rect (dialog -> dp, (x + 1), (y + 1), (x2 + 1), (y2 + 1), GUI_SHADOW_COLOR);
        

                if (GUI_GRADIENT_START_COLOR == GUI_GRADIENT_END_COLOR)
                {
                    rectfill (screen, x, y, (x + (dialog -> w - 1)), y2, GUI_GRADIENT_START_COLOR);
                }
                else
                {
                    video_create_gradient (GUI_GRADIENT_START_COLOR, GUI_GRADIENT_END_COLOR, dialog -> w, NIL, NIL);
    
    
                    for (pixel = 0; pixel < dialog -> w; pixel ++)
                    {
                        int x_offset;
    
                        int y_offset;
    
    
                        x_offset = (x + pixel);
    
    
                        for (y_offset = y; y_offset <= y2; y_offset ++)
                        {
                            putpixel (screen, x_offset, y_offset, video_create_gradient (NIL, NIL, NIL, x_offset, y_offset));
                        }
                    }
                }

        
                rect (dialog -> dp, x, y, x2, y2, GUI_BORDER_COLOR);


                rectfill (dialog -> dp, (x + 1), (y + 1), (x2 - 1), ((text_height (font) + text_y) + 4), GUI_FILL_COLOR);


                /* textout_ex (dialog -> dp, font, dialog -> dp2, (text_x + 1), (text_y + 1), dialog -> d1, -1); */

                textout_ex (dialog -> dp, font, dialog -> dp2, (text_x + 1), (text_y + 1), GUI_SHADOW_COLOR, -1);

                textout_ex (dialog -> dp, font, dialog -> dp2, text_x, text_y, GUI_TEXT_COLOR, -1);


                hline (dialog -> dp, (x + 1), ((text_height (font) + text_y) + 4), (x2 - 1), GUI_BORDER_COLOR);


                /* hline (dialog -> dp, ((x + 1) - 1), (((text_height (font) + text_y) + 4) + 1), ((x2 - 1) + 1), dialog -> d1); */

                hline (dialog -> dp, ((x + 1) - 1), (((text_height (font) + text_y) + 4) + 1), ((x2 - 1) + 1), GUI_SHADOW_COLOR);

                hline (dialog -> dp, ((x + 1) - 1), (((text_height (font) + text_y) + 4) + 2), ((x2 - 1) + 1), GUI_BORDER_COLOR);
            }


            break;


        case MSG_CLICK:
        {

            int box_was_drawn = FALSE;

            int old_x = mouse_x, old_y = mouse_y;

            move_x = old_x;

            move_y = old_y;


            while (mouse_b & 1)
            {
                xor_mode (TRUE);


                if (dialog -> dp)
                {
                    int current_x = mouse_x, current_y = mouse_y;

                    if ((move_x != current_x) || (move_y != current_y))
                    {
                        scare_mouse ();


                        if (box_was_drawn)
                        {
                            rect (dialog -> dp,
                                (dialog -> x + move_x - old_x),
                                (dialog -> y + move_y - old_y),
                                (dialog -> x + move_x - old_x +
                                dialog -> w - 1),
                                (dialog -> y + move_y - old_y +
                                dialog -> h - 1),
                                GUI_BORDER_COLOR);
                        }


                        move_x = current_x;

                        move_y = current_y;

                        rect (dialog -> dp,
                            (dialog -> x + move_x - old_x),
                            (dialog -> y + move_y - old_y),
                            (dialog -> x + move_x - old_x + dialog -> w - 1),
                            (dialog -> y + move_y - old_y + dialog -> h - 1),
                            GUI_BORDER_COLOR);

                        box_was_drawn = TRUE;


                        unscare_mouse ();
                    }

                }


                solid_mode ();
            }


            dialog_x = dialog -> x + move_x - old_x;

            dialog_y = dialog -> y + move_y - old_y;


            restart_dialog = TRUE;


            if (old_font)
            {
                font = old_font;
            }


            return (D_CLOSE);


            break;

        }


        default:


            break;
    }


    if (old_font)
    {
        font = old_font;
    }


    return (D_O_K);
}


static void sl_draw_menu_item (MENU * menu, int x, int y, int width, int height, int bar, int selected)
{
   int i, j;
   char buf[256], *tok;

    int old_fg = 0;


    if (ugetc (menu -> text))
    {
        i = 0;

        j = ugetc (menu -> text);


        while ((j) && (j != '\t'))
        {
            i += usetc ((buf + i), j);

            j = ugetc ((menu -> text + i));
        }


        usetc ((buf + i), 0);


        if ((! bar) && (! selected))
        {
            int pixel;
    

            if (GUI_GRADIENT_START_COLOR == GUI_GRADIENT_END_COLOR)
            {
                rectfill (screen, x, y, (x + (width - 1)), (y + (text_height (font) + 3)), GUI_GRADIENT_START_COLOR);
            }
            else
            {
                video_create_gradient (GUI_GRADIENT_START_COLOR, GUI_GRADIENT_END_COLOR, width, NIL, NIL);
    
    
                for (pixel = 0; pixel < width; pixel ++)
                {
                    int x_offset;
    
                    int y_offset;
    
    
                    x_offset = (x + pixel);
    
    
                    for (y_offset = y; y_offset <= (y + (text_height (font) + 3)); y_offset ++)
                    {
                        putpixel (screen, x_offset, y_offset, video_create_gradient (NIL, NIL, NIL, x_offset, y_offset));
                    }
                }
            }
        }
        else if (selected)
        {
            rectfill (screen, x, y, (x + (width - 1)), (y + (text_height (font) + 3)), GUI_SELECTED_COLOR);
        }
        else
        {
            rectfill (screen, x, y, (x + (width - 1)), (y + (text_height (font) + 3)), GUI_MENU_BAR_COLOR);
        }


        gui_textout_ex (screen, buf, (x + 9), (y + 2), GUI_SHADOW_COLOR, -1, FALSE);


        if (menu -> flags & D_DISABLED)
        {
            gui_textout_ex (screen, buf, (x + 8), (y + 1), GUI_DISABLED_COLOR, -1, FALSE);
        }
        else
        {
            gui_textout_ex (screen, buf, (x + 8), (y + 1), GUI_TEXT_COLOR, -1, FALSE);
        }


        if (j == '\t')
        {
            tok = ((menu -> text + i) + uwidth ((menu -> text + i)));


            gui_textout_ex (screen, tok, (x + ((width - (gui_strlen (tok) - 10)) + 1)), (y + 2), GUI_SHADOW_COLOR, -1, FALSE);


            if (menu -> flags & D_DISABLED)
            {
                gui_textout_ex (screen, tok, (x + (width - (gui_strlen (tok) - 10))), (y + 1), GUI_DISABLED_COLOR, -1, FALSE);
            }
            else
            {
                gui_textout_ex (screen, tok, (x + (width - (gui_strlen (tok) - 10))), (y + 1), GUI_TEXT_COLOR, -1, FALSE);
            }
        }


        if ((menu -> child) && (! bar))
        {
            int centre_y;


            centre_y = (y + (text_height (font) / 2));


            triangle (screen, ((x + (width - 4)) + 1), (centre_y + 1), ((x + (width - 8)) + 1), ((centre_y - 4) + 1), ((x + (width - 8)) + 1), ((centre_y + 4) + 1), GUI_SHADOW_COLOR);
                                             

            if (menu -> flags & D_DISABLED)
            {
                triangle (screen, (x + (width - 4)), centre_y, (x + (width - 8)), (centre_y - 4), (x + (width - 8)), (centre_y + 4), GUI_DISABLED_COLOR);
            }
            else
            {
                triangle (screen, (x + (width - 4)), centre_y, (x + (width - 8)), (centre_y - 4), (x + (width - 8)), (centre_y + 4), GUI_TEXT_COLOR);
            }
        }
    }
    else
    {
        int pixel;


        if (GUI_GRADIENT_START_COLOR == GUI_GRADIENT_END_COLOR)
        {
            rectfill (screen, x, y, (x + (width - 1)), (y + (text_height (font) + 3)), GUI_GRADIENT_START_COLOR);
        }
        else
        {
            video_create_gradient (GUI_GRADIENT_START_COLOR, GUI_GRADIENT_END_COLOR, width, NIL, NIL);
    
    
            for (pixel = 0; pixel < width; pixel ++)
            {
                int x_offset;
    
                int y_offset;
    
    
                x_offset = (x + pixel);
    
    
                for (y_offset = y; y_offset <= (y + (text_height (font) + 3)); y_offset ++)
                {
                    putpixel (screen, x_offset, y_offset, video_create_gradient (NIL, NIL, NIL, x_offset, y_offset));
                }
            }
        }


        hline (screen, x, (y + (text_height (font) / 2)), (x + width), GUI_LIGHT_SHADOW_COLOR);

        hline (screen, x, (y + ((text_height (font) / 2) + 2)), (x + (width - 1)), GUI_BORDER_COLOR);


        hline (screen, (x - 1), (y + ((text_height (font) / 2) + 1)), (x + width), GUI_SHADOW_COLOR);
    }


    if (menu -> flags & D_SELECTED)
    {
        circlefill (screen, ((x + 3) + 1), ((y + (text_height (font) / 2)) + 1), 2, GUI_SHADOW_COLOR);


        if (menu -> flags & D_DISABLED)
        {
            circlefill (screen, (x + 3), (y + (text_height (font) / 2)), 2, GUI_DISABLED_COLOR);
        }
        else
        {
            circlefill (screen, (x + 3), (y + (text_height (font) / 2)), 2, GUI_TEXT_COLOR);
        }
    }
}


static void sl_draw_menu (int x, int y, int width, int height)
{
    /* Bug fix (Allegro 4.1.1+). */

    width --;

    height --;


    vline (screen, (x + width), (y + 1), (y + height), GUI_SHADOW_COLOR);

    hline (screen, (x + 1), (y + height), (x + width), GUI_SHADOW_COLOR);


    rect (screen, x, y, (x + (width - 1)), (y + (height - 1)), GUI_BORDER_COLOR);
}


int sl_button (int message, DIALOG * dialog, int key)
{
    int (* handler) (DIALOG *);


    int result;


    handler = dialog -> dp2;


    switch (message)
    {
        case MSG_CLICK:

        case MSG_KEY:

            if (handler)
            {
                dialog -> flags |= D_SELECTED;


                scare_mouse ();

                object_message (dialog, MSG_DRAW, 0);

                unscare_mouse ();


                result = handler (dialog);


                dialog -> flags &= ~D_SELECTED;


                scare_mouse ();

                object_message (dialog, MSG_DRAW, 0);

                unscare_mouse ();


                return (result);
            }


            break;


        default:

            break;
    }


    return (d_button_proc (message, dialog, key));
}


int sl_checkbox (int message, DIALOG * dialog, int key)
{
    int (* handler) (DIALOG *);


    handler = dialog -> dp2;


    switch (message)
    {
        case MSG_CLICK:

        case MSG_KEY:

            dialog -> flags ^= D_SELECTED;


            scare_mouse ();

            object_message (dialog, MSG_DRAW, 0);

            unscare_mouse ();


            if (handler)
            {
                return (handler (dialog));
            }


            break;


        default:

            break;
    }


    return (d_check_proc (message, dialog, key));
}


int sl_listbox (int message, DIALOG * dialog, int key)
{
    int (* handler) (DIALOG *);


    int result;


    handler = dialog -> dp3;


    result = d_list_proc (message, dialog, key);


    switch (message)
    {
        case MSG_CLICK:

        case MSG_KEY:

            if (handler)
            {
                handler (dialog);
            }


            break;


        default:

            break;
    }


    return (result);
}


int sl_viewer (int message, DIALOG * dialog, int key)
{
    switch (message)
    {
        case MSG_CHAR:

            switch ((key >> 8))
            {
                case KEY_UP:

                case KEY_DOWN:

                case KEY_LEFT:

                case KEY_RIGHT:

                case KEY_HOME:

                case KEY_END:

                case KEY_PGUP:

                case KEY_PGDN:

                    return (d_textbox_proc (message, dialog, key));


                    break;


                default:

                    /* Prevent typing. */

                    break;
            }


        default:

            return (d_textbox_proc (message, dialog, key));


            break;
    }


    return (D_O_K);
}


int sl_radiobox (int message, DIALOG * dialog, int key)
{
    int (* handler) (DIALOG *);


    int value;


    int data;


    handler = dialog -> dp2;


    data = dialog -> d2;


    dialog -> d2 = 1;

    value = d_radio_proc (message, dialog, key);


    dialog -> d2 = data;


    switch (message)
    {
        case MSG_CLICK:

        case MSG_KEY:

            if (handler)
            {
                return (handler (dialog));
            }


            break;


        default:

            break;
    }


    return (value);
}


int sl_x_button (int message, DIALOG * dialog, int key)
{
    /* Note: Hack to always use small font.  Not a complete sl_ widget. */

    FONT * old_font = font;


    int result;


    font = small_font;

    result = d_button_proc (message, dialog, key);

    font = old_font;


    return (result);
}
