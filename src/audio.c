

/*

FakeNES - A portable, Open Source NES emulator.

audio.c: Implementation of the audio interface.

Copyright (c) 2002, Randy McDowell and Ian Smith.
Portions copyright (c) 2002, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <allegro.h>


#include "audio.h"


#include "misc.h"


#include "timing.h"


SAMPLE * audio_sample = NULL;

int audio_voice = 0;


int audio_enable_output = FALSE;


int audio_sample_rate = 0;

int audio_sample_size = 0;


int audio_buffer_length = 0;


int audio_pseudo_stereo = FALSE;


static int disable_wait = FALSE;


volatile int audio_fps = 0;


int audio_init (void)
{
    int speed;


    audio_enable_output = get_config_int ("audio", "enable_output", TRUE);


    audio_sample_rate = get_config_int ("audio", "sample_rate", 22050);

    audio_sample_size = get_config_int ("audio", "sample_size", 16);


    audio_buffer_length = get_config_int ("audio", "buffer_length", 10);


    audio_pseudo_stereo = get_config_int ("audio", "pseudo_stereo", TRUE);


    disable_wait = get_config_int ("audio", "disable_wait", TRUE);


    if (audio_enable_output)
    {
        set_volume_per_voice (0);


        if (install_sound (DIGI_AUTODETECT, MIDI_NONE, NULL) != 0)
        {
            audio_enable_output = FALSE;

            return (1);
        }


        speed = (machine_type == MACHINE_TYPE_NTSC ? 60 : 50);


        audio_sample = create_sample (audio_sample_size, audio_pseudo_stereo,
            audio_sample_rate, ((audio_sample_rate / speed) * audio_buffer_length));


        if (! audio_sample)
        {
            remove_sound ();


            audio_enable_output = FALSE;

            return (1);
        }

        audio_voice = allocate_voice (audio_sample);


        audio_buffer = audio_sample -> data;
    }


    return (0);
}


void audio_exit (void)
{
    if (audio_enable_output)
    {
        remove_sound ();


        if (audio_sample)
        {
            destroy_sample (audio_sample);

            deallocate_voice (audio_voice);
        }
    }


    set_config_int ("audio", "enable_output", audio_enable_output);


    set_config_int ("audio", "sample_rate", audio_sample_rate);

    set_config_int ("audio", "sample_size", audio_sample_size);


    set_config_int ("audio", "buffer_length", audio_buffer_length);


    set_config_int ("audio", "pseudo_stereo", audio_pseudo_stereo);


    set_config_int ("audio", "disable_wait", disable_wait);
}


void audio_start (void)
{
    voice_start (audio_voice);
}


void audio_stop (void)
{
    int offset;


    if (! disable_wait)
    {
        do
        {
            offset = voice_get_position (audio_voice);
        }
        while (offset != -1);
    }


    voice_stop (audio_voice);


    audio_fps += audio_buffer_length;
}


void audio_suspend (void)
{
    if (audio_enable_output)
    {
        voice_stop (audio_voice);
    }
}


void audio_resume (void)
{
    if (audio_enable_output)
    {
        voice_start (audio_voice);
    }
}
