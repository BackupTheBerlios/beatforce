/*
  Beatforce/ Startup of beatforce

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
#include <stdlib.h>

#include "configfile.h"
#include "plugin.h"
#include "songdb.h"
#include "input.h"
#include "player.h"
#include "output.h"
#include "mixer.h"
#include "osa.h"

// user interface include
#include "main_ui.h"

ConfigFile     *cfgfile;
AudioConfig    *audiocfg;
SongDBConfig   *songdbcfg;
PlayerConfig   *playercfg0, *playercfg1;
SamplerConfig  *samplercfg;
MixerConfig    *mixercfg;

int
main(int argc, char *argv[])
{
    OSA_Init();
    MAINUI_Init(); 

    //beatforce
    cfgfile = bf_cfg_open_default_file ();
    if (cfgfile == NULL)
    {
        printf("Could not create nor open the configfile!\n");
    }

    audiocfg   = bf_cfg_read_AudioConfig   (cfgfile);
    songdbcfg  = bf_cfg_read_SongDBConfig  (cfgfile);
    playercfg0 = bf_cfg_read_PlayerConfig  (cfgfile, 0);
    playercfg1 = bf_cfg_read_PlayerConfig  (cfgfile, 1);
    samplercfg = bf_cfg_read_SamplerConfig (cfgfile);
    mixercfg   = bf_cfg_read_MixerConfig   (cfgfile);
    bf_cfg_free (cfgfile);

    PLUGIN_Init (PLUGIN_TYPE_INPUT);
    PLUGIN_Init (PLUGIN_TYPE_OUTPUT);

    AUDIOOUTPUT_Init (audiocfg);
    SONGDB_Init (songdbcfg);
    MIXER_Init  ();
    PLAYER_Init (0, playercfg0);
    PLAYER_Init (1, playercfg1);

    /*beatforce UI*/
    MAINUI_Main(NULL);


    AUDIOOUTPUT_Cleanup();
    // save everything on exit
    cfgfile = bf_cfg_open_default_file ();
    bf_cfg_write_AudioConfig   (cfgfile, audiocfg);
    bf_cfg_write_SongDBConfig  (cfgfile, songdbcfg);
    bf_cfg_write_PlayerConfig  (cfgfile, playercfg0,0);
    bf_cfg_write_PlayerConfig  (cfgfile, playercfg1,1);
    bf_cfg_write_MixerConfig   (cfgfile, mixercfg);
    bf_cfg_write_file (cfgfile, bf_cfg_get_default_filename ());

    return 0;

}
