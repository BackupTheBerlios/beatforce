/*
   BeatForce
   output.c  -	audio output 
   
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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>


#include "audio_output.h"
#include "configfile.h"
#include "err.h"
#include "output.h"
#include "output_plugin.h"
#include "plugin.h"

#define MODULE_ID OUTPUT
#include "debug.h"


int
output_dev_init (AudioConfig * cfg)
{
    /* Nothing for now */
    if(cfg == NULL)
        return ERROR_INVALID_ARG;
    return 0;
}

int
output_plugin_init (struct OutGroup *grp, AudioConfig * cfg, int i)
{
    BFList *list, *next;

    if(grp == NULL || cfg == NULL)
    {
        return ERROR_INVALID_ARG;
    }

    list = PLUGIN_GetList (PLUGIN_TYPE_OUTPUT);
    if(list == NULL)
    {
        return ERROR_NO_FILE_LOADED;
    }

    if(grp->magic != AUDIO_OUTPUT_MAGIC)
    {
        return ERROR_INVALID_ARG;
    }
    grp->dev = malloc (sizeof (OutputDevice));
    memset (grp->dev, 0, sizeof (OutputDevice));

    if (strcmp (cfg->output_id[i], "none"))
    {
        next = list;
        while (next)
        {
            if (strcmp (OPLUGIN (next)->shortname, cfg->output_id[i]) == 0)
            {
                int err;

                grp->dev->op = OPLUGIN (next);
                grp->dev->device = cfg->device_id[i];
                grp->dev->shortname = cfg->output_id[i];
                err = grp->dev->op->init (&grp->dev->priv);
                if (err != 0)
                {
                    ERROR("outputplugininit: Failed to initalize output plugin for group %d: %d\n",i, -err);
                    grp->dev->op = NULL;
                    grp->dev->priv = NULL;
                }
                return err;
            }
            next = next->next;
        }

    }
    grp->dev->shortname = "none";
    grp->dev->device = "";
    return ERROR_NO_OUTPUT_SELECTED;
}


int
Output_PluginOpen (struct OutGroup *grp, AudioConfig * cfg, int i, int ch,
                    long rate, int fmt)
{
    int err;

    if(grp == NULL || cfg == NULL)
        return ERROR_INVALID_ARG;

    if (grp->dev->shortname
        && (strcmp (grp->dev->shortname, "none") == 0))
    {
        return 0;
    }

    if(grp->dev->op == NULL)
        return ERROR_INVALID_ARG;

    err =
        grp->dev->op->open (grp->dev->priv, cfg->device_id[i],
                            (int) cfg->FragmentSize, cfg->LowWatermark,
                            cfg->HighWatermark, ch, rate, fmt);
    if (err)
    {
        ERROR("Unable to open output for group %d: error %d\n", i, -err);
    }

    return err;
}


int OUTPUT_PluginClose(struct OutGroup *grp)
{
    int err;

    if(grp == NULL)
        return ERROR_INVALID_ARG;

    if (grp->dev->device && (strcmp(grp->dev->shortname, "none") == 0))
    {
        return 0;
    }

    err = grp->dev->op->close (grp->dev->priv);
    if (err)
    {
        printf("Unable to close output: error 0x%x\n", -err);
    }
    return err;

}

int OUTPUT_PluginWrite (struct OutGroup *grp, void *buffer, int size)
{
    int err;
//    char* buffe=(char*)buffer;

    if(grp == NULL)
        return ERROR_INVALID_ARG;

    if (grp->dev->device && (strcmp (grp->dev->shortname, "none") == 0))
    {
        exit(0);
//        return 0;
    }
    err = grp->dev->op->write (grp->dev->priv, buffer, size);
    if (err < 0)
    {
        printf("Unable to write output: error 0x%x\n", -err);
    }
    else if (err < size)
    {
        printf("Written=%d size=%d!\n", err, size);
    }

    return err;
}

int
output_plugin_pause (struct OutGroup *grp, int pause)
{
    int err;

    if(grp == NULL)
        return ERROR_INVALID_ARG;

    if (grp->dev->device && (strcmp (grp->dev->shortname, "none") == 0))
    {
        return 0;
    }

    err = grp->dev->op->pause (grp->dev->priv, pause);
    if (err)
    {
        printf("Unable to %s output: error 0x%x\n",
                   ((pause == 1) ? ("pause") : ("unpause")), -err);
    }

    return err;

}

int OUTPUT_PluginGetVolume (struct OutGroup *grp)
{
    int err=1;
    float volume;

    TRACE("OUTPUT_PluginGetVolume enter");
    if(grp == NULL)
        return ERROR_INVALID_ARG;

    if (grp->dev->device && (strcmp (grp->dev->shortname, "none") == 0))
    {
        grp->fader_percent = 0.0;
        return 0;
    }

    if (grp->dev->op->get_volume == NULL)
        return ERROR_NOT_SUPPORTED;

    if( grp->dev->op->get_volume (grp->dev->priv, &volume) <= 0)
    {
        printf("Unable to get volume: error 0x%x\n", -err);
    }
    grp->mainvolume=(int)volume;
    TRACE("OUTPUT_PluginGetVolume leave");
    return err;
}

int
OUTPUT_PluginSetVolume (struct OutGroup *grp)
{
    int err;

    if(grp == NULL)
        return ERROR_INVALID_ARG;
    
    if (grp->dev->device && (strcmp (grp->dev->shortname, "none") == 0))
    {
        return 0;
    }

    if (grp->dev->op->set_volume == NULL)
    {
        printf("Not supported\n");
        return ERROR_NOT_SUPPORTED;
    }

    err = grp->dev->op->set_volume (grp->dev->priv, grp->mainvolume);
    if (err)
    {
        ERROR("Unable to set volume: error %d", -err);
    }
    return err;
}

int
output_plugin_cleanup(struct OutGroup *grp)
{
    int err;

    if(grp == NULL || grp->dev == NULL)
        return ERROR_INVALID_ARG;
    
    if (grp->dev->device && (strcmp (grp->dev->shortname, "none") == 0))
    {
        return 0;
    }

    if (grp->dev->op->cleanup == NULL)
    {
        printf("Not supported\n");
        return ERROR_NOT_SUPPORTED;
    }

    err = grp->dev->op->cleanup(grp->dev->priv);
    if (err)
    {
        printf("Unable to set volume: error 0x%x\n", -err);
    }
    return err;
}

