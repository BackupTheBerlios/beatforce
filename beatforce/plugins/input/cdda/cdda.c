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

#include "types.h"
#include "input_plugin.h"
#include "err.h"
#include "osa.h"
#include "cdda.h"



#define MP3_INPUT_BUFFER_SIZE	(40000*2)
#define MP3_OUTPUT_BUFFER_SIZE	((575+1152)*4*2)

int cdda_play_loop (void *);

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

void readtoc();

typedef struct trackinfo
{
    char *name;
    char *title;
}trackinfo;

struct trackinfo ti[100];

InputPlugin cdda_ip = {
    NULL,                       /* handle, BeatForce fills it */
    NULL,                       /* filename, BeatForce filled */
    "cdda input plugin ",	/* Description */

    CDDA_Init,
    NULL,
    NULL,
    cdda_is_our_file,

    cdda_get_tag,
    cdda_get_add_info,
    NULL,                        /* write_tag */
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

    return &cdda_ip;
}


/*ch_id is equal to player_nr */
int
CDDA_Init(Private ** p, int ch_id)
{
    cddaPrivate *cdda_priv;


    if(p == NULL)
        return 0;

    cdda_priv = malloc (sizeof (cddaPrivate));
    if (cdda_priv == NULL)
	return ERROR_NO_MEMORY;
    memset (cdda_priv, 0, sizeof (cddaPrivate));

    cdda_priv->input_buffer  = malloc (MP3_INPUT_BUFFER_SIZE);
    cdda_priv->output_buffer = malloc (MP3_OUTPUT_BUFFER_SIZE);
    if (!cdda_priv->input_buffer || !cdda_priv->output_buffer)
    {
	free (cdda_priv->input_buffer);
	free (cdda_priv->output_buffer);
	return ERROR_NO_MEMORY;
    }
    memset (cdda_priv->input_buffer, 0, MP3_INPUT_BUFFER_SIZE);
    memset (cdda_priv->output_buffer, 0, MP3_OUTPUT_BUFFER_SIZE);

    cdda_priv->output_size = MP3_OUTPUT_BUFFER_SIZE;
    cdda_priv->input_size = MP3_INPUT_BUFFER_SIZE;
    cdda_priv->fd = -1;
    cdda_priv->cachedid=0;

    cfg = malloc (sizeof (struct config));
    
    if (cfg == NULL)
	return 0;
    memset (cfg, 0, sizeof (struct config));
    cfg->lengthcalc = 0;

    cdda_priv->ch_id = ch_id;

    //to be done
    cdda_priv->going     =1 ;
//    cdda_priv->decode_thread=OSA_CreateThread(cdda_play_loop, (void *)cdda_priv);    
    *p = (Private *) cdda_priv;

    return 0;
}

int cdda_configure(Private *p,struct SongDBEntry *e)
{


}

