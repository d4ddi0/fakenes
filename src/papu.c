

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

papu.c: Implementation of the APU abstraction.

Copyright (c) 2004, Randy McDowell.
Copyright (c) 2004, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <allegro.h>


#include <stdio.h>

#include <string.h>


#include "audio.h"

#include "apu.h"

#include "input.h"

#include "papu.h"


#include "misc.h"


#include "timing.h"


static apu_t * default_apu = NIL;


int papu_enable_square_1 = TRUE;

int papu_enable_square_2 = TRUE;


int papu_enable_triangle = TRUE;

int papu_enable_noise = TRUE;


int papu_enable_dmc = TRUE;


int papu_enable_exsound = TRUE;


int papu_swap_channels = 0;


int papu_ideal_triangle = TRUE;


int papu_linear_echo = TRUE;


int papu_spatial_stereo = PAPU_SPATIAL_STEREO_MODE_2;


int papu_dithering = FALSE;


int papu_is_recording = FALSE;


static int filter_list = 0;


#define ECHO_DEPTH  3


static void * echo_buffers [ECHO_DEPTH];


static int echo_buffer_size = 0;


void papu_update (void)
{
    apu_setchan (0, papu_enable_square_1);

    apu_setchan (1, papu_enable_square_2);


    apu_setchan (2, papu_enable_triangle);

    apu_setchan (3, papu_enable_noise);


    apu_setchan (4, papu_enable_dmc);


    apu_setchan (5, papu_enable_exsound);


    apu_setmode (APUMODE_IDEAL_TRIANGLE, papu_ideal_triangle);
}


int papu_reinit (void)
{
    int speed;


    int index;


    speed = (machine_type == MACHINE_TYPE_NTSC ? 60 : 50);


    apu_setparams (audio_sample_rate, speed, 0, audio_sample_size);


    if (timing_half_speed)
    {
        speed /= 2;
    }


    echo_buffer_size = ((audio_sample_rate / speed) * audio_buffer_length);

    if (audio_sample_size == 16)
    {
        echo_buffer_size *= 2;
    }

    if (audio_pseudo_stereo)
    {
        echo_buffer_size *= 2;
    }


    if ((! papu_linear_echo) || (! audio_enable_output))
    {
        return (0);
    }


    for (index = 0; index < ECHO_DEPTH; index ++)
    {
        if (echo_buffers [index])
        {
            free (echo_buffers [index]);
        }


        echo_buffers [index] = malloc (echo_buffer_size);


        /* Fix me: memory leak. */

        if (! echo_buffers [index])
        {
            return (2);
        }
    }


    if (audio_sample_size == 16)
    {
        UINT16 * buffers [ECHO_DEPTH];


        int offset;


        for (index = 0; index < ECHO_DEPTH; index ++)
        {
            buffers [index] = echo_buffers [index];


            for (offset = 0; offset < (echo_buffer_size / 2); offset ++)
            {
                buffers [index] [offset] = 0x8000;
            }
        }
   }
   else
   {
        UINT8 * buffers [ECHO_DEPTH];


        int offset;


        for (index = 0; index < ECHO_DEPTH; index ++)
        {
            buffers [index] = echo_buffers [index];


            for (offset = 0; offset < echo_buffer_size; offset ++)
            {
                buffers [index] [offset] = 0x80;
            }
        }
   }


    return (0);
}



int papu_init (void)
{
    int result;


    /* We must supply an initial speed, like 60. */

    default_apu = apu_create (audio_sample_rate, 60, 0, audio_sample_size);

    if (! default_apu)
    {
        return (1);
    }


    papu_linear_echo = get_config_int ("papu", "linear_echo", TRUE);


    papu_spatial_stereo = get_config_int ("audio", "spatial_stereo", PAPU_SPATIAL_STEREO_MODE_2);


    if ((result = papu_reinit ()) != 0)
    {
        return (result);
    }

    
    filter_list = get_config_int ("audio", "filter_list", PAPU_FILTER_LOW_PASS_MODE_2);

    papu_set_filter_list (filter_list);


    papu_swap_channels = get_config_int ("audio", "swap_channels", FALSE);


    papu_enable_square_1 = get_config_int ("papu", "enable_square_1", TRUE);

    papu_enable_square_2 = get_config_int ("papu", "enable_square_2", TRUE);

    papu_enable_triangle = get_config_int ("papu", "enable_triangle", TRUE);

    papu_enable_noise = get_config_int ("papu", "enable_noise", TRUE);

    papu_enable_dmc = get_config_int ("papu", "enable_dmc", TRUE);

    papu_enable_exsound = get_config_int ("papu", "enable_exsound", TRUE);


    papu_ideal_triangle = get_config_int ("papu", "ideal_triangle", TRUE);


    papu_dithering = get_config_int ("audio", "dithering", FALSE);


    papu_update ();


    return (0);
}


void papu_exit (void)
{
    int index;


    apu_destroy (&default_apu);


    for (index = 0; index < ECHO_DEPTH; index ++)
    {
        if (echo_buffers [index])
        {
            free (echo_buffers [index]);
        }
    }


    set_config_int ("audio", "filter_list", filter_list);


    set_config_int ("audio", "swap_channels", papu_swap_channels);


    set_config_int ("papu", "enable_square_1", papu_enable_square_1);

    set_config_int ("papu", "enable_square_2", papu_enable_square_2);

    set_config_int ("papu", "enable_triangle", papu_enable_triangle);

    set_config_int ("papu", "enable_noise", papu_enable_noise);

    set_config_int ("papu", "enable_dmc", papu_enable_dmc);

    set_config_int ("papu", "enable_exsound", papu_enable_exsound);


    set_config_int ("papu", "ideal_triangle", papu_ideal_triangle);


    set_config_int ("papu", "linear_echo", papu_linear_echo);


    set_config_int ("audio", "spatial_stereo", papu_spatial_stereo);


    set_config_int ("audio", "dithering", papu_dithering);
}


