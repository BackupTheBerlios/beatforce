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
#include "osa.h"

#define MODULE_ID SAMPLER
#include "debug.h"

BFList *SamplerPlugins[10];

Sample samples[10];

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
    int i=0;

#if 0
    samples[0].filename=strdup("/home/beuving/test.mp3");
    samples[1].filename=strdup("/home/beuving/test.ogg");
    samples[2].filename=strdup("/home/beuving/test.ogg");
    samples[3].filename=strdup("/home/beuving/test.ogg");
    samples[4].filename=strdup("/home/beuving/test.ogg");
    samples[5].filename=strdup("/home/beuving/test.ogg");
    samples[6].filename=strdup("/home/beuving/test.ogg");
    samples[7].filename=strdup("/home/beuving/test.ogg");
    samples[8].filename=strdup("/home/beuving/test.ogg");
    samples[9].filename=strdup("/home/beuving/test.ogg");
#endif

    for(i=0;i<10;i++)
        SamplerPlugins[i] = INPUT_Init (i+2, PLUGIN_GetList(PLUGIN_TYPE_INPUT));

    for(i=1;i<10;i++)
    {
        if(samples[i].filename)
        {
            samples[i].buffer=malloc(4000000);
            samples[i].size=0;
            samples[i].l = INPUT_WhoseFile (SamplerPlugins[i],samples[i].filename);
            samples[i].channel=i+2;
            samples[i].playing=0;

            INPUT_SetInputInterface(samples[i].l,&sampler_if);
            INPUT_LoadFile(samples[i].l,samples[i].filename);
               
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
    return 1;
}

int SAMPLER_Play(int s)
{
    if(s> 0 && s < 10)
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
    if((len + samples[c-2].size) < 4000000)
    {
        memcpy((samples[c-2].buffer+samples[c-2].size),buf,len);
        samples[c-2].size += len;
    }
    else
    {
        INPUT_Pause(samples[c-2].l);
        INPUT_CloseFile(samples[c-2].l);
    }
    return 1;
}

