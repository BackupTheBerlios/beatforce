#include <sys/types.h>
#include <sys/stat.h>
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

int OSACDROM_TestDrive(char *dev);
void DisplayToc ( );
extern SongDBGroup *MainGroup;
static struct cdrom_read_audio arg;
static int cddb_sum(n)
    int n;
{
    int ret;

    for (ret = 0; n > 0; n /= 10) {
        ret += (n % 10);
    }

    return ret;
}
static unsigned int OSACDROM_CalcCDDBId()
{
    unsigned int i;
    unsigned int t = 0;
    unsigned int n = 0;

    for (i = 0; i < cdtracks; i++) 
    {
        n += cddb_sum(g_toc[i].dwStartSector/75 + 2);
    }
    
    t = g_toc[i].dwStartSector/75 - g_toc[0].dwStartSector/75;

    return (n % 0xff) << 24 | (t << 8) | cdtracks;
}

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
//    printf("No Of drives %d\n",SDL_CDNumDrives());
    for ( i=0; i<SDL_CDNumDrives(); ++i ) 
    {
        OSACDROM_TestDrive(SDL_CDName(i));
    

    }
}

int OSACDROM_TestDrive(char *dev)
{
    struct stat statstruct;
    int fd=0;
    static struct cdrom_tochdr hdr;
    static struct cdrom_tocentry entry[100];
    struct SongDBSubgroup *sg;
    int i,j,err,tracks;
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
    
    id=OSACDROM_CalcCDDBId();
    
    
    if(tracks > 0)
    {
        char trackname[255];
        SONGDB_AddSubgroup(MainGroup,dev);

        sg=SONGDB_GetSubgroup();
        /* Go the the last added subgroup */
        while(sg->next)
            sg=sg->next;

        SONGDB_SubgroupSetVolatile(sg);
        for(i=1;i<=tracks;i++)
        {
            sprintf(trackname,"%s/track%02d.cdda",dev,i);
            SONGDB_AddFileToSubgroup(sg,trackname);
        }

    }
}

