/*
  Beatforce/ Mixer user interface

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003-2004 John Beuving (john.beuving@beatforce.org)

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
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <SDLTk.h>

#include "playlist.h"

#include "effect.h"
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
    EFFECT_Play();
//    SAMPLER_Play(8);
}

void playsample2(void *s)
{
//    SAMPLER_Play(1);
}

void* MIXERUI_CreateWindow(ThemeMixer *tm)
{
    MixerWidgets *w;
    unsigned int val;
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
    
    w = (MixerWidgets*)malloc(sizeof(MixerWidgets));
    memset(w,0,sizeof(MixerWidgets));
    
    while(Image)
    {
        SDL_Widget *ss;
        ss=SDL_WidgetCreate(SDL_PANEL,Image->x,Image->y,Image->w,Image->h);
        SDL_PanelSetImage(ss,IMG_Load(Image->filename));
        SDL_WidgetShow(ss);
        Image=Image->next;
    }

    while(VolumeBar)
    {
        w->MainVolumeIndicator=SDL_WidgetCreate(SDL_VOLUMEBAR,VolumeBar->x,VolumeBar->y,VolumeBar->w,VolumeBar->h);
        SDL_VolumeBarSetCurrentValue(w->MainVolumeIndicator,100.0);
        SDL_WidgetShow(w->MainVolumeIndicator);
        VolumeBar=VolumeBar->next;
    }


    while(Slider)
    {
        switch(Slider->action)
        {
        case SLIDER_MAIN_VOLUME:
            // Main volume 
            w->MainVolume=SDL_WidgetCreate(SDL_SLIDER,Slider->x,Slider->y,Slider->w,Slider->h);
            SDL_WidgetPropertiesOf(w->MainVolume,SET_BUTTON_IMAGE,IMG_Load(Slider->button));
            SDL_WidgetPropertiesOf(w->MainVolume,SET_MAX_VALUE,100);
            SDL_WidgetPropertiesOf(w->MainVolume,SET_MIN_VALUE,0);
            SDL_WidgetPropertiesOf(w->MainVolume,SET_NORMAL_STEP_SIZE,1);
            SDL_SignalConnect(w->MainVolume,"value-changed",MIXERUI_MainVolumeChanged,w);
            SDL_WidgetShow(w->MainVolume);
            break;
        case SLIDER_FADER:
            /* fade slider */
            w->Fader=SDL_WidgetCreate(SDL_SLIDER,Slider->x,Slider->y,Slider->w,Slider->h);
            SDL_SignalConnect(w->Fader,"value-changed",MIXERUI_FaderChanged,w->Fader);
            SDL_WidgetPropertiesOf(w->Fader,SET_BUTTON_IMAGE,IMG_Load(Slider->button));
            MIXER_SetFaderValue(0);
            MIXER_GetFaderValue (&val);
            SDL_WidgetPropertiesOf(w->Fader,SET_MAX_VALUE,1000);
            SDL_WidgetPropertiesOf(w->Fader,SET_MIN_VALUE,0);
            SDL_WidgetPropertiesOf(w->Fader,SET_NORMAL_STEP_SIZE,100);
            SDL_SliderSetCurrentValue(w->Fader,val);
            SDL_WidgetShow(w->Fader);
            break;
        }
        Slider=Slider->next;
    }

    while(Button)
    {
        if(Button->action == BUTTON_RESET_FADER)
        {
            SDL_Widget *ss;
            ss=SDL_WidgetCreate(SDL_BUTTON,Button->x,Button->y,Button->w,Button->h);
            SDL_SignalConnect(ss,"clicked",MIXERUI_AutoFadeButtonClicked,ss);
            SDL_WidgetShow(ss);
        }
        Button=Button->next;
    }
    return w;
}

int MIXERUI_Redraw(void *w)
{
    MixerWidgets *mw;
    
    int volume;
    unsigned int value=0;
    int state;

    if(w == NULL)
        return 0;

    mw=(MixerWidgets*)w;

    if(mw->MainVolumeIndicator && mw->MainVolume )
    {
        AUDIOOUTPUT_GetMainVolume(&volume);    
        
        SDL_VolumeBarSetCurrentValue(mw->MainVolumeIndicator,(double) volume);
        volume= 100 - volume;
        SDL_SliderSetCurrentValue(mw->MainVolume,volume);
    }
    if(mw->Fader)
    {
        SDL_WidgetPropertiesOf(mw->Fader,GET_STATE,&state);    
        if(state != SLIDER_DRAG)
        {
            if(MIXER_GetFaderValue(&value))
            {
                SDL_SliderSetCurrentValue(mw->Fader,value);
            }
        }
    }
    
    return 1;
}

/* Internal callback functions */
static void MIXERUI_AutoFadeButtonClicked(void *data)
{
//    SDL_Widget *w=(SDL_Widget*)data;
/*    SDL_WidgetMove((SDL_Widget*)data,w->Rect.x+5,w->Rect.y+5); */
//    SDL_WidgetResize(image,50,50);
    if(!MIXER_FadeInProgress())
        MIXER_DoFade(1,0);  //param 1 is autofade param2 is instant
}

static void MIXERUI_FaderChanged(void *data)
{
    SDL_Widget *widget=(SDL_Widget*)data;
    SDL_Slider *slider=(SDL_Slider *)widget;
    MIXER_SetFaderValue(slider->CurrentValue);
}

void MIXERUI_MainVolumeChanged(void *data)
{
    MixerWidgets *w=(MixerWidgets *)data;
    SDL_Widget *widget=(SDL_Widget *)w->MainVolume;
    SDL_Slider *slider=(SDL_Slider *)widget;
    int volume;

    volume=slider->MaxValue - slider->CurrentValue;
    SDL_VolumeBarSetCurrentValue(w->MainVolumeIndicator,(double) volume);
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
