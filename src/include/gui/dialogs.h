

int gui_raised_label_object (int message, DIALOG * dialog, int key)
{
    int x, y;

    FONT * old_font;


    switch (message)
    {
        case MSG_DRAW:

            if (dialog -> dp)
            {
                x = dialog -> x;
        
                y = dialog -> y;
        

                if (! dialog -> dp3)
                {
                    gui_textout (dialog -> dp, dialog ->
                        dp2, (x + 1), (y + 1), dialog -> d1, FALSE);

                    gui_textout (dialog -> dp,
                        dialog -> dp2, x, y, dialog -> fg, FALSE);
                }
                else
                {
                    old_font = font;

                    font = dialog -> dp3;


                    gui_textout (dialog -> dp, dialog ->
                        dp2, (x + 1), (y + 1), dialog -> d1, FALSE);

                    gui_textout (dialog -> dp,
                        dialog -> dp2, x, y, dialog -> fg, FALSE);


                    font = old_font;
                }
            }


            break;


        default:


            break;
    }


    return (D_O_K);
}


int gui_window_object (int message, DIALOG * dialog, int key)
{
    int x, y, x2, y2;

    int text_x, text_y;


    switch (message)
    {
        case MSG_DRAW:

            if (dialog -> dp)
            {
                x = dialog -> x;

                y = dialog -> y;


                x2 = ((dialog -> x + dialog -> w) - 1);

                y2 = ((dialog -> y + dialog -> h) - 1);


                text_x = (dialog -> x + 6);

                text_y = (dialog -> y + 6);


                rect (dialog -> dp, (x + 1),
                    (y + 1), (x2 + 1), (y2 + 1), dialog -> d1);
        
        
                rectfill (dialog -> dp, x, y, x2, y2, dialog -> bg);
        
                rect (dialog -> dp, x, y, x2, y2, dialog -> fg);


                if (! dialog -> dp3)
                {
                    rectfill (dialog -> dp, (x + 1), (y + 1), (x2 - 1),
                        ((text_height (font) + text_y) + 4), dialog -> d2);


                    textout (dialog -> dp, font, dialog ->
                        dp2, (text_x + 1), (text_y + 1), dialog -> d1);

                    textout (dialog -> dp, font,
                        dialog -> dp2, text_x, text_y, dialog -> fg);


                    hline (dialog -> dp, (x + 1), ((text_height
                        (font) + text_y) + 4), (x2 - 1), dialog -> fg);
                }
                else
                {
                    rectfill (dialog -> dp, (x + 1), (y + 1), (x2 - 1),
                        ((text_height (dialog -> dp3) + text_y) + 4), dialog -> d2);


                    textout (dialog -> dp, dialog -> dp3, dialog ->
                        dp2, (text_x + 1), (text_y + 1), dialog -> d1);

                    textout (dialog -> dp, dialog -> dp3,
                        dialog -> dp2, text_x, text_y, dialog -> fg);


                    hline (dialog -> dp, (x + 1), ((text_height (dialog
                        -> dp3) + text_y) + 4), (x2 - 1), dialog -> fg);
                }

            }


            break;


        default:


            break;
    }


    return (D_O_K);
}
