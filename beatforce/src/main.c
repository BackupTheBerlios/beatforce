/*
  Beatforce/ Startup of beatforce

  Copyright (C) 2003-2004 John Beuving (john.beuving@beatforce.org)

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
#include <stdlib.h>

#include "configfile.h"
#include "plugin.h"
#include "songdb.h"
#include "input.h"
#include "player.h"
#include "output.h"
#include "mixer.h"
#include "osa.h"
#include "osa_cdrom.h"
#include "sampler.h"
#include "effect.h"
#include "ui.h"

int main(int argc, char *argv[])
{
    OSA_Init();
    
    CONFIGFILE_OpenDefaultFile();

    THEME_Init();     

    
    FILEWINDOW_Init();


    UI_Init(argc,argv);


    /* Create lists of all available plugins */
    PLUGIN_Init (PLUGIN_TYPE_INPUT);
    PLUGIN_Init (PLUGIN_TYPE_OUTPUT);
    PLUGIN_Init (PLUGIN_TYPE_EFFECT);
    
    AUDIOOUTPUT_Init ();
    EFFECT_Init();
    MIXER_Init  ();
    SAMPLER_Init();

//    OSACDROM_Init();

    MAINWINDOW_Open();

    /*beatforce UI*/
    UI_Main(); /* main loop */


    AUDIOOUTPUT_Cleanup();

    SONGDB_Exit();
    EFFECT_Cleanup();
    PLUGIN_Cleanup();

    return 1;

}
