
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

unsigned int CDDB_CalcID(TOC *g_toc,int cdtracks)
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

