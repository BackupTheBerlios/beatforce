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

#include "llist.h"

typedef struct
{
    char *key;
    char *value;
}
ConfigLine;

typedef struct
{
    char *name;
    BFList *lines;
}
ConfigSection;

typedef struct
{
    BFList *sections;
}
ConfigFile;

char *bf_cfg_get_default_filename (void);

ConfigFile *bf_cfg_new (void);
ConfigFile *bf_cfg_open_file (char * filename);
int bf_cfg_write_file (ConfigFile * cfg, char * filename);
void bf_cfg_free (ConfigFile * cfg);
ConfigFile *bf_cfg_open_default_file (void);
int bf_cfg_create_dir (void);
int bf_cfg_write_default_file (ConfigFile * cfg);

int bf_cfg_read_string (ConfigFile * cfg, char * section, char * key,
			     char ** value);
int bf_cfg_read_int (ConfigFile * cfg, char * section, char * key,
			  int * value);
int bf_cfg_read_boolean (ConfigFile * cfg, char * section, char * key,
			      int * value);
int bf_cfg_read_float (ConfigFile * cfg, char * section, char * key,
			    float * value);
int bf_cfg_read_double (ConfigFile * cfg, char * section, char * key,
			     double * value);

void bf_cfg_write_string (ConfigFile * cfg, char * section, char * key,
			  char * value);
void bf_cfg_write_int (ConfigFile * cfg, char * section, char * key,
		       int value);
void bf_cfg_write_boolean (ConfigFile * cfg, char * section, char * key,
			   int value);
void bf_cfg_write_float (ConfigFile * cfg, char * section, char * key,
			 float value);
void bf_cfg_write_double (ConfigFile * cfg, char * section, char * key,
			  double value);

void bf_cfg_remove_key (ConfigFile * cfg, char * section, char * key);


typedef struct
{
    int x;
    int y;
    int height;
    int width;
    int show;
}
PositionConfig;


typedef struct
{
    int RemoveAfterPlay;

    PositionConfig pos;

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

    PositionConfig pos;
}
SongDBConfig;

typedef struct
{

    PositionConfig pos;
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

    PositionConfig pos;
}
MixerConfig;

AudioConfig *bf_cfg_read_AudioConfig (ConfigFile *);
SongDBConfig *bf_cfg_read_SongDBConfig (ConfigFile *);
PositionConfig *bf_cfg_read_PositionConfig (ConfigFile *, char *,
					    PositionConfig *);

PlayerConfig *bf_cfg_read_PlayerConfig (ConfigFile *, int);
SamplerConfig *bf_cfg_read_SamplerConfig (ConfigFile *);
MixerConfig *bf_cfg_read_MixerConfig (ConfigFile *);

int bf_cfg_write_AudioConfig (ConfigFile *, AudioConfig *);
int bf_cfg_write_SongDBConfig (ConfigFile *, SongDBConfig *);
int bf_cfg_write_PositionConfig (ConfigFile *, char *, PositionConfig *);
int bf_cfg_write_PlayerConfig (ConfigFile *, PlayerConfig *, int);
int bf_cfg_write_SamplerConfig (ConfigFile *, SamplerConfig *);
int bf_cfg_write_MixerConfig (ConfigFile *, MixerConfig *);

#endif /* __CONFIGFILE_H__ */
