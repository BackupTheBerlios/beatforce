/*
  BeatForce
  sampler.c Sampler implementation
   
  Copyright (c) 2004, John Beuving (john.beuving@home.nl)
  

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

#include <malloc.h>
#include <string.h>
#include <ladspa.h>
#include <stdio.h>

#include "player.h"
#include "songdb.h"
#include "types.h"
#include "input.h"
#include "plugin.h"
#include "sampler.h"
#include "audio_output.h"

#define MODULE_ID SAMPLER
#include "debug.h"

BFList *SamplerPlugins;

Sample samples[10];
Sample *current;

int SAMPLER_Write (int c, void* buf, int len);

InputInterface sampler_if = 
{
    AUDIOOUTPUT_Open,
    SAMPLER_Write,
    AUDIOOUTPUT_Pause,
    AUDIOOUTPUT_BufferFree,
    AUDIOOUTPUT_GetTime,
    AUDIOOUTPUT_Close,
    INPUT_EOF
};

int SAMPLER_Init()
{
    int i;
    samples[0].filename=strdup("/home/beuving/test.mp3");
    samples[1].filename=strdup("/home/beuving/test.ogg");

    SamplerPlugins = INPUT_Init (2, PLUGIN_GetList(PLUGIN_TYPE_INPUT));

    current=NULL;
    for(i=0;i<10;i++)
    {
        if(samples[i].filename)
        {
            while(current != NULL)
                SDL_Delay(10);
            
            samples[i].buffer=malloc(4000000);
            samples[i].size=0;
            samples[i].l = INPUT_WhoseFile (SamplerPlugins,samples[i].filename);
            samples[i].channel=i+2;
            samples[i].playing=0;
            
            INPUT_SetInputInterface(samples[i].l,&sampler_if);
            
            while(!INPUT_LoadFile(samples[i].l,samples[i].filename))
                SDL_Delay(5);
            
            current=&samples[i];
               
        }
    }
    
    return 1;
}


int play(void *data)
{
    unsigned long teller=0;
    int maxb=40000;
    int written;
    Sample *d=(Sample*)data;

    d->playing=1;
    AUDIOOUTPUT_Close(d->channel);
    AUDIOOUTPUT_Open(d->channel,FMT_S16_NE,44100,2, &maxb);

    AUDIOOUTPUT_Pause (d->channel,0);

    while(teller < d->size && d->playing)
    {
        while(AUDIOOUTPUT_BufferFree(d->channel) < 10000)
            SDL_Delay(10);
            written=AUDIOOUTPUT_Write(d->channel,d->buffer+teller,10000);
           
        teller+=written;
    }
    AUDIOOUTPUT_Close(d->channel);
    d->playing=0;
}

int SAMPLER_Play(int s)
{
    if(s==0 || s==1)
    {
        if(samples[s].playing == 0)
           OSA_CreateThread(play,&samples[s]);
        else
            samples[s].playing=0;
           
    }
    return 1;
}

int SAMPLER_Write (int c, void* buf, int len)
{
    if(current == NULL)
        return 0;
    if((len + current->size) < 4000000)
    {
        memcpy((current->buffer+current->size),buf,len);
        current->size += len;
    }
    else
    {
        INPUT_Pause(current->l);
        INPUT_CloseFile(current->l);
        current=NULL;
    }
    return 1;
}

