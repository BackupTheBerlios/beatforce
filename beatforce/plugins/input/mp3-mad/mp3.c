/*
   BeatForce plugin
   mp3.c - mpeg layer 1/2/3 ouput plugin  using libmad
   
   Copyright (c) 2001, Patrick Prasse (patrick.prasse@gmx.net)

   Credits to Rob Leslie (rob@mars.org) who wrote the mad winamp plugin
   on which this plugin is based.

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


#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <errno.h>
#include <time.h>


#include <math.h>

#include <mad.h>
#include <id3tag.h>

#include "types.h"
#include "input_plugin.h"
#include "err.h"
#include "osa.h"
#include "mp3.h"

#define MODULE_ID MP3
#include "debug.h"

#ifndef TRACE
#endif

#define MP3_INPUT_BUFFER_SIZE	(40000*2)
#define MP3_OUTPUT_BUFFER_SIZE	((575+1152)*4*2)

void *mp3_play_loop (void *);

struct config *cfg;

char *str_mpeg1_l1=NULL;
char *str_mpeg1_l2=NULL;
char *str_mpeg1_l3=NULL;
char *str_mpeg25_l3=NULL;

InputPlugin mp3_ip = {
    NULL, 						/* handle, BeatForce fills it */
    NULL, 						/* filename, BeatForce filled */
    "mp3 input plugin (mad)",	/* Description */

    mp3_init,
    mp3_configure,
    NULL,
    mp3_is_our_file,

    mp3_get_tag,
    mp3_get_add_info,

    mp3_load_file,
    mp3_close_file,
    mp3_play,
    mp3_pause,
    NULL,

    mp3_seek,
    mp3_get_time,
    NULL, 						/* WE FILL IT */

    NULL,
    NULL,

    mp3_cleanup
};

InputInterface mp3_if = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

InputPlugin *
get_input_info (InputInterface *api)
{
    mp3_if.input_eof          = api->input_eof;   
    mp3_if.output_buffer_free = api->output_buffer_free;
    mp3_if.output_close       = api->output_close;
    mp3_if.output_get_time    = api->output_get_time;
    mp3_if.output_open        = api->output_open;
    mp3_if.output_pause       = api->output_pause;
    mp3_if.output_write       = api->output_write;

    return &mp3_ip;
}


/*ch_id is equal to player_nr */
int
mp3_init (Private ** p, int ch_id)
{
    mp3Private *mp3_priv;

    TRACE("mp3_init enter: %d", ch_id);

    if(p == NULL)
        return ERROR_INVALID_ARG;


    if( !str_mpeg1_l1 )
	str_mpeg1_l1 = strdup ("MPEG 1, Layer I");
    if( !str_mpeg1_l2 )
	str_mpeg1_l2 = strdup ("MPEG 1, Layer II");
    if( !str_mpeg1_l3 )
	str_mpeg1_l3 = strdup ("MPEG 1, Layer III");
    if( !str_mpeg25_l3 )
	str_mpeg25_l3 = strdup("MPEG 2.5, Layer III");


    mp3_priv = malloc (sizeof (mp3Private));
    if (mp3_priv == NULL)
	return ERROR_NO_MEMORY;
    memset (mp3_priv, 0, sizeof (mp3Private));

    mp3_priv->input_buffer  = malloc (MP3_INPUT_BUFFER_SIZE);
    mp3_priv->output_buffer = malloc (MP3_OUTPUT_BUFFER_SIZE);
    if (!mp3_priv->input_buffer || !mp3_priv->output_buffer)
    {
	free (mp3_priv->input_buffer);
	free (mp3_priv->output_buffer);
	return ERROR_NO_MEMORY;
    }
    memset (mp3_priv->input_buffer, 0, MP3_INPUT_BUFFER_SIZE);
    memset (mp3_priv->output_buffer, 0, MP3_OUTPUT_BUFFER_SIZE);

    mp3_priv->output_size = MP3_OUTPUT_BUFFER_SIZE;
    mp3_priv->input_size = MP3_INPUT_BUFFER_SIZE;

    cfg = malloc (sizeof (struct config));
    
    if (cfg == NULL)
	return ERROR_NO_MEMORY;
    memset (cfg, 0, sizeof (struct config));
    cfg->lengthcalc = 1;

    mp3_priv->ch_id = ch_id;
    mp3_priv->magic = MP3MAD_MAGIC;

    //to be done
    mp3_priv->going     =1 ;
//    mp3_priv->decode_thread=OSA_CreateThread(mp3_play_loop, (void *)mp3_priv);
    *p = (Private *) mp3_priv;

    TRACE("mp3_init leave");
    return 0;
}

