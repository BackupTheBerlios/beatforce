/*
   BeatForce
   raw.c  - output plugin to a PCM data file
   
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#include <sys/soundcard.h>

#define PLUGIN
#include "types.h"
#include "err.h"
#include "output_plugin.h"


#define RAW_MAGIC  0x4F535366	   /* "OSSf" */

typedef struct
{
    long magic;
    char  *name;

    FILE  *raw_fd;
    FILE  *mixer_fd;

    int raw_card;
    int raw_device;

    int fragsize;
    int lw;
    int hw;
    int ch;
    long rate;
    int fmt;
}
raw_private;

int raw_close(Private *);


int
raw_init (Private ** private)
{
    raw_private *p;

    p = malloc (sizeof (raw_private));
    if(p == NULL)
        return ERROR_NO_MEMORY;
    memset (p, 0, sizeof (raw_private));

    p->magic = RAW_MAGIC;

    *private = (Private *) p;

    return 0;
}

int
raw_cleanup (Private * p)
{
    free (p);
    return 0;
}


int
raw_open (Private * P, char *name, int fragsize, int lw, int hw, int ch,
          long rate, int fmt)
{
    raw_private *p = (raw_private *) P;
    int err;
    static int val;
    int hops;
  
    if(p == NULL || p->magic != RAW_MAGIC)
        return ERROR_INVALID_ARG;

    p->fragsize = fragsize;
    p->lw = lw;
    p->hw = hw;
    p->ch = ch;
    p->rate = rate;
    p->fmt = fmt;

    p->name = malloc(255);
    sprintf(p->name,"%s/%s/%s",getenv("HOME"),".beatforce",name);    

    if ((p->raw_fd = fopen (p->name, "wb")) == NULL)
    {
	printf ("RAW: error opening %s: %s", name, strerror (errno));
	return ERROR_OPEN_ERROR;
    }
    return 0;
}


int
raw_close (Private * P)
{
    raw_private *p = (raw_private *) P;

    if(p == NULL || p->magic != RAW_MAGIC)
        return ERROR_INVALID_ARG;

    if (p->raw_fd)
    {
	fclose (p->raw_fd);
    }
    else
	return ERROR_NOT_OPEN;

    memset (p, 0, sizeof (raw_private));
    p->magic = RAW_MAGIC;
    return 0;
}


int
raw_pause (Private * P, int pause)
{
    return 0;
}

int
raw_write (Private * P, void *data, int count)
{
    raw_private *p = (raw_private *) P;

    if(p == NULL || p->magic != RAW_MAGIC)
        return ERROR_INVALID_ARG;

    if (!p->raw_fd)
	return ERROR_NOT_OPEN;

    return fwrite (data, 1, count, p->raw_fd);
}


static OutputPlugin raw_output = {
    NULL,
    NULL,
    "raw",
    "RAW output plugin v0.99",
    raw_init,
    NULL,
    NULL,
    raw_open,
    raw_close,
    raw_write,
    raw_pause,

    NULL,
    NULL,

    raw_cleanup,
};


OutputPlugin *
get_output_info (void)
{
    return &raw_output;
}






