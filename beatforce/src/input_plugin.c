/*
   BeatForce
   input.c	-  audio input, input plugins, ...
   
   Copyright (c) 2003-2004, John Beuving (john.beuving@beatforce.org)

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

#include "songdb.h"
#include "llist.h"
#include "input_plugin.h"
#include "err.h"
#include "types.h"
#include "player.h"
#include "plugin.h"
#include "interface.h" 

#define MODULE_ID INPUTPLUGIN
#include "debug.h"


InputPluginData *INPUTPLUGIN_Init (InputPlugin *Plugin,int channel)
{
    InputPluginData *ipd;

    TRACE("INPUTPLUGIN_Init");

    ipd = malloc(sizeof(InputPluginData));
    if (ipd == NULL)
        return NULL;

    memset (ipd, 0, sizeof(InputPluginData));
        
    ipd->Plugin = Plugin;

    ipd->Plugin->init (&ipd->priv,channel);
        
    /* Init the plugin with the default write/read function */
    ipd->Plugin->set_api(ipd->priv,&beatforce_if);
    
    return ipd;
}


InputPlugin *INPUTPLUGIN_WhoseFile(char *filename)
{
    InputPlugin *Plugin;
    BFList *next;

    TRACE("INPUTPLUGIN_WhoseFile %s",filename);
    if (filename == NULL)
    {
        return NULL;
    }

    next = PLUGIN_GetList(PLUGIN_TYPE_INPUT);
  
    if(next==NULL)
    {
        ERROR("No plugins loaded");
        exit(1);
    }
    while (next)
    {
        Plugin = (InputPlugin*) next->data;
        if(Plugin->is_our_file)
        {
            printf("%s\n",Plugin->filename);
            //          printf("Is our file %s\n",ipd->ip->filename);
            if (Plugin->is_our_file(filename) == TRUE)
            {
                return next->data;
            }
        }
        else
        {
            printf("%s\n",Plugin->filename);
            ERROR("No function implemented");
//            return NULL;
        }
        next = next->next;
    }

    ERROR("Unknown File format: %s\n", filename);

    return NULL;
}

int INPUTPLUGIN_GetTag(InputPluginData *Plugin,char *filename, struct SongDBEntry *e)
{
    TRACE("INPUTPLUGIN_GetTag enter %s",filename);
    
    return Plugin->Plugin->get_tag (Plugin->priv, filename, e);
}

int input_get_add_info (InputPluginData *l,char *filename, struct SongAddInfo *info)
{
    return l->Plugin->get_add_info (l->priv, filename, info);
}

int INPUTPLUGIN_WriteTag(InputPluginData *l,char *filename,struct SongDBEntry *e)
{
    TRACE("INPUTPLUGIN_WriteTag");
    return l->Plugin->write_tag (l->priv, filename, e);
}

int INPUTPLUGIN_LoadFile (InputPluginData *Plugin,char *filename)
{
    TRACE("INPUTPLUGIN_LoadFile %s",filename);
    if(Plugin == NULL || filename == NULL)
    {
        ERROR("Invalid parameters");
        return 0;
    }
    return Plugin->Plugin->load_file (Plugin->priv, filename);
}

int
INPUTPLUGIN_Close(InputPluginData *Plugin)
{
    TRACE("INPUTPLUGIN_CloseFile");

    if (Plugin == NULL)
    {
        ERROR("Invalid parameter");
        return 0;
    }

    return Plugin->Plugin->close_file (Plugin->priv);
}

int INPUTPLUGIN_Play (InputPluginData * Plugin)
{
    TRACE("INPUTPLUGIN_Play");
    if (Plugin == NULL)
    {
        ERROR("Plugin == NULL");
        return 0;
    }
    
    return Plugin->Plugin->play (Plugin->priv);
}

int INPUTPLUGIN_Pause (InputPluginData* Plugin)
{
    TRACE("INPUTPLUGIN_Pause");
    if (Plugin == NULL)
        return 0;

    return Plugin->Plugin->pause (Plugin->priv);
}

int INPUTPLUGIN_Seek (InputPluginData* Plugin, long msecs)
{
    TRACE("INPUTPLUGIN_Seek");
    if (Plugin == NULL)
        return 0;

    return Plugin->Plugin->seek (Plugin->priv, msecs);
}

long INPUTPLUGIN_GetTime(InputPluginData *Plugin)
{
    long time;
//    TRACE("INPUTPLUGIN_GetTime");
    if (Plugin == NULL)
    {
//        DEBUG("No plugin loaded");
        return 0;
    }
    time=Plugin->Plugin->get_time (Plugin->priv);
    if(time < 0)
        return 0;
    else
        return time;
}

int INPUTPLUGIN_EOF(int ch_id)
{
    return PLAYER_EOF(ch_id);
}

int INPUTPLUGIN_SetInputInterface(InputPluginData *Plugin,InputInterface *iif)
{
    if(iif == NULL)
        return 0;
    else
        return Plugin->Plugin->set_api(Plugin->priv,iif);
}
