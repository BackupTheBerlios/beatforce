/*
   BeatForce
   oss.c  - output plugin for OSS
   
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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


#define OSS_MAGIC  0x4F535366	   /* "OSSf" */

typedef struct
{
    long magic;
    char  *name;

    FILE  *oss_fd;
    FILE  *mixer_fd;

    int oss_card;
    int oss_device;

    int fragsize;
    int lw;
    int hw;
    int ch;
    long rate;
    int fmt;
}
oss_private;

int oss_close(Private *);


int
oss_init (Private ** private)
{
    oss_private *p;

    p = malloc (sizeof (oss_private));
    if(p == NULL)
        return ERROR_NO_MEMORY;
    memset (p, 0, sizeof (oss_private));

    p->magic = OSS_MAGIC;

    *private = (Private *) p;

    return 0;
}

int
oss_cleanup (Private * p)
{
    free (p);
    return 0;
}


int
oss_open (Private * P, char *name, int fragsize, int lw, int hw, int ch,
          long rate, int fmt)
{
    oss_private *p = (oss_private *) P;
    int err;
    static int val;
    int hops;
  
    if(p == NULL || p->magic != OSS_MAGIC)
        return ERROR_INVALID_ARG;

    p->fragsize = fragsize;
    p->lw = lw;
    p->hw = hw;
    p->ch = ch;
    p->rate = rate;
    p->fmt = fmt;


    if (name[0] != '/')
    {
        name = "/dev/dsp";
    }
    p->name = name;
    

    if ((p->oss_fd = fopen (name, "w")) == NULL)
    {
	printf ("OSS: error opening %s: %s", name, strerror (errno));
	return ERROR_OPEN_ERROR;
    }
    if ((p->mixer_fd = fopen ("/dev/mixer", "r+")) == NULL)
    {
	printf ("OSS: error opening mixer: %s", strerror (errno));
    }

    err = ioctl (fileno(p->oss_fd), SNDCTL_DSP_SPEED, &rate);
    if(err < 0 )
    {
	printf ("OSS: error setting sample_rate %ld: %s\n", rate, strerror(err));
	goto _err;
    }


    fragsize *= 4;
    for (hops = 0; fragsize >>= 1; hops++);

    val = (p->hw << 16) + hops;
    err = ioctl (fileno(p->oss_fd), SNDCTL_DSP_SETFRAGMENT, &val);
    if(err < 0 )
    {
	printf ("OSS: error setting fragment %d: %s\n", val, strerror(err));
	goto _err;
    }

    val = AFMT_S16_NE;
    err = ioctl (fileno(p->oss_fd), SNDCTL_DSP_SETFMT, &val);
    if(err < 0 )
    {
	printf ("OSS: error setting format %d: %s\n", val, strerror(err));
	goto _err;
    }

    val = p->ch - 1;
    err = ioctl (fileno(p->oss_fd), SNDCTL_DSP_STEREO, &val);
    if(err < 0 )
    {
	printf ("OSS: error setting stereo to %d: %s\n", val, strerror(err));
	goto _err;
    }

    err = ioctl (fileno(p->oss_fd), SNDCTL_DSP_GETBLKSIZE, &val);
    if(err < 0 )
    {
	printf ("OSS: error getting blocksize: %s\n", strerror(err));
	goto _err;
    }

    return 0;

 _err:
    oss_close(p);
    return ERROR_OPEN_ERROR;

}


int
oss_close (Private * P)
{
    oss_private *p = (oss_private *) P;

    if(p == NULL || p->magic != OSS_MAGIC)
        return ERROR_INVALID_ARG;

    if (p->oss_fd)
    {
	fclose (p->oss_fd);
    }
    else
	return ERROR_NOT_OPEN;

    memset (p, 0, sizeof (oss_private));
    p->magic = OSS_MAGIC;
    return 0;
}


int
oss_pause (Private * P, int pause)
{
    return 0;
}

int
oss_write (Private * P, void *data, int count)
{
    oss_private *p = (oss_private *) P;

    if(p == NULL || p->magic != OSS_MAGIC)
        return ERROR_INVALID_ARG;

    if (!p->oss_fd)
	return ERROR_NOT_OPEN;

    return fwrite (data, 1, count, p->oss_fd);
}


int oss_set_volume(Private * P, float volume)
{
    int err;
    int vol;
    int devmask;
    oss_private *p = (oss_private *) P;

    if(volume < 0 || volume > 100)
        return -1;

    if(p == NULL || p->magic != OSS_MAGIC)
        return ERROR_INVALID_ARG;

    if (!p->oss_fd)
	return ERROR_NOT_OPEN;


    ioctl(fileno(p->mixer_fd), MIXER_READ(SOUND_MIXER_OGAIN), &vol);
        
    vol = (100 << 8) | 100;
    err = ioctl (fileno(p->mixer_fd), MIXER_WRITE(SOUND_MIXER_OGAIN), &vol);
    if(err < 0 )
    {
	printf ("OSS: error setting volume for ogain %d: %s\n", vol, strerror(err));
    }

    vol =(int)(volume);
    vol = ( ((vol&0xff) <<8) | (vol & 0xff));
    err = ioctl (fileno(p->mixer_fd), MIXER_WRITE(SOUND_MIXER_PCM), &vol);
    if(err < 0 )
    {
	printf ("OSS: error setting volume for pcm %d: %s\n", vol, strerror(err));
    }

}

static OutputPlugin oss_output = {
    NULL,
    NULL,
    "oss",
    "OSS output plugin v0.99",
    oss_init,
    NULL,
    NULL,
    oss_open,
    oss_close,
    oss_write,
    oss_pause,

    NULL,
    oss_set_volume,					/*oss_set_volume */

    oss_cleanup,
};


OutputPlugin *
get_output_info (void)
{
    return &oss_output;
}






