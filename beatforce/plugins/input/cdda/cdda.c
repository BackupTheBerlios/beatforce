/*
   BeatForce plugin
   cdda.c - audio cd to beatforce reader
   
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


#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/cdrom.h>
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
#include "cdda.h"



#define MP3_INPUT_BUFFER_SIZE	(40000*2)
#define MP3_OUTPUT_BUFFER_SIZE	((575+1152)*4*2)

void *cdda_play_loop (void *);

struct config *cfg;

#if 0
/*
 * extensions for transferring audio frames
 * currently used by sbpcd.c, cdu31a.c, ide-cd.c
 */
struct cdrom_read_audio
{
    union cdrom_addr addr; /* frame address */
    unsigned char addr_format; /* CDROM_LBA or CDROM_MSF */
    int nframes; /* number of 2352-byte-frames to read at once, limited by the drivers */
    unsigned char *buf; /* frame buffer (size: nframes*2352 bytes) */
};
#endif

static struct cdrom_read_audio arg;
static struct cdrom_tochdr hdr;
static struct cdrom_tocentry entry[100];

typedef struct TOC {	/* structure of table of contents (cdrom) */
    unsigned char reserved1;
    unsigned char bFlags;
    unsigned char bTrack;
    unsigned char reserved2;
    unsigned int dwStartSector;
    unsigned char ISRC[15];
} TOC;

TOC *ourtoc;
TOC toc[90];
int ourtracks;

char *str_mpeg1_l1=NULL;
static int err;
char *str_mpeg1_l2=NULL;
char *str_mpeg1_l3=NULL;
char *str_mpeg25_l3=NULL;

void readtoc();

InputPlugin mp3_ip = {
    NULL, 						/* handle, BeatForce fills it */
    NULL, 						/* filename, BeatForce filled */
    "cdda input plugin ",	/* Description */

    cdda_init,
    NULL,
    NULL,
    cdda_is_our_file,

    cdda_get_tag,
    cdda_get_add_info,
    NULL,                   // write_tag
    cdda_load_file,
    cdda_close_file,
    cdda_play,
    cdda_pause,
    NULL,

    cdda_seek,
    cdda_get_time,
    NULL, 						/* WE FILL IT */

    NULL,
    NULL,

    cdda_cleanup
};

InputInterface cdda_if = {
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
    cdda_if.input_eof          = api->input_eof;   
    cdda_if.output_buffer_free = api->output_buffer_free;
    cdda_if.output_close       = api->output_close;
    cdda_if.output_get_time    = api->output_get_time;
    cdda_if.output_open        = api->output_open;
    cdda_if.output_pause       = api->output_pause;
    cdda_if.output_write       = api->output_write;

    return &mp3_ip;
}


/*ch_id is equal to player_nr */
int
cdda_init (Private ** p, int ch_id)
{
    mp3Private *mp3_priv;


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
    mp3_priv->fd = -1;

    cfg = malloc (sizeof (struct config));
    
    if (cfg == NULL)
	return ERROR_NO_MEMORY;
    memset (cfg, 0, sizeof (struct config));
    cfg->lengthcalc = 0;

    mp3_priv->ch_id = ch_id;

    //to be done
    mp3_priv->going     =1 ;
//    mp3_priv->decode_thread=OSA_CreateThread(cdda_play_loop, (void *)mp3_priv);    
    *p = (Private *) mp3_priv;

    return 0;
}

int cdda_configure(Private *p,struct SongDBEntry *e)
{


}

int
cdda_cleanup (Private * p)
{
    mp3Private *mp3_priv = (mp3Private *) p;

    if(p == NULL)
        return ERROR_INVALID_ARG;

 
    free (mp3_priv);
    free (cfg);

    return 0;
}

int
cdda_is_our_file (Private * h, char *filename)
{
    char *ext;

    if (h == NULL || filename == NULL)
        return FALSE;

    ext = strrchr (filename, '.');
    printf("cdda_is_ourfile %s %s\n",filename,ext);
    if (ext)
	if (!strcmp (ext, ".cdda"))
	{
            return TRUE;
	}
    return FALSE;
}


int
cdda_get_tag (Private * h, char *path, struct SongDBEntry *e)
{ 
    
    int t,tt;
    char tm[20];
    char *s;
    unsigned long dw;
    int mins;
    int secnds;
    int centi_secnds;
    int i=1;
    
    readtoc();
    dw = (unsigned long) (ourtoc[i+1].dwStartSector - ourtoc[i].dwStartSector /* + 150 - 150 */);

    printf("Song startsector %ld - %ld\n",ourtoc[i+1].dwStartSector,ourtoc[i].dwStartSector);
    mins         =         dw / ( 60*75 );
    secnds       =       ( dw % ( 60*75 )) / 75;
    centi_secnds = ( 4 * ( dw %      75 ) + 1 ) / 3;

    e->time=mins*60*1000+secnds*1000+centi_secnds*10;
    return 1;

}



int
cdda_get_add_info (Private * h, char *filename, struct SongAddInfo *info)
{
    return ERROR_NOT_SUPPORTED;
}



