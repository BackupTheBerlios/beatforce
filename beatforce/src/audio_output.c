/*
  BeatForce
  audio_output.c  - audio output
   
  Copyright (c) 2001, Patrick Prasse (patrick.prasse@gmx.net)
  Copyright (c) 2003-2004, John Beuving (john.beuving@wanadoo.nl)
  

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

#include <stdio.h>
#include <errno.h>
#include <time.h>


#include <math.h>
//#include <fftw3.h>

#include "ringbuffer.h"
#include "audio_output.h"
#include "llist.h"
#include "err.h"
#include "output.h"
#include "osa.h"
#include "mixer.h"
#include "effect.h"
#include "config.h"

#define MODULE_ID AUDIO_OUTPUT
#include "debug.h"

AudioConfig *audiocfg;
extern BeatforceConfig *cfgfile;

#define mutex_unlock( m )	(m=0)
#define mutex_lock( m ) 	while( m );  m = 1

#define c_re(c) ((c)[0])
#define c_im(c) ((c)[1])

//ruct OutChannel *ch[OUTPUT_N_CHANNELS];

struct OutChannelList *ChannelList;

struct OutGroup *group[3];

int output_thread;
int output_thread_stop;

int runningcount;

int channel_id;

//fftw_plan fftplan_out;
//fftw_complex *fftw_in, *fftw_out;

#define _TO_ATT( _dB )		( (_dB > -31) ? ( pow( 10, 0.05 * _dB ) ) : (0) )

/* main thread */
static int AUDIOOUTPUT_Loop (void *);


void __inline ADD_TO_OUTPUT_BUFFER (output_word *, float);

void __inline
ADD_TO_OUTPUT_BUFFER (output_word * _buf, float _ch)
{
    float tmp1;
    tmp1 = _ch + (output_word) * _buf;
    if (tmp1 > 32767.0)
        tmp1 = 32767.0;
    if (tmp1 < -32767.0)
        tmp1 = -32767.0;
    (output_word) (*_buf) = (output_word) tmp1;
}



int AUDIOOUTPUT_Init ()
{
    int i;
    TRACE("AUDIO_OUTPUT_Init");

    audiocfg=CONFIGFILE_GetCurrentAudioConfig();

    if(audiocfg == NULL)
        return 0;

    ChannelList = NULL;
    channel_id  = 0;

/* Init output */
    if (!OUTPUT_DevInit (audiocfg))
    {
        printf ("Error initalizing Audio Output!\n");
    }

    for (i = 0; i <= 2; i++)
    {
        group[i] = malloc (sizeof (struct OutGroup));
        memset (group[i], 0, sizeof (struct OutGroup));
        group[i]->out_buffer = malloc (OUTPUT_BUFFER_SIZE (audiocfg));

        if(!OUTPUT_PluginInit (group[i], audiocfg, i))
        {
            if (i == 0)               /* dialog only when group is MASTER(0) */
            {
                ERROR("AUDIOOUTPUT_Init: can not open master output device");
                return 0;
            }
        }
        else
        {
            
            if (!OUTPUT_PluginOpen (group[i], audiocfg, i, 2, 44100, FMT_S16_NE))
            {
                char str[255];
                sprintf (str,"Could not open output device of Group %d!\n"
                         "Edit your Preferences and restart BeatForce\n"
                         "Maybe you entered a wrong or not existing output device. Valid devices are for example:\n"
                         "ALSA: \"hw:0,0\", \"default\"  OSS: \"/dev/dsp\"\n"
                         "(error 0x%x)", i, i);
                printf(str);
                return 0;

            }
        }
    }



/* Init FFTW */

//    fftw_in  = malloc (sizeof (fftw_complex) * (audiocfg->FragmentSize + 1));
//    fftw_out = malloc (sizeof (fftw_complex) * (audiocfg->FragmentSize + 3));

//    if(fftw_in == NULL || fftw_out == NULL)
//        return 0;

//    fftplan_out =
//        fftw_plan_dft_1d(audiocfg->FragmentSize,
//                         fftw_in, 
//                         fftw_out, 
//                         FFTW_FORWARD, 
//                         FFTW_ESTIMATE);
    


/* Init output thread */
    output_thread_stop = 0;

    runningcount=0;
    output_thread=i=OSA_CreateThread(AUDIOOUTPUT_Loop,NULL);

    return 1;
}


