/*
   BeatForce plugin
   ogg.c - ogg vorbis decoder plugin
   
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


#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <errno.h>
#include <time.h>


#include <math.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "types.h"
#include "input_plugin.h"
#include "osa.h"

#include "ogg.h"

#define DEBUG printf
#define ERROR printf
#define TRACE printf
#define LOG printf

void *ogg_play_loop (void *);

struct config *cfg;

char *str_mpeg1_l1=NULL;
char *str_mpeg1_l2=NULL;
char *str_mpeg1_l3=NULL;
char *str_mpeg25_l3=NULL;

#define OGG_INPUT_BUFFER_SIZE	(40000*2)
#define OGG_OUTPUT_BUFFER_SIZE	((575+1152)*4*2)

InputPlugin ogg_ip = {
    NULL, 						/* handle, BeatForce fills it */
    NULL, 						/* filename, BeatForce filled */
    "ogg input plugin",                                 /* Description */

    ogg_init,
    ogg_configure,
    NULL,
    ogg_is_our_file,

    ogg_get_tag,
    ogg_get_add_info,
    NULL,                   // write_tag
    ogg_load_file,
    ogg_close_file,
    ogg_play,
    ogg_pause,
    NULL,

    ogg_seek,
    ogg_get_time,
    NULL, 						/* WE FILL IT */

    NULL,
    NULL,

    ogg_cleanup
};

InputInterface ogg_if = {
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
    ogg_if.input_eof          = api->input_eof;   
    ogg_if.output_buffer_free = api->output_buffer_free;
    ogg_if.output_close       = api->output_close;
    ogg_if.output_get_time    = api->output_get_time;
    ogg_if.output_open        = api->output_open;
    ogg_if.output_pause       = api->output_pause;
    ogg_if.output_write       = api->output_write;

    return &ogg_ip;
}


/*ch_id is equal to player_nr */
int
ogg_init (Private ** p, int ch_id)
{
    oggPrivate *ogg_priv;

    TRACE("ogg_init enter: %d", ch_id);

    if(p == NULL)
    {
        ERROR("Invalid argument");
        return 0;
    }


    if( !str_mpeg1_l1 )
	str_mpeg1_l1 = strdup ("MPEG 1, Layer I");
    if( !str_mpeg1_l2 )
	str_mpeg1_l2 = strdup ("MPEG 1, Layer II");
    if( !str_mpeg1_l3 )
	str_mpeg1_l3 = strdup ("MPEG 1, Layer III");
    if( !str_mpeg25_l3 )
	str_mpeg25_l3 = strdup("MPEG 2.5, Layer III");


    ogg_priv = malloc (sizeof (oggPrivate));
    if (ogg_priv == NULL)
    {
        ERROR("Not enough memory");
	return 0;
    }

    
    memset (ogg_priv, 0, sizeof (oggPrivate));

    ogg_priv->input_buffer  = malloc (OGG_INPUT_BUFFER_SIZE);
    ogg_priv->output_buffer = malloc (OGG_OUTPUT_BUFFER_SIZE);
    if (!ogg_priv->input_buffer || !ogg_priv->output_buffer)
    {
	free (ogg_priv->input_buffer);
	free (ogg_priv->output_buffer);
        ERROR("Not enough memory");
	return 0;
    }
    memset (ogg_priv->input_buffer, 0, OGG_INPUT_BUFFER_SIZE);
    memset (ogg_priv->output_buffer, 0, OGG_OUTPUT_BUFFER_SIZE);

    ogg_priv->output_size = OGG_OUTPUT_BUFFER_SIZE;
    ogg_priv->input_size = OGG_INPUT_BUFFER_SIZE;

    cfg = malloc (sizeof (struct config));
    
    if (cfg == NULL)
    {
        ERROR("Not enough memory");
	return 0;
    }
    memset (cfg, 0, sizeof (struct config));
    cfg->lengthcalc = 0;

    ogg_priv->ch_id = ch_id;

    //to be done
    ogg_priv->going     =1 ;

    *p = (Private *) ogg_priv;

    TRACE("ogg_init leave");
    return 1;
}

int ogg_configure(Private *p,struct SongDBEntry *e)
{


}

int
ogg_cleanup (Private * p)
{
    oggPrivate *ogg_priv = (oggPrivate *) p;

    if(p == NULL)
        return 0;

    free (ogg_priv);
    free (cfg);

    return 0;
}

int
ogg_is_our_file (Private * h, char *filename)
{
    char *ext;

    if (h == NULL || filename == NULL)
        return FALSE;

    ext = strrchr (filename, '.');
    if (ext)
	if (!strcmp (ext, ".ogg"))
	{
            return TRUE;
	}
    return FALSE;
}


int
ogg_get_tag (Private * h, char *path, struct SongDBEntry *e)
{ 
    oggPrivate *private = (oggPrivate *) h;
    OggVorbis_File *vobf;
    vorbis_comment *comments;

    double time;
    FILE *fp;

    {
        vobf=malloc(sizeof(OggVorbis_File));
        printf("get tag %s\n",path);
        fp=fopen("/home/beuving/test.ogg","rb");
        if(fp == NULL)
            return 0;
        
        memset(vobf,0,sizeof(OggVorbis_File));
        
        ov_open(fp,vobf, NULL, 0);

        comments=ov_comment(vobf,-1);
        
        e->title=strdup(*comments->user_comments);
        time=ov_time_total(vobf,-1);
        
       
        e->time=(long)(time*1000);
        
        free(vobf);
        fclose(fp);
    }
    return 1;

}