int
cdda_load_file (Private * h, char *filename)
{
    mp3Private *private = (mp3Private *) h;
    int length=private->length;

    printf("CDDA_LOAD_FILE %s\n",filename);
    
    if (h == NULL || filename == NULL)
        return ERROR_INVALID_ARG;
  
    if (cdda_is_our_file (h, filename) != TRUE)
    {
        printf("This is not our file cdda\n");
	return ERROR_UNKNOWN_FILE;
    }

    private->fd = open("/dev/hdb",O_RDONLY);
    if (private->fd < 0) 
    {
        fprintf(stderr, "while opening %s :", "/dev/hdb");
        perror("ioctl cdrom device open error: ");
    }
    else
    {
        printf("Filedescriport %d\n",private->fd);
        private->going   = 1;
        private->rate    = 44100;
        private->channels= 2;

        
        
        if (cdda_if.output_open
            (private->ch_id,
             FMT_S16_NE, private->rate, private->channels, &private->max_bytes))
        {
            fclose (private->fd);
            private->fd = -1;
            printf("Can not open audio\n");
            return ERROR_OUTPUT_ERROR;
        }
        printf("Restarting thread %p\n",private);
        private->decode_thread=OSA_CreateThread(cdda_play_loop, (void *)private);
    }


    

    return 0;
}

int
cdda_close_file (Private * h)
{
    mp3Private *private = (mp3Private *) h;

    if( h == NULL )
        return ERROR_INVALID_ARG;
  
    printf("Closing file\n");
    printf("Closing file\n");
    printf("Closing file\n");
#if 0
    if (private->going && private->fd >= 0)
    {
	private->going = 0;
        OSA_RemoveThread(private->decode_thread);
	cdda_if.output_close (private->ch_id);
	private->fd = -1;
	return 0;
    }
#endif

    return ERROR_NOT_OPEN;
}


void *
cdda_play_loop (void *param)
{
    mp3Private *private = param;

    unsigned int input_length = 0, output_length = 0;
    int resolution = 16;
    unsigned int *buffer;
    int lba=32;
    int filedes=private->fd;

    buffer=malloc(sizeof(unsigned int) * 2352 * 75);
    printf("Starting thread %p %d\n",param,private->fd);
   
    while (1)
    {
        if(private->going)
        {
            arg.addr.lba = lba;
            arg.addr_format = CDROM_LBA;
            arg.nframes = 5;
            arg.buf = (unsigned char *) &buffer[0];

            if(filedes >= 0)
            {
                if(ioctl(private->fd, CDROMREADAUDIO, &arg) < 0)
                {
                    printf("Error\n");
                }
                while(cdda_if.output_buffer_free (private->ch_id) < (2352*5))
                    SDL_Delay(5);

                {
                    cdda_if.output_write (private->ch_id, buffer,2352*5);
                    lba+=5;
                    private->position+=67;
                }
            }
//            printf("Read %d\t",private->fd);
        }
       
    }
    
    return 0;
}


int
cdda_play (Private * h)
{
    mp3Private *private = (mp3Private *) h;

    if( h == NULL)
        return 0;
   
    cdda_if.output_pause (private->ch_id, 0);
    return 1;
}

int
cdda_pause (Private * h)
{
    mp3Private *private = (mp3Private *) h;
    if( h == NULL )
        return ERROR_INVALID_ARG;
  
    cdda_if.output_pause (private->ch_id, 1);
    return 1;
}

int
cdda_seek (Private * h, long msecs)
{
    mp3Private *private = (mp3Private *) h;
   
    if( h == NULL )
        return ERROR_INVALID_ARG;

    private->seek = (int) msecs;
    private->eof = FALSE;
    return 0;
}


long
cdda_get_time (Private * h)
{
    mp3Private *private = (mp3Private *) h;

    if( h == NULL)
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

            if (bytes <= 0)
            {
		if (bytes == -1)
		{
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
//                    printf ("decoding error!\n");
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




void readtoc()
{
    int fd,i;
    int tracks;
    
    fd = open("/dev/hdb",O_RDONLY);

    if (fd < 0) 
    {
        fprintf(stderr, "while opening %s :", "/dev/hdb");
        perror("ioctl cdrom device open error: ");
        exit(1);
    }

    ioctl( fd, CDROMREADTOCHDR, &hdr );

    for ( i = 0; i < hdr.cdth_trk1; i++ ) 
    {
	entry[i].cdte_track = 1+i;
	entry[i].cdte_format = CDROM_LBA;
	err = ioctl(fd, CDROMREADTOCENTRY, &entry[i] );
	if ( err != 0 ) 
        {
	    /* error handling */
	    fprintf( stderr, "can't get TocEntry #%d (error %d).\n", i+1, err );
	    exit( -1 );
	}
    }
    entry[i].cdte_track = CDROM_LEADOUT;
    entry[i].cdte_format = CDROM_LBA;
    err = ioctl( fd, CDROMREADTOCENTRY, &entry[i] );
    if ( err != 0 ) 
    {
	/* error handling */
	fprintf( stderr, "can't get TocEntry LEADOUT (error %d).\n", err );
	exit( -1 );
    }
    tracks = hdr.cdth_trk1+1;
    for (i = 0; i < tracks; i++) 
    {
        toc[i].bFlags = (entry[i].cdte_adr << 4) | (entry[i].cdte_ctrl & 0x0f);
        toc[i].bTrack = entry[i].cdte_track;
        toc[i].dwStartSector = entry[i].cdte_addr.lba;
    }
    ourtoc=toc;
    printf("Number of tracks %d\n",tracks);
    ourtracks=tracks-1;

}
