/*
   BeatForce
   input.c	-  audio input, resposible for creating audio channels
   
   Copyright (c) 2003-2004, John Beuving (john.beuving@beatforce.org)

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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "audio_output.h"
#include "input.h"
#include "songdb.h"
#include "llist.h"
#include "input_plugin.h"
#include "err.h"
#include "types.h"
#include "player.h"


#define MODULE_ID INPUT
#include "debug.h"

struct InputDevice *INPUT_Open()
{
    struct InputDevice *Input;
    struct OutChannel *Channel;

    TRACE("INPUT_Open");
    Channel = AUDIOCHANNEL_New();
    AUDIOOUTPUT_ChannelRegister(Channel);
    
    Input=(InputDevice*)malloc(sizeof(InputDevice));

    Input->Channel     = Channel->id;
    Input->PluginData  = NULL;
    
    return Input;
}

int INPUT_Close(InputDevice *Input)
{
    TRACE("INPUT_Close");
    free(Input);
    return 1;
}

int INPUT_CloseFile(InputDevice *Input)
{
    TRACE("INPUT_CloseFile");
    return INPUTPLUGIN_Close(Input->PluginData);
}

int INPUT_GetTag(char *filename, struct SongDBEntry *e)
{
    InputPlugin     *Plugin;
    InputPluginData *PluginData;
#if 0
    TRACE("INPUT_GetTag %s",filename);
    Plugin = INPUTPLUGIN_WhoseFile(filename);
    if(Plugin == NULL)
        return 0;

    PluginData = INPUTPLUGIN_Init(Plugin,-1);

    INPUTPLUGIN_GetTag(PluginData,filename,e);

    INPUTPLUGIN_Close(PluginData);
#endif
    return 1;
}

int INPUT_LoadFile(InputDevice *Input,char *filename)
{
    InputPlugin     *Plugin;
    InputPluginData *PluginData;

    printf("load file\n");
    TRACE("INPUT_LoadFile %s",filename);
    Plugin=INPUTPLUGIN_WhoseFile(filename);
    if(Plugin == NULL)
        return 0;

    PluginData=INPUTPLUGIN_Init(Plugin,Input->Channel);

    Input->PluginData = PluginData;


    return INPUTPLUGIN_LoadFile(PluginData,filename);
}

int INPUT_Pause (InputDevice *Input)
{
    TRACE("INPUT_Pause");
    return INPUTPLUGIN_Pause(Input->PluginData);
}

int INPUT_Play (InputDevice *Input)
{
    TRACE("INPUT_Play");
    return INPUTPLUGIN_Play(Input->PluginData);
}

long INPUT_GetTime(InputDevice *Input)
{
///    TRACE("INPUT_GetTime");
    return INPUTPLUGIN_GetTime(Input->PluginData);
}

int INPUT_Seek (InputDevice *Input, long time)
{
    TRACE("INPUT_Seek");
    return INPUTPLUGIN_Seek(Input->PluginData,time);
}