int
ogg_get_add_info (Private * h, char *filename, struct SongAddInfo *info)
{
    return 0;
}



int
ogg_load_file (Private * h, char *filename)
{
    oggPrivate *private = (oggPrivate *) h;
    int length=private->length;

    
    TRACE("ogg_load_file %s\n",filename);

    if (h == NULL || filename == NULL)
    {
        ERROR("Invalid argument");
        return 0;
    }
  
    if (private->fd && private->going)
    {
        ERROR("File already open");
        return 0;
    }

    if (ogg_is_our_file (h, filename) != TRUE)
    {
        ERROR("unknown file");
	return 0;
    }
    
    private->fd = fopen (filename, "rb");
    if (private->fd == NULL)
    {
        ERROR("Opening file");
	return 0;
    }
    
    if(ov_open(private->fd, &private->vf, NULL, 0) < 0) 
    {
        fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
    }

    
    fseek(private->fd,0,SEEK_END);
    private->size=ftell(private->fd);
    fseek(private->fd,0,SEEK_SET);

    if(length == 0)
    {
        printf("Illegal size");
#if 0
        fseek (private->fd, 0, SEEK_SET);
        scan_file (private->fd, &length, NULL);
        fseek (private->fd, 0, SEEK_SET);
#endif
    }


    length=(unsigned long)ov_pcm_total(&private->vf,-1);

  {
      char **ptr=ov_comment(&private->vf,-1)->user_comments;
      vorbis_info *vi=ov_info(&private->vf,-1);
      while(*ptr)
      {
          fprintf(stderr,"%s\n",*ptr);
          ++ptr;
      }
      fprintf(stderr,"\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
      fprintf(stderr,"\nDecoded length: %ld samples\n",
              (long)ov_pcm_total(&private->vf,-1));
      fprintf(stderr,"Encoded by: %s\n\n",ov_comment(&private->vf,-1)->vendor);
      fprintf(stderr,"Total file time %g\n",ov_time_total(&private->vf,-1));
  }
  

    private->length  = length;
    private->bitrate = 0;
    private->seek    = -1;
    private->eof     = 0;

    private->channels = 2;
    private->rate = 44100;

    if (ogg_if.output_open
        (private->ch_id,
         FMT_S16_NE, private->rate, private->channels, &private->max_bytes))
    {
	private->audio_error = TRUE;
	fclose (private->fd);
	private->fd = NULL;
        ERROR("Audio open");
	return 0;
    }

    private->going    = 1;
    private->position = 0;
    private->decode_thread=OSA_CreateThread(ogg_play_loop, (void *)private);
    return 1;
}

int
ogg_close_file (Private * h)
{
    oggPrivate *private = (oggPrivate *) h;

    TRACE("ogg_close_file");
    if( h == NULL)
    {
        ERROR("Invalid arguments");
        return 0;
    }
  

    if (private->going && private->fd != NULL)
    {
        DEBUG("Stopping thread");
	private->going = 0;
        OSA_RemoveThread(private->decode_thread);
	ogg_if.output_close (private->ch_id);
	fclose (private->fd);
	private->fd = NULL;

    }

    return 1;
}


void *
ogg_play_loop (void *param)
{
    oggPrivate *private = param;
    unsigned int input_length = 0, output_length = 0;
    int resolution = 16;
    char *pcmout;
    int current_section;
    
    int avgbitrate, bitrate, last_bitrate = 0, seek_skip = 0, last_error = 0;

    pcmout=malloc(2352*75);
    if(param == NULL || private->input_buffer == NULL)
        return (void *) 0;
  
    if(private->output_buffer == NULL)
        return (void *) 0;

#define OGGBUG 4096
    while(private->going)
    {
        while(!private->eof)
        {
            long ret=ov_read(&private->vf,pcmout,OGGBUG,0,2,1,&current_section);
            double time=ov_time_tell(&private->vf);
            time=time*100;
            if (ret == 0) 
            {
                /* EOF */
                private->eof=1;
                private->going=0;
            } 
            else if (ret < 0) 
            {
                /* error in the stream.  Not a problem, just reporting it in
                   case we (the app) cares.  In this case, we don't. */
            } 
            else 
            {
                while(ogg_if.output_buffer_free (private->ch_id) < (ret))
                {
                    SDL_Delay(10);
                }
                
                /* we don't bother dealing with sample rate changes, etc, but
                   you'll have to*/
                ogg_if.output_write (private->ch_id, pcmout,ret);
                private->position=(long)(time);
                private->position*=10;
            }
        }
    }
 
 
    return 0;
}


int
ogg_play (Private * h)
{
    oggPrivate *private = (oggPrivate *) h;

    if( h == NULL )
        return 0;
   
    ogg_if.output_pause (private->ch_id, 0);
    return 1;
}

int
ogg_pause (Private * h)
{
    oggPrivate *private = (oggPrivate *) h;
    if( h == NULL)
        return 0;
  
    ogg_if.output_pause (private->ch_id, 1);
    return 1;
}

int
ogg_seek (Private * h, long msecs)
{
    oggPrivate *private = (oggPrivate *) h;
   
    if( h == NULL)
        return 0;

    private->seek = (int) msecs;
    private->eof = 0;
    return 1;
}


long
ogg_get_time (Private * h)
{
    oggPrivate *private = (oggPrivate *) h;

    if(h == NULL)
        return 0;
  
    if (private->eof)
	return 0;

    if (!private->going)
	return 0;
    
    return private->position;
}



