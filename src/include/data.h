

#ifndef DATA_H_INCLUDED

#define DATA_H_INCLUDED


#include "datafile.h"


#define DATA_TO_FONT(id)        ((FONT *) datafile_data [DATAFILE_ ##id].dat)

#define DATA_TO_BITMAP(id)      ((BITMAP *) datafile_data [DATAFILE_ ##id].dat)


#define DATA_TO_RGB(id)         ((RGB *) datafile_data [DATAFILE_ ##id].dat)


#endif /* ! DATA_H_INCLUDED */