int mp3_configure(Private *p,struct SongDBEntry *e)
{


}

int
mp3_cleanup (Private * p)
{
    mp3Private *mp3_priv = (mp3Private *) p;

    if(p == NULL)
        return ERROR_INVALID_ARG;

    if(mp3_priv->magic != MP3MAD_MAGIC)
        return ERROR_INVALID_ARG;
  
    free (mp3_priv);
    free (cfg);

    return 0;
}

int
mp3_is_our_file (Private * h, char *filename)
{
    char *ext;

    if (h == NULL || filename == NULL)
        return FALSE;

    ext = strrchr (filename, '.');
    if (ext)
	if (!strcmp (ext, ".mp3") || !strcmp (ext, ".mp2")
            || !strcmp (ext, ".mpg") || !strcmp (ext, ".mpeg"))
	{
            return TRUE;
	}
    return FALSE;
}

static void
scan_file (FILE * fd, int *length, int *bitrate)
{
    struct mad_stream stream;
    struct mad_header header;
    mad_timer_t timer;
    unsigned char buffer[8192];
    unsigned int buflen = 0;

    mad_stream_init (&stream);
    mad_header_init (&header);

    timer = mad_timer_zero;

    while (1)
    {
	if (buflen < 8192)
	{
            int bytes = 0;

            bytes = fread (buffer + buflen, 1, 8192 - buflen, fd);
            if (bytes <= 0)
		break;

            buflen += bytes;
	}

	mad_stream_buffer (&stream, buffer, buflen);

	while (1)
	{
            if (mad_header_decode (&header, &stream) == -1)
            {
		if (!MAD_RECOVERABLE (stream.error))
                    break;
		continue;
            }
            if (length)
		mad_timer_add (&timer, header.duration);

	}

	if (stream.error != MAD_ERROR_BUFLEN)
            break;

	memmove (buffer, stream.next_frame, &buffer[buflen] - stream.next_frame);
	buflen -= stream.next_frame - &buffer[0];
    }

    mad_header_finish (&header);
    mad_stream_finish (&stream);

    if (length)
	*length = mad_timer_count (timer, MAD_UNITS_MILLISECONDS);
}

