/*
  Beatforce/ Mixer user interface

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003 John Beuving (john.beuving@home.nl)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <config.h>
#include <stdio.h>
#include <string.h>
#include "SDL_Widget.h"
#include "SDL_Slider.h"
#include "SDL_Font.h"
#include "songdb.h"
#include "playlist.h"

#include "mixer.h"

#include "audio_output.h" 


extern int mixer_dB (int ch, float dB);

/* Local callback functions */
void MixerUI_AutoFadeButtonClicked(void *data);

void *slideroffader;
void *mainvolumeind;
void *mainslider;

void MainVolumeControlHandler(void *data)
{
    SDL_Slider *slider=(SDL_Slider *)data;
    int volume;

    volume=slider->MaxValue - slider->CurrentValue;
    SDL_WidgetPropertiesOf(mainvolumeind,SET_CUR_VALUE,(double)volume);
    output_set_main_volume(volume);
    
}

void FaderControlHandler(void *data)
{
    SDL_Slider *slider=(SDL_Slider *)data;
    MIXER_SetFaderValue(slider->CurrentValue);
}

void MIXERUI_Redraw(double value)
{
    SDL_WidgetPropertiesOf(slideroffader,SET_CUR_VALUE,value);
}

void MIXERUI_CreateWindow()
{
    double val;

    MIXER_SetCallback(MIXERUI_Redraw);

    // Main volume 
    mainslider=SDL_WidgetCreate(SDL_SLIDER,478,4,47,138);    
    SDL_WidgetProperties(SET_BUTTON_IMAGE,THEME_DIR"/beatforce/slibut.bmp");
    SDL_WidgetProperties(SET_MAX_VALUE,100);
    SDL_WidgetProperties(SET_MIN_VALUE,0);
    SDL_WidgetProperties(SET_NORMAL_STEP_SIZE,1.0);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CHANGED,MainVolumeControlHandler,mainslider);


    mainvolumeind=SDL_WidgetCreate(SDL_VOLUMEBAR,527,11,4,124);
    SDL_WidgetProperties(SET_CUR_VALUE,100.0);
    
    /* fade slider */
    slideroffader=SDL_WidgetCreate(SDL_SLIDER,432,162,138,45);    
    SDL_WidgetProperties(SET_CALLBACK,SDL_CHANGED,FaderControlHandler,slideroffader);
    SDL_WidgetProperties(SET_BUTTON_IMAGE,THEME_DIR"/beatforce/slibut_horizontal.bmp");
    MIXER_SetFaderValue(0.0);
    MIXER_GetFaderValue (&val);
    SDL_WidgetProperties(SET_MAX_VALUE,1);
    SDL_WidgetProperties(SET_MIN_VALUE,0);
    SDL_WidgetProperties(SET_CUR_VALUE,val);
    SDL_WidgetProperties(SET_NORMAL_STEP_SIZE,0.1);

    SDL_WidgetCreate(SDL_BUTTON,490,140,20,20);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,MixerUI_AutoFadeButtonClicked,NULL);
}


void MixerUI_AutoFadeButtonClicked(void *data)
{
    if(!MIXER_FadeInProgress())
        MIXER_DoFade(1,0);  //param 1 is autofade param2 is instant
}


