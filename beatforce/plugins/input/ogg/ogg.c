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

#define MODULE_ID OGG
#include "debug.h"

void *ogg_play_loop (void *);

struct config *cfg;

char *str_mpeg1_l1=NULL;
char *str_mpeg1_l2=NULL;
char *str_mpeg1_l3=NULL;
char *str_mpeg25_l3=NULL;

#define OGG_INPUT_BUFFER_SIZE	(40000*2)
#define OGG_OUTPUT_BUFFER_SIZE	((575+1152)*4*2)
#define VORBIS_READ_BUFFER_SIZE 4096

InputPlugin ogg_ip = {
    NULL, 						/* handle, BeatForce fills it */
    NULL, 						/* filename, BeatForce filled */
    "ogg input plugin",         /* Description */

    ogg_init,
    ogg_configure,
    NULL,
    ogg_is_our_file,

    ogg_get_tag,
    ogg_get_add_info,
    NULL,                       /* write_tag */
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

    ogg_cleanup,
    ogg_set_interface
};

InputPlugin *
get_input_info ()
{
     return &ogg_ip;
}

int
ogg_set_interface(Private *p,InputInterface *api)
{   
    /* ToDo check for NULL */
    oggPrivate *ogg_priv = (oggPrivate *) p;
    
    ogg_priv->ogg_if.output_open        = api->output_open;
    ogg_priv->ogg_if.output_write       = api->output_write;
    ogg_priv->ogg_if.output_pause       = api->output_pause;
    ogg_priv->ogg_if.output_buffer_free = api->output_buffer_free;
    ogg_priv->ogg_if.output_get_time    = api->output_get_time;
    ogg_priv->ogg_if.output_close       = api->output_close;
    ogg_priv->ogg_if.input_eof          = api->input_eof;
    return 1;
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
    if (NULL == ogg_priv)
    {
        ERROR("Not enough memory");
	return 0;
    }
    memset (ogg_priv, 0, sizeof (oggPrivate));

    ogg_priv->input_buffer  = malloc (OGG_INPUT_BUFFER_SIZE);
    ogg_priv->output_buffer = malloc (OGG_OUTPUT_BUFFER_SIZE);
    if (!ogg_priv->input_buffer || !ogg_priv->output_buffer)
    {
        ERROR("Not enough memory");
	return (ogg_cleanup((oggPrivate *) ogg_priv));
    }
    memset (ogg_priv->input_buffer, 0, OGG_INPUT_BUFFER_SIZE);
    memset (ogg_priv->output_buffer, 0, OGG_OUTPUT_BUFFER_SIZE);

    ogg_priv->output_size = OGG_OUTPUT_BUFFER_SIZE;
    ogg_priv->input_size = OGG_INPUT_BUFFER_SIZE;

    ogg_priv->vorbis_buffer = malloc (VORBIS_READ_BUFFER_SIZE);
    if (ogg_priv->vorbis_buffer == NULL)
    {
        ERROR("Not enough memory");
	return (ogg_cleanup((oggPrivate *) ogg_priv));
    }
    memset (ogg_priv->vorbis_buffer, 0, VORBIS_READ_BUFFER_SIZE);

    cfg = malloc (sizeof (struct config));
    if (cfg == NULL)
    {
        ERROR("Not enough memory");
	return (ogg_cleanup((oggPrivate *) ogg_priv));
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

    if (NULL != ogg_priv)
    {
        if (NULL != ogg_priv->fd)
            fclose(ogg_priv->fd);

        if (NULL != ogg_priv->input_buffer)
	    free (ogg_priv->input_buffer);
        if (NULL != ogg_priv->output_buffer)
	    free (ogg_priv->output_buffer);
        if (NULL != ogg_priv->vorbis_buffer)
	    free (ogg_priv->vorbis_buffer);
         free (ogg_priv);
        ogg_priv = NULL;
    }	
    if (NULL != cfg)
    {
        free (cfg);
        cfg = NULL;
    }
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
    char *title, *t;
    double time;
    FILE *fp;

    {
        vobf=malloc(sizeof(OggVorbis_File));
        fp=fopen(path,"rb");
        if(fp == NULL)
            return 0;
        
        memset(vobf,0,sizeof(OggVorbis_File));
        
        ov_open(fp,vobf, NULL, 0);

        comments=ov_comment(vobf,-1);
        
        /* ToDo scan through all strings */        
        title=strdup(*comments->user_comments);
       if (NULL != title)
        {
            /* strip "title=" title */
            t = strstr(title, "title=");
            if (NULL != t)
            {
                t = &t[6];
                e->title=strdup(t);
            }
            /* didn't find title, maybe something of interest? */
            else
                e->title = strdup(title);
            free(title);
        } 
       
        time=ov_time_total(vobf,-1);
        /* ToDo oggs never reach eof? */       
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
    fseek(private->fd,0,SEEK_END);
    private->size=ftell(private->fd);
    fseek(private->fd,0,SEEK_SET);

    if (ov_open(private->fd, &private->vf, NULL, 0) < 0) 
    {
        fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
    }

    length=(unsigned long)ov_pcm_total(&private->vf,-1);

#if 0
      vorbis_info *vi=ov_info(&private->vf,-1);
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
#endif

    private->length  = length;
    private->bitrate = 0;
    private->seek    = -1;
    private->eof     = 0;

    private->channels = 2;;
    private->rate = 44100;

    if (!private->ogg_if.output_open(private->ch_id,FMT_S16_NE, private->rate, private->channels, 
                                     &private->max_bytes))
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
        private->ogg_if.output_close (private->ch_id);
        fclose (private->fd);
        private->fd = NULL;
    }

    return 1;
}


void *
ogg_play_loop (void *param)
{
    oggPrivate *private = param;
    double ov_pos;
    int current_section;
    long ret;
    double time;
    
    if(param == NULL || private->input_buffer == NULL)
        return (void *) 0;
  
    if(private->output_buffer == NULL)
        return (void *) 0;

    while(!private->eof && private->going)
    {
        if (private->seek != -1 && private->length >= 0)
        {
            if (private->seek < 0)
            {
                ov_pos = (double) (private->length * -private->seek) / 1000;
                private->position = (int) (ov_pos * 1000);
            }
            else
            {
                ov_pos = (double) private->seek / 1000;
                private->position = private->seek;
            }
            ov_time_seek(&private->vf, ov_pos);
            private->seek = -1;
        }
        ret=ov_read(&private->vf,
                    private->vorbis_buffer,
                    VORBIS_READ_BUFFER_SIZE,
                    0, 2, 1, &current_section);
        time=ov_time_tell(&private->vf);
        time=time*100;
        if (ret == 0)   /* EOF */
        {
            private->eof = 1;
            private->going = 0;
            break;
        } 
        else 
        {
            while(private->ogg_if.output_buffer_free (private->ch_id) < (ret))
            {
                if (!private->going)
                    break; 
                SDL_Delay(10);
            }
            
                /* we don't bother dealing with sample rate changes, etc, but
                you'll have to*/
            private->ogg_if.output_write (private->ch_id,
                                    private->vorbis_buffer,ret);
            private->position=(long)(time);
            private->position*=10;
        }
    }
    if (private->eof)           /* we never get here? */
    {
        private->ogg_if.input_eof (private->ch_id);
        SDL_Delay(30);
    }
    return 0;
}


int
ogg_play (Private * h)
{
    oggPrivate *private = (oggPrivate *) h;

    if( h == NULL )
        return 0;
   
    private->ogg_if.output_pause (private->ch_id, 0);
    return 1;
}

int
ogg_pause (Private * h)
{
    oggPrivate *private = (oggPrivate *) h;
    if( h == NULL)
        return 0;
  
    private->ogg_if.output_pause (private->ch_id, 1);
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



