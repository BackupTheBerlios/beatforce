#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/major.h>
#include <linux/cdrom.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <SDL/SDL.h>

#include "songdb.h"

typedef struct TOC {	/* structure of table of contents (cdrom) */
    unsigned char reserved1;
    unsigned char bFlags;
    unsigned char bTrack;
    unsigned char reserved2;
    unsigned int dwStartSector;
    unsigned char ISRC[15];
} TOC;

TOC *g_toc;
int cdtracks;

static char * cddb_generate_offset_string()
{
	char *buffer;
	int i;

	buffer = malloc((cdtracks-1) * 7 + 1);

        
	sprintf(buffer, "%d", g_toc[0].dwStartSector+150);

	for (i = 1; i < cdtracks; i++)
		sprintf(buffer, "%s+%d", buffer, g_toc[i].dwStartSector+150);

	return buffer;
}

int OSACDROM_TestDrive(char *dev);
void DisplayToc ( );
extern SongDBGroup *MainGroup;



long GetStartSector ( p_track )
    unsigned long p_track;
{
    unsigned long i;

    for (i = 0; i < cdtracks; i++) {
        if (g_toc [i].bTrack == p_track) {
            unsigned long dw = g_toc [i].dwStartSector;
            if ((g_toc [i].bFlags & CDROM_DATA_TRACK) != 0)
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

    for ( i = 1; i <= cdtracks; i++ ) {
        if ( g_toc [i-1].bTrack == p_track ) {
            unsigned long dw = g_toc [i].dwStartSector;
            return dw-1;
        }
    }

    return -1;
}

int OSACDROM_Init()
{
    int i;
    
    /* Find out how many CD-ROM drives are connected to the system */
    for ( i=0; i<SDL_CDNumDrives(); ++i ) 
    {
        OSACDROM_TestDrive((char*)SDL_CDName(i));
    }
    return 1;
}

int OSACDROM_TestDrive(char *dev)
{
    struct stat statstruct;
    int fd=0;
    static struct cdrom_tochdr hdr;
    static struct cdrom_tocentry entry[100];
    struct SongDBSubgroup *sg;
    int i=0,j,err,tracks;
    TOC toc[90];
    int id;
    

    if (stat(dev, &statstruct)) 
    {
        return 0;
    }

    if(!S_ISCHR(statstruct.st_mode) &&
       !S_ISBLK(statstruct.st_mode))
    {
        return 0;
    }

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
            fprintf(stderr, "%s is not a block device\n",SDL_CDName(i));
            return 0;
        }
        break;
    }
    fd = open(dev,O_RDONLY);
    if (fd < 0) 
    {
        return 0;
    }

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
    entry[j].cdte_track = CDROM_LEADOUT;
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
    g_toc=toc;
    cdtracks=tracks-1;
    
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

        sprintf(temp,
        "GET /~cddb/cddb.cgi?cmd=cddb+read+%s+%08x%s&proto=%d HTTP/1.0\r\n\r\n",
                "misc", id,"&hello=nobody+localhost+beatforce+0.0.1",2);
        printf("\nstrind %s\n\n",temp);


    }
#endif

    if(tracks > 0)
    {
        char trackname[255];
        SONGDB_AddSubgroup(MainGroup,dev);

        sg=SONGDB_GetSubgroup();
        /* Go the the last added subgroup */
        while(sg->next)
            sg=sg->next;

        SONGDB_SubgroupSetVolatile(sg);
        
        for(i=1;i<=cdtracks;i++)
        {
            sprintf(trackname,"%s/track%02d.cdda",dev,i);
            SONGDB_AddFileToSubgroup(sg,trackname);
        }

    }

    return 1;
}