int
mp3_get_tag (Private * h, char *path, struct SongDBEntry *e)
{
    FILE *fd;
    mp3Private *mp3_priv = (mp3Private *) h;
    int length = 0;
    long size = 0;

    char *empty = strdup ("");
    char *title = NULL, *artist = NULL, *album = NULL;
    char *comment = NULL, *year = NULL, *genre = NULL;


    if(h == NULL || path == NULL || e ==NULL)
        return ERROR_INVALID_ARG;
  

    {
	struct id3_file *id3file;
	struct id3_tag *id3tag;
	id3_ucs4_t *ucs4;
	id3_latin1_t *latin1;
	struct id3_frame *frame;

	id3file = id3_file_open (path, ID3_FILE_MODE_READONLY);
	if (id3file == NULL)
	{
            printf ("get_tag: Error opening file: %s\n", strerror (errno));
            return -1;
	}

	id3tag = id3_file_tag (id3file);

	frame = id3_tag_findframe (id3tag, ID3_FRAME_TITLE, 0);
	if (frame != NULL)
	{
            ucs4 =
		(id3_ucs4_t *) id3_field_getstrings ((union id3_field const *)
                                                     &frame->fields[1], 0);
            if (ucs4 != NULL)
            {
		latin1 = id3_ucs4_latin1duplicate (ucs4);
		if (latin1 != NULL)
		{
                    title = strdup (latin1);
		}
            }
	}

	frame = id3_tag_findframe (id3tag, ID3_FRAME_ARTIST, 0);
	if (frame != NULL)
	{
            ucs4 = (id3_ucs4_t *) id3_field_getstrings (&frame->fields[1], 0);
            if (ucs4 != NULL)
            {
		latin1 = id3_ucs4_latin1duplicate (ucs4);
		if (latin1 != NULL)
		{
                    artist = strdup (latin1);
		}
            }
	}

	frame =
            id3_tag_findframe ((struct id3_tag const *) id3tag, ID3_FRAME_ALBUM, 0);
	if (frame != NULL)
	{
            ucs4 = (id3_ucs4_t *) id3_field_getstrings (&frame->fields[1], 0);
            if (ucs4 != NULL)
            {
		latin1 = id3_ucs4_latin1duplicate (ucs4);
		if (latin1 != NULL)
		{
                    album = strdup (latin1);
		}
            }
	}

	frame =
            id3_tag_findframe ((struct id3_tag const *) id3tag, ID3_FRAME_COMMENT,
                               0);
	if (frame != NULL)
	{
            ucs4 = (id3_ucs4_t *) id3_field_getstrings (&frame->fields[1], 0);
            if (ucs4 != NULL)
            {
		latin1 = id3_ucs4_latin1duplicate (ucs4);
		if (latin1 != NULL)
		{
                    comment = strdup (latin1);
		}
            }
	}

	frame =
            id3_tag_findframe ((struct id3_tag const *) id3tag, ID3_FRAME_YEAR, 0);
	if (frame != NULL)
	{
            ucs4 = (id3_ucs4_t *) id3_field_getstrings (&frame->fields[1], 0);
            if (ucs4 != NULL)
            {
		latin1 = id3_ucs4_latin1duplicate (ucs4);
		if (latin1 != NULL)
		{
                    year = strdup (latin1);
		}
            }
	}

	frame =
            id3_tag_findframe ((struct id3_tag const *) id3tag, ID3_FRAME_GENRE, 0);
	if (frame != NULL)
	{
            ucs4 = (id3_ucs4_t *) id3_field_getstrings (&frame->fields[1], 0);
            if (ucs4 != NULL)
            {
		ucs4 = (id3_ucs4_t *) id3_genre_name (ucs4);
		if (ucs4 != NULL)
		{
                    latin1 = id3_ucs4_latin1duplicate (ucs4);
                    if (latin1 != NULL)
                    {
			genre = strdup (latin1);
                    }
		}
            }
	}


	id3_file_close (id3file);
    }



    e->title   = title;
    e->artist  = artist;
    e->album   = album;
    e->comment = comment;
    e->year    = year;
    e->genre   = genre;


/* length calculation stuff */
    {
	struct mad_header header;
	struct xing xing;

	if( e->AddInfo == NULL )
	{
            e->AddInfo = malloc( sizeof(struct SongAddInfo) );
            if(e->AddInfo == NULL)
                return ERROR_NO_MEMORY;
            memset( e->AddInfo, 0, sizeof( struct SongAddInfo ));
	}

	fd = fopen (path, "rb");
	if (fd == NULL)
	{
            printf ("get_tag: Error opening file %s:%s\n", path, strerror (errno));
            return ERROR_OPEN_ERROR;
	}

        fseek(fd,0,SEEK_END);
        size=ftell(fd);
        fseek(fd,0,SEEK_SET);
	e->filesize = size;
        
	if (scan_header (fd, &header, &xing) == -1)
	{
            fclose (fd);

            printf ("get_tag: Error Reading File\n");
            return ERROR_READ_SEEK_ERROR;
	}

	switch( header.layer )
	{
        case MAD_LAYER_I: e->AddInfo->type = str_mpeg1_l1; break;
        case MAD_LAYER_II: e->AddInfo->type = str_mpeg1_l2; break;
        case MAD_LAYER_III:
            if( header.flags & MAD_FLAG_MPEG_2_5_EXT )
                e->AddInfo->type = str_mpeg25_l3;
            else 
                e->AddInfo->type = str_mpeg1_l3;
            break;
        default: e->AddInfo->type = NULL; break;
	}
	e->AddInfo->n_ch = MAD_NCHANNELS(&header);
	e->AddInfo->SampleRate = header.samplerate;
	e->AddInfo->bitrate = header.bitrate;

	e->AddInfo->err_protection = ((header.flags & MAD_FLAG_PROTECTION) >0);
	e->AddInfo->copyright = ((header.flags & MAD_FLAG_COPYRIGHT) >0);
	e->AddInfo->original = ((header.flags & MAD_FLAG_ORIGINAL) >0);

	fseek (fd, 0, SEEK_SET);

//	   bitrate = 0;

	if (xing.flags & XING_FRAMES)
	{
            mad_timer_t timer;

            timer = header.duration;
            mad_timer_multiply (&timer, xing.frames);

            length = mad_timer_count (timer, MAD_UNITS_MILLISECONDS);
/*
  if (xing.flags & XING_BYTES)
  bitrate = xing.bytes * 8 / length;
*/
//				printf ("XING header w/ XING_FRAMES found. length = %d\n", length);

	}
	else
	{
            if (!cfg->lengthcalc)
		length = -(size * 8 / (header.bitrate / 1000)); /* est. */
            else
            {
		fseek (fd, 0, SEEK_SET);
		scan_file (fd, &length, NULL);
            }
	}

	e->time   = length;
        mp3_priv->length = length;
    }

    fclose (fd);
    mp3_priv->size = size;
    mp3_priv->position = 0;
    return 0;
}

