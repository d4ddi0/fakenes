

/*

FakeNES - A portable, open-source NES emulator.

audio.c: Implementation of the audio interface.

Copyright (c) 2001, Randy McDowell and Ian Smith.
All rights reserved.  See 'LICENSE' for details.

*/


#include <allegro.h>


#include "audio.h"


AUDIOSTREAM * audio_stream;


int audio_enable_output = FALSE;


int audio_sample_rate = 0;

int audio_sample_size = 0;


int audio_buffer_length = 0;


int audio_pseudo_stereo = FALSE;


int audio_init (void)
{
    audio_enable_output =
        get_config_int ("audio", "enable_output", TRUE);


    if (audio_enable_output)
    {
        reserve_voices (1, 0);
    
        if (install_sound (DIGI_AUTODETECT, MIDI_NONE, NULL) != 0)
        {
            return (1);
        }
    }


    /* Sampling rate & size. */

    audio_sample_rate =
        get_config_int ("audio", "sample_rate", 22050);

    audio_sample_size =
        get_config_int ("audio", "sample_size", 16);


    /* Buffer length in frames. */

    audio_buffer_length =
        get_config_int ("audio", "buffer_length", 10);


    audio_pseudo_stereo =
        get_config_int ("audio", "pseudo_stereo", TRUE);


    if (audio_enable_output)
    {
        if (audio_pseudo_stereo)
        {
            audio_stream = play_audio_stream (AUDIO_BUFFER_SIZE,
                audio_sample_size, TRUE, audio_sample_rate, 255, 128);
        }
        else
        {
            audio_stream = play_audio_stream (AUDIO_BUFFER_SIZE,
                audio_sample_size, FALSE, audio_sample_rate, 255, 128);
        }
    
        if (! audio_stream)
        {
            return (1);
        }
    }


    return (0);
}


void audio_exit (void)
{
    if (audio_enable_output)
    {
        remove_sound ();
    }


    set_config_int ("audio", "enable_output", audio_enable_output);


    set_config_int ("audio", "sample_rate", audio_sample_rate);

    set_config_int ("audio", "sample_size", audio_sample_size);


    set_config_int ("audio", "buffer_length", audio_buffer_length);


    set_config_int ("audio", "pseudo_stereo", audio_pseudo_stereo);
}


void audio_update (void)
{
    audio_buffer = get_audio_stream_buffer (audio_stream);

    if (audio_buffer)
    {
        voice_stop (audio_stream -> voice);
    }
}


void audio_suspend (void)
{
    if (audio_enable_output)
    {
        voice_stop (audio_stream -> voice);
    }
}


void audio_resume (void)
{
    if (audio_enable_output)
    {
        voice_start (audio_stream -> voice);
    }
}


void audio_start (void)
{
    audio_buffer = get_audio_stream_buffer (audio_stream);
}


void audio_stop (void)
{
    voice_start (audio_stream -> voice);

    free_audio_stream_buffer (audio_stream);
}
