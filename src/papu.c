

/*

FakeNES - A portable, open-source NES emulator.

papu.c: Implementation of the APU interface.

Copyright (c) 2002, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#include <allegro.h>


#include <stdio.h>

#include <string.h>


#include "audio.h"

#include "apu.h"

#include "papu.h"


#include "misc.h"


#include "timing.h"


static apu_t * default_apu = NULL;


int papu_filter_type = 0;

int papu_swap_channels = 0;


int papu_init (void)
{
    int speed;


    speed = (machine_type == MACHINE_TYPE_NTSC ? 60 : 50);


    default_apu = apu_create (audio_sample_rate, speed, 0, audio_sample_size);

    if (! default_apu)
    {
        return (1);
    }


    papu_filter_type = get_config_int ("audio", "filter_type", APU_FILTER_DYNAMIC);

    apu_setfilter (papu_filter_type);


    papu_swap_channels = get_config_int ("audio", "swap_channels", FALSE);


    return (0);
}


void papu_reinit (void)
{
    int speed;


    speed = (machine_type == MACHINE_TYPE_NTSC ? 60 : 50);


    apu_setparams (audio_sample_rate, speed, 0, audio_sample_size);
}


void papu_exit (void)
{
    apu_destroy (&default_apu);


    set_config_int ("audio", "filter_type", papu_filter_type);

    set_config_int ("audio", "swap_channels", papu_swap_channels);
}


void papu_reset (void)
{
    apu_setext (default_apu, NULL);

    apu_reset ();
}


UINT8 papu_read (UINT16 address)
{
    return (apu_read (address));
}


void papu_write (UINT16 address, UINT8 value)
{
    apu_write (address, value);
}


static int frames = 0;


void papu_update (void)
{
    int speed;


    frames ++;


    if (frames == audio_buffer_length)
    {
        if (audio_enable_output)
        {
            audio_stop ();


            speed = (machine_type == MACHINE_TYPE_NTSC ? 60 : 50);


            if (audio_pseudo_stereo)
            {
                apu_process_stereo (audio_buffer, ((audio_sample_rate / speed) * frames), papu_swap_channels);
            }
            else
            {
                apu_process (audio_buffer, ((audio_sample_rate / speed) * frames));
            }


            audio_start ();
        }


        frames = 0;
    }
}