int
mp3_get_add_info (Private * h, char *filename, struct SongAddInfo *info)
{
    return ERROR_NOT_SUPPORTED;
}



int
mp3_load_file (Private * h, char *filename)
{
    mp3Private *private = (mp3Private *) h;
    struct mad_header header;
    int length=private->length;

#ifdef MP3MAD_DEBUG
    printf ("mp3_load_file: %s\n", filename);
#endif

    if (h == NULL || filename == NULL || private->magic != MP3MAD_MAGIC)
        return ERROR_INVALID_ARG;
  
    if (private->fd && private->going)
    {
	printf ("Already open!\n");
	return ERROR_ALREADY_OPEN;
    }

    if (mp3_is_our_file (h, filename) != TRUE)
    {
	printf ("Unknown file: %s!\n", filename);
	return ERROR_UNKNOWN_FILE;
    }

    private->frame  = malloc (sizeof (struct mad_frame));
    private->stream = malloc (sizeof (struct mad_stream));
    private->synth  = malloc (sizeof (struct mad_synth));
    if (!private->frame || !private->stream || !private->synth)
    {
	printf ("Unable to allocate memory!\n");
	return ERROR_NO_MEMORY;
    }

    memset (private->stream, 0, sizeof (struct mad_stream));
    memset (private->frame, 0, sizeof (struct mad_frame));
    memset (private->synth, 0, sizeof (struct mad_synth));

    mad_stream_init (private->stream);
    mad_frame_init (private->frame);
    mad_synth_init (private->synth);


    private->fd = fopen (filename, "rb");
    if (private->fd == NULL)
    {
	fprintf (stderr, "Error: %s\n", strerror (errno));
	return ERROR_OPEN_ERROR;
    }

    if (scan_header (private->fd, &header, NULL) == -1)
    {
	fclose (private->fd);
	private->fd = NULL;
	printf ("Error Reading Stream\n");
	return ERROR_FILE_FORMAT_ERROR;
    }

    fseek(private->fd,0,SEEK_END);
    private->size=ftell(private->fd);
    fseek(private->fd,0,SEEK_SET);

    if(length == 0)
    {
        fseek (private->fd, 0, SEEK_SET);
        scan_file (private->fd, &length, NULL);
        fseek (private->fd, 0, SEEK_SET);
    }
  
    private->length  = length;
    private->bitrate = 0;
    private->going   = 1;
    private->seek    = -1;
    private->eof     = 0;

    private->channels = MAD_NCHANNELS (&header);
    private->rate = header.samplerate;

    if (mp3_if.output_open
        (private->ch_id,
         FMT_S16_NE, private->rate, private->channels, &private->max_bytes))
    {
	private->audio_error = TRUE;
	fclose (private->fd);
	private->fd = NULL;
	printf ("Error opening output!\n");
	return ERROR_OUTPUT_ERROR;
    }

    private->decode_thread=OSA_CreateThread(mp3_play_loop, (void *)private);

#ifdef MP3MAD_DEBUG
    printf ("mp3_load_file end.\n");
#endif
    private->position = 0;
    return 0;
}

