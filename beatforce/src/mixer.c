/*
   BeatForce
   mixer.c	- mixer interface - real mixing takes place in audio_output.c !!
   
   Copyright (c) 2003, John Beuving (john.beuving@home.nl)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public Licensse as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>


#include "audio_output.h"
#include "configfile.h"
#include "osa.h"
#include "output.h"
#include "mixer.h"
#include "err.h"
#include "player.h"

#define MODULE_ID MIXER
#include "debug.h"


/* Local protoypes */
static int MIXER_FadeTimeout();      /* Timer function */


#ifdef __USE_FADER_MUTEX__
int fader_mutex;
#define mutex_unlock( m )	(m=0)
#define mutex_lock( m ) 	while( m );  m = 1
#else
#define mutex_unlock( m )
#define mutex_lock( m )
#endif


int mixer_timeout (void*);
CrossFader *cfader;
int curr_playing;

int update_timer;

int beat_fallof_timer[2];
int beat_fallof_time = 100;

int MIXER_Init ()
{
    int i;

    cfader = malloc (sizeof (CrossFader));
    memset( cfader, 0, sizeof(CrossFader));

    for (i = 0; i < OUTPUT_N_CHANNELS; i++)
    {
        AUDIOOUTPUT_SetVolume(i,MIXER_DEFAULT_dB);
    }

    cfader->value = 0;
    cfader->timer = 0;
    cfader->from_player = 0;
    cfader->in_progress = FALSE;

    mutex_unlock (fader_mutex);



    return 0;
}

int mixer_finalize ()
{
    mutex_unlock (fader_mutex);
    return 0;
}

int MIXER_SetCallback(void *callback)
{
    return 1;
}

int MIXER_Mute (int ch, int mute)
{
    return AUDIOOUTPUT_Mute (ch, mute);
}

int MIXER_MuteGroup (int group, int mute)
{
    if (group > 2 || group < 0)
    {
        ERROR("Unknown channel");
        return 0;
    }
    return 1;
}

int mixer_dB (int ch, float dB)
{
    return AUDIOOUTPUT_SetVolume(ch, dB);
}

int MIXER_DoFade(int autofade, int instant)
{
    int from = -1;
    int to   = -1;
    
    if (PLAYER_IsPlaying (0))
        from = 0;
    
    if (PLAYER_IsPlaying (1))
    {
        if (from != -1)
        {
            LOG("autofade: 2 players are playing, can't decide which to fade");
            return 0;
        }
        from = 1;
    }
    
    if (from == -1)
    {
        LOG("autofade: no player playing!");
        return 0;
    }
    
    if (from == 1)
        to = 0;
    else
        to = 1;
    
    cfader->to_player = to;
    cfader->from_player = from;

    if (instant)
    {
        PLAYER_Pause (from);
        if (!MIXER_FadeInProgress())
        {
            PLAYER_Play (to);
        }

    }
    else
    {
        float step_width = 0.05000000;

        if (!MIXER_FadeInProgress ())
        {
            MIXER_GetFaderValue (&cfader->value);
            cfader->inc = ((to == 1) ? (step_width) : (-step_width));
            if(PLAYER_Play(to))
            {
                cfader->in_progress = TRUE;
                cfader->autofade = autofade;
                if(cfader->fade_time == 0)
                    cfader->fade_time = MIXER_DEFAULT_CF_FadeTime *10;

#if 0
                cfader->timer =
                    OSA_StartTimer(cfader->fade_time / (long) (1 / step_width),
                                   MIXER_FadeTimeout, (void*) win);
#endif
                cfader->timer = OSA_StartTimer(1, MIXER_FadeTimeout, NULL);

            }
        }
        else
        {
            ERROR("Fade already in progress");
            return 0;
        }

    }
    return 1;
}

static int MIXER_FadeTimeout ()
{
    int end = 0;

    
    if (cfader->inc < 0 && cfader->value < 0.0000000)
        end = 1;

    if (cfader->inc > 0 && cfader->value >= 1)
        end = 1;

    
    if (end)
    {
        PLAYER_Pause (cfader->from_player);
        cfader->timer = 0;
        cfader->in_progress = FALSE;
        cfader->from_player = 0;
        return 0;
    }


    cfader->value += cfader->inc;
    if(cfader->value < 0.00)
    {
        cfader->value = 0.000000;
        end=1;
    }
    MIXER_SetFaderValue (cfader->value);


    if (end)
    {
        PLAYER_Pause (cfader->from_player);
        cfader->timer = 0;
        cfader->in_progress = FALSE;
        cfader->from_player = 0;

        return 0;
    }
    if(cfader->value == 1.000)
        return 0;

    return 250;
}

int MIXER_FadeInProgress (void)
{
    return cfader->in_progress;
}

int MIXER_GetAutofade(void)
{

    mutex_lock (fader_mutex);

    if (cfader == NULL)
    {
        mutex_unlock (fader_mutex);
        return -1;
    }

    if (&cfader->value == NULL)
    {
        mutex_unlock (fader_mutex);
        return -1;
    }

    mutex_unlock (fader_mutex);

    return cfader->autofade;
}

int MIXER_GetFadeTime(void)
{
    return cfader->fade_time;
}

int MIXER_SetAutofade(int autofade)
{

    mutex_lock (fader_mutex);

    if (cfader == NULL)
    {
        mutex_unlock (fader_mutex);
        return -1;
    }

    if (&cfader->value == NULL)
    {
        mutex_unlock (fader_mutex);
        return -1;
    }
    cfader->autofade = (int) (autofade != 0);
    mutex_unlock (fader_mutex);
    return 0;
}

int MIXER_GetFaderValue (double *val)
{

    mutex_lock (fader_mutex);

    if (cfader == NULL)
    {
        mutex_unlock (fader_mutex);
        return 0;
    }

    if (&cfader->value == NULL)
    {
        mutex_unlock (fader_mutex);
        return 0;
    }

    mutex_unlock (fader_mutex);

    if (cfader->value == -0)
    {
        *val=0.000000;
        cfader->value = *val;
        return 1;
    }


    if(val)
    {
        *val = cfader->value;
        return 1;
    }
    else
    {
        return 0;
    }
    return 0;
}

int MIXER_SetFaderValue (double value)
{

    mutex_lock (fader_mutex);

    if (cfader == NULL)
    {
        mutex_unlock (fader_mutex);
        return -1;
    }

    if (&cfader->value == NULL)
    {
        mutex_unlock (fader_mutex);
        return -1;
    }
    cfader->value = value;
    mutex_unlock (fader_mutex);

    return 0;
}

int MIXER_DecreaseMainVolume()
{
    int db=0;
    AUDIOOUTPUT_GetMainVolume(&db);
    db-=5;
    if(db < 0)
        db=0;
    AUDIOOUTPUT_SetMainVolume(db);
    return 1;
}

int MIXER_IncreaseMainVolume()
{
    int db=0;
    AUDIOOUTPUT_GetMainVolume(&db);
    db+=5;
    if(db > 100)
        db=100;
    AUDIOOUTPUT_SetMainVolume(db);
    return 1;
}


