

/*

FakeNES - A portable, Open Source NES emulator.

papu.c: Implementation of the APU interface.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

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


int papu_enable_square_1 = TRUE;

int papu_enable_square_2 = TRUE;


int papu_enable_triangle = TRUE;

int papu_enable_noise = TRUE;


int papu_enable_dmc = TRUE;


int papu_swap_channels = 0;


int papu_ideal_triangle = FALSE;


int papu_smooth_envelope = TRUE;

int papu_smooth_sweep = TRUE;


int papu_linear_echo = TRUE;

int papu_surround_sound = TRUE;


static void * echo_buffer_a = NULL;

static void * echo_buffer_b = NULL;


static int echo_buffer_size = 0;


void papu_update (void)
{
    apu_setchan (0, papu_enable_square_1);

    apu_setchan (1, papu_enable_square_2);


    apu_setchan (2, papu_enable_triangle);

    apu_setchan (3, papu_enable_noise);


    apu_setchan (4, papu_enable_dmc);


    apu_setmode (APUMODE_IDEAL_TRIANGLE, papu_ideal_triangle);


    apu_setmode (APUMODE_SMOOTH_ENVELOPE, papu_smooth_envelope);

    apu_setmode (APUMODE_SMOOTH_SWEEP, papu_smooth_sweep);
}


int papu_reinit (void)
{
    int speed;


    speed = (machine_type == MACHINE_TYPE_NTSC ? 60 : 50);


    apu_setparams (audio_sample_rate, speed, 0, audio_sample_size);


    if ((! papu_linear_echo) || (! audio_enable_output))
    {
        return (0);
    }


    if (echo_buffer_a)
    {
        free (echo_buffer_a);
    }

    if (echo_buffer_b)
    {
        free (echo_buffer_b);
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


    echo_buffer_a = malloc (echo_buffer_size);

    echo_buffer_b = malloc (echo_buffer_size);

    if ((! echo_buffer_a) || (! echo_buffer_b))
    {
        return (2);
    }


    if (audio_sample_size == 16)
    {
        int index;


        UINT16 * buffer_a = echo_buffer_a;

        UINT16 * buffer_b = echo_buffer_b;


        for (index = 0; index < (echo_buffer_size / 2); index ++)
        {
            buffer_a [index] = 0x8000;

            buffer_b [index] = 0x8000;
        }
   }
   else
   {
        int index;


        UINT8 * buffer_a = echo_buffer_a;

        UINT8 * buffer_b = echo_buffer_b;


        for (index = 0; index < echo_buffer_size; index ++)
        {
            buffer_a [index] = 0x80;

            buffer_b [index] = 0x80;
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

    papu_surround_sound = get_config_int ("papu", "surround_sound", TRUE);


    if ((result = papu_reinit ()) != 0)
    {
        return (result);
    }

    
    papu_filter_type = get_config_int ("audio", "filter_type", APU_FILTER_DYNAMIC);

    apu_setfilter (papu_filter_type);


    papu_swap_channels = get_config_int ("audio", "swap_channels", FALSE);


    papu_enable_square_1 = get_config_int ("papu", "enable_square_1", TRUE);

    papu_enable_square_2 = get_config_int ("papu", "enable_square_2", TRUE);

    papu_enable_triangle = get_config_int ("papu", "enable_triangle", TRUE);

    papu_enable_noise = get_config_int ("papu", "enable_noise", TRUE);

    papu_enable_dmc = get_config_int ("papu", "enable_dmc", TRUE);


    papu_ideal_triangle = get_config_int ("papu", "ideal_triangle", FALSE);


    papu_smooth_envelope = get_config_int ("papu", "smooth_envelope", TRUE);

    papu_smooth_sweep = get_config_int ("papu", "smooth_sweep", TRUE);


    papu_update ();


    return (0);
}


void papu_exit (void)
{
    apu_destroy (&default_apu);


    if (echo_buffer_a)
    {
        free (echo_buffer_a);
    }

    if (echo_buffer_b)
    {
        free (echo_buffer_b);
    }


    set_config_int ("audio", "filter_type", papu_filter_type);

    set_config_int ("audio", "swap_channels", papu_swap_channels);


    set_config_int ("papu", "enable_square_1", papu_enable_square_1);

    set_config_int ("papu", "enable_square_2", papu_enable_square_2);

    set_config_int ("papu", "enable_triangle", papu_enable_triangle);

    set_config_int ("papu", "enable_noise", papu_enable_noise);

    set_config_int ("papu", "enable_dmc", papu_enable_dmc);


    set_config_int ("papu", "ideal_triangle", papu_ideal_triangle);


    set_config_int ("papu", "smooth_evelope", papu_smooth_envelope);

    set_config_int ("papu", "smooth_sweep", papu_smooth_sweep);


    set_config_int ("papu", "linear_echo", papu_linear_echo);

    set_config_int ("papu", "surround_sound", papu_surround_sound);
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


static INLINE void apply_echo (void)
{
    int offset;


    UINT8 * buffer = audio_buffer;


    UINT8 * buffer_a = echo_buffer_a;

    UINT8 * buffer_b = echo_buffer_b;


    for (offset = 0; offset < echo_buffer_size; offset ++)
    {
        buffer_b [offset] /= 2;

        buffer_b [offset] += (buffer_a [offset] / 2);
    }


    for (offset = 0; offset < echo_buffer_size; offset ++)
    {
        buffer [offset] /= 2;

        buffer [offset] += (buffer_b [offset] / 2);
    }
}


static int is_recording = FALSE;

static FILE * dump_file = NULL;


int papu_start_record (void)
{
    dump_file = fopen ("apudump.raw", "wb");

    if (! dump_file)
    {
        return (1);
    }


    is_recording = TRUE;


    return (0);
}


void papu_stop_record (void)
{
    if (is_recording)
    {
        fclose (dump_file);

        is_recording = FALSE;
    }
}


static int frames = 0;


#define PAPU_BUFFER_SIZE    ((audio_sample_rate / speed) * frames)


static int fast_forward = FALSE;

static int current_buffer = 0;


void papu_process (void)
{
    int speed;


    speed = (machine_type == MACHINE_TYPE_NTSC ? 60 : 50);


    if (key [KEY_TILDE])
    {
        if (! fast_forward)
        {
            audio_suspend ();

            fast_forward = TRUE;
        }


        apu_process (NULL, (audio_sample_rate / speed));

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
                apu_process_stereo (audio_buffer, PAPU_BUFFER_SIZE, papu_swap_channels, papu_surround_sound);
            }
            else
            {
                apu_process (audio_buffer, PAPU_BUFFER_SIZE);
            }


            if (papu_linear_echo)
            {
                apply_echo ();


                if (current_buffer == 1)
                {
                    memcpy (echo_buffer_a, audio_buffer, echo_buffer_size);
  
                    current_buffer = 0;
                }
                else
                {
                    memcpy (echo_buffer_b, audio_buffer, echo_buffer_size);
  
                    current_buffer = 1;
                }
            }


            if (is_recording)
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
}