int
mp3_close_file (Private * h)
{
    mp3Private *private = (mp3Private *) h;

#ifdef MP3MAD_DEBUG
    printf ("mp3_close_file: %d\n", private->ch_id);
#endif

    if( h == NULL || private->magic != MP3MAD_MAGIC)
        return ERROR_INVALID_ARG;
  

    if (private->going && private->fd != NULL)
    {
	private->going = 0;
        OSA_RemoveThread(private->decode_thread);
	//pthread_join (private->decode_thread, NULL);
	mp3_if.output_close (private->ch_id);
	fclose (private->fd);
	private->fd = NULL;

	mad_synth_finish (private->synth);
	mad_frame_finish (private->frame);
	mad_stream_finish (private->stream);

	free (private->synth);
	free (private->stream);
	free (private->frame);

	return 0;
    }

    return ERROR_NOT_OPEN;
}


void *
mp3_play_loop (void *param)
{
    mp3Private *private = param;
    unsigned int input_length = 0, output_length = 0;
    int resolution = 16;

    mad_timer_t timer, duration;
    int avgbitrate, bitrate, last_bitrate = 0, seek_skip = 0, last_error = 0;

    timer = duration = mad_timer_zero;

    printf("Starting thread\n");
    if(param == NULL || private->input_buffer == NULL)
        return (void *) ERROR_INVALID_ARG;
  
    if(private->stream == NULL || private->frame == NULL)
        return (void *) ERROR_INVALID_ARG;

    if(private->synth == NULL || private->output_buffer == NULL)
        return (void *) ERROR_INVALID_ARG;
  
    if(private->magic != MP3MAD_MAGIC)
        return (void *) ERROR_INVALID_ARG;
  
    
    while (private->going)
    {
	if (private->eof)
	{
            mp3_if.input_eof (private->ch_id);
	}

	while (!private->eof)
	{
            if (input_length < private->input_size / 2)
            {
		int bytes;

		bytes = fread (private->input_buffer + input_length, 1,
                               private->input_size - input_length, private->fd);

		if (bytes == 0)
		{
                    private->eof = 1;
                    
                    while (bytes < MAD_BUFFER_GUARD)
			private->input_buffer[input_length + bytes++] = 0;
		}
		else if (bytes < 0)
		{
                    printf ("Error Reading Data: %s\n", strerror (errno));
                    break;
		}

		input_length += bytes;
            }

            mad_stream_buffer (private->stream, private->input_buffer,
                               input_length);

            while (private->going)
            {
		char *output_ptr;
		int nch, bytes;
		mad_fixed_t const *ch1, *ch2;

		if (private->seek != -1 && private->length >= 0)
		{
                    int new_position;
                    long pos;

                    printf("Private size %d\n",private->size);
                 
                    if (private->seek < 0)
			new_position = (double) private->length * -private->seek / 1000;
                    else
			new_position = private->seek;

                    private->position = new_position;
                    private->seek = -1;
                    
                    pos = private->position * private->size / private->length;
                    
                    printf("Vars %d %d %d\n",new_position,private->size,private->length);
                    printf("Seeking to position %ld\n",((long)((double) new_position *
                                                             private->size / private->length)));

                    if (fseek (private->fd, (long)((double) new_position *
                               private->size / private->length), SEEK_SET) != -1)
                    {
			mad_timer_set (&timer, new_position / 1000,
                                       new_position % 1000, 1000);

			mad_frame_mute (private->frame);
			mad_synth_mute (private->synth);

			input_length =
                            ((unsigned int) private->stream->next_frame) -
                            ((unsigned int) & private->input_buffer[0]);
			private->stream->error = MAD_ERROR_BUFLEN;
			private->stream->sync = 0;

			if (new_position > 0)
                            seek_skip = new_position;
/*
  else
  module.outMod->Flush (0);
*/
			break;
                    }
                    else
                    {
                        printf("Error seeking in file\n");
                    }
		}

		if (mad_frame_decode (private->frame, private->stream) == -1)
		{
                    if (!MAD_RECOVERABLE (private->stream->error))
			break;

                    // module.SetInfo (-1, -1, -1, 0);
                    last_bitrate = 0;

                    continue;
		}
		else
                    last_error = 0;

		++private->stats.frames;

		switch (private->frame->header.
                        flags & (MAD_FLAG_MS_STEREO | MAD_FLAG_I_STEREO))
		{
		case MAD_FLAG_MS_STEREO:
                    ++private->stats.ms_joint;
                    break;

		case MAD_FLAG_I_STEREO:
                    ++private->stats.i_joint;
                    break;

		case (MAD_FLAG_MS_STEREO | MAD_FLAG_I_STEREO):
                    ++private->stats.ms_i_joint;
                    break;
		}

		avgbitrate = private->bitrate ?
                    private->bitrate : vbr_update (&private->stats,
                                                   private->frame->header.bitrate);

		bitrate = private->frame->header.bitrate / 1000;

		if (bitrate != last_bitrate)
		{
//				  module.SetInfo (bitrate, -1, -1, 1);
                    last_bitrate = bitrate;
		}


		mad_synth_frame (private->synth, private->frame);

		nch = private->synth->pcm.channels;
		ch1 = private->synth->pcm.samples[0];
		ch2 = private->synth->pcm.samples[1];

		output_length +=
                    pack_pcm (private->output_buffer + output_length,
                              private->synth->pcm.length, ch1, ch2, resolution,
                              &private->stats.clipped, &private->stats.clipping);

		output_ptr = private->output_buffer;

		mad_timer_set (&duration, 0, PCM_CHUNK,
                               private->synth->pcm.samplerate);

		bytes = PCM_CHUNK * (resolution / 8) * nch;

		while (output_length >= bytes)
		{

                    while (private->going
                           && !(private->seek != -1 && private->length >= 0)
                           && mp3_if.output_buffer_free (private->ch_id) < bytes)
                        SDL_Delay(10);
                    //beatforce_usleep (10);

                    if (!private->going
                        || (private->seek != -1 && private->length >= 0))
			break;

                    mad_timer_add (&timer, duration);
                    private->position = mad_timer_count (timer, MAD_UNITS_MILLISECONDS);

                    mp3_if.output_write (private->ch_id, output_ptr, bytes);

                    output_ptr += bytes;
                    output_length -= bytes;
		}

		if (private->seek != -1 && private->length >= 0)
                    output_length = 0;
		else if (output_length)
                    memmove (private->output_buffer, output_ptr, output_length);
            }

            if (!private->going || private->stream->error != MAD_ERROR_BUFLEN)
		break;


            input_length =
		(int) ((unsigned int) private->input_buffer -
                       (unsigned int) private->stream->next_frame) + input_length;
            memmove (private->input_buffer, private->stream->next_frame,
                     input_length);
	}

	if (private->going)
	{
            private->eof = 1;
            mp3_if.input_eof (private->ch_id);
	}
    }


    return 0;
}


