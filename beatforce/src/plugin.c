/*
  Beatforce/ Plugin 

  one line to give the program's name and an idea of what it does.
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "plugin.h"
#include "llist.h"
#include "input_plugin.h"
#include "output_plugin.h"
#include "osa.h"
#include "interface.h"

#define MODULE_ID PLUGIN
#include "debug.h"


static BFList *input_list;
static BFList *output_list;


/* Local prototypes */
void plugin_ScanPlugins (char *dirname, int type, BFList ** plugins);
void plugin_AddPlugin(char * filename, int type, BFList ** plugins);

int PLUGIN_Init(int type)
{
    char dirname[255];
    BFList **list;

    TRACE("PLUGIN_Init enter %d",type);

    switch (type)
    {
    case PLUGIN_TYPE_INPUT:
        sprintf(dirname,"%s/%s", PLUGIN_ROOT_DIR, INPUT_DIR);
        input_list = NULL;
        list = &input_list;
        break;

    case PLUGIN_TYPE_OUTPUT:
        sprintf(dirname,"%s/%s", PLUGIN_ROOT_DIR, OUTPUT_DIR);
        output_list = NULL;
        list = &output_list;
        break;

    default:
        return -1;
    }

    plugin_ScanPlugins (dirname, type, list);

    return 0;
}

BFList *PLUGIN_GetList (int type)
{
    switch (type)
    {
    case PLUGIN_TYPE_INPUT:
        return input_list;

    case PLUGIN_TYPE_OUTPUT:
        return output_list;

    default:
        return NULL;
    }
    return NULL;
}

void plugin_AddPlugin(char * filename, int type, BFList ** plugins)
{
    void *h;
    void *(*gpi) (InputInterface*);
    void *(*gpo) (void);

    if ((h = OSA_LoadLibrary(filename)) != NULL)
    {
        if ((gpi = OSA_GetFunctionAddress(h, "get_input_info")) != NULL && type == PLUGIN_TYPE_INPUT)
        {
            InputPlugin *p;

            DEBUG("...is input plugin");
            p = (InputPlugin *) gpi (&beatforce_if);
            p->handle = h;
            p->filename = (char*)strdup (filename);
            DEBUG("....desc: %s", p->description);
            *plugins = LLIST_Append (*plugins, (void*) p);

        }
        else if ((gpo = OSA_GetFunctionAddress (h, "get_output_info")) != NULL
                 && type == PLUGIN_TYPE_OUTPUT)
        {
            OutputPlugin *p;

            DEBUG("...is output plugin");
            p = (OutputPlugin *) gpo ();
            p->handle = h;
            p->filename = (char*)strdup (filename);
            DEBUG("....desc: %s", p->description);

            *plugins = LLIST_Append (*plugins, (void*) p);

        }
        else
        {
            OSA_CloseLibrary(h);
        }

    }
    else
    {
        ERROR("Error loading plugin");
    }

}


void
plugin_ScanPlugins (char *dirname, int type, BFList ** plugins)
{
    BFList *files  = NULL;
    void *data;
    
    TRACE("Plugin_ScanPlugins %s",dirname);

    files=OSA_FindFiles(dirname,(char*)OSA_GetSharedLibExtension(),1);
    
    while(files)
    {
        plugin_AddPlugin (files->data, type, plugins);
        data=files->data;
        files=LLIST_Remove(files,data);
        free(data);
    }

}
