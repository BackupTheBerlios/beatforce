/*
  BeatForce
  sampler.c Sampler implementation
   
  Copyright (c) 2004, John Beuving (john.beuving@beatforce.org)
  

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
#include "audio_channel.h"
#include "audio_output.h"
#include "osa.h"

#define MODULE_ID SAMPLER
#include "debug.h"


Sample samples[10];

int SAMPLER_Init()
{
    int i=0;
    TRACE("SAMPLER_Init");

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

    for(i=1;i<10;i++)
    {
        samples[i].Input=INPUT_Open();
        samples[i].playing=0;
        if(samples[i].filename)
            INPUT_LoadFile(samples[i].Input,samples[i].filename);
    }
    return 1;
}

int SAMPLER_Play(int s)
{
    TRACE("SAMPLER_Play %d",s);
    if(s> 0 && s < 10)
    {
        if(samples[s].playing == 0)
        {
            INPUT_Play(samples[s].Input);
            samples[s].playing=1;
        }
        else
        {
            INPUT_Pause(samples[s].Input);
            samples[s].playing=0;
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

