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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

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
int mixer_FadeTimeout (void* ptr);      /* Timer function */


#ifdef __USE_FADER_MUTEX__
int fader_mutex;
#define mutex_unlock( m )	(m=0)
#define mutex_lock( m ) 	while( m );  m = 1
#else
#define mutex_unlock( m )
#define mutex_lock( m )
#endif


extern MixerConfig *mixercfg;
extern struct OutChannel *ch[OUTPUT_N_CHANNELS];

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
        ch[i]->fader_dB = MIXER_DEFAULT_dB;
    /*if (mixer_read_config () != 0)
        mixer_write_config ();*/

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

    //mixer_write_config( );
    return 0;
}

int MIXER_SetCallback(void *callback)
{
    return 1;
}

int
mixer_GroupA (int ch, int on)
{
    return output_set_group (ch, GROUP_A, on);
}

int
mixer_GroupB (int ch, int on)
{
    return output_set_group (ch, GROUP_B, on);
}

int
mixer_Master (int ch, int on)
{
    return output_set_group (ch, GROUP_MASTER, on);
}

int
mixer_mute (int ch, int mute)
{
    return output_mute (ch, mute);
}

int
mixer_mute_group (int group, int mute)
{
    if (group > 2 || group < 0)
        return ERROR_UNKNOWN_CHANNEL;

    return 0;
}

int
mixer_dB (int ch, float dB)
{
    return output_set_volume (ch, dB);
}

int MIXER_DoFade(int autofade, int instant)
{
    void *win =NULL;
    int from = -1;
    int to   = -1;
    
    if (PLAYER_IsPlaying (0))
        from = 0;
    
    if (PLAYER_IsPlaying (1))
    {
        if (from != -1)
        {
            LOG("autofade: 2 players are playing, can't decide which to fade");
            return -1;
        }
        from = 1;
    }
    
    if (from == -1)
    {
        LOG("autofade: no player playing!");
        return -1;
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
        player_next_track (from);
//	  player_track_rf (win, 1);
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
            PLAYER_Play(to);
            cfader->in_progress = TRUE;
            cfader->autofade = autofade;
            if(cfader->fade_time == 0)
                cfader->fade_time = MIXER_DEFAULT_CF_FadeTime *10;

            cfader->timer =
                //SDL_AddTimer(250,fade_timeout,NULL);
                OSA_StartTimer(cfader->fade_time / (long) (1 / step_width),
                               mixer_FadeTimeout, (void*) win);
        }
        else
        {
            printf ("fade in progress.\n");
        }

    }

    return 0;
}

int mixer_FadeTimeout (void* ptr)
{
    int end = 0;

    
    if (cfader->inc < 0 && cfader->value < 0.0000000)
        end = 1;

    if (cfader->inc > 0 && cfader->value >= 1)
        end = 1;

    
    if (end)
    {
        PLAYER_Pause (cfader->from_player);
        if( !cfader->autofade )
        {
            /* fade has ended and was not autofade */
            if( mixercfg->TForwAfterFadeNow )
                player_next_track(cfader->from_player );
        }

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
        if( !cfader->autofade )
        {
            /* fade has ended and was not autofade */
            if( mixercfg->TForwAfterFadeNow )
                player_next_track(cfader->from_player );
        }

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
        return 0.50;
    }

    if (&cfader->value == NULL)
    {
        mutex_unlock (fader_mutex);
        return 0.50;
    }

    mutex_unlock (fader_mutex);

    if (cfader->value == -0)
    {
        *val=0.000000;
        cfader->value = *val;
        return 0;
    }


    if(val)
    {
        *val = cfader->value;
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

/*
 * Config stuff ...
 */

/*
int
mixer_read_config (void)
{
    ConfigFile *cfg;
    int ret = 0;
    int i;

    cfg =
        bf_cfg_open_file (g_strconcat
                          (g_get_home_dir (), "/.beatforce/mixer", NULL));
    if (cfg == NULL)
    {
        ret = 1;
        cfg = bf_cfg_new ();
    }


    for (i = 0; i < OUTPUT_N_CHANNELS; i++)
    {
        char name[64];



        sprintf (name, "%d.dB", i);
        if (!bf_cfg_read_float (cfg, "channels", name, &ch[i]->fader_dB))
        {
            ch[i]->fader_dB = MIXER_DEFAULT_dB;
            ret = 1;
        }

        sprintf (name, "%d.Mute", i);
        if (!bf_cfg_read_boolean
            (cfg, "channels", name, (gboolean *) & ch[i]->mute))
        {
            ch[i]->mute = MIXER_DEFAULT_Mute;
            ret = 1;
        }

        sprintf (name, "%d.Mask", i);
        if (!bf_cfg_read_int (cfg, "channels", name, (int *) &ch[i]->mask))
        {
            ch[i]->mask = MIXER_DEFAULT_Mask;
            ret = 1;
        }
    }


    if (!bf_cfg_read_float
        (cfg, "CrossFader", "Value", (float *) &cfader->value))
    {
        cfader->value = MIXER_DEFAULT_CF_Value;
        ret = 1;
    }

#if 0
  if (!bf_cfg_read_boolean
  (cfg, "CrossFader", "auto", (gboolean *) & cfader->autofade))
  {
  cfader->autofade = 0;
  ret = 1;
  }
#endif
    cfader->autofade = 0;

    if (!bf_cfg_read_int
        (cfg, "CrossFader", "FadeTime", (int *) &cfader->fade_time))
    {
        cfader->fade_time = MIXER_DEFAULT_CF_FadeTime;
        ret = 1;
    }

    bf_cfg_free (cfg);



    return ret;
}


int
mixer_write_config (void)
{
    ConfigFile *cfg;
    int ret = 0;
    int i;

    cfg = bf_cfg_new ();
    g_return_val_if_fail( cfg != NULL, -1 );

    for (i = 0; i < OUTPUT_N_CHANNELS; i++)
    {
        char name[64];

        sprintf (name, "%d.dB", i);
        bf_cfg_write_float (cfg, "channels", name, ch[i]->fader_dB);

        sprintf (name, "%d.Mute", i);
        bf_cfg_write_boolean (cfg, "channels", name, (gboolean) ch[i]->mute);

        sprintf (name, "%d.Mask", i);
        bf_cfg_write_int (cfg, "channels", name, (int) ch[i]->mask);

    }

    bf_cfg_write_float (cfg, "CrossFader", "Value", cfader->value);

#if 0
  bf_cfg_write_boolean (cfg, "CrossFader", "auto", cfader->autofade);
#endif

    bf_cfg_write_int (cfg, "CrossFader", "FadeTime", cfader->fade_time);

    ret = bf_cfg_write_file (cfg,
                             g_strconcat (g_get_home_dir (), "/.beatforce/mixer",
                                          NULL));
    if( ret == FALSE )
    {
        printf( "Error writing mixer config file!\n" );
    }
    bf_cfg_free (cfg);

    return 0;
}*/

