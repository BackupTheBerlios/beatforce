

#include <malloc.h>
#include <string.h>

#include "audio_output.h"
#include "input.h"
#include "plugin.h"
#include "mixer.h"

#define MODULE_ID AUDIO_OUTPUT
#include "debug.h"

AudioConfig *audiocfg;
extern BeatforceConfig *cfgfile;

/* test how much dat can be written in the channel's ringbuffer */
long AUDIOCHANNEL_BufferFree(int c)
{
    struct OutChannel *Channel;
    long free = -1;
    
    TRACE("AUDIOOUTPUT_BufferFree");

    Channel=AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;



    free = rb_free (Channel->rb);

    return free;
}

int AUDIOCHANNEL_Close(int c)
{
    struct OutChannel *Channel;

    TRACE("AUDIOCHANNEL_Close");
    Channel=AUDIOOUTPUT_GetChannelByID(c);


    if (!Channel->Open)
        return 0;
    Channel->Paused = 1;
    Channel->Open = 0;
    Channel->aformat = FMT_UNKNOWN;
    Channel->n_ch = 0;
    Channel->rate = 0;
    free (Channel->buffer2);
    Channel->buffer2 = NULL;
    Channel->buffer2_size = 0;
    //todo g_timer_stop (Channel->last_beat);
#if (ENABLE_CRUDE_BPMCOUNT >= 1)
    printf ("INFORMAL: BPMCOUNT: prescale was %ld\n", Channel->bpm_prescale);
#endif
    /* empty ring buffer */
//    rb_clear (Channel->rb);
    memset (Channel->buffer, 0, OUTPUT_BUFFER_SIZE (audiocfg));

    return 0;
}

long AUDIOCHANNEL_GetTimePlayed(int c)
{
    struct OutChannel *Channel;
    double time;

    TRACE("AUDIOCHANNEL_GetTimePlayed");
    Channel = AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;
    


    time = ((double) Channel->bytes_written) / (2 * Channel->rate * Channel->n_ch);
    time = (int) (time * 1000);
    return (long) time;
}

int AUDIOCHANNEL_GetVolume(int c, float *db)
{
    struct OutChannel *Channel;

    TRACE("AUDIOCHANNEL_GetVolume");
    Channel = AUDIOOUTPUT_GetChannelByID(c);
    if(Channel == NULL)
        return 0;

    *db = Channel->fader_dB;
    return 0;
}

int AUDIOCHANNEL_GetVolumeLevel(int channel,int *left,int *right)
{
    struct OutChannel *Channel;

    TRACE("AUDIOCHANNEL_GetVolumeLevel");
    Channel = AUDIOOUTPUT_GetChannelByID(channel);

    if(Channel == NULL)
        return 0;

    if(Channel->Paused)
    {
        *left=0;
        *right=0;
    }
    else
    {
        *left  = Channel->volumeleft;
        *right = Channel->volumeright;
    }
    return 1;
}

int AUDIOCHANNEL_Mute (int c, int mute)
{
    struct OutChannel *Channel;

    TRACE("AUDIOCHANNEL_Mute");
    Channel = AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;

    Channel->Mute = (int) (mute != 0);
    return 0;
}

struct OutChannel *AUDIOCHANNEL_New()
{
    struct OutChannel *channel;
    
    TRACE("AUDIOCHANNEL_New");

    channel = malloc (OUT_CHANNEL_SIZE);
    memset (channel, 0, OUT_CHANNEL_SIZE);
    

    channel->buffer = malloc (OUTPUT_BUFFER_SIZE (audiocfg) * 2);
    if(channel->buffer == NULL)
        return 0;
    memset (channel->buffer, 0, OUTPUT_BUFFER_SIZE (audiocfg) * 2);

    channel->id = 0; /* Will be set later upon registration */    
#if 0
    channel->buffer2 = malloc (OUTPUT_BUFFER_SIZE (audiocfg));
    if(channel->buffer2 == NULL)
        return 0;
    memset (channel->buffer2, 0, OUTPUT_BUFFER_SIZE (audiocfg));
#endif
    
    rb_init (&channel->rb, OUTPUT_RING_SIZE (audiocfg));
    
    channel->last_beat = NULL;//g_timer_new ();
    
    channel->bpm_prescale = 7000;
    channel->speed = 1.0;   /* Normal playback speed */
    channel->fader_dB  = MIXER_DEFAULT_dB;

    return channel;
}

