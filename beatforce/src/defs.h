/*
   BeatForce
   defs.h  - definitions, ...
   
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


#ifndef __DEFS_H__
#define __DEFS_H__

/* ERROR numbers */
#include "err.h"

#include "types.h"



#include "debug.h"
#include "error.h"

#include "configfile.h"
#include "songdb.h"
#include "input_plugin.h"
#include "output_plugin.h"

#ifndef PLUGIN
#include "main.h"
#include "interface.h"


#include "util.h"

#include "about.h"
#include "preferences.h"


#include "playlist.h"
#include "player.h"

#include "sampler.h"

#include "audio_output.h"

#include "input.h"
#include "plugin.h"
#include "output.h"

#include "mixer.h"
#endif
#endif
