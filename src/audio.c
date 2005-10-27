

/*

FakeNES - A portable, Open Source NES emulator.

Distributed under the Clarified Artistic License.

audio.c: Implementation of the audio interface.

Copyright (c) 2005, Randy McDowell.
Copyright (c) 2005, Charles Bilyue'.

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/


#include <allegro.h>


#include <stdio.h>


#include "audio.h"

#include "gui.h"


#include "misc.h"


#include "timing.h"


#ifdef USE_OPENAL

#include <al.h>

#include <alut.h>


#include "alstream.h"

#endif


static AUDIOSTREAM * audio_stream;

#ifdef USE_OPENAL

static ALSTREAM * al_stream;

#endif


int audio_enable_output = FALSE;


int audio_subsystem = AUDIO_SUBSYSTEM_ALLEGRO;


int audio_sample_rate = 0;

int audio_sample_size = 0;


int audio_buffer_length = 1;


int audio_pseudo_stereo = FALSE;


int audio_hard_sync = FALSE;


volatile int audio_fps = 0;


int audio_unsigned_samples = TRUE;


#define BUFFER_SIZE 2048    /* In samples. */


int audio_init (void)
{
    int speed;


    audio_enable_output = get_config_int ("audio", "enable_output", TRUE);


    audio_subsystem = get_config_int ("audio", "subsystem", AUDIO_SUBSYSTEM_ALLEGRO);


    audio_sample_rate = get_config_int ("audio", "sample_rate", 48000);

    audio_sample_size = get_config_int ("audio", "sample_size", 16);


    audio_pseudo_stereo = get_config_int ("audio", "pseudo_stereo", 3);


    audio_hard_sync = get_config_int ("audio", "hard_sync", FALSE);


    if (audio_enable_output)
    {
        speed = ((machine_type == MACHINE_TYPE_NTSC) ? 60 : 50);


        if (timing_half_speed)
        {
            speed /= 2;
        }
            

        switch (audio_subsystem)
        {
            case AUDIO_SUBSYSTEM_ALLEGRO:
            {
                set_volume_per_voice (0);
        
            
                if (install_sound (DIGI_AUTODETECT, MIDI_NONE, NIL) != 0)
                {
                    return (1);
                }
        
        
                if (digi_driver -> id == DIGI_NONE)
                {
                    return (1);
                }
        
        
                if (! gui_is_active)
                {
                    printf ("Audio initialized: %s (%d kHz).\n\n",
                        digi_driver -> name, (audio_sample_rate / 1000));
                }


                audio_stream = play_audio_stream (((audio_sample_rate /
                    speed) * audio_buffer_length), audio_sample_size,
                        audio_pseudo_stereo, audio_sample_rate, 255, 128);
            
        
                if (! audio_stream)
                {
                    return (2);
                }
            

                audio_unsigned_samples = TRUE;


                break;
            }


            case AUDIO_SUBSYSTEM_OPENAL:
            {
#ifdef USE_OPENAL

                alutInit (0, NIL);

                AL_CHECK ();


                if (! gui_is_active)
                {
                    printf ("Audio initialized: OpenAL (%d kHz).\n\n",
                        (audio_sample_rate / 1000));
                }


                al_stream = play_al_stream (((audio_sample_rate / speed) *
                    audio_buffer_length), audio_sample_size,
                        audio_pseudo_stereo, audio_sample_rate);
            
        
                if (! al_stream)
                {
                    return (2);
                }


                if (audio_sample_size == 16)
                {
                    audio_unsigned_samples = FALSE;
                }
                else
                {
                    audio_unsigned_samples = TRUE;
                }

#endif

                break;
            }
        }
    }


    return (0);
}


void audio_exit (void)
{
    if (audio_enable_output)
    {
        switch (audio_subsystem)
        {
            case AUDIO_SUBSYSTEM_ALLEGRO:
            {
                if (audio_stream)
                {
                    stop_audio_stream (audio_stream);
                }


                remove_sound ();


                break;
            }


            case AUDIO_SUBSYSTEM_OPENAL:
            {
#ifdef USE_OPENAL

                if (al_stream)
                {
                    stop_al_stream (al_stream);
                }


                alutExit ();

#endif

                break;
            }
        }
    }


    set_config_int ("audio", "enable_output", audio_enable_output);


    set_config_int ("audio", "subsystem", audio_subsystem);


    set_config_int ("audio", "sample_rate", audio_sample_rate);

    set_config_int ("audio", "sample_size", audio_sample_size);


    set_config_int ("audio", "pseudo_stereo", audio_pseudo_stereo);


    set_config_int ("audio", "hard_sync", audio_hard_sync);
}


void audio_suspend (void)
{
    if (! audio_enable_output)
    {
        return;
    }


    switch (audio_subsystem)
    {
        case AUDIO_SUBSYSTEM_ALLEGRO:
        {
            voice_stop (audio_stream -> voice);


            break;
        }


        case AUDIO_SUBSYSTEM_OPENAL:
        {
#ifdef USE_OPENAL

            alSourceStop (al_stream -> source);

            AL_CHECK ();

#endif

            break;
        }
    }
}


void audio_resume (void)
{
    if (! audio_enable_output)
    {
        return;
    }


    switch (audio_subsystem)
    {
        case AUDIO_SUBSYSTEM_ALLEGRO:
        {
            voice_start (audio_stream -> voice);


            break;
        }


        case AUDIO_SUBSYSTEM_OPENAL:
        {
#ifdef USE_OPENAL

            alSourcePlay (al_stream -> source);

            AL_CHECK ();

#endif

            break;
        }
    }
}


void audio_poll (void)
{
    if (! audio_enable_output)
    {
        return;
    }


    switch (audio_subsystem)
    {
        case AUDIO_SUBSYSTEM_ALLEGRO:
        {
            if (audio_hard_sync)
            {
                do
                {
                    audio_buffer = get_audio_stream_buffer (audio_stream);


                    rest (0);
                }
                while (! audio_buffer);
            }
            else
            {
                audio_buffer = get_audio_stream_buffer (audio_stream);
            }


            break;
        }


        case AUDIO_SUBSYSTEM_OPENAL:
        {
#ifdef USE_OPENAL

            if (audio_hard_sync)
            {
                do
                {
                    audio_buffer = get_al_stream_buffer (al_stream);


                    rest (0);
                }
                while (! audio_buffer);
            }
            else
            {
                audio_buffer = get_al_stream_buffer (al_stream);
            }

#endif

            break;
        }
    }
}


void audio_play (void)
{
    if (! audio_enable_output)
    {
        return;
    }


    switch (audio_subsystem)
    {
        case AUDIO_SUBSYSTEM_ALLEGRO:
        {
            free_audio_stream_buffer (audio_stream);


            break;
        }

        case AUDIO_SUBSYSTEM_OPENAL:
        {
#ifdef USE_OPENAL

            free_al_stream_buffer (al_stream);

#endif

            break;
        }
    }


    audio_fps += audio_buffer_length;
}
