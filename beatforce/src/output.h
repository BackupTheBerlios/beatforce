/*
   BeatForce
   input.h	-  output plugins, ...
   
   Copyright (c) 2003, John Beuving (john.beuving@home.nl)

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

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "audio_output.h"

int OUTPUT_DevInit (AudioConfig *);

int OUTPUT_PluginInit(struct OutGroup *, AudioConfig *, int);
int OUTPUT_PluginOpen (struct OutGroup *, AudioConfig *, int, int, long, int);
int OUTPUT_PluginClose (struct OutGroup *);
int OUTPUT_PluginWrite (struct OutGroup *, void *, int);
int OUTPUT_PluginPause (struct OutGroup *, int);
int OUTPUT_PluginGetVolume (struct OutGroup *);
int OUTPUT_PluginSetVolume (struct OutGroup *);
int OUTPUT_PluginCleanup(struct OutGroup *grp);

#endif