int AUDIOOUTPUT_ChannelNew()
{
    struct OutChannel *channel;

    channel = malloc (OUT_CHANNEL_SIZE);
    memset (channel, 0, OUT_CHANNEL_SIZE);
    

    channel->buffer = malloc (OUTPUT_BUFFER_SIZE (audiocfg) * 2);
    if(channel->buffer == NULL)
        return 0;
    memset (channel->buffer, 0, OUTPUT_BUFFER_SIZE (audiocfg) * 2);

    channel->id = channel_id;    
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
    channel_id++;
    
    if(ChannelList == NULL)
    {
        ChannelList = malloc(sizeof(OutChannelList));
        memset(ChannelList,0,sizeof(OutChannelList));
        
        ChannelList->Channel = channel;
    }
    else
    {
        struct OutChannelList *l=ChannelList;
        
        while(l->Next)
            l=l->Next;

        l->Next = malloc(sizeof(OutChannelList));
        memset(l->Next,0,sizeof(OutChannelList));
        
        l->Next->Channel = channel;

        
    }
    return channel->id;
}

/* Kills the output thread */
int AUDIOOUTPUT_Cleanup (void)
{
    int i;
    output_thread_stop = 1;
    
    for (i = 0; i <= 2; i++)
    {
        OUTPUT_PluginCleanup(group[i]);
    }
    return 1;
}

struct OutChannel *AUDIOOUTPUT_GetChannelByID(int c)
{
    struct OutChannelList *l;

    if (c >= channel_id || c < 0)
    {
        ERROR("Wrong channel number");
        return NULL;
    }

    if(ChannelList == NULL)
    {
        ERROR("Channel not initialised");
        return NULL;
    }
    l=ChannelList;
    
