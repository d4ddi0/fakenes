#include <allegro.h>
/* XPM */
static const char *allegico_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 4 1",
"  c #706800",
". c #D80000",
"X c #F8AB00",
"o c None",
/* pixels */
"oooooooooo..........oooooooooooo",
"oooooooooo..........oooooooooooo",
"oooooooo..................oooooo",
"oooooooo..................oooooo",
"oooooooo      XXXX  XXoooooooooo",
"oooooooo      XXXX  XXoooooooooo",
"oooooo  XX  XXXXXX  XXXXXXoooooo",
"oooooo  XX  XXXXXX  XXXXXXoooooo",
"oooooo  XX    XXXXXX  XXXXXXoooo",
"oooooo  XX    XXXXXX  XXXXXXoooo",
"oooooo    XXXXXXXX        oooooo",
"oooooo    XXXXXXXX        oooooo",
"ooooooooooXXXXXXXXXXXXXXoooooooo",
"ooooooooooXXXXXXXXXXXXXXoooooooo",
"oooooooo    ..      oooooooooooo",
"oooooooo    ..      oooooooooooo",
"oooooo      ..    ..      oooooo",
"oooooo      ..    ..      oooooo",
"oooo        ........        oooo",
"oooo        ........        oooo",
"ooooXXXX  ..XX....XX..  XXXXoooo",
"ooooXXXX  ..XX....XX..  XXXXoooo",
"ooooXXXXXX............XXXXXXoooo",
"ooooXXXXXX............XXXXXXoooo",
"ooooXXXX................XXXXoooo",
"ooooXXXX................XXXXoooo",
"oooooooo......oooo......oooooooo",
"oooooooo......oooo......oooooooo",
"oooooo      oooooooo      oooooo",
"oooooo      oooooooo      oooooo",
"oooo        oooooooo        oooo",
"oooo        oooooooo        oooo"
};
#if defined ALLEGRO_WITH_XWINDOWS && defined ALLEGRO_USE_CONSTRUCTOR
extern void *allegro_icon;
CONSTRUCTOR_FUNCTION(static void _set_allegro_icon(void));
static void _set_allegro_icon(void)
{
    allegro_icon = allegico_xpm;
}
#endif
