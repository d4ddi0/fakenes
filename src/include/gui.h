

/*

FakeNES - A portable, open-source NES emulator.

gui.h: Declarations for the GUI interface.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#ifndef __GUI_H__
#define __GUI_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "misc.h"


int show_gui (void);


void gui_message (int, CONST UINT8 *, ...);


int gui_is_active;


#ifdef __cplusplus
}
#endif

#endif /* ! __GUI_H__ */
