#if 0
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/major.h>
#include <linux/cdrom.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#endif

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

void OSACDROM_Init()
{
#if 0
    struct stat statstruct;
    int fd=0;
    static struct cdrom_tochdr hdr;
    static struct cdrom_tocentry entry[100];
    static struct cdrom_read_audio arg;
    int i,err,tracks;
    TOC toc[90];
    int id;

    if (stat("/dev/hdd", &statstruct)) 
    {
        fprintf(stderr, "cannot stat cd (%s)\n", "/dev/hdd");
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
    fd = open("/dev/hdd",O_RDONLY);
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
#endif
}
