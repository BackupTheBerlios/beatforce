/*
   BeatForce
   output_plugin.h	-  output plugins definitions
   
   Copyright (c) 2002, Patrick Prasse (patrick.prasse@gmx.net)

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

#ifndef __EFFECT_PLUGIN_H__
#define __EFFECT_PLUGIN_H__

#include "ladspa.h"

typedef struct
{
    void *handle;                 /* we fill it */
    char *filename;               /* we fill it */
    char *shortname;              /* short name like 'alsa-05x' */
    char *description;            /* The description that is shown in the preferences box */

    LADSPA_Descriptor_Function dis;
    

}EffectPlugin;


#endif
