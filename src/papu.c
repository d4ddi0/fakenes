

/*

FakeNES - A portable, open-source NES emulator.

papu.c: Implementation of the pAPU emulation.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#include <allegro.h>


#include <stdio.h>

#include <string.h>


#include "audio.h"

#include "apu.h"

#include "papu.h"


#include "misc.h"


static apu_t * default_apu;


int papu_filter_type = 0;

int papu_swap_channels = 0;


static UINT8 * frame_buffer;


int papu_init (void)
{
    int frame_buffer_size = 0;


    default_apu = apu_create
        (audio_sample_rate, 60, 0, audio_sample_size);

    if (! default_apu)
    {
        return (1);
    }


    papu_filter_type = get_config_int
        ("audio", "filter_type", APU_FILTER_WEIGHTED);

    apu_setfilter (papu_filter_type);


    papu_swap_channels =
        get_config_int ("audio", "swap_channels", FALSE);


    if (audio_enable_output)
    {
        frame_buffer_size = AUDIO_BUFFER_SIZE;
    
        if (audio_sample_size == 16)
        {
            frame_buffer_size *= 2;
        }
    
        if (audio_pseudo_stereo)
        {
            frame_buffer_size *= 2;
        }
    
    
        frame_buffer = malloc (frame_buffer_size);
    
        if (! frame_buffer)
        {
            apu_destroy (&default_apu);
    
            return (1);
        }
    }


    return (0);
}


void papu_exit (void)
{
    apu_destroy (&default_apu);


    if (audio_enable_output)
    {
        free (frame_buffer);
    }


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


static int frame_counter = 1;

static int frame_offset = 0;


void papu_process_frame (void)
{
    if (! audio_enable_output)
    {
        apu_process (NULL, (audio_sample_rate / 60));

        return;
    }


    if (audio_pseudo_stereo)
    {
        apu_process_stereo ((frame_buffer +
            frame_offset), (audio_sample_rate / 60), papu_swap_channels);
    }
    else
    {
        apu_process ((frame_buffer +
            frame_offset), (audio_sample_rate / 60));
    }


    if (audio_sample_size == 8)
    {
        if (audio_pseudo_stereo)
        {
            frame_offset += ((audio_sample_rate / 60) * 2);
        }
        else
        {
            frame_offset += (audio_sample_rate / 60);
        }
    }
    else
    {
        if (audio_pseudo_stereo)
        {
            frame_offset += ((audio_sample_rate / 60) * 4);
        }
        else
        {
            frame_offset += ((audio_sample_rate / 60) * 2);
        }
    }


    if (frame_counter == audio_buffer_length)
    {
        do
        {
            audio_start ();
        }
        while (! audio_buffer);


        if (audio_sample_size == 8)
        {
            if (audio_pseudo_stereo)
            {
                memcpy (audio_buffer,
                    frame_buffer, (AUDIO_BUFFER_SIZE * 2));
            }
            else
            {
                memcpy (audio_buffer,
                    frame_buffer, AUDIO_BUFFER_SIZE);
            }
        }
        else
        {
            if (audio_pseudo_stereo)
            {
                memcpy (audio_buffer,
                    frame_buffer, (AUDIO_BUFFER_SIZE * 4));
            }
            else
            {
                memcpy (audio_buffer,
                    frame_buffer, (AUDIO_BUFFER_SIZE * 2));
            }
        }


        audio_stop ();


        frame_counter = 1;

        frame_offset = 0;
    }
    else
    {
        audio_update ();

        frame_counter ++;
    }
}