int
mp3_play (Private * h)
{
    mp3Private *private = (mp3Private *) h;

    if( h == NULL || private->magic != MP3MAD_MAGIC)
        return ERROR_INVALID_ARG;
   
    mp3_if.output_pause (private->ch_id, 0);
    return 0;
}

int
mp3_pause (Private * h)
{
    mp3Private *private = (mp3Private *) h;
    if( h == NULL || private->magic != MP3MAD_MAGIC)
        return ERROR_INVALID_ARG;
  
    mp3_if.output_pause (private->ch_id, 1);
    return 0;
}

int
mp3_seek (Private * h, long msecs)
{
    mp3Private *private = (mp3Private *) h;
   
    if( h == NULL || private->magic != MP3MAD_MAGIC)
        return ERROR_INVALID_ARG;

    private->seek = (int) msecs;
    printf("Seeking to %d\n",private->seek);
    private->eof = FALSE;
    return 0;
}


long
mp3_get_time (Private * h)
{
    mp3Private *private = (mp3Private *) h;

    if( h == NULL || private->magic != MP3MAD_MAGIC)
        return ERROR_INVALID_ARG;
  
    if (private->eof)
	return ERROR_EOF;
    if (!private->going)
	return ERROR_NO_FILE_LOADED;
    
    return private->position;
}


