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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "configfile.h"
#include "effect.h"
#include "event.h"
#include "input.h"
#include "mixer.h"
#include "osa.h"
#include "osa_cdrom.h"
#include "output.h"
#include "player.h"
#include "plugin.h"
#include "sampler.h"
#include "songdb.h"
#include "theme.h"
#include "ui.h"

static void MAIN_ParseArgs(int argc,char **argv);
static void MAIN_SegfaultHandler(int sig);

int main(int argc, char *argv[])
{

    /* Create lists of all available plugins */
    PLUGIN_Init (PLUGIN_TYPE_INPUT);
    PLUGIN_Init (PLUGIN_TYPE_OUTPUT);
    PLUGIN_Init (PLUGIN_TYPE_EFFECT);

    SONGDB_Init();
    MAIN_ParseArgs(argc,argv);

    signal(SIGSEGV, MAIN_SegfaultHandler);

    OSA_Init();
    EVENT_Init();
    CONFIGFILE_OpenDefaultFile();
    
    THEME_Init();     
    
    FILEWINDOW_Init();

    UI_Init(argc,argv);

    AUDIOOUTPUT_Init ();
    EFFECT_Init();
    MIXER_Init  ();
//    SAMPLER_Init();

//    OSACDROM_Init();

    
    MAINWINDOW_Open();

    /*beatforce UI*/
    UI_Main(); /* main loop */


    AUDIOOUTPUT_Cleanup();

    EFFECT_Cleanup();

    SONGDB_Exit();

    PLUGIN_Cleanup();

    return 1;

}

static void MAIN_ParseArgs(int argc,char **argv)
{
    int i;

    for(i=1;i<argc;i++)
    {
        SONGDB_Add(argv[i]);
    }
}

static void MAIN_SegfaultHandler(int sig)
{
    printf("\nSegmentation fault\n\n");
    printf("You've probably found a bug in beatforce\n");
    printf("Please send a mail to john.beuving@beatforce.org\n\n");
    exit(1);
}



