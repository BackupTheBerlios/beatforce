/*
  Beatforce/ Operating System Abstraction layer

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003 John Beuving (john.beuving@home.nl)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/major.h>
#include <linux/cdrom.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include <SDL/SDL.h>

#include "songdb.h"
#include "osa_cdrom.h"





#if 0
static char * cddb_generate_offset_string()
{
	char *buffer;
	int i;

	buffer = (char*)malloc((cdtracks-1) * 7 + 1);

        
	sprintf(buffer, "%d", g_toc[0].dwStartSector+150);

	for (i = 1; i < cdtracks; i++)
		sprintf(buffer, "%s+%d", buffer, g_toc[i].dwStartSector+150);

	return buffer;
}
#endif

#if 0
long GetStartSector (unsigned long p_track)
{
    unsigned long i;

    for (i = 0; i < AudioDiscs->cdtracks; i++) 
    {
        if (AudioDiscs->g_toc [i].bTrack == p_track) 
        {
            unsigned long dw = AudioDiscs->g_toc [i].dwStartSector;
            if ((AudioDiscs->g_toc [i].bFlags & CDROM_DATA_TRACK) != 0)
                return -1;
            return dw;
        }
    }

    return -1;
}

long GetEndSector ( p_track )
    unsigned long p_track;
{
    unsigned long i;

    for ( i = 1; i <= AudioDiscs->cdtracks; i++ ) 
    {
        if ( AudioDiscs->g_toc [i-1].bTrack == p_track ) 
        {
            unsigned long dw = AudioDiscs->g_toc [i].dwStartSector;
            return dw-1;
        }
    }

    return -1;
}
#endif

int OSACDROM_Init()
{
#if 0
    int count,i;

    count=SDL_CDNumDrives();
    AudioDiscs=calloc(count,sizeof(CDDrives));
    memset(AudioDiscs,0,count*sizeof(CDDrives));
    
    for(i=0;i<count;i++)
        AudioDiscs[i].path=strdup(SDL_CDName(i));
#endif
    return 1;
}

int OSACDROM_CheckForDiscs()
{
#if 0
    int i;
    /* Find out how many CD-ROM drives are connected to the system */
    for ( i=0; i<SDL_CDNumDrives(); ++i ) 
        OSACDROM_TestDrive(&AudioDiscs[i]);

    return 1;
#endif
}

int OSACDROM_NumberOfDrives()
{
    return SDL_CDNumDrives();
}

int OSACDROM_GetTOC(char *drive)
{


}

int OSACDOM_GetNrOfTracks(char *drive)
{

}

int OSACDROM_TestDrive()
{
#if 0
    struct stat statstruct;
    int fd=0;
    static struct cdrom_tochdr hdr;
    static struct cdrom_tocentry entry[100];
    struct SongDBSubgroup *sg;
    int i=0,j,err,tracks;
    TOC toc[90];
    
    if(ToTest == NULL)
        return 0;

    /* Get te statistics of the device "/dev/??" */
    if (stat(ToTest->path, &statstruct)) 
        return 0;

    /* The drive has to be a character or block device */
    if(!S_ISCHR(statstruct.st_mode) && !S_ISBLK(statstruct.st_mode))
        return 0;
 
    switch ((int) (statstruct.st_rdev >> 8L)) 
    {
    case SCSI_GENERIC_MAJOR:	/* generic */
        if (!S_ISCHR(statstruct.st_mode)) 
        {
            fprintf(stderr, " is not a char device\n");
            exit(1);
        }
        break;
    case SCSI_CDROM_MAJOR:      /* scsi cd */
    default:
        if (!S_ISBLK(statstruct.st_mode)) 
        {
            fprintf(stderr, "%s is not a block device\n",ToTest->path);
            return 0;
        }
        break;
    }
    fd = open(ToTest->path,O_RDONLY);
    if (fd < 0) 
         return 0;
 
    /* read toc */
    ioctl( fd, CDROMSTOP,  NULL );
    ioctl( fd, CDROMSTART, NULL );
    ioctl( fd, CDROMREADTOCHDR, &hdr );
        
    for ( j = 0; j < hdr.cdth_trk1; j++ ) 
    {
        entry[j].cdte_track = 1+j;
        entry[j].cdte_format = CDROM_LBA;
        err = ioctl(fd, CDROMREADTOCENTRY, &entry[j] );
        if ( err != 0 ) 
        {
            /* error handling */
//            fprintf( stderr, "can't get TocEntry #%d (error %d).\n", j+1, err );
            return 0;
        }
    }
    entry[j].cdte_track  = CDROM_LEADOUT;
    entry[j].cdte_format = CDROM_LBA;
    err = ioctl( fd, CDROMREADTOCENTRY, &entry[j] );
    if ( err != 0 ) 
    {
        /* error handling */
//        fprintf( stderr, "can't get TocEntry LEADOUT (error %d).\n", err );
        return 0;
    }
    tracks = hdr.cdth_trk1+1;
    for (j = 0; j < tracks; j++) 
    {
        toc[j].bFlags = (entry[j].cdte_adr << 4) | (entry[j].cdte_ctrl & 0x0f);
        toc[j].bTrack = entry[j].cdte_track;
        toc[j].dwStartSector = entry[j].cdte_addr.lba;
    }
    ToTest->g_toc=toc;
    ToTest->cdtracks=tracks-1;
    
#if 0
    {
        int totaltime;
        char temp[255];

        totaltime=(ToTest->g_toc[ToTest->cdtracks].dwStartSector+150) /(75);
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

        sprintf(temp,
        "GET /~cddb/cddb.cgi?cmd=cddb+read+%s+%08x%s&proto=%d HTTP/1.0\r\n\r\n",
                "misc", id,"&hello=nobody+localhost+beatforce+0.2.0",2);
        printf("\nstrind %s\n\n",temp);


    }
#endif

    if(tracks > 0)
    {
        char trackname[255];
        SONGDB_AddSubgroup(MainGroup,ToTest->path);

        /* Get the list of sibgroups */
        sg=SONGDB_GetSubgroupList();

        /* Go the the last added subgroup */
        while(sg->next)
            sg=sg->next;

        /* Tell songdb it is removable storage */
        SONGDB_SubgroupSetVolatile(sg);
        
        for(i=1;i<=ToTest->cdtracks;i++)
        {
            sprintf(trackname,"%s/track%02d.cdda",ToTest->path,i);
            SONGDB_AddFileToSubgroup(sg,trackname);
        }

    }
#endif
    return 1;
}






