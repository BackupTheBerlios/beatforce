/*
   BeatForce
   configfile.h  - config & config file stuff  (header)
   
   Thanks to the people from XMMS (xmms.org) from which this code was taken.
	   (Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies)   
	 
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

#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

typedef struct
{
    int RemoveAfterPlay;
}
PlayerConfig;


typedef struct
{

    int mute;
    float fader_dB;
    int detect_beat;

    int display_fft;

}
ChannelConfig;

typedef struct
{

    int RingBufferSize;		/* RingBufferSize in fragments */
    int FragmentSize;		/* Size of one Fragment in samples */

    int LowWatermark;		/* ALSA: frags_min */
    int HighWatermark;		/* ALSA: frags_max */

    int FFTW_Policy;		/* FFTW: 0 == FFTW_ESTIMATE , 1 == FFTW_MEASURE */
    int FFTW_UseWisdom;		/* use FFTW_WISDOM */

    char *output_id[3];		/* plugin ID */
    char *device_id[3];		/* device ID */

    ChannelConfig cfg[4];
}
AudioConfig;



typedef struct
{
    int Tabs;
    char **TabTitle;
    char **TabString;

    char *database_file;
    int db_autoload;
    int db_autosave;
    int db_compress;

    int CatClickButton;
    int CatClickAction;
    int CatDragButton;

    int SongClickButton;
    int SongClickAction;
    int SongDragButton;
   
}
SongDBConfig;

typedef struct
{
    char SampleFilename[9];
}
SamplerConfig;

typedef struct
{
    int TForwAfterFadeNow;

    /* settings */
    float fader_dB[4];
    int muted[4];
    int a[4];
    int b[4];
    int m[4];
    int beatcount[4];

    float crossfader_dB;
    float fade_time;
    int auto_fade;

    
}
MixerConfig;


typedef struct 
{
    AudioConfig   *Audio;
    MixerConfig   *Mixer;
    SamplerConfig *Sampler;
}
BeatforceConfig;

void CONFIGFILE_Free (BeatforceConfig * cfg);
void CONFIGFILE_OpenDefaultFile (void);
AudioConfig *CONFIGFILE_GetCurrentAudioConfig();
int CONFIGFILE_WriteDefaultFile (BeatforceConfig * cfg);



#endif /* __CONFIGFILE_H__ */