static __inline signed long
linear_dither (unsigned int bits, mad_fixed_t sample,
               mad_fixed_t * error, unsigned long *clipped,
               mad_fixed_t * clipping)
{
    mad_fixed_t quantized, check;

    /* dither */
    sample += *error;

    /* clip */
    quantized = sample;
    check = (sample >> MAD_F_FRACBITS) + 1;
    if (check & ~1)
    {
	if (sample >= MAD_F_ONE)
	{
            quantized = MAD_F_ONE - 1;
            ++*clipped;
            if (sample - quantized > *clipping &&
                mad_f_abs (*error) < (MAD_F_ONE >> (MAD_F_FRACBITS + 1 - bits)))
		*clipping = sample - quantized;
	}
	else if (sample < -MAD_F_ONE)
	{
            quantized = -MAD_F_ONE;
            ++*clipped;
            if (quantized - sample > *clipping &&
                mad_f_abs (*error) < (MAD_F_ONE >> (MAD_F_FRACBITS + 1 - bits)))
		*clipping = quantized - sample;
	}
    }

    /* quantize */
    quantized &= ~((1L << (MAD_F_FRACBITS + 1 - bits)) - 1);

    /* error */
    *error = sample - quantized;

    /* scale */
    return quantized >> (MAD_F_FRACBITS + 1 - bits);
}


static unsigned int
pack_pcm (unsigned char *data, unsigned int nsamples,
          mad_fixed_t const *left, mad_fixed_t const *right,
          int resolution, unsigned long *clipped, mad_fixed_t * clipping)
{
    static mad_fixed_t left_err, right_err;
    unsigned char const *start;
    register signed long sample0, sample1;
    int effective, bytes;

    start = data;
    effective = (resolution > 24) ? 24 : resolution;
    bytes = resolution / 8;

    if (right)
    {							/* stereo */
	while (nsamples--)
	{
            sample0 = linear_dither (effective, *left++, &left_err,
                                     clipped, clipping);
            sample1 = linear_dither (effective, *right++, &right_err,
                                     clipped, clipping);

            switch (resolution)
            {
            case 8:
		data[0] = sample0 + 0x80;
		data[1] = sample1 + 0x80;
		break;

            case 32:
		sample0 <<= 8;
		sample1 <<= 8;
		data[3] = sample0 >> 24;
		data[bytes + 3] = sample1 >> 24;
            case 24:
		data[2] = sample0 >> 16;
		data[bytes + 2] = sample1 >> 16;
            case 16:
		data[1] = sample0 >> 8;
		data[bytes + 1] = sample1 >> 8;
		data[0] = sample0 >> 0;
		data[bytes + 0] = sample1 >> 0;
            }

            data += bytes * 2;
	}
    }
    else
    {							/* mono */
	while (nsamples--)
	{
            sample0 = linear_dither (effective, *left++, &left_err,
                                     clipped, clipping);

            switch (resolution)
            {
            case 8:
		data[0] = sample0 + 0x80;
		break;

            case 32:
		sample0 <<= 8;
		data[3] = sample0 >> 24;
            case 24:
		data[2] = sample0 >> 16;
            case 16:
		data[1] = sample0 >> 8;
		data[0] = sample0 >> 0;
            }
            data += bytes;
	}
    }

    return data - start;
}

