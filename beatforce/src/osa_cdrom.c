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

void OSACDROM_Init()
{
    struct stat statstruct;
    int fd=0;
    static struct cdrom_tochdr hdr;
    static struct cdrom_tocentry entry[100];
    struct SongDBSubgroup *sg;
    int i,err,tracks;
    TOC toc[90];
    int id;
    
    /* Find out how many CD-ROM drives are connected to the system */
    printf("Drives available: %d\n", SDL_CDNumDrives());
    for ( i=0; i<SDL_CDNumDrives(); ++i ) 
    {
        printf("Drive %d:  \"%s\"\n", i, SDL_CDName(i));
    }


    if (stat("/dev/hdb", &statstruct)) 
    {
        fprintf(stderr, "cannot stat cd (%s)\n", "/dev/hdb");
        exit(1);
    }

    if(!S_ISCHR(statstruct.st_mode) &&
       !S_ISBLK(statstruct.st_mode))
    {
        printf("not a device\n");
        return ;
    }
    switch ((int) (statstruct.st_rdev >> 8L)) 
    {
    case SCSI_GENERIC_MAJOR:	/* generic */
        if (!S_ISCHR(statstruct.st_mode)) 
        {
            fprintf(stderr, " is not a char device\n");
            exit(1);
        }
        printf("SCSI device\n");
        break;
    case SCSI_CDROM_MAJOR:      /* scsi cd */
    default:
	if (!S_ISBLK(statstruct.st_mode)) 
        {
	    fprintf(stderr, " is not a block device\n");
	    exit(1);
	}
        printf("Cooked ioctl\n");
        break;
    }
    fd = open("/dev/hdb",O_RDONLY);
    if (fd < 0) 
    {
        fprintf(stderr, "while opening %s :", "/dev/hdd");
        perror("ioctl cdrom device open error: ");
        exit(1);
    }

    /* read toc */
    ioctl( fd, CDROMSTOP,  NULL );
    ioctl( fd, CDROMSTART, NULL );
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
    g_toc=toc;
    printf("Number of tracks %d\n",tracks);
    cdtracks=tracks-1;
    
    id=OSACDROM_CalcCDDBId();
    printf("SONGDB ID %x %d\n",id,id);
    
    DisplayToc();
    
    if(tracks > 0)
    {
        char trackname[255];
        SONGDB_AddSubgroup(MainGroup,"CDROM");
        sg=SONGDB_GetSubgroup();
        /* Go the the last added subgroup */
        while(sg->next)
            sg=sg->next;

        for(i=1;i<=tracks;i++)
        {
            sprintf(trackname,"/dev/hdd/track%02d.cdda",i);
            SONGDB_AddFileToSubgroup(sg,trackname);
        }

    }
#if 0
    {
        int s  = GetStartSector(1);
        int se = GetEndSector(1)+1;
        FILE *pcm;
        unsigned int *buffertje;
        
        buffertje=malloc(sizeof(unsigned int)*2352*75);
        pcm=fopen("calc.pcm","ab");
        
        while(s<se)
        {
            printf("s %d\n",s);
            printf("se %d\n",se);
            arg.addr.lba = s;
            arg.addr_format = CDROM_LBA;
            arg.nframes = 74;
            arg.buf = (unsigned char *) &buffertje[0];
            if(ioctl(fd, CDROMREADAUDIO, &arg) < 0)
            {
                perror("ioctl\n");
            }
            else
            {
                printf("Writing stream\n");
                fwrite(buffertje,1,2352*74,pcm);
            }
            s+=74;
        }
       
    }
#endif
}


