/*
   BeatForce
   mixer.h  - mixer interface
   
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

#ifndef __MIXER_H__
#define __MIXER_H__


#define MIXER_DEFAULT_dB	(0.0)
#define MIXER_DEFAULT_Mask  (0)
#define MIXER_DEFAULT_Mute  (FALSE)

#define MIXER_DEFAULT_CF_Value	 (0.50)
#define MIXER_DEFAULT_CF_Auto	 (TRUE)
#define MIXER_DEFAULT_CF_FadeTime  (5000)


typedef struct
{
    double value;			/* 0.0 - 1.0 */
    int autofade; 		/* autofade */
    int fade_time;		/* fade_time in msec */
    int mask;

    int in_progress;
    int timer;
    float inc;
    int from_player;
    int to_player;
}
CrossFader;


int MIXER_Init();
int MIXER_FadeInProgress (void);
int MIXER_GetAutofade(void);
int MIXER_SetAutofade(int autofade);
int MIXER_GetFadeTime(void);

int MIXER_GetFaderValue (double *value);
int MIXER_SetFaderValue (double value);

int MIXER_DoFade(int autofade, int instant);
int MIXER_SetCallback(void *callback);

int MIXER_DecreaseMainVolume();
int MIXER_IncreaseMainVolume();


#endif
