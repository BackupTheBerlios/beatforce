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
#include "theme.h"

extern int mixer_dB (int ch, float dB);

/* Local callback functions */
void mixerui_AutoFadeButtonClicked(void *data);
static void MIXERUI_FaderChanged(void *data);
void MIXERUI_MainVolumeChanged(void *data);

void *slideroffader;
void *mainvolumeind;
void *mainslider;

void MIXERUI_CreateWindow(ThemeMixer *tm)
{
    double val;
    ThemeImage     *Image     = NULL;
    ThemeButton    *Button    = NULL;
    ThemeSlider    *Slider    = NULL;
    ThemeVolumeBar *VolumeBar = NULL;

    if(tm == NULL)
        return;
        
    Image     = tm->Image;
    VolumeBar = tm->VolumeBar;
    Slider    = tm->Slider;
    Button    = tm->Button;

    while(Image)
    {
        SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_WidgetProperties(SET_NORMAL_IMAGE,Image->filename);
        Image=Image->next;
    }

    while(VolumeBar)
    {
        if(mainvolumeind == NULL)
        {
            // main volume ind
            mainvolumeind=SDL_WidgetCreateR(SDL_VOLUMEBAR,VolumeBar->Rect);
            SDL_WidgetProperties(SET_CUR_VALUE,100.0);
        }
        VolumeBar=VolumeBar->next;
    }


    while(Slider)
    {
        switch(Slider->action)
        {
        case SLIDER_MAIN_VOLUME:
            // Main volume 
            mainslider=SDL_WidgetCreateR(SDL_SLIDER,Slider->Rect);
            SDL_WidgetProperties(SET_BUTTON_IMAGE,Slider->button);
            SDL_WidgetProperties(SET_MAX_VALUE,100);
            SDL_WidgetProperties(SET_MIN_VALUE,0);
            SDL_WidgetProperties(SET_NORMAL_STEP_SIZE,1.0);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CHANGED,MIXERUI_MainVolumeChanged,mainslider);
            break;
        case SLIDER_FADER:
            /* fade slider */
            slideroffader=SDL_WidgetCreateR(SDL_SLIDER,Slider->Rect);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CHANGED,MIXERUI_FaderChanged,slideroffader);
            SDL_WidgetProperties(SET_BUTTON_IMAGE,Slider->button);
            MIXER_SetFaderValue(0.0);
            MIXER_GetFaderValue (&val);
            SDL_WidgetProperties(SET_MAX_VALUE,1);
            SDL_WidgetProperties(SET_MIN_VALUE,0);
            SDL_WidgetProperties(SET_CUR_VALUE,val);
            SDL_WidgetProperties(SET_NORMAL_STEP_SIZE,0.1);
            break;
        }
        Slider=Slider->next;
    }

    while(Button)
    {
        if(Button->action == BUTTON_RESET_FADER)
        {
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,"c:\\beatforce\\themes\\beatforce\\butje.bmp");
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,mixerui_AutoFadeButtonClicked,NULL);
        }
        Button=Button->next;
    }
}

int MIXERUI_Redraw()
{
    int volume;
    double value=0.0;
    int state;
    AUDIOOUTPUT_GetMainVolume(&volume);    

    SDL_WidgetPropertiesOf(mainvolumeind,SET_CUR_VALUE,(double) volume);
    volume= 100.0 - volume;
    SDL_WidgetPropertiesOf(mainslider   ,SET_CUR_VALUE,(double) volume);


    
    SDL_WidgetPropertiesOf(slideroffader,GET_STATE,&state);    
    if(state != SLIDER_DRAG)
    {
        if(MIXER_GetFaderValue(&value))
        {
            SDL_WidgetPropertiesOf(slideroffader,SET_CUR_VALUE,value);    
        }
    }
    
    return 1;
}

/* Internal callback functions */

void mixerui_AutoFadeButtonClicked(void *data)
{
    if(!MIXER_FadeInProgress())
        MIXER_DoFade(1,0);  //param 1 is autofade param2 is instant

}

static void MIXERUI_FaderChanged(void *data)
{
    SDL_Slider *slider=(SDL_Slider *)data;
    MIXER_SetFaderValue(slider->CurrentValue);
}

void MIXERUI_MainVolumeChanged(void *data)
{
    SDL_Slider *slider=(SDL_Slider *)data;
    int volume;

    volume=slider->MaxValue - slider->CurrentValue;
    SDL_WidgetPropertiesOf(mainvolumeind,SET_CUR_VALUE,(double)volume);
    AUDIOOUTPUT_SetMainVolume(volume);
    
}


int MIXERUI_DecreaseMainVolume()
{
    MIXER_DecreaseMainVolume();
    MIXERUI_Redraw();
    return 1;
}


int MIXERUI_IncreaseMainVolume()
{
    MIXER_IncreaseMainVolume();
    MIXERUI_Redraw();
    return 1;
}
