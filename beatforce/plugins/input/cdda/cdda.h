/*
   BeatForce plugin
   mp3.h - mpeg layer 2/3 ouput plugin	(libmad)
   
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

#ifndef _CDDA_H__
#define _CDDA_H__

#define FRAG_SIZE 512


enum channel
{
    CHANNEL_STEREO = 0,
    CHANNEL_MONO = 1,
    CHANNEL_LEFT = 2,
    CHANNEL_RIGHT = 3,
    CHANNEL_REVERSE = 4
};

struct stats
{
    int vbr;
    unsigned int bitrate;
    unsigned long frames;
    unsigned long vbr_rate;
    unsigned long clipped;
    unsigned long sync_errors;
    unsigned long crc_errors;
    unsigned long other_errors;
    unsigned long ms_joint;
    unsigned long i_joint;
    unsigned long ms_i_joint;
};

struct config
{
    AFormat format;				/* bits per output sample */
    int autoatt;			/* auto clipping attenuation? */
    long attsensitivity;			/* auto attenuation sensitivity */
    int lengthcalc;			/* full (slow) length calculation? */
    int avgbitrate;			/* display average bitrate? */
};


typedef struct
{
    char *filename;				/* currently playing path/URL */
    int fd;

    long size;					/* file size in bytes */
    int length;					/* total playing time in ms */
    int bitrate;					/* average bitrate in kbps */
    int position; 				/* current playing position in ms */
    int paused;					/* are we paused? */
    int seek;					/* seek target in ms, or -1 */
    int going;					/* stop flag */
    int eof;

    int track;
    enum channel channel; 		/* channel selection */

    struct stats stats;			/* statistics */

    int ch_id;
    int rate;
    int channels;
    long magic;

    int max_bytes;

    int decode_thread;

    int audio_error;


    char *input_buffer;
    int input_size;

    char *output_buffer;
    int output_size;

}
mp3Private;

struct id3tag
{
    char tag[3];
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    unsigned char genre;
};



//InputPlugin *get_input_info (void);

int cdda_init (Private **, int);
int cdda_cleanup (Private *);
int cdda_about (Private *);
int cdda_is_our_file (Private *, char *);

int cdda_get_tag (Private *, char *, struct SongDBEntry *);
int cdda_get_add_info (Private *, char *, struct SongAddInfo *);

int cdda_load_file (Private *, char *);
int cdda_close_file (Private *);

int cdda_play (Private *);
int cdda_pause (Private *);
int cdda_seek (Private *, long);

long cdda_get_time (Private *);



#endif
