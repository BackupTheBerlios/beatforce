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
#include "mixer_ui.h"

#include "audio_output.h" 
#include "theme.h"
#include "player.h"
#include "sampler.h"

/* Local callback functions */
static void MIXERUI_AutoFadeButtonClicked(void *data);
static void MIXERUI_FaderChanged(void *data);
void MIXERUI_MainVolumeChanged(void *data);

void playsample(void *s)
{
    SAMPLER_Play(8);
}

void playsample2(void *s)
{
    SAMPLER_Play(1);
}

void* MIXERUI_CreateWindow(ThemeMixer *tm)
{
    MixerWidgets *w;
    double val;
    ThemeImage     *Image     = NULL;
    ThemeButton    *Button    = NULL;
    ThemeSlider    *Slider    = NULL;
    ThemeVolumeBar *VolumeBar = NULL;

    if(tm == NULL)
        return NULL;
        
    Image     = tm->Image;
    VolumeBar = tm->VolumeBar;
    Slider    = tm->Slider;
    Button    = tm->Button;
    
    w=malloc(sizeof(MixerWidgets));
    
    while(Image)
    {
        SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_WidgetProperties(SET_IMAGE,IMG_Load(Image->filename));
        Image=Image->next;
    }

    while(VolumeBar)
    {
        w->MainVolumeIndicator=SDL_WidgetCreateR(SDL_VOLUMEBAR,VolumeBar->Rect);
        SDL_WidgetProperties(SET_CUR_VALUE,100.0);
        VolumeBar=VolumeBar->next;
    }


    while(Slider)
    {
        switch(Slider->action)
        {
        case SLIDER_MAIN_VOLUME:
            // Main volume 
            w->MainVolume=SDL_WidgetCreateR(SDL_SLIDER,Slider->Rect);
            SDL_WidgetProperties(SET_BUTTON_IMAGE,IMG_Load(Slider->button));
            SDL_WidgetProperties(SET_MAX_VALUE,100);
            SDL_WidgetProperties(SET_MIN_VALUE,0);
            SDL_WidgetProperties(SET_NORMAL_STEP_SIZE,1.0);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CHANGED,MIXERUI_MainVolumeChanged,w);
            break;
        case SLIDER_FADER:
            /* fade slider */
            w->Fader=SDL_WidgetCreateR(SDL_SLIDER,Slider->Rect);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CHANGED,MIXERUI_FaderChanged,w->Fader);
            SDL_WidgetProperties(SET_BUTTON_IMAGE,IMG_Load(Slider->button));
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
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,MIXERUI_AutoFadeButtonClicked,NULL);
        }
        Button=Button->next;
    }

    SDL_WidgetCreate(SDL_BUTTON,390,34,20,20);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,playsample,NULL);

    SDL_WidgetCreate(SDL_BUTTON,415,34,20,20);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,playsample2,NULL);
    return w;
}

int MIXERUI_Redraw(void *w)
{
    int volume;
    double value=0.0;
    int state;
    MixerWidgets *mw=(MixerWidgets*)w;

    AUDIOOUTPUT_GetMainVolume(&volume);    

    SDL_WidgetPropertiesOf(mw->MainVolumeIndicator,SET_CUR_VALUE,(double) volume);
    volume= 100.0 - volume;
    SDL_WidgetPropertiesOf(mw->MainVolume   ,SET_CUR_VALUE,(double) volume);


    
    SDL_WidgetPropertiesOf(mw->Fader,GET_STATE,&state);    
    if(state != SLIDER_DRAG)
    {
        if(MIXER_GetFaderValue(&value))
        {
            SDL_WidgetPropertiesOf(mw->Fader,SET_CUR_VALUE,value);    
        }
    }
    
    return 1;
}

/* Internal callback functions */

static void MIXERUI_AutoFadeButtonClicked(void *data)
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
    MixerWidgets *w=(MixerWidgets *)data;
    SDL_Slider *slider=(SDL_Slider *)w->MainVolume;
    int volume;

    volume=slider->MaxValue - slider->CurrentValue;
    SDL_WidgetPropertiesOf(w->MainVolumeIndicator,SET_CUR_VALUE,(double)volume);
    AUDIOOUTPUT_SetMainVolume(volume);
    
}


int MIXERUI_DecreaseMainVolume()
{
    MIXER_DecreaseMainVolume();
    return 1;
}


int MIXERUI_IncreaseMainVolume()
{
    MIXER_IncreaseMainVolume();
    return 1;
}
