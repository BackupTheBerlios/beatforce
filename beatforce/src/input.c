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

#define MODULE_ID INPUT
#include "debug.h"


/* 
   INPUT_Init; initialises all input plugins for use with an audio channel
   
   input:
   channel - the channel which can be used for audio output 
   plugin_list - list of plugins of the type InputPlugin

   output:
   returns a linked list of type InputPluginData
*/
BFList *INPUT_Init (int channel, BFList * plugin_list)
{
    BFList *ip_plugins = NULL;
    InputPluginData *ipd;
    BFList *next;

    TRACE("INPUT_Init enter %d",channel);
    if (plugin_list == NULL)
    {
        ERROR("INPUT_Init (channel %d): plugin_list == NULL",channel);
        return NULL;
    }
    next = plugin_list;
    if(next==NULL)
    {
        ERROR("No plugins available");
        return 0;
    }
    while (next)
    {
        ipd = malloc (INPUT_PLUGIN_DATA_LEN);
        if (ipd == NULL)
            return NULL;

        memset (ipd, 0, INPUT_PLUGIN_DATA_LEN);
        
        ipd->ip = (InputPlugin *) next->data;

        ipd->ip->init (&ipd->priv, channel);
        
        /* Init the plugin with the default write/read function */
        ipd->ip->set_api(ipd->priv,&beatforce_if);

        ip_plugins = LLIST_Append(ip_plugins, (void*) ipd);
        
        next = next->next;
    }
    if(ip_plugins == NULL)
        exit(1);
    
    return ip_plugins;
}


InputPluginData *
INPUT_WhoseFile(BFList *input_plugins, char *filename)
{
    InputPluginData *ipd;
    BFList *next;

    TRACE("INPUT_WhoseFile %s",filename);
    if (filename == NULL)
    {
        return NULL;
    }

    next = input_plugins;
  
    if(next==NULL)
    {
        ERROR("No plugins loaded");
        exit(1);
    }
    while (next)
    {
        ipd = (InputPluginData *) next->data;
        if(ipd->ip->is_our_file)
        {
            //          printf("Is our file %s\n",ipd->ip->filename);
            if (ipd->ip->is_our_file(ipd->priv, filename) == TRUE)
            {
                return next->data;
            }
        }
        else
        {
            ERROR("No function implemented");
            return NULL;
        }
        next = next->next;
    }

    ERROR("Unknown File format: %s\n", filename);

    return NULL;
}

int INPUT_GetTag(BFList *input_list,char *filename, struct SongDBEntry *e)
{
    InputPluginData *l;

    TRACE("INPUT_GetTag enter %s",filename);
    
    l = INPUT_WhoseFile(input_list, e->filename);
    if (l == NULL)
    {
        ERROR("No File format registred");
        return -10;
    }
    return l->ip->get_tag (l->priv, filename, e);
}

int input_get_add_info (char *filename, struct SongAddInfo *info)
{
    InputPluginData *l;
    
    l = INPUT_WhoseFile (PLUGIN_GetList(PLUGIN_TYPE_INPUT), filename);
    if (l == NULL)
    {
        ERROR("Not supported");
        return 0;
    }
    
    return l->ip->get_add_info (l->priv, filename, info);
}

int INPUT_WriteTag(char *filename,struct SongDBEntry *e)
{
    InputPluginData *l;
 
    TRACE("INPUT_WriteTag enter %s",filename);
    
    l = INPUT_WhoseFile(PLUGIN_GetList(PLUGIN_TYPE_INPUT), e->filename);
    if (l == NULL)
        return -10;

    return l->ip->write_tag (l->priv, filename, e);


}

int INPUT_LoadFile (InputPluginData *Plugin,char *filename)
{
    TRACE("INPUT_LoadFile %s",filename);
    if(Plugin == NULL || filename == NULL)
    {
        ERROR("Invalid parameters");
        return 0;
    }
    return Plugin->ip->load_file (Plugin->priv, filename);
}

int
INPUT_CloseFile(InputPluginData *Plugin)
{
    TRACE("INPUT_CloseFile");

    if (Plugin == NULL)
    {
        ERROR("Invalid parameter");
        return 0;
    }

    return Plugin->ip->close_file (Plugin->priv);
}

int INPUT_Play (InputPluginData * Plugin)
{
    if (Plugin == NULL)
        return 0;
    
    return Plugin->ip->play (Plugin->priv);
}

int INPUT_Pause (InputPluginData* Plugin)
{
    if (Plugin == NULL)
        return 0;

    return Plugin->ip->pause (Plugin->priv);
}

int INPUT_Seek (InputPluginData* Plugin, long msecs)
{
    if (Plugin == NULL)
        return 0;

    return Plugin->ip->seek (Plugin->priv, msecs);
}

long INPUT_GetTime(InputPluginData *Plugin)
{
    long time;
    if (Plugin == NULL)
    {
        ERROR("No file loaded");
        return 0;
    }
    time=Plugin->ip->get_time (Plugin->priv);
    if(time < 0)
        return 0;
    else
        return time;
}

int INPUT_EOF(int ch_id)
{
    printf("End of file\n");
    return PLAYER_EOF(ch_id);
}

int INPUT_SetInputInterface(InputPluginData *Plugin,InputInterface *iif)
{
    if(iif == NULL)
        return 0;
    else
        return Plugin->ip->set_api(Plugin->priv,iif);
    
    
}
