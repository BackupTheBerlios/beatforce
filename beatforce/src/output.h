/*
   BeatForce
   input.h	-  output plugins, ...
   
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

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "audio_output.h"

int output_dev_init (AudioConfig *);

int output_plugin_init (struct OutGroup *, AudioConfig *, int);

int Output_PluginOpen (struct OutGroup *, AudioConfig *, int, int, long, int);
int output_plugin_close (struct OutGroup *);
int OUTPUT_PluginWrite (struct OutGroup *, void *, int);
int output_plugin_pause (struct OutGroup *, int);
int OUTPUT_PluginGetVolume (struct OutGroup *);
int OUTPUT_PluginSetVolume (struct OutGroup *);

int output_plugin_cleanup(struct OutGroup *grp);

#endif