int
cdda_cleanup (Private * p)
{
    cddaPrivate *cdda_priv = (cddaPrivate *) p;

    if(p == NULL)
        return ERROR_INVALID_ARG;

 
    free (cdda_priv);
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
    cddaPrivate *cdda_priv = (cddaPrivate *) h;    
    int t,tt;
    char tm[20];
    char *s;
    unsigned long dw;
    int mins;
    int secnds;
    int centi_secnds;
    int i=0;
    char *drive;
    char *set;

    drive=strdup(path);
    set=strrchr(drive,'/');
    *set=0;    

    set+=8;
    *set=0;
    set-=2;
    i = atoi(set);
    i--;
    readtoc(drive);
    dw = (unsigned long) (ourtoc[i+1].dwStartSector - ourtoc[i].dwStartSector /* + 150 - 150 */);


    mins         =         dw / ( 60*75 );
    secnds       =       ( dw % ( 60*75 )) / 75;
    centi_secnds = ( 4 * ( dw %      75 ) + 1 ) / 3;

    e->time=mins*60*1000+secnds*1000+centi_secnds*10;
#if 0
    {
        int totaltime;
        char temp[255];

        totaltime=(g_toc[cdtracks].dwStartSector+150) /(75);
        printf("time %d\n",totaltime);
	sprintf(temp,
		"GET /~cddb/cddb.cgi?cmd=cddb+query+%08x+%d+%s+%d%s&proto=%d HTTP/1.0\r\n\r\n",
		id, 
                cdtracks,
		cddb_generate_offset_string(),
                totaltime,
		"&hello=nobody+localhost+beatforce+0.0.1",
                2);
        
        printf("\nstrind %s\n\n",temp);
#endif
        {
            
            char temp[255];
            char buffer[256];
            char what[255];
            int sock;
            char *getstr;

            if(cdda_priv->cachedid == 0)
            {

                cdda_priv->cachedid = CDDB_CalcID(ourtoc,ourtracks);
                sprintf(temp,
                        "GET /~cddb/cddb.cgi?cmd=cddb+read+%s+%08x%s&proto=%d HTTP/1.0\r\n\r\n",
                        "misc", CDDB_CalcID(ourtoc,ourtracks),"&hello=nobody+localhost+beatforce+0.0.1",2);
                
                sock=HTTP_OpenConnection("www.freedb.org",80);
                if(sock <= 0)
                    printf("Could not connect\n");
                
                getstr=strdup(temp);
                
                send(sock, getstr, strlen(getstr),0);
                free(getstr);
                while(HTTP_ReadFirstLine(sock, buffer, 256) > 0)
                {
                    if(strstr(buffer,"DTITLE"))
                    {
                        getstr=strrchr(buffer,'/');
                        if(getstr)
                            *getstr=0;
                        getstr=strrchr(buffer,'=');
                        if(getstr)
                        {
                            int k;
                            for(k=0;k<ourtracks;k++)
                            {
                                ti[k].name=strdup(getstr+1);
                            }
                            e->artist=strdup(getstr+1);
                        }
                    }
                    
                    if(strstr(buffer,"TTITLE"))
                    {
                        char *b;
                        int getal;
                        b=strstr(buffer,"=");
                        *b=0;
                        getal=atoi(getstr);
                        
                        b++;
                        if(getal < 0 || getal > 100)
                            break;
                        else
                            ti[getal].title=strdup(b);
                        if(getal == i)
                        {
                            e->title=strdup(b);
                        }
                    }
                    
                }
                HTTP_CloseConnection(sock);
            }
            else
            {
                if(i<ourtracks)
                {
                    if(ti[i].name)
                        e->artist=strdup(ti[i].name);
                    if(ti[i].title)
                        e->title=strdup(ti[i].title);
                }

            }
        }

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
    cddaPrivate *private = (cddaPrivate *) h;
    int length=private->length;
    char *drive;
    char *set;


    
    if (h == NULL || filename == NULL)
        return ERROR_INVALID_ARG;
  
    if (cdda_is_our_file (h, filename) != TRUE)
    {

	return ERROR_UNKNOWN_FILE;
    }
    
    drive=strdup(filename);
    set=strrchr(drive,'/');
    *set=0;

    set+=8;
    *set=0;
    set-=2;


    private->track = atoi(set);

    private->fd    = open(drive,O_RDONLY);
    
    if (private->fd < 0)
    {

    }
    else
    {

        private->going   = 1;
        private->rate    = 44100;
        private->channels= 2;

        
        
        if (cdda_if.output_open
            (private->ch_id,
             FMT_S16_NE, private->rate, private->channels, &private->max_bytes))
        {
            close (private->fd);
            private->fd = -1;

            return 0;
        }
        private->decode_thread=OSA_CreateThread(cdda_play_loop, (void *)private);
    }
    return 1;
}

int
cdda_close_file (Private * h)
{
    cddaPrivate *private = (cddaPrivate *) h;

    if( h == NULL )
        return 0;
  
    if (private->going && private->fd >= 0 )
    {
	private->going = 0;
        OSA_RemoveThread(private->decode_thread);
	cdda_if.output_close (private->ch_id);
	private->fd = -1;
        private->position=0;
	return 1;
    }
    return 0;
}


int
cdda_play_loop (void *param)
{
    cddaPrivate *private = param;

    unsigned int input_length = 0, output_length = 0;
    int resolution = 16;
    unsigned int *buffer;
    int lba=ourtoc[private->track-1].dwStartSector;
    int filedes=private->fd;

    buffer=malloc(sizeof(unsigned int) * 2352 * 75);
   
    while (private->going)
    {

        if (private->seek != -1 && private->length >= 0)
        {
            int new_position;
            long pos;

            if (private->seek < 0)
                new_position = (double) private->length * -private->seek / 1000;
            else
                new_position = private->seek;

            private->position = new_position;
            private->seek = -1;
                    
            lba = ourtoc[private->track-1].dwStartSector;
            lba += private->position/67;
                    
        }

        arg.addr.lba = lba;
        arg.addr_format = CDROM_LBA;
        arg.nframes = 5;
        arg.buf = (unsigned char *) &buffer[0];

        if(filedes >= 0)
        {
            if(ioctl(private->fd, CDROMREADAUDIO, &arg) < 0)
            {

            }

            while(cdda_if.output_buffer_free (private->ch_id) < (2352*5) && private->going)
                SDL_Delay(5);

            {
                cdda_if.output_write (private->ch_id, buffer,2352*5);
                lba+=5;
                private->position+=67;
            }
        }
        if(lba > ourtoc[private->track].dwStartSector)
        {
            cdda_if.input_eof (private->ch_id);
            break;
        }
       
    }
    free(buffer);

    return 0;
}


int
cdda_play (Private * h)
{
    cddaPrivate *private = (cddaPrivate *) h;

    if( h == NULL)
        return 0;
   
    cdda_if.output_pause (private->ch_id, 0);
    return 1;
}

int
cdda_pause (Private * h)
{
    cddaPrivate *private = (cddaPrivate *) h;
    if( h == NULL )
        return ERROR_INVALID_ARG;
  
    cdda_if.output_pause (private->ch_id, 1);
    return 1;
}

int
cdda_seek (Private * h, long msecs)
{
    cddaPrivate *private = (cddaPrivate *) h;
   
    if( h == NULL )
        return ERROR_INVALID_ARG;

    private->seek = (int) msecs;
    private->eof = FALSE;
    return 0;
}


long
cdda_get_time (Private * h)
{
    cddaPrivate *private = (cddaPrivate *) h;

    if( h == NULL)
        return ERROR_INVALID_ARG;
  
    if (private->eof)
        return ERROR_EOF;
    if (!private->going)
        return ERROR_NO_FILE_LOADED;
    
    return private->position;
}








void readtoc(char *dev)
{
    int fd,i;
    int tracks;
    
    fd = open(dev,O_RDONLY);

    if (fd < 0) 
    {

    }

    ioctl( fd, CDROMREADTOCHDR, &hdr );

    for ( i = 0; i < hdr.cdth_trk1; i++ ) 
    {
        entry[i].cdte_track = 1+i;
        entry[i].cdte_format = CDROM_LBA;
        ioctl(fd, CDROMREADTOCENTRY, &entry[i] );
    }
    entry[i].cdte_track = CDROM_LEADOUT;
    entry[i].cdte_format = CDROM_LBA;
    ioctl( fd, CDROMREADTOCENTRY, &entry[i] );
    tracks = hdr.cdth_trk1+1;
    for (i = 0; i < tracks; i++) 
    {
        toc[i].bFlags = (entry[i].cdte_adr << 4) | (entry[i].cdte_ctrl & 0x0f);
        toc[i].bTrack = entry[i].cdte_track;
        toc[i].dwStartSector = entry[i].cdte_addr.lba;
    }
    ourtoc=toc;
    ourtracks=tracks-1;

}
