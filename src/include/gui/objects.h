

int sl_text (int message, DIALOG * dialog, int key)
{
    int x;

    int y;


    FONT * old_font = NULL;


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

                gui_textout (dialog -> dp, dialog -> dp2, (x + 1), (y + 1), dialog -> d1, FALSE);


                if (dialog -> flags & D_DISABLED)
                {
                    gui_textout (dialog -> dp, dialog -> dp2, x, y, gui_mg_color, FALSE);
                }
                else
                {
                    gui_textout (dialog -> dp, dialog -> dp2, x, y, gui_fg_color, FALSE);
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


    FONT * old_font = NULL;


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

                rect (dialog -> dp, (x + 1), (y + 1), (x2 + 1), (y2 + 1), dialog -> d1);
        
        
                for (pixel = 0; pixel < dialog -> w; pixel ++)
                {
                    int shade;


                    if (dialog -> w > 64)
                    {
                        int y_offset;
        
        
                        for (y_offset = y; y_offset <= y2; y_offset ++)
                        {
                            if (((pixel % 2) && (y_offset % 2)) && ((pixel + 1) < dialog -> w))
                            {
                                shade = (191 - (((pixel + 1) * 128) / dialog -> w));
                            }
                            else if (((! (pixel % 2)) && (y_offset % 2)) && ((pixel - 1) >= 0))
                            {
                                shade = (191 - (((pixel - 1) * 128) / dialog -> w));
                            }
                            else
                            {
                                shade = (191 - ((pixel * 128) / dialog -> w));
                            }
        
        
                            putpixel (screen, (x + pixel), y_offset, makecol (shade, shade, shade));
                        }
                    }
                    else
                    {
                        shade = (191 - ((pixel * 128) / dialog -> w));
    
    
                        vline (screen, (x + pixel), y, y2, makecol (shade, shade, shade));
                    }
                }

        
                rect (dialog -> dp, x, y, x2, y2, gui_fg_color);


                rectfill (dialog -> dp, (x + 1), (y + 1), (x2 - 1), ((text_height (font) + text_y) + 4), gui_bg_color);


                textout (dialog -> dp, font, dialog -> dp2, (text_x + 1), (text_y + 1), dialog -> d1);

                textout (dialog -> dp, font, dialog -> dp2, text_x, text_y, gui_fg_color);


                hline (dialog -> dp, (x + 1), ((text_height (font) + text_y) + 4), (x2 - 1), gui_fg_color);


                hline (dialog -> dp, ((x + 1) - 1), (((text_height (font) + text_y) + 4) + 1), ((x2 - 1) + 1), dialog -> d1);

                hline (dialog -> dp, ((x + 1) - 1), (((text_height (font) + text_y) + 4) + 2), ((x2 - 1) + 1), gui_fg_color);
            }


            break;


        case MSG_CLICK:

            move_x = mouse_x;

            move_y = mouse_y;


            while (mouse_b & 1)
            {
                xor_mode (TRUE);


                if (dialog -> dp)
                {
                    if ((move_x != mouse_x) || (move_y != mouse_y))
                    {
                        scare_mouse ();


                        rect (dialog -> dp, move_x, move_y, (move_x + dialog -> w), (move_y + dialog -> h), gui_fg_color);

                        rect (dialog -> dp, mouse_x, mouse_y, (mouse_x + dialog -> w), (mouse_y + dialog -> h), gui_fg_color);


                        unscare_mouse ();
                    }


                    move_x = mouse_x;

                    move_y = mouse_y;
                }


                solid_mode ();
            }


            dialog_x = move_x;

            dialog_y = move_y;


            restart_dialog = TRUE;


            if (old_font)
            {
                font = old_font;
            }


            return (D_CLOSE);


            break;


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


    if (menu -> flags & D_DISABLED)
    {
        old_fg = gui_fg_color;

        gui_fg_color = gui_mg_color;
    }


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
    
    
            for (pixel = 0; pixel < width; pixel ++)
            {
                int shade;


                if (width > 64)
                {
                    int y_offset;
    
    
                    for (y_offset = y; y_offset <= (y + (text_height (font) + 3)); y_offset ++)
                    {
                        if (((pixel % 2) && (y_offset % 2)) && ((pixel + 1) < width))
                        {
                            shade = (191 - (((pixel + 1) * 128) / width));
                        }
                        else if (((! (pixel % 2)) && (y_offset % 2)) && ((pixel - 1) >= 0))
                        {
                            shade = (191 - (((pixel - 1) * 128) / width));
                        }
                        else
                        {
                            shade = (191 - ((pixel * 128) / width));
                        }
    
    
                        putpixel (screen, (x + pixel), y_offset, makecol (shade, shade, shade));
                    }
                }
                else
                {
                    shade = (191 - ((pixel * 128) / width));


                    vline (screen, (x + pixel), y, (y + (text_height (font) + 3)), makecol (shade, shade, shade));
                }
            }
        }
        else if (selected)
        {
            rectfill (screen, x, y, (x + (width - 1)), (y + (text_height (font) + 3)), makecol (0, 0, 0));
        }
        else
        {
            rectfill (screen, x, y, (x + (width - 1)), (y + (text_height (font) + 3)), gui_bg_color);
        }


        gui_textout (screen, buf, (x + 9), (y + 2), makecol (0, 0, 0), FALSE);

        gui_textout (screen, buf, (x + 8), (y + 1), gui_fg_color, FALSE);


        if (j == '\t')
        {
            tok = ((menu -> text + i) + uwidth ((menu -> text + i)));


            gui_textout (screen, tok, (x + ((width - (gui_strlen (tok) - 10)) + 1)), (y + 2), makecol (0, 0, 0), FALSE);

            gui_textout (screen, tok, (x + (width - (gui_strlen (tok) - 10))), (y + 1), gui_fg_color, FALSE);
        }


        if ((menu -> child) && (! bar))
        {
            int centre_y;


            centre_y = (y + (text_height (font) / 2));


            triangle (screen, ((x + (width - 4)) + 1), (centre_y + 1), ((x + (width - 8)) + 1), ((centre_y - 4) + 1), ((x + (width - 8)) + 1), ((centre_y + 4) + 1), makecol (0, 0, 0));
                                             
            triangle (screen, (x + (width - 4)), centre_y, (x + (width - 8)), (centre_y - 4), (x + (width - 8)), (centre_y + 4), gui_fg_color);
        }
    }
    else
    {
        {
            int pixel;
    
    
            for (pixel = 0; pixel < width; pixel ++)
            {
                int shade;


                if (width > 64)
                {
                    int y_offset;
    
    
                    for (y_offset = y; y_offset <= (y + (text_height (font) + 3)); y_offset ++)
                    {
                        if (((pixel % 2) && (y_offset % 2)) && ((pixel + 1) < width))
                        {
                            shade = (191 - (((pixel + 1) * 128) / width));
                        }
                        else if (((! (pixel % 2)) && (y_offset % 2)) && ((pixel - 1) >= 0))
                        {
                            shade = (191 - (((pixel - 1) * 128) / width));
                        }
                        else
                        {
                            shade = (191 - ((pixel * 128) / width));
                        }
    
    
                        putpixel (screen, (x + pixel), y_offset, makecol (shade, shade, shade));
                    }
                }
                else
                {
                    shade = (191 - ((pixel * 128) / width));


                    vline (screen, (x + pixel), y, (y + (text_height (font) + 3)), makecol (shade, shade, shade));
                }
            }
        }


        hline (screen, x, (y + (text_height (font) / 2)), (x + width), gui_mg_color);

        hline (screen, x, (y + ((text_height (font) / 2) + 2)), (x + (width - 1)), gui_fg_color);


        hline (screen, (x - 1), (y + ((text_height (font) / 2) + 1)), (x + width), makecol (0, 0, 0));
    }


    if (menu -> flags & D_SELECTED)
    {
        circlefill (screen, ((x + 3) + 1), ((y + (text_height (font) / 2)) + 1), 2, makecol (0, 0, 0));

        circlefill (screen, (x + 3), (y + (text_height (font) / 2)), 2, gui_fg_color);
    }


    if (menu -> flags & D_DISABLED)
    {
        gui_fg_color = old_fg;
    }
}


static void sl_draw_menu (int x, int y, int width, int height)
{
    /* 0 = shadow color. */

    vline (screen, (x + width), (y + 1), (y + height), 0);

    hline (screen, (x + 1), (y + height), (x + width), 0);


    rect (screen, x, y, (x + (width - 1)), (y + (height - 1)), gui_fg_color);
}
