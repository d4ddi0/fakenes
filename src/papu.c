

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

papu.c: Implementation of the APU abstraction.

Copyright (c) 2001-2006, Randy McDowell.
Copyright (c) 2001-2006, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <allegro.h>


#include <math.h>

#include <stdio.h>

#include <stdlib.h>

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


int papu_interpolate = 0;

int papu_spatial_stereo = 0;


int papu_dithering = FALSE;


int papu_is_recording = FALSE;


static int filter_list = 0;


static void * papu_buffer_base = NIL;

static void * papu_buffer = NIL;


static int papu_buffer_frame_size = 0;

static int papu_buffer_size = 0;


static int papu_frame_size = 0;


#define ECHO_DEPTH  3


static void * echo_buffers [ECHO_DEPTH];


#define INTERPOLATION_POINTS    (pow (2, papu_interpolate))


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

    int rate;


    int index;


    speed = (machine_type == MACHINE_TYPE_NTSC ? 60 : 50);


    if (timing_half_speed)
    {
        speed /= 2;
    }


    rate = audio_sample_rate;

    if (papu_interpolate > 0)
    {
        rate *= INTERPOLATION_POINTS;
    }

    apu_setparams (rate, speed, audio_sample_size);


    if (! audio_enable_output)
    {
        return (0);
    }


    papu_buffer_frame_size = papu_frame_size = (rate / speed);

    if (audio_sample_size == 16)
    {
        papu_buffer_frame_size *= 2;
    }

    if (audio_pseudo_stereo)
    {
        papu_buffer_frame_size *= 2;
    }                


    papu_buffer_size = (papu_buffer_frame_size * audio_buffer_length);


    if (papu_buffer_base)
    {
        free (papu_buffer_base);
    }

    papu_buffer = papu_buffer_base = malloc (papu_buffer_size);

    if (! papu_buffer_base)
    {
        return (1);
    }


    if (audio_sample_size == 16)
    {
        UINT16 * buffer = papu_buffer_base;


        for (index = 0; index < (papu_buffer_size / 2); index ++)
        {
            buffer [index] = 0x8000;
        }
    }
    else
    {
        UINT8 * buffer = papu_buffer_base;


        for (index = 0; index < papu_buffer_size; index ++)
        {
            buffer [index] = 0x80;
        }
    }


    if (! papu_linear_echo)
    {
        return (0);
    }


    for (index = 0; index < ECHO_DEPTH; index ++)
    {
        if (echo_buffers [index])
        {
            free (echo_buffers [index]);
        }


        echo_buffers [index] = malloc (papu_buffer_frame_size);


        /* Fix me: memory leak. */

        if (! echo_buffers [index])
        {
            free (papu_buffer_base);


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


            for (offset = 0; offset < (papu_buffer_frame_size / 2); offset ++)
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


            for (offset = 0; offset < papu_buffer_frame_size; offset ++)
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

    default_apu = apu_create (audio_sample_rate, 60, audio_sample_size);

    if (! default_apu)
    {
        return (1);
    }


    papu_linear_echo = get_config_int ("papu", "linear_echo", FALSE);


    papu_interpolate = get_config_int ("audio", "interpolate", 0);

    papu_spatial_stereo = get_config_int ("audio", "spatial_stereo", FALSE);


    if ((result = papu_reinit ()) != 0)
    {
        return (result);
    }

    
    filter_list = get_config_int ("audio", "filter_list", 0);

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


    if (papu_buffer_base)
    {
        free (papu_buffer_base);
    }


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


    set_config_int ("audio", "interpolate", papu_interpolate);

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


    UINT8 * buffer = papu_buffer;


    UINT8 * buffers [ECHO_DEPTH];


    int offset;


    for (index = 0; index < ECHO_DEPTH; index ++)
    {
        buffers [index] = echo_buffers [index];
    }


    for (offset = 0; offset < papu_buffer_frame_size; offset ++)
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


static INLINE void interpolate (void)
{
    int total_points = INTERPOLATION_POINTS;


    int points = 0;


    void * start;


    if (papu_interpolate <= 0)
    {
        return;
    }


    start = (papu_buffer_base + ((papu_buffer - papu_buffer_base) / total_points));


    if (audio_pseudo_stereo)
    {
        int bytes = 0;


        void * read_buffer = papu_buffer;

        void * write_buffer = start;


        int total_left = 0;

        int total_right = 0;


        while (bytes < papu_buffer_frame_size)
        {
            if (audio_sample_size == 16)
            {
                UINT16 * buffer = read_buffer;


                total_left += * buffer ++;

                total_right += * buffer ++;


                read_buffer = buffer;


                bytes += 4;
            }
            else
            {
                UINT8 * buffer = read_buffer;


                total_left += * buffer ++;

                total_right += * buffer ++;


                read_buffer = buffer;


                bytes += 2;
            }


            if (++ points == total_points)
            {
                points = 0;


                total_left = (((double) total_left / total_points) + 0.5);

                total_right = (((double) total_right / total_points) + 0.5);


                if (audio_sample_size == 16)
                {
                    UINT16 * buffer = write_buffer;


                    * buffer ++ = total_left;

                    * buffer ++ = total_right;


                    write_buffer = buffer;
                }
                else
                {
                    UINT8 * buffer = write_buffer;


                    * buffer ++ = total_left;

                    * buffer ++ = total_right;


                    write_buffer = buffer;
                }


                total_left = 0;

                total_right = 0;
            }
        }
    }
    else
    {
        int bytes = 0;


        void * read_buffer = papu_buffer;

        void * write_buffer = start;


        int total = 0;


        while (bytes < papu_buffer_frame_size)
        {
            if (audio_sample_size == 16)
            {
                UINT16 * buffer = read_buffer;


                total += * buffer ++;


                read_buffer = buffer;


                bytes += 2;
            }
            else
            {
                UINT8 * buffer = read_buffer;


                total += * buffer ++;


                read_buffer = buffer;


                bytes ++;
            }


            if (++ points == total_points)
            {
                points = 0;


                total = (((double) total / total_points) + 0.5);


                if (audio_sample_size == 16)
                {
                    UINT16 * buffer = write_buffer;


                    * buffer ++ = total;


                    write_buffer = buffer;
                }
                else
                {
                    UINT8 * buffer = write_buffer;


                    * buffer ++ = total;


                    write_buffer = buffer;
                }


                total = 0;
            }
        }
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


static INLINE void purge (void)
{
    int size;


    if (papu_interpolate > 0)
    {
        size = (papu_buffer_size / INTERPOLATION_POINTS);
    }
    else
    {
        size = papu_buffer_size;
    }


    if (papu_is_recording)
    {
        fwrite (papu_buffer_base, size, 1, dump_file);
    }


    papu_buffer = papu_buffer_base;


    audio_poll ();


    if (! audio_buffer)
    {
        return;
    }


    memcpy (audio_buffer, papu_buffer_base, size);


    audio_play ();
}


void papu_process (void)
{
    if (audio_enable_output)
    {
        /* Check if buffer is full. (TODO: Check math.) */

        if ((papu_buffer - papu_buffer_base) > (papu_buffer_size - papu_buffer_frame_size))
        {
            /* Scrap the oldest frame of audio in the buffer, to make room
               for the new frame of audio. */

            memcpy (papu_buffer_base, (papu_buffer_base + papu_buffer_frame_size), (papu_buffer_size - papu_buffer_frame_size));

            papu_buffer -= papu_buffer_frame_size;
        }


        /* Emulate and render a frame. */

        if (audio_pseudo_stereo)
        {
            apu_process_stereo (papu_buffer, papu_frame_size, papu_dithering, audio_pseudo_stereo, papu_swap_channels, papu_spatial_stereo);
        }
        else
        {
            apu_process (papu_buffer, papu_frame_size, papu_dithering);
        }


        /* Do any postprocessing. */

        if (papu_linear_echo)
        {
            int index;


            for (index = 1; index < ECHO_DEPTH; index ++)
            {
                memcpy (echo_buffers [(index - 1)], echo_buffers [index], papu_buffer_frame_size);
            }


            memcpy (echo_buffers [(ECHO_DEPTH - 1)], papu_buffer, papu_buffer_frame_size);


            apply_echo ();
        }


        if (papu_interpolate > 0)
        {
            interpolate ();
        }


        papu_buffer += papu_buffer_frame_size;


        purge ();
    }
    else
    {
        /* Just emulate a frame. */

        apu_process (NIL, papu_frame_size, FALSE);
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