void DisplayToc ( )
{
    unsigned i;
    unsigned long dw;
    unsigned mins;
    unsigned secnds;
    unsigned centi_secnds;	/* hundreds of a second */
    int count_audio_trks;


    /* get total time */
    dw = (unsigned long) g_toc[cdtracks].dwStartSector + 150;
    mins	       =       dw / ( 60*75 );
    secnds       =     ( dw % ( 60*75 ) ) / 75;
    centi_secnds = (4* ( dw %      75   ) +1 ) /3; /* convert from 1/75 to 1/100 */
    /* g_toc [i].bFlags contains two fields:
       bits 7-4 (ADR) : 0 no sub-q-channel information
       : 1 sub-q-channel contains current position
       : 2 sub-q-channel contains media catalog number
       : 3 sub-q-channel contains International Standard
       Recording Code ISRC
       : other values reserved
       bits 3-0 (Control) :
       bit 3 : when set indicates there are 4 audio channels else 2 channels
       bit 2 : when set indicates this is a data track else an audio track
       bit 1 : when set indicates digital copy is permitted else prohibited
       bit 0 : when set indicates pre-emphasis is present else not present
    */

    {
        unsigned ii;

        /* summary */
        count_audio_trks = 0;
        i = 0;
        while ( i < cdtracks ) {
            int from;

            from = g_toc [i].bTrack;
            while ( i < cdtracks && g_toc [i].bFlags == g_toc [i+1].bFlags ) i++;
            if (i >= cdtracks) i--;
      
            if (g_toc[i].bFlags & 4) {
                fputs( " DATAtrack recorded      copy-permitted tracktype\n" , stderr);
                fprintf(stderr, "     %2d-%2d %13.13s %14.14s      data\n",from,g_toc [i].bTrack,
			g_toc [i].bFlags & 1 ? "incremental" : "uninterrupted", /* how recorded */
			g_toc [i].bFlags & 2 ? "yes" : "no" /* copy-perm */
                    );
            } else { 
                fputs( "AUDIOtrack pre-emphasis  copy-permitted tracktype channels\n" , stderr);
                fprintf(stderr, "     %2d-%2d %12.12s  %14.14s     audio    %1c\n",from,g_toc [i].bTrack,
			g_toc [i].bFlags & 1 ? "yes" : "no", /* pre-emph */
			g_toc [i].bFlags & 2 ? "yes" : "no", /* copy-perm */
			g_toc [i].bFlags & 8 ? '4' : '2'
                    );
                count_audio_trks++;
            }
            i++;
        }
        fprintf ( stderr, 
                  "Table of Contents: total tracks:%u, (total time %u:%02u.%02u)\n",
                  cdtracks, mins, secnds, centi_secnds );

        for ( i = 0, ii = 0; i < cdtracks; i++ ) {
            if ( g_toc [i].bTrack <= 99 ) 
            {
                dw = (unsigned long) (g_toc[i+1].dwStartSector - g_toc[i].dwStartSector /* + 150 - 150 */);
                mins         =         dw / ( 60*75 );
                secnds       =       ( dw % ( 60*75 )) / 75;
                centi_secnds = ( 4 * ( dw %      75 ) + 1 ) / 3;
                if ( (g_toc [i].bFlags & CDROM_DATA_TRACK) != 0 ) 
                {
                    fprintf ( stderr, " %2u.[%2u:%02u.%02u]",
                              g_toc [i].bTrack,mins,secnds,centi_secnds );
                } 
                else 
                {
                    fprintf ( stderr, " %2u.(%2u:%02u.%02u)",
                              g_toc [i].bTrack,mins,secnds,centi_secnds );
                }
                ii++;
            }
            if ( ii % 5 == 0 )
                fputs( "\n", stderr );
            else
                fputc ( ',', stderr );
        }
        if ( (ii+1) % 5 != 0 )
            fputs( "\n", stderr );

        {
            fputs ("\nTable of Contents: starting sectors\n", stderr);
            for ( i = 0; i < cdtracks; i++ ) {
                fprintf ( stderr, " %2u.(%8u)", g_toc [i].bTrack, g_toc[i].dwStartSector
#ifdef DEBUG_CDDB
                          +150
#endif
                    );
                if ( (i+1) % 5 == 0 )
                    fputs( "\n", stderr );
                else
                    fputc ( ',', stderr );
            }
            fprintf ( stderr, " lead-out(%8u)", g_toc[i].dwStartSector);
            fputs ("\n", stderr);
        }
        
        
    }
}
