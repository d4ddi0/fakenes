

#ifndef MMC_SHARED_H_INCLUDED

#define MMC_SHARED_H_INCLUDED


#define MMC_PSEUDO_CLOCKS_PER_SCANLINE  114


#define MMC_LAST_PSEUDO_SCANLINE        342


/* These are defined in 16k pages. */

#define MMC_FIRST_ROM_BLOCK     0

#define MMC_LAST_ROM_BLOCK      (ROM_PRG_ROM_PAGES - 1)


/* Defined in core.h. */

#undef byte

#undef word


typedef struct _MMC_COMBO16
{
    struct
    {
#ifdef LSB_FIRST

        UINT8 low, high;
#else

        UINT8 high, low;
#endif
    }
    bytes;


    UINT16 word;
}
MMC_COMBO16;


#endif /* ! MMC_SHARED_H_INCLUDED */