/* interface to input plugin */
int AUDIOCHANNEL_Open(int c, AFormat fmt, int rate, int nch, int *max_bytes)
{
    struct OutChannel *Channel;

    TRACE("AUDIOCHANNEL_Open");
    Channel=AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;

    if (Channel->Open)
    {
        ERROR("AUDIOCHANNEL_Open: someone tries to open channel already open!");
        return 0;
    }
    if (max_bytes != NULL)
    {
        *max_bytes =
            nch *
            ((fmt == FMT_S8
              || fmt ==
              FMT_U8) ? (1) : (2)) * audiocfg->FragmentSize *
            audiocfg->RingBufferSize;
        DEBUG("Setting max_bytes = %d", *max_bytes);
    }
//    Channel->mask    = GROUP_MASTER;
    Channel->aformat = fmt;
    Channel->rate    = rate;
    Channel->n_ch    = nch;
    Channel->Open    = 1;
    Channel->Paused  = 1;
    Channel->bytes_written = 0;
    //todo g_timer_start (Channel->last_beat);
    Channel->bpm_prescale = 7000;
    Channel->beats = 0;
    Channel->bpm = 0;

    return 1;
}

int AUDIOCHANNEL_Pause (int c, int pause)
{
    struct OutChannel *Channel;

    TRACE("AUDIOCHANNEL_Pause");
    
    Channel = AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;
  
    DEBUG("AUDIOCHANNEL_Pause >%d< >%d<\n",c,pause);
    Channel->Paused = (int) (pause != 0);
    return 1;
}

int AUDIOCHANNEL_Read (struct OutChannel *Channel,int len)
{
    int nread,n_to_read;
    unsigned char tempbuf[20000];
    int i,j;
    unsigned char *buf = (unsigned char *)Channel->buffer;

    TRACE("AUDIOCHANNEL_Read");
    n_to_read=len;

    if(Channel->speed != 1.0)
    {
        n_to_read = (int)((float)(n_to_read/4)*Channel->speed)*4; //4 bytes for 16 bit stereo
    }

    memset(buf,0,len);

    if(Channel == NULL)
        return 0;

    //read n_to_read bytes from the ring buffer into our tempbuf
    nread = rb_read (Channel->rb, tempbuf, n_to_read);
    if(nread <= 0)
        return 0;
    // copy the part needed at the speed to the channel output buffer
    // This part is currently for 16 bit stereo
    for(i=0;i<len;i+=4)
    {
        j =  (int)((float)(i/4)*Channel->speed)*4;
        
        buf[i]  = tempbuf[j];
        buf[i+1]= tempbuf[j+1];
        buf[i+2]= tempbuf[j+2];
        buf[i+3]= tempbuf[j+3];
    }
    
    /* Bytes_written is used to determine time in song.
     * Therefore update is done when data is _really_ given to
     * output thread.
     */

    Channel->bytes_written += nread;

    if (len < 0)
    {
        ERROR("Audio read error: %d", len);
        len = 0;
    }
    return len;
}

int AUDIOCHANNEL_SetSpeed(int channel, int speed)
{
    struct OutChannel *Channel;

    Channel = AUDIOOUTPUT_GetChannelByID(channel);
    if(Channel == NULL)
        return 0;
  
    
    Channel->speed = (float)speed/100;

    return 0;
}

/* interface to mixer */
int AUDIOCHANNEL_SetVolume(int c, float db)
{
    struct OutChannel *Channel;

    TRACE("AUDIOCHANNEL_SetVolume"); 

    Channel = AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;

    Channel->fader_dB = db;
    return 0;
}


/* write data to the channel 's ringbuffer */
int AUDIOCHANNEL_Write (int c, void* buf, int len)
{
    struct OutChannel *Channel;
    int written;
    int newlen;
    
    TRACE("AUDIOCHANNEL_Write");

    Channel=AUDIOOUTPUT_GetChannelByID(c);

    if (Channel == NULL)
        return 0;

    if (Channel->buffer2_size < len * 2)
    {
        Channel->buffer2 = realloc (Channel->buffer2, len * 2);
        if (Channel->buffer2 == NULL)
        {
            Channel->buffer2_size = 0;
            return 0;
        }
        Channel->buffer2_size = len * 2;
    }

    /* Convert to the internal format */
    newlen = convert_buffer (Channel->aformat, Channel->n_ch, buf, Channel->buffer2, len);

    /* Write to the ringbuffer */
    written = rb_write (Channel->rb, (unsigned char *) Channel->buffer2, newlen);
    return written;
}