    while(l)
    {
        if(c == l->Channel->id)
            return l->Channel;
        l=l->Next;
    }
    return NULL;
}
/* interface to input plugin */
int AUDIOOUTPUT_Open(int c, AFormat fmt, int rate, int nch, int *max_bytes)
{
    struct OutChannel *Channel;

    Channel=AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;

    if (Channel->Open)
    {
        ERROR("AUDIOOUTPUT_Open: someone tries to open channel already open!");
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

int AUDIOOUTPUT_Close(int c)
{
    struct OutChannel *Channel;
    
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

int output_read (struct OutChannel *Channel,int len)
{
    int nread,n_to_read;
    unsigned char tempbuf[20000];
    int i,j;
    unsigned char *buf = (unsigned char *)Channel->buffer;

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

    return len;
}


int AUDIOOUTPUT_Write (int c, void* buf, int len)
{
    struct OutChannel *Channel;
    int written;
    int newlen;

    TRACE("AUDIOOUTPUT_Write");

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
    newlen =
        convert_buffer (Channel->aformat, Channel->n_ch, buf, Channel->buffer2, len);

    /* Write to the ringbuffer */
    written = rb_write (Channel->rb, (unsigned char *) Channel->buffer2, newlen);
    return written;
}

long AUDIOOUTPUT_BufferFree(int c)
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


int AUDIOOUTPUT_Pause (int c, int pause)
{
    struct OutChannel *Channel;

    TRACE("AUDIOOUTPUT_Pause");

    Channel = AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;
  
    if(pause == 0)
        runningcount++;
    else
        runningcount--;

    TRACE("AUDIOOUTPUT_Pause >%d< >%d<\n",c,pause);
    Channel->Paused = (int) (pause != 0);
    return 1;
}

int AUDIOOUTPUT_SetSpeed(int channel, int speed)
{
    struct OutChannel *Channel;

    Channel = AUDIOOUTPUT_GetChannelByID(channel);
    if(Channel == NULL)
        return 0;
  
    
    Channel->speed = (float)speed/100;

    return 0;
}

long AUDIOOUTPUT_GetTime(int c)
{
    struct OutChannel *Channel;
    double time;

    Channel = AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;
    


    time = ((double) Channel->bytes_written) / (2 * Channel->rate * Channel->n_ch);
    time = (int) (time * 1000);
    return (long) time;
}

int AUDIOOUTPUT_GetVolumeLevel(int channel,int *left,int *right)
{
    struct OutChannel *Channel;

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

/* interface to mixer */
int AUDIOOUTPUT_SetVolume(int c, float db)
{
    struct OutChannel *Channel;

    TRACE("AUDIOOUTPUT_SetVolume"); 

    Channel = AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;

    Channel->fader_dB = db;
    return 0;
}

int AUDIOOUTPUT_SetMainVolume(int value)
{
    TRACE("AUDIOOUTPUT_SetMainVolume");
    group[0]->mainvolume = value;
    OUTPUT_PluginSetVolume(group[0]);
    return 1;
}

int AUDIOOUTPUT_GetMainVolume(int *value)
{
    OUTPUT_PluginGetVolume(group[0]);

    *value = group[0]->mainvolume;
    return 1;
}

int AUDIOOUTPUT_GetChannelVolume(int c, float *db)
{
    struct OutChannel *Channel;

    Channel = AUDIOOUTPUT_GetChannelByID(c);
    if(Channel == NULL)
        return 0;


    *db = Channel->fader_dB;
    return 0;
}

int AUDIOOUTPUT_Mute (int c, int mute)
{
    struct OutChannel *Channel;

    Channel = AUDIOOUTPUT_GetChannelByID(c);

    if(Channel == NULL)
        return 0;

    Channel->Mute = (int) (mute != 0);
    return 0;
}


/*  Internal function which calculates the volume
 *  of the channel (left and right) 
 */
static void AUDIOOUTPUT_CalculateVolume(struct OutChannel *ch)
{
    int sample;
    int l=0,l1=0;
    int r=0,r1=0;

    for (sample = 0; sample < OUTPUT_BUFFER_SIZE_SAMPLES (audiocfg); sample++)
    {
        if(sample % 2 == 0)
        {
            l1=abs(ch->buffer[sample]);
            if(l1>l)
                l=l1;
        }
        else
        {
            r1=abs(ch->buffer[sample]);
            if(r1>r)
                r=r1;
        }

    }

    l /= 376; //volumecorrection
    r /= 376; //volumecorrection
    ch->volumeleft  = 20 * log(l);
    ch->volumeright = 20 * log(r);
}

/* 
 * Combines the signals of channel 0 and channel 1
 * according to mixer value
 */
static void AUDIOOUTPUT_Crossfade(struct OutChannel *Channel1,struct OutChannel *Channel2)
{
    int sample;


    if (!((Channel1->fader_dB <= -31) && (Channel2->fader_dB <= -31)))
    {
        double ch0   = 0;
        double ch1   = 0;
        unsigned int value = 0;
        MIXER_GetFaderValue(&value);
        
        if (value > 1000)
        {
            value = 500;
        }
        
        ch1 = (double)((double)value/1000);
        
        ch0 = 1.00 - (double)((double)value/1000);
        
        if (ch0 != 0)
        {
            ch0 = pow (10,0.05 * (double) ((ch0 * 2 - 1) * 10)) * _TO_ATT (Channel1->fader_dB);
        }
        
        if (ch1 != 0)
        {
            ch1 = pow (10,0.05 * (double) ((ch1 * 2 - 1) * 10)) * _TO_ATT (Channel2->fader_dB);
        }
        for (sample = 0; sample < OUTPUT_BUFFER_SIZE_SAMPLES (audiocfg); sample++)
        {
            double ch0_tmp = 0;
            double ch1_tmp = 0;
            double tmp;
            if (!Channel1->Paused)
                ch0_tmp = (double) Channel1->buffer[sample] * ch0;
            
            if (!Channel2->Paused)
                ch1_tmp = (double) Channel2->buffer[sample] * ch1;
            
            tmp = (int) (ch1_tmp + ch0_tmp);
            if (tmp > 32767.0)
            {
                tmp = 32767.0;
            }
            if (tmp < -32767.0)
            {
                tmp = -32767.0;
            }
            
            ADD_TO_OUTPUT_BUFFER (&group[0]->out_buffer[sample], tmp);
        }                         /* for ... */
    }                           /* { */
}

/* main thread */
static int AUDIOOUTPUT_Loop(void *arg)
{
    int channel=0, i=0;
    unsigned int sample=0;
    struct OutChannelList *ListItem;
    output_word *tmp_buf = malloc (2 * OUTPUT_BUFFER_SIZE (audiocfg));
    
    memset(tmp_buf,0,(2 * OUTPUT_BUFFER_SIZE (audiocfg)));
    if (tmp_buf == NULL)
        return 0;

    while (!output_thread_stop)
    {
        for (i = 0; i < 3; i++)
        {
            memset (group[i]->out_buffer, 0, OUTPUT_BUFFER_SIZE (audiocfg));
        }

        ListItem = ChannelList;
        while(ListItem)
        {
            int n_read=0;

            if(!ListItem->Channel->Paused)
            {
                /* output_read reads at a speed ListItem->Channel->speed */
                n_read = output_read (ListItem->Channel,OUTPUT_BUFFER_SIZE (audiocfg));

                if(ListItem->Channel->id == 0)
                {
                    EFFECT_Run(OUTPUT_BUFFER_SIZE(audiocfg));
                }

                
                if (n_read < 0)
                {
                    printf ("Audio read error: 0x%x\n", n_read);
                    n_read = 0;
                }
                
                /* if we could not read all 1024 samples, move read to buffer end */
                if (n_read < OUTPUT_BUFFER_SIZE (audiocfg))
                {
                    int n_zeroes = OUTPUT_BUFFER_SIZE (audiocfg) - n_read;
                    /* 
                     * TODO: add padding handler which notifies user if we are padding a lot 
                     *       in order to advice him to turn up ring buffer size
                     */
//                printf ("Channel %d: padding %d zeroes.\n", channel, n_zeroes);
                    memcpy (tmp_buf, ListItem->Channel->buffer, n_read * 2);
                    memset (ListItem->Channel->buffer, 0, n_zeroes * 2);
                    memcpy (&ListItem->Channel->buffer[n_zeroes], tmp_buf, n_read * 2);
                }
                
                
                /* perform fft for beat detection */
//            if (channel == 0 || channel == 1)
//            {
//                do_fft(channel, NULL);
//            }
                
                /* if channel is muted, we discard the bytes read */
                if (ListItem->Channel->Mute)
                {
//                    memset (ListItem->Channel->buffer, 0, OUTPUT_BUFFER_SIZE (audiocfg));
                }
                
                if (ListItem->Channel->fader_dB > -31)
                {
                    
                    if (channel == 0 || channel == 1)
                        AUDIOOUTPUT_CalculateVolume(ListItem->Channel);
                    
                    /* attenuate or amplify by db */
                    for (sample = 0; sample < OUTPUT_BUFFER_SIZE_SAMPLES (audiocfg); sample++)
                    {
                        double tmp;
                        if (ListItem->Channel->fader_dB < -100.0 || ListItem->Channel->fader_dB > 100.0)
                            ListItem->Channel->fader_dB = 0.0;
                        tmp =
                            (double) ListItem->Channel->buffer[sample] * pow (10,
                                                                              0.05 * (double) ListItem->Channel->fader_dB);
                        tmp = (int) (tmp);
                        
                        if (tmp > 32767.0)
                        {
                            tmp = 32767.0;
                            ListItem->Channel->clipping_count++;
                        }
                        if (tmp < -32767.0)
                        {
                            tmp = -32767.0;
                            ListItem->Channel->clipping_count++;
                        }
                        
                        
                        /* add to master buffer if selected */
                        /* ch 0 and ch1 are added later -> CrossFader !!! */
                        if ((ListItem->Channel->mask & GROUP_MASTER))
                        {
                            ADD_TO_OUTPUT_BUFFER (&group[0]->out_buffer[sample], tmp);
                        }
                        
                        if (ListItem->Channel->mask & GROUP_MONITOR)
                        {
                            ADD_TO_OUTPUT_BUFFER (&group[1]->out_buffer[sample], tmp);
                        }
                    }
                }
            }
            ListItem=ListItem->Next;
        }

        if(ChannelList && ChannelList->Next)
        {
            AUDIOOUTPUT_Crossfade(ChannelList->Channel,ChannelList->Next->Channel);
        }

        /* output pcm data to output plugin */
        {
            int err;
            err =
                convert_output (FMT_S16_NE, 2,
                                group[0]->out_buffer, audiocfg->FragmentSize);
            if (err < 0)
                printf ("Audio Convert error: %d\n", err);

            err = OUTPUT_BUFFER_SIZE_SAMPLES (audiocfg) * 2;
            err = OUTPUT_PluginWrite (group[0], group[0]->out_buffer, err);
        }
    }/* for(;;) */

    free(tmp_buf);

    return 0;
}



int
do_fft (int c, output_word * buf)
{
    int j;
    double miliseconds=0;
    struct OutChannel *Channel;
#define VAL( x )   (c_re((x)))
#define CALC_BPM( _msec )	(60000/_msec)

    Channel=AUDIOOUTPUT_GetChannelByID(c);


    /* fill the fftw input buffer */
    for (j = 0; j < audiocfg->FragmentSize; j++)
    {
//        VAL (fftw_in[j]) =
//            (Channel->buffer[j * 2] >> 1) + (Channel->buffer[j * 2 + 1] >> 1);
    }

    /* Execute the 1d DFT, output is written to fftw_out */
//    fftw_execute(fftplan_out);

    /* if the detect_beat  button is toggled i the mixer window */
//    if (Channel->detect_beat)
    {
        double beat = 0;
        for (j = 1; j < 5; j++)  //de amplitudes van beneden de 2 * 43 Hz
        {
//            beat +=
            /* real = sqrt(im^2 + re^2) */
//                sqrt (c_re(fftw_out[j]) * c_re(fftw_out[j]) +
//                      c_im(fftw_out[j]) * c_im(fftw_out[j]));

            
        }
        

        beat = beat / 5; //de gemiddelde amplitude
        beat = beat / audiocfg->FragmentSize * 2;  
        beat = beat / 18;


//        printf("BEAT %f %d\n",beat,(int)beat);

        if (beat >= 1.0)
        {
            double newbpm = CALC_BPM (miliseconds);
            //todo g_timer_reset (Channel->last_beat);
            if (miliseconds < 200)
            {
                if (Channel->beats >= 20)
                    Channel->bpm_prescale += 10;
                return 0;
            }
            if (miliseconds > 800)
            {
                Channel->bpm_prescale -= 100;
                if (Channel->bpm_prescale < 2500)
                    Channel->bpm_prescale = 2500;
                printf ("prescale -= 100: %ld\n", Channel->bpm_prescale);
                return 0;
            }
            Channel->bpm = newbpm;
            Channel->beats++;
            printf ("s: %f\n", (float) miliseconds);
            printf ("bpm: %f\n", (float) newbpm);
//                                                                                                                                                                                                                                                              mixer_set_beat_widget( c, 1 );
            printf ("%f\n", (float) beat);
            //todo g_timer_start (Channel->last_beat);
        }

    }

    return 0;
}


#ifdef OUTPUT_SIZE_32
int
convert_output (int format, int nch, output_word * buf, int samples)
{
    int convert = samples * nch;
    int i;
    output_word *buf_in = buf;
    if (snd_pcm_format_width (format) == 16)
    {
        gint16 *buf_out_s16 = (gint16 *) buf;
        for (i = 0; i < convert; i++)
        {
//            printf( "a%lx\n", buf_in[i] );
            buf_out_s16[i] = (gint16) (buf_in[i] >> 16);
//            printf( "b%x\n", buf_out_s16[i] );
        }
//        printf( "%d\n", convert * sizeof(gint16));
        return (convert * sizeof (gint16));
    }
    else if (snd_pcm_format_width (format) == 24)
    {
        gint32 *buf_out_s24 = (gint32 *) buf;
        for (i = 0; i < convert; i++)
        {
            buf_out_s24[i] = (gint32) (buf_in[i] >> 8);
        }
        return (i + 1) * sizeof (gint32);
    }
    else
    {
        printf ("unknown output format width: %d\n",
                snd_pcm_format_width (format));
        return -1;
    }
}

int
convert_buffer (AFormat afmt, int nch,
                void* buffer, output_word * out_buf, int length)
{
    /* out_buf is the output buffer. it is machine endianess */

    int i;
    if (nch > 2 || nch < 1)
        return ERROR_WRONG_CH_NUMBER;
    
//    printf( "convert_buffer( %d, %d, ., ., %d )\n", afmt, nch, length );
    if (afmt == FMT_S8)
    {
        gint8 *ptr2 = buffer;
        for (i = 0; i < length; i++)
        {
            out_buf[i] = ptr2[i] << 24;
        }

        length *= 2;
    }
    if (afmt == FMT_U8)
    {
        guint8 *ptr2 = buffer;
        for (i = 0; i < length; i++)
            out_buf[i] = (ptr2[i] - 127) << 24;
        length *= 2;
    }


    if (afmt == FMT_S16_BE)
    {
        output_word *ptr = buffer;
        for (i = 0; i < length >> 1; i++)
            out_buf[i] = /*GINT16_FROM_BE*/ (ptr[i]) << 16;
    }

    if (afmt == FMT_S16_LE)
    {
        output_word *ptr = buffer;
        for (i = 0; i < length >> 1; i++)
        {
            out_buf[i] = /*GINT16_FROM_LE*/ (ptr[i]) << 16;
        }
    }

    if (afmt == FMT_S16_NE)
    {
        output_word *ptr = buffer;
        for (i = 0; i < length >> 1; i++)
            out_buf[i] = ptr[i] << 16;
    }

    if (afmt == FMT_U16_BE)
    {
        guint16 *ptr1 = buffer;
        for (i = 0; i < length >> 1; i++)
            out_buf[i] = ((short) (/*GUINT16_FROM_BE*/ (ptr1[i]) - 32768)) << 16;
    }
    if (afmt == FMT_U16_LE)
    {
        guint16 *ptr1 = buffer;
        for (i = 0; i < length >> 1; i++)
            out_buf[i] = ((short) (/*GUINT16_FROM_LE*/ (ptr1[i]) - 32768)) << 16;
    }
    if (afmt == FMT_U16_NE)
    {
        guint16 *ptr1 = buffer;
        for (i = 0; i < length >> 1; i++)
            out_buf[i] = ((short) (ptr1[i] - 32768)) << 16;
    }

    return length * 2;
}


#else /* ifdef OUTPUT_SIZE_32 */

int
convert_output (int format, int nch, output_word * buf, int samples)
{
    return nch * samples * OUTPUT_SAMPLE_LEN;
}

int
convert_buffer (AFormat afmt, int nch,
                void* buffer, output_word * out_buf, int length)
{
    /* out_buf is the output buffer. it is machine endianess */

    int i;
    if (nch > 2 || nch < 1)
        return 0;

//    printf( "convert_buffer( %d, %d, ., ., %d )\n", afmt, nch, length );
    if (afmt == FMT_S8)
    {
        unsigned char *ptr2 = buffer;
        for (i = 0; i < length; i++)
        {
            out_buf[i] = ptr2[i] * 256;
        }
        length *= 2;
    }

    if (afmt == FMT_U8)
    {
        unsigned char *ptr2 = buffer;
        for (i = 0; i < length; i++)
        {
            out_buf[i] = (ptr2[i] - 127) * 256;
        }
        length *= 2;
    }


    if (afmt == FMT_S16_BE)
    {
        output_word *ptr = buffer;
        for (i = 0; i < length >> 1; i++)
            out_buf[i] = /*GINT16_FROM_BE*/ (ptr[i]);
    }
    if (afmt == FMT_S16_LE)
    {
        output_word *ptr = buffer;
        for (i = 0; i < length >> 1; i++)
        {
            out_buf[i] = /*GINT16_FROM_LE*/ (ptr[i]);

            //printf( "%x:%x  ", out_buf[i], ptr[i] );
        }
    }
    if (afmt == FMT_S16_NE)
    {
        output_word *ptr = buffer;
        for (i = 0; i < length >> 1; i++)
            out_buf[i] = ptr[i];
    }


    if (afmt == FMT_U16_BE)
    {
        unsigned short *ptr1 = buffer;
        for (i = 0; i < length >> 1; i++)
            out_buf[i] = /*GUINT16_FROM_BE*/ (ptr1[i]) - 32768;
    }
    if (afmt == FMT_U16_LE)
    {
        unsigned short *ptr1 = buffer;
        for (i = 0; i < length >> 1; i++)
            out_buf[i] = /*GUINT16_FROM_LE*/ (ptr1[i]) - 32768;
    }
    if (afmt == FMT_U16_NE)
    {
        unsigned short *ptr1 = buffer;
        for (i = 0; i < length >> 1; i++)
            out_buf[i] = ptr1[i] - 32768;
    }
    if(afmt == FMT_FLOAT32)
    {
        ERROR("FMT_FLOAT32 not supported");
    }
    return length;
}




#endif /* ifdef OUTPUT_SIZE_32 */
