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

#include "player.h"
#include "songdb.h"
#include "input.h"
#include "plugin.h"

struct SongDBEntry *sample;
BFList *SamplerPlugins[2];


int SAMPLER_Init()
{
    SamplerPlugins[0] = INPUT_Init (2, PLUGIN_GetList(PLUGIN_TYPE_INPUT));
    SamplerPlugins[1] = INPUT_Init (3, PLUGIN_GetList(PLUGIN_TYPE_INPUT));
    sample=malloc(sizeof(struct SongDBEntry));
    memset(sample,0,sizeof(struct SongDBEntry));
    sample->filename=strdup("/home/beuving/test.mp3");
    return 1;
}

int SAMPLER_Play(int s)
{
    InputPluginData *l;

    if(s < 0 || s > 1)
        return 0;
    l = INPUT_WhoseFile (SamplerPlugins[s],sample->filename);

    INPUT_CloseFile(l);
    INPUT_LoadFile(l,sample->filename);
    INPUT_Play (l);
    
    return 1;
}




