/*
   BeatForce
   audio_output.h  - audio output (header)
   
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

#ifndef __AUDIO_OUTPUT_H__
#define __AUDIO_OUTPUT_H__

#include "types.h"
#ifdef OUTPUT_SIZE_32
typedef signed int output_word;

#else
typedef signed short output_word;
#endif


#define OUTPUT_SAMPLE_LEN	(sizeof(output_word))
#define OUTPUT_N_CH 		2

#if 1
#define OUTPUT_BUFFER_SIZE_SAMPLES( cfg ) (cfg->FragmentSize*OUTPUT_N_CH)
#define OUTPUT_BUFFER_SIZE( cfg )	 (OUTPUT_BUFFER_SIZE_SAMPLES( cfg )*OUTPUT_SAMPLE_LEN)
#define OUTPUT_RING_SIZE( cfg ) 	 (cfg->RingBufferSize*OUTPUT_BUFFER_SIZE( cfg ))
#else
#define OUTPUT_BUFFER_SIZE	(512*OUTPUT_SAMPLE_LEN*OUTPUT_N_CH)
#define OUTPUT_RING_BUF_SIZE	5
#define OUTPUT_RING_SIZE	(OUTPUT_BUFFER_SIZE*OUTPUT_RING_BUF_SIZE)
#endif

#define OUTPUT_BPM_HIST 	10
#define OUTPUT_FFT_BANDS	15
#define OUTPUT_FFT_HIST 	5

#define BPM_START_BAND		1
#define BPM_STOP_BAND		4


/* sub-groups in mask */
#define GROUP_FADER     1
#define GROUP_MASTER	2
#define GROUP_MONITOR   4

/* channels */
#define CHANNEL_Player0 	0
#define CHANNEL_Player1 	1
#define CHANNEL_PreH		2
#define CHANNEL_Sampler 	3

#include "audio_channel.h"
#include "ringbuffer.h"
#include "output_plugin.h"
#include "configfile.h"


typedef struct OutChannelList 
{
    struct OutChannel     *Channel;
    struct OutChannelList *Next;
}OutChannelList;

#define OUT_CHANNEL_SIZE   (sizeof( struct OutChannel ))


struct OutGroup
{
    long magic;

    output_word *out_buffer;

    int clipping;
    int clipping_count;

    float fader_percent;
    int mainvolume;

    OutputDevice *dev;
    void *handle;
};

/* init function */
int AUDIOOUTPUT_Init ();
int AUDIOOUTPUT_Cleanup (void);


/* interface to input plugin */
int AUDIOOUTPUT_Open (int, AFormat, int, int, int *);
int AUDIOOUTPUT_SetSpeed(int channel, int speed);

/* Interface for plugins */
int AUDIOOUTPUT_Close (int);
int output_read (struct OutChannel *Channel,int len);
int AUDIOOUTPUT_Write(int, void*, int);
int output_write_blocking (int, void*, int);
long AUDIOOUTPUT_BufferFree(int c);
int AUDIOOUTPUT_Pause (int c, int pause);
int output_set_time (int, long);
long AUDIOOUTPUT_GetTime(int channel);

/* interface to mixer */
int AUDIOOUTPUT_SetVolume(int, float);

int AUDIOOUTPUT_SetMainVolume(int value);
int AUDIOOUTPUT_GetMainVolume(int *value);
int output_get_volume (int, float *);
int output_set_group (int, int, int);
int AUDIOOUTPUT_Mute (int, int);
int output_set_beatcount (int, int);

/* to player + mixer */
double output_get_bpm (int);
int AUDIOOUTPUT_GetVolumeLevel(int channel,int *left,int *right);
struct OutChannel *AUDIOOUTPUT_GetChannelByID(int c);
int AUDIOOUTPUT_ChannelRegister(struct OutChannel *channel);

int do_fft (int, output_word *);

/* convert */
int convert_output (int, int, output_word *, int);
int convert_buffer (AFormat, int, void*, output_word *, int);




#endif
