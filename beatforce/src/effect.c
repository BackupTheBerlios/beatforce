/*
  Beatforce/ Effect

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

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "ladspa.h"
#include "plugin.h"

#include "effect_plugin.h"
#include "types.h"
#include "audio_output.h"

#define MODULE_ID EFFECT
#include "debug.h"

EffectPlugin *m;
LADSPA_Descriptor *dc;
int run;
extern struct OutChannel *ch[OUTPUT_N_CHANNELS];

int EFFECT_Init()
{

    BFList *effects;
    int i;
    LADSPA_Descriptor *dc;
    EffectPlugin *m;

    return 0;
    run = 0;
    TRACE("EFFECT_Init");
    effects=PLUGIN_GetList (PLUGIN_TYPE_EFFECT);
    
    while(effects)
    {
        m=(EffectPlugin*)effects->data;
        for(i=0;;i++)
        {
            if(m->dis)
            {
                dc=m->dis(i);
                if(dc == NULL)
                    break;
                printf("dc %s\n",dc->Label);
            }
        }
        effects=effects->next;
    }
}

int EFFECT_Run(int size)
{
    return 0;
    if(run && dc)
        dc->run(m->handle,size);
}

int EFFECT_Play()
{
    BFList *effects;
    int i;

    LADSPA_PortRangeHintDescriptor hint;
    LADSPA_Data amplitude;

    int max_bytes=20000;

    return 0;
    printf("EFFECT_Play\n");

    if(run)
    {
        run=0;
        return 1;
    }

    effects=PLUGIN_GetList (PLUGIN_TYPE_EFFECT);
    while(effects)
    {
        m=(EffectPlugin*)effects->data;
        if(m)
        {
            printf("%s\n",m->filename);
            if(strstr(m->filename,"noise"))
                break;
        }
        effects=effects->next;
    }
    
    printf("Found %s\n",m->filename);

    dc=m->dis(0);

    printf("%s\n",dc->Name);
    printf("%s\n",dc->Label);
    
    printf("PortCount %d\n",dc->PortCount);
    
    for(i=0;i<dc->PortCount;i++)
    {
        if(LADSPA_IS_PORT_INPUT(dc->PortDescriptors[i]) && LADSPA_IS_PORT_CONTROL(dc->PortDescriptors[i]))
        {
            printf("Port %d:%s is a input control plugin\n",i,dc->PortNames[i]);
            hint=dc->PortRangeHints[i].HintDescriptor;
            if(LADSPA_IS_HINT_BOUNDED_BELOW(hint) || LADSPA_IS_HINT_BOUNDED_ABOVE(hint))
            {
                printf("Information about port range available\n");
                if(LADSPA_IS_HINT_BOUNDED_BELOW(hint))
                {
                    printf("Lower Bound %f\n",dc->PortRangeHints[i].LowerBound);
                    if(LADSPA_IS_HINT_SAMPLE_RATE(hint))
                        printf("Lower bound * samplerate\n");
                    if(LADSPA_IS_HINT_DEFAULT_1(hint))
                    {
                        printf("Control port has default value of 1\n");
                    }
                    
                    amplitude = 1.0f;
                    dc->connect_port(m->handle,i,&amplitude);
                }
                if(LADSPA_IS_HINT_BOUNDED_ABOVE(hint))
                {
                    printf("Upper Bound %f\n",dc->PortRangeHints[i].UpperBound);
                    if(LADSPA_IS_HINT_SAMPLE_RATE(hint))
                        printf("Upper bound * samplerate\n");
                }
                           
            }

        }
        
        /* Connect an audio output port */
        if(LADSPA_IS_PORT_OUTPUT(dc->PortDescriptors[i]) && LADSPA_IS_PORT_AUDIO(dc->PortDescriptors[i]))
        {
            dc->connect_port(m->handle,i,ch[0]->buffer);
        }
        
        
    }
    /* Activate */
    if(dc->activate)
        dc->activate(m->handle);

    run =1;

    
}

int EFFECT_Cleanup()
{
    return 0;
    dc->cleanup(m->handle);
}
