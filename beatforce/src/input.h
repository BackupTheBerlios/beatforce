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

BFList *INPUT_Init (int channel, BFList * plugin_list);


InputPluginData *INPUT_WhoseFile (BFList *, char *);

int INPUT_GetTag(BFList *input_list,char *filename, struct SongDBEntry *e);
int input_get_add_info (int, char *, struct SongAddInfo *);

int INPUT_LoadFile (InputPluginData *Plugin,char *filename);
int INPUT_CloseFile (InputPluginData *Plugin);

int INPUT_Play (InputPluginData *Plugin);
int INPUT_Pause (InputPluginData *Plugin);
int INPUT_Seek (InputPluginData *Plugin, long time);

long INPUT_GetTime (InputPluginData *Plugin);

int INPUT_EOF(int);

#endif
