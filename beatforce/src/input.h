/*
   BeatForce
   input.h	-  audio input, input plugins, ...
   
   Copyright (c) 2001, Patrick Prasse (patrick.prasse@gmx.net)

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

#ifndef __INPUT_H__
#define __INPUT_H__

#include "llist.h"
#include "songdb.h"
#include "input_plugin.h"
#include "types.h"

typedef struct InputDevice
{
    InputPluginData *PluginData;
    int Channel;
}InputDevice;


struct InputDevice *INPUT_Open();
int  INPUT_Close(InputDevice *Input);
int  INPUT_CloseFile(InputDevice *Input);
long INPUT_GetTime(InputDevice *Input);
int  INPUT_GetTag(char *filename, struct SongDBEntry *e);
int  INPUT_LoadFile(InputDevice *Input,char *filename);
int  INPUT_Pause (InputDevice *Input);
int  INPUT_Play (InputDevice *Input);
int  INPUT_Seek (InputDevice *Input, long time);



#endif /* __INPUT_H__ */
