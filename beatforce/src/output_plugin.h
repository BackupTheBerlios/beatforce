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

#ifndef __OUTPUT_PLUGIN_H__
#define __OUTPUT_PLUGIN_H__



typedef struct
{
  void *handle;                 /* we fill it */
  char *filename;               /* we fill it */
  char *shortname;              /* short name like 'alsa-05x' */
  char *description;            /* The description that is shown in the preferences box */

  /* init plugin
   */
  int (*init) (void **);     /* Called when the plugin is loaded */

  /* about and configure
   */
  int (*about) (void);          /* Show the about box */
  int (*configure) (void);

  /* open output
   * params: 1st = void data, 2nd = Device name,
   *             3rd = fragment size, 4th = low watermark,
   *             5th = high watermark,
   *             6th = channels, 7th = samplerate
   *             8th = sample format (yet unused)
   */
  int (*open) (void *, char *, int, int, int, int, long, int);

  /* close output
   */
  int (*close) (void *);

  /* write samples
   * params: 1st = void data
   *             2nd = buffer, 3rd = size in bytes
   */
  int (*write) (void *, void *, int);

  /* pause output
   * params: 1st = void data, 2nd = 1/0 (pause/unpause)
   */
  int (*pause) (void *, int);        /* pause */

  int (*get_volume) (void *, float *);       /* get volume for the channels */
  int (*set_volume) (void *, float); /* set volume */

  int (*cleanup) (void *);

}
OutputPlugin;

#define OPLUGIN( list )   ((OutputPlugin*)list->data)


typedef struct
{
  char *device;
  char *shortname;
  OutputPlugin *op;
  void *priv;

}
OutputDevice;


#endif