void papu_reset (void)
{
    apu_setext (default_apu, NIL);

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


void papu_exwrite (UINT16 address, UINT8 value)
{
    ex_write (address, value);
}


void papu_set_exsound (int chip)
{
    default_apu -> ex_chip = chip;
}


void papu_clear_exsound (void)
{
    default_apu -> ex_chip = 0;
}


static INLINE void apply_echo (void)
{
    int index;


    UINT8 * buffer = audio_buffer;


    UINT8 * buffers [ECHO_DEPTH];


    int offset;


    for (index = 0; index < ECHO_DEPTH; index ++)
    {
        buffers [index] = echo_buffers [index];
    }


    for (offset = 0; offset < echo_buffer_size; offset ++)
    {
        int accumulator;


        for (index = 0; index < ECHO_DEPTH; index ++)
        {
            if (index == 0)
            {
                accumulator = buffers [index] [offset];
            }
            else
            {
                accumulator /= 3;
    
                accumulator += ((buffers [index] [offset] / 3) * 2);
            }
        }


        buffer [offset] = accumulator;
    }
}


static FILE * dump_file = NIL;


int papu_start_record (void)
{
    dump_file = fopen ("apudump.raw", "wb");

    if (! dump_file)
    {
        return (1);
    }


    papu_is_recording = TRUE;


    return (0);
}


void papu_stop_record (void)
{
    if (papu_is_recording)
    {
        fclose (dump_file);

        papu_is_recording = FALSE;
    }
}


static int frames = 0;


#define PAPU_BUFFER_SIZE    ((audio_sample_rate / speed) * frames)


static int fast_forward = FALSE;


void papu_process (void)
{
    int speed;


    int index;


    speed = (machine_type == MACHINE_TYPE_NTSC ? 60 : 50);


    if (timing_half_speed)
    {
        speed /= 2;
    }


    if ((key [KEY_TILDE]) && (! (input_mode & INPUT_MODE_CHAT)))
    {
        if (! fast_forward)
        {
            audio_suspend ();

            fast_forward = TRUE;
        }


        apu_process (NIL, (audio_sample_rate / speed), papu_dithering);

        return;
    }
    else
    {
        if (fast_forward)
        {
            audio_resume ();

            fast_forward = FALSE;
        }
    }


    frames ++;


    if (frames == audio_buffer_length)
    {
        if (audio_enable_output)
        {
            do
            {
                audio_start ();
            }
            while (! audio_buffer);


            if (audio_pseudo_stereo)
            {
                apu_process_stereo (audio_buffer, PAPU_BUFFER_SIZE, papu_dithering,
                    audio_pseudo_stereo, papu_swap_channels, papu_spatial_stereo);
            }
            else
            {
                apu_process (audio_buffer, PAPU_BUFFER_SIZE, papu_dithering);
            }


            if (papu_linear_echo)
            {
                for (index = 1; index < ECHO_DEPTH; index ++)
                {
                    memcpy (echo_buffers [(index - 1)], echo_buffers [index], echo_buffer_size);
                }


                memcpy (echo_buffers [(ECHO_DEPTH - 1)], audio_buffer, echo_buffer_size);


                apply_echo ();
            }


            if (papu_is_recording)
            {
                fwrite (audio_buffer, echo_buffer_size, 1, dump_file);
            }


            audio_stop ();
        }


        frames = 0;
    }
    else
    {
        if (audio_enable_output)
        {
            audio_update ();
        }
    }


    sync_apu_register ();
}


void papu_save_state (PACKFILE * file, int version)
{
    int index;


    PACKFILE * file_chunk;


    file_chunk = pack_fopen_chunk (file, FALSE);


    for (index = 0; index < 2; index ++)
    {
        int index2;


        for (index2 = 0; index2 < 4; index2 ++)
        {
            pack_putc (default_apu -> apus.rectangle [index].regs [index2], file_chunk);
        }
    }


    for (index = 0; index < 3; index ++)
    {
        pack_putc (default_apu -> apus.triangle.regs [index], file_chunk);
    }


    for (index = 0; index < 3; index ++)
    {
        pack_putc (default_apu -> apus.noise.regs [index], file_chunk);
    }


    for (index = 0; index < 4; index ++)
    {
        pack_putc (default_apu -> apus.dmc.regs [index], file_chunk);
    }


    pack_putc (default_apu -> ex_chip, file_chunk);


    pack_fclose_chunk (file_chunk);
}


void papu_load_state (PACKFILE * file, int version)
{
    int index;


    PACKFILE * file_chunk;


    file_chunk = pack_fopen_chunk (file, FALSE);


    for (index = 0; index < 0x16; index ++)
    {
        int value;


        if (index == 0x14)
        {
            continue;
        }


        value = pack_getc (file_chunk);


        /* Write the DMC registers directly. */

        if ((index >= 0x10) && (index <= 0x13))
        {
            default_apu -> apus.dmc.regs [index - 0x10] = value;
        }
        else
        {
            apu_write ((0x4000 + index), value);

            apu_write_cur ((0x4000 + index), value);
        }
    }


    papu_set_exsound (pack_getc (file_chunk));


    pack_fclose_chunk (file_chunk);
}


void papu_set_filter_list (int filters)
{
    apu_setfilterlist ((filter_list = filters));
}


int papu_get_filter_list (void)
{
    return (filter_list);
}

