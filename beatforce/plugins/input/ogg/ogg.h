/*
   BeatForce plugin
   oggh - Ogg vorbis decoder plugin
   
   Copyright (c) 2004, John Beuving (john.beuving@home.nl)

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

#ifndef __OGG_H__
#define __OGG_H__
#include <vorbis/vorbisfile.h>


#define FRAG_SIZE 512
#define PCM_CHUNK		576

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
  OggVorbis_File vf;
  FILE *fd;

  long size;					/* file size in bytes */
  int length;					/* total playing time in ms */
  int bitrate;					/* average bitrate in kbps */
  int position; 				/* current playing position in ms */
  int paused;					/* are we paused? */
  int seek;					/* seek target in ms, or -1 */
  int going;					/* stop flag */
  int eof;

  enum channel channel; 		/* channel selection */

  struct stats stats;			/* statistics */

  int ch_id;
  int rate;
  int channels;
  long magic;

  int max_bytes;

    int decode_thread;
  //pthread_t decode_thread;

  int audio_error;

  char *input_buffer;
  int input_size;

  char *output_buffer;
  int output_size;

}
oggPrivate;




//InputPlugin *get_input_info (void);

int ogg_init (Private **, int);
int ogg_cleanup (Private *);
int ogg_about (Private *);
int ogg_configure (Private *,struct SongDBEntry *);
int ogg_is_our_file (Private *, char *);

int ogg_get_tag (Private *, char *, struct SongDBEntry *);
int ogg_get_add_info (Private *, char *, struct SongAddInfo *);

int ogg_load_file (Private *, char *);
int ogg_close_file (Private *);

int ogg_play (Private *);
int ogg_pause (Private *);
int ogg_seek (Private *, long);

long ogg_get_time (Private *);

int ogg_set_input_interface(InputInterface *api);

#endif