static int
vbr_update (struct stats *stats, unsigned long bitrate)
{
    bitrate /= 1000;
    stats->vbr_rate += bitrate;

    stats->vbr += (stats->bitrate && stats->bitrate != bitrate) ? 128 : -1;
    if (stats->vbr < 0)
	stats->vbr = 0;
    else if (stats->vbr > 512)
	stats->vbr = 512;

    stats->bitrate = bitrate;

    return stats->vbr ?
	((stats->vbr_rate * 2) / stats->frames + 1) / 2 : stats->bitrate;
}

static int
parse_xing (struct xing *xing, struct mad_bitptr ptr, unsigned int bitlen)
{
    if (bitlen < 64 || mad_bit_read (&ptr, 32) != XING_MAGIC)
	goto fail;

    xing->flags = mad_bit_read (&ptr, 32);
    bitlen -= 64;

    if (xing->flags & XING_FRAMES)
    {
	if (bitlen < 32)
            goto fail;

	xing->frames = mad_bit_read (&ptr, 32);
	bitlen -= 32;
    }

    if (xing->flags & XING_BYTES)
    {
	if (bitlen < 32)
            goto fail;

	xing->bytes = mad_bit_read (&ptr, 32);
	bitlen -= 32;
    }

    if (xing->flags & XING_TOC)
    {
	int i;

	if (bitlen < 800)
            goto fail;

	for (i = 0; i < 100; ++i)
            xing->toc[i] = mad_bit_read (&ptr, 8);

	bitlen -= 800;
    }

    if (xing->flags & XING_SCALE)
    {
	if (bitlen < 32)
            goto fail;

	xing->scale = mad_bit_read (&ptr, 32);
	bitlen -= 32;
    }

    return 0;

 fail:
    xing->flags = 0;
    return -1;
}

int
scan_header (FILE * fd, struct mad_header *header, struct xing *xing)
{
    struct mad_stream stream;
    struct mad_frame frame;
    unsigned char buffer[8192];
    unsigned int buflen = 0;
    int count = 0, result = 0;

    mad_stream_init (&stream);
    mad_frame_init (&frame);

    while (1)
    {
	if (buflen < 8192)
	{
            int bytes;

            bytes = fread (buffer + buflen, 1, 8192 - buflen, fd);
#ifdef MP3MAD_DEBUG
            printf ("read %d\n", bytes);
#endif
            if (bytes <= 0)
            {
		if (bytes == -1)
		{
                    printf ("could not read\n");
                    result = -1;
		}
		break;
            }

            buflen += bytes;
	}

	mad_stream_buffer (&stream, buffer, buflen);

	while (1)
	{
            if (mad_frame_decode (&frame, &stream) == -1)
            {
		if (!MAD_RECOVERABLE (stream.error))
		{
                    printf ("decoding error!\n");
                    break;
		}

//				  if (do_error (&stream, 0, input, 0, 0))
		continue;
            }

            if (count++ ||
                (xing
                 && parse_xing (xing, stream.anc_ptr, stream.anc_bitlen) == -1))
		break;
	}

	if (count || stream.error != MAD_ERROR_BUFLEN)
            break;

	memmove (buffer, stream.next_frame,
                 buflen = &buffer[buflen] - stream.next_frame);
    }

    if (count)
    {
	if (header)
            *header = frame.header;
    }
    else
	result = -1;

    mad_frame_finish (&frame);
    mad_stream_finish (&stream);

    return result;
}
