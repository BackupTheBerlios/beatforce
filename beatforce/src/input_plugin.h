/*
   BeatForce
   input_plugin.h  -  input plugin defs
   
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

#ifndef __INPUT_PLUGIN_H__
#define __INPUT_PLUGIN_H__

#include "songdb.h"
/*
 * INFO: A input plugin can be instaciated multiple times by calling init more than once.
 *		 Therefore all private data of the input plugin has to be stored in  void * !
 */

typedef struct
{
    void *handle;                 /* we fill it */
    char *filename;               /* we fill it */
    char *description;            /* The description that is shown in the preferences box */

    int (*init) (void **, int);        /* Called when the plugin is loaded */

    int (*about) (void);          /* Show the about box */
    int (*configure) (struct SongDBEntry *e);  /* Set the configuration parameters bitrate/samplerate */

                                                int (*is_our_file) (void *, char *filename);       /* Return 1 if the plugin can handle the file */

    int (*get_tag) (void *, char *filename, struct SongDBEntry * e);   /* get artist, title, ... */
    int (*get_add_info) (void *, char *filename, struct SongAddInfo * e);      /* get add. info (bitrate,...) */

    int (*load_file) (void *, char *); /* load file for playing - DO NOT START TO PLAY ! */
    int (*close_file) (void *);        /* close file */

    int (*play) (void *);      /* play */
    int (*pause) (void *);     /* pause */
    int (*stop) (void *);      /* stop (called before close) */

    int (*seek) (void *, long msec);   /* seek to offset */

    long (*get_time) (void *); /* get current written time */
    long (*get_output_time) (void *);  /* we fill it */

    int (*get_volume) (void *, int **ch);      /* get volume for the channels */
    int (*set_volume) (void *, int *ch);       /* set volume */

    int (*cleanup) (void *);

    //InputPluginOutput *output;

}
InputPlugin;

#define IPLUGIN( list )   ((InputPluginData*)list->data)

typedef struct
{

    InputPlugin *ip;

    void *priv;

}
InputPluginData;

#define INPUT_PLUGIN_DATA_LEN		(sizeof(InputPluginData))


#endif
