/*
   BeatForce
   input.c	-  audio input, input plugins, ...
   
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

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

#define MODULE_ID INPUT
#include "debug.h"

int INPUT_Init (int player_nr, BFList * plugin_list)
{
    BFList *ip_plugins = NULL;
    InputPluginData *ipd;
    BFList *next;

    TRACE("input_init enter");
    if (plugin_list == NULL)
    {
        ERROR("input_init: plugin_list == NULL");
        return -1;
    }
    
    next = plugin_list;
    if(next==NULL)
    {
        ERROR("No plugins available");
        exit(2);
    }
    while (next)
    {
        ipd = malloc (INPUT_PLUGIN_DATA_LEN);
        if (ipd == NULL)
            return -1;
        memset (ipd, 0, INPUT_PLUGIN_DATA_LEN);
        
        ipd->ip = (InputPlugin *) next->data;
        ipd->ip->init (&ipd->priv, player_nr);

        ip_plugins = LLIST_Append(ip_plugins, (void*) ipd);
        
        next = next->next;
    }
    if(ip_plugins == NULL)
        exit(1);
    object_get_data(player_nr)->ip_plugins = ip_plugins;
    
    return 0;
}

InputPluginData *
input_whose_file (BFList *plugins, char *filename)
{
    InputPluginData *ipd;
    BFList *next;

    if (filename == NULL)
    {
        return NULL;
    }

    next = plugins;
  
    if(next==NULL)
    {
        printf("No input plugins\n");
        exit(1);
    }
    while (next)
    {
        ipd = (InputPluginData *) next->data;
        if (ipd->ip->is_our_file (ipd->priv, filename) == TRUE)
        {
            return next->data;
        }
        next = next->next;
    }

    printf ("Unknown File: %s\n", filename);

    return NULL;
}

int
input_get_tag (int player_nr, char *filename, struct SongDBEntry *e)
{
    InputPluginData *l;

    printf("input_get_tag\n");
    
    l = input_whose_file (object_get_data(player_nr)->ip_plugins, e->filename);
    if (l == NULL)
        return -10;

    return l->ip->get_tag (l->priv, filename, e);
}

int
input_get_add_info (int player_nr, char *filename, struct SongAddInfo *info)
{
    InputPluginData *l;
    
    printf("input_get_add_info");
    l = input_whose_file (object_get_data(player_nr)->ip_plugins, filename);
    if (l == NULL)
        return ERROR_NOT_SUPPORTED;
    
    return l->ip->get_add_info (l->priv, filename, info);
}

int
input_load_file (int player_nr,struct SongDBEntry *e)
{
    InputPluginData *l;
    
    l = input_whose_file (object_get_data(player_nr)->ip_plugins, e->filename);
    if (l == NULL)
    {
        printf("Impossible\n");
        return -10;
    }
    if(e->AddInfo->SampleRate != 44100)
    {
        printf("Unable to play such a low bitrate\n");
    }
    object_get_data(player_nr)->current_plugin = l;
    if(object_get_data(player_nr)->playing_id == SONGDB_ID_UNKNOWN)
        object_get_data(player_nr)->playing_id = e->id;

    return l->ip->load_file (l->priv, e->filename);
}

int
input_close_file (InputPluginData *current_plugin)
{
    if (current_plugin == NULL)
        return 0;

    return current_plugin->ip->close_file (current_plugin->priv);
}

int
INPUT_Play (InputPluginData * current_plugin)
{
    if (current_plugin == NULL)
    {
        printf("No current plugin selected\n");
        return 0;
    }
    else
    {
        printf("Playing now\n");
    }
    return current_plugin->ip->play (current_plugin->priv);
}

int INPUT_Pause (InputPluginData* current_plugin)
{
    if (current_plugin == NULL)
        return 0;

    return current_plugin->ip->pause (current_plugin->priv);
}

int INPUT_Seek (InputPluginData* current_plugin, long msecs)
{
    if (current_plugin == NULL)
        return 0;

    return current_plugin->ip->seek (current_plugin->priv, msecs);
}

long INPUT_GetTime(InputPluginData *current_plugin)
{
    long time;
    if (current_plugin == NULL)
    {
        return ERROR_NO_FILE_LOADED;
    }
    time=current_plugin->ip->get_time (current_plugin->priv);
    if(time < 0)
        return 0;
    else
        return time;
}

int
input_eof (int ch_id)
{
    return player_eof(ch_id);
}
