/*
  BeatForce
  audio_output.c  - audio output
   
  Copyright (c) 2001, Patrick Prasse (patrick.prasse@gmx.net)

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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <stdio.h>
#include <errno.h>
#include <time.h>


#include <math.h>
#include <fftw3.h>

#include "ringbuffer.h"
#include "audio_output.h"
#include "llist.h"
#include "err.h"
#include "output.h"
#include "osa.h"
#include "mixer.h"


#define MODULE_ID AUDIO_OUTPUT
#include "debug.h"



#define mutex_unlock( m )	(m=0)
#define mutex_lock( m ) 	while( m );  m = 1

#define c_re(c) ((c)[0])
#define c_im(c) ((c)[1])

struct OutChannel *ch[OUTPUT_N_CHANNELS];
int n_open;

struct OutGroup *group[3];

int output_thread;
int output_thread_stop;


fftw_plan fftplan_out;
fftw_complex *fftw_in, *fftw_out;

AudioConfig *audiocfg;

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

int AUDIOOUTPUT_Init (AudioConfig * audio_cfg)
{
    int i, err;

    TRACE("AUDIO_OUTPUT_Init %d",OUTPUT_N_CHANNELS);
    audiocfg = audio_cfg;

    for (i = 0; i < OUTPUT_N_CHANNELS; i++)
    {
        ch[i] = malloc (OUT_CHANNEL_SIZE);
        if(ch[i] == NULL)
            return ERROR_NO_MEMORY;
        memset (ch[i], 0, OUT_CHANNEL_SIZE);

        ch[i]->buffer = malloc (OUTPUT_BUFFER_SIZE (audiocfg) * 2);
        if(ch[i]->buffer == NULL)
            return ERROR_NO_MEMORY;
        memset (ch[i]->buffer, 0, OUTPUT_BUFFER_SIZE (audiocfg) * 2);

#if 0
        ch[i]->buffer2 = malloc (OUTPUT_BUFFER_SIZE (audiocfg));
        if(ch[i]->buffer2 == NULL)
            return ERROR_NO_MEMORY;
        memset (ch[i]->buffer2, 0, OUTPUT_BUFFER_SIZE (audiocfg));
#endif

        rb_init (&ch[i]->rb, OUTPUT_RING_SIZE (audiocfg));
        
        ch[i]->last_beat = NULL;//g_timer_new ();

        ch[i]->bpm_prescale = 7000;
        ch[i]->magic = AUDIO_OUTPUT_MAGIC;

        ch[i]->speed = 1.0; /* Normal playback speed */
    }

/* Init output */
    if ((err = output_dev_init (audiocfg)))
    {
        printf ("Error initalizing Audio Output!\n");
    }

    for (i = 0; i <= 2; i++)
    {
        group[i] = malloc (sizeof (struct OutGroup));
        memset (group[i], 0, sizeof (struct OutGroup));
        group[i]->magic = AUDIO_OUTPUT_MAGIC;
        group[i]->out_buffer = malloc (OUTPUT_BUFFER_SIZE (audiocfg));

        err = output_plugin_init (group[i], audiocfg, i);
        if (err == ERROR_NO_OUTPUT_SELECTED)
        {
           
            continue;
        }
        else if (err)
        {
            char str[255];

            sprintf(str,"output_init: error initializing output device of group %d!\n(error 0x%x)\n",
                    i, -err);
            printf(str);

            if (i == 0)               /* dialog only when group is MASTER(0) */
            {
                printf (str);
                return 0;
            }
        }
        else
        {
            err = Output_PluginOpen (group[i], audiocfg, i, 2, 44100, FMT_S16_NE);
            if (err)
            {
                char str[255];
                sprintf (str,"Could not open output device of Group %d!\n"
                         "Edit your Preferences and restart BeatForce\n"
                         "Maybe you entered a wrong or not existing output device. Valid devices are for example:\n"
                         "ALSA: \"hw:0,0\", \"default\"  OSS: \"/dev/dsp\"\n"
                         "(error 0x%x)", i, -err);
                printf(str);
                exit(1);
                return 0;

            }
        }
    }



/* Init FFTW */

    fftw_in  = malloc (sizeof (fftw_complex) * (audiocfg->FragmentSize + 1));
    fftw_out = malloc (sizeof (fftw_complex) * (audiocfg->FragmentSize + 3));

    if(fftw_in == NULL || fftw_out == NULL)
        return ERROR_NO_MEMORY;

    fftplan_out =
        fftw_plan_dft_1d(audiocfg->FragmentSize,
                         fftw_in, 
                         fftw_out, 
                         FFTW_FORWARD, 
                         FFTW_ESTIMATE);
    


/* Init output thread */
    output_thread_stop = 0;
    n_open = 0;
    output_thread=i=OSA_CreateThread(AUDIOOUTPUT_Loop,NULL);
   

    return 0;
}


/* Kills the output thread */
int AUDIOOUTPUT_Cleanup (void)
{
    int i;
    output_thread_stop = 1;
    
    for (i = 0; i <= 2; i++)
    {
        output_plugin_cleanup(group[i]);
    }
    return 0;
}


/* interface to input plugin */
int AUDIOOUTPUT_Open(int c, AFormat fmt, int rate, int nch, int *max_bytes)
{

    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;
    if(ch[c] == NULL)
        return ERROR_INVALID_ARG;
    output_magic_check (ch[c], ERROR_INVALID_ARG);
    if (ch[c]->open)
    {
        fprintf (stderr,"output_open: someone tries to open channel already open!\n");
        return ERROR_ALREADY_OPEN;
    }

    if (max_bytes != NULL)
    {
        *max_bytes =
            nch *
            ((fmt == FMT_S8
              || fmt ==
              FMT_U8) ? (1) : (2)) * audiocfg->FragmentSize *
            audiocfg->RingBufferSize;
#ifdef DEBUG_OUTPUT_TRACE_PLUGIN_CALLS
        printf ("setting max_bytes = %d\n", *max_bytes);
#endif
    }

    ch[c]->aformat = fmt;
    ch[c]->rate = rate;
    ch[c]->n_ch = nch;
    ch[c]->open = 1;
    ch[c]->paused = 1;
    ch[c]->bytes_written = 0;
    //todo g_timer_start (ch[c]->last_beat);
    ch[c]->bpm_prescale = 7000;
    ch[c]->beats = 0;
    ch[c]->bpm = 0;
    n_open++;
    return 0;
}

int AUDIOOUTPUT_Close(int c)
{

    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;
    if(ch[c] == NULL)
        return ERROR_INVALID_ARG;

    output_magic_check (ch[c], ERROR_INVALID_ARG);
    if (!ch[c]->open)
        return ERROR_NOT_OPEN;
    ch[c]->paused = 1;
    ch[c]->open = 0;
    ch[c]->aformat = FMT_UNKNOWN;
    ch[c]->n_ch = 0;
    ch[c]->rate = 0;
    free (ch[c]->buffer2);
    ch[c]->buffer2 = NULL;
    ch[c]->buffer2_size = 0;
    //todo g_timer_stop (ch[c]->last_beat);
#if (ENABLE_CRUDE_BPMCOUNT >= 1)
    printf ("INFORMAL: BPMCOUNT: prescale was %ld\n", ch[c]->bpm_prescale);
#endif
    n_open--;
#if 1
    /* empty ring buffer */
    rb_clear (ch[c]->rb);
//                                                                                                                              rb_read (ch[c]->rb, (char *) ch[c]->buffer, OUTPUT_RING_SIZE (audiocfg));
    memset (ch[c]->buffer, 0, OUTPUT_BUFFER_SIZE (audiocfg));
#endif

    return 0;
}

int
output_read (int channel, unsigned char * buf, int len)
{
    int nread,n_to_read;
    unsigned char tempbuf[20000];
    int i,j;

    n_to_read=len;

    if(ch[channel]->speed != 1.0)
    {
        n_to_read = (int)((float)(n_to_read/4)*ch[channel]->speed)*4; //4 bytes for 16 bit stereo
    }

    memset(buf,0,len);

    if (channel >= OUTPUT_N_CHANNELS || channel < 0)
        return ERROR_UNKNOWN_CHANNEL;

    if(ch[channel] == NULL)
        return ERROR_INVALID_ARG;

    output_magic_check (ch[channel], ERROR_INVALID_ARG);


    //read n_to_read bytes from the ring buffer into our tempbuf
    nread = rb_read (ch[channel]->rb, tempbuf, n_to_read);
    if(nread <= 0)
        return 0;

    

    // copy the part needed at the speed to the channel output buffer
    // This part is currently for 16 bit stereo
    for(i=0;i<len;i+=4)
    {
        j =  (int)((float)(i/4)*ch[channel]->speed)*4;
        
        buf[i]  = tempbuf[j];
        buf[i+1]= tempbuf[j+1];
        buf[i+2]= tempbuf[j+2];
        buf[i+3]= tempbuf[j+3];
    }
    
    /* Bytes_written is used to determine time in song.
     * Therefore update is done when data is _really_ given to
     * output thread.
     */

    ch[channel]->bytes_written += nread;

    return len;
}


int output_write (int c, void* buf, int len)
{
    int written;
    int newlen;

    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;
    if (ch[c] == NULL)
        return ERROR_INVALID_ARG;

    output_magic_check (ch[c], ERROR_INVALID_ARG);
    if (ch[c]->buffer2_size < len * 2)
    {
        printf( "audioutput_write: buffer-realloc from %d to %d!\n", ch[c]->buffer2_size, len*2 );

        ch[c]->buffer2 = realloc (ch[c]->buffer2, len * 2);
        if (ch[c]->buffer2 == NULL)
        {
            ch[c]->buffer2_size = 0;
            return ERROR_NO_MEMORY;
        }
        ch[c]->buffer2_size = len * 2;
    }

    newlen =
        convert_buffer (ch[c]->aformat, ch[c]->n_ch, buf, ch[c]->buffer2, len);
    written = rb_write (ch[c]->rb, (unsigned char *) ch[c]->buffer2, newlen);
    return written;
}

long AUDIOOUTPUT_BufferFree(int c)
{
    long free = -1;

    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;
    if(ch[c] == NULL)
        return ERROR_INVALID_ARG;

    output_magic_check (ch[c], ERROR_INVALID_ARG);

    free = rb_free (ch[c]->rb);

    return free;
}


int AUDIOOUTPUT_Pause (int c, int pause)
{
    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;

    if(ch[c] == NULL)
        return ERROR_INVALID_ARG;
  
    output_magic_check (ch[c], ERROR_INVALID_ARG);
    ch[c]->paused = (int) (pause != 0);
    return 0;
}

int AUDIOOUTPUT_SetSpeed(int channel, float speed)
{
    if (channel >= OUTPUT_N_CHANNELS || channel < 0)
        return ERROR_UNKNOWN_CHANNEL;
    if(ch[channel] == NULL)
        return ERROR_INVALID_ARG;
  
    output_magic_check (ch[channel], ERROR_INVALID_ARG);
    
    ch[channel]->speed = speed;

    return 0;
}

long
output_get_time (int c)
{
    double time;
    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;

    if(ch[c] == NULL)
        return ERROR_INVALID_ARG;
    
    output_magic_check (ch[c], ERROR_INVALID_ARG);

    time = ((double) ch[c]->bytes_written) / (2 * ch[c]->rate * ch[c]->n_ch);
    time = (int) (time * 1000);
    return (long) time;
}

int AUDIOOUTPUT_GetVolumeLevel(int channel,int *left,int *right)
{
    if(ch[channel]->paused)
    {
        *left=0;
        *right=0;
    }
    else
    {
        *left  = ch[channel]->volumeleft;
        *right = ch[channel]->volumeright;
    }
    return 1;
}

/* interface to mixer */
int AUDIOOUTPUT_SetVolume(int c, float db)
{
    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;
    
    if(ch[c] == NULL)
        return ERROR_INVALID_ARG;
    
    output_magic_check (ch[c], ERROR_INVALID_ARG);
    ch[c]->fader_dB = db;
    return 0;
}

int AUDIOOUTPUT_SetMainVolume(int value)
{
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
    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;
    if(ch[c] == NULL)
        return ERROR_INVALID_ARG;
    output_magic_check (ch[c], ERROR_INVALID_ARG);

    *db = ch[c]->fader_dB;
    return 0;
}

int
output_set_group (int c, int group, int on)
{
    int mask;
    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;
    if(ch[c] == NULL)
        return ERROR_INVALID_ARG;
    output_magic_check (ch[c], ERROR_INVALID_ARG);

    mask = ch[c]->mask;
    if (on)
        mask = mask | group;
    else
        mask = mask & (~group);

    ch[c]->mask = mask;
    return 0;
}

int AUDIOOUTPUT_Mute (int c, int mute)
{
    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;
    if(ch[c] == NULL)
        return ERROR_INVALID_ARG;
    output_magic_check (ch[c], ERROR_INVALID_ARG);
    ch[c]->mute = (int) (mute != 0);
    return 0;
}

int
output_set_beatcount (int c, int on)
{
    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;

    if(ch[c]==NULL)
        return ERROR_INVALID_ARG;

    output_magic_check (ch[c], ERROR_INVALID_ARG);

    ch[c]->detect_beat = (int) (on != 0);
    return 0;
}

double
output_get_bpm (int c)
{
    if (c >= OUTPUT_N_CHANNELS || c < 0)
        return ERROR_UNKNOWN_CHANNEL;

    if(ch[c] == NULL)
        return ERROR_INVALID_ARG;

    output_magic_check (ch[c], ERROR_INVALID_ARG);
    return ch[c]->bpm;
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

/* main thread */
static int AUDIOOUTPUT_Loop(void *arg)
{
    int channel=0, i=0, sample=0;

    output_word *tmp_buf = malloc (2 * OUTPUT_BUFFER_SIZE (audiocfg));
    
    memset(tmp_buf,0,(2 * OUTPUT_BUFFER_SIZE (audiocfg)));
    
    if (tmp_buf == NULL)
        return -1;

    while (!output_thread_stop)
    {

        for (i = 0; i < 3; i++)
        {
            memset (group[i]->out_buffer, 0, OUTPUT_BUFFER_SIZE (audiocfg));
        }

        for (channel = 0; channel < OUTPUT_N_CHANNELS; channel++)
        {
            int n_read=0;

            if(ch[channel]->paused)
                continue;

            /* output_read reads at a speed ch[channel]->speed */
            n_read =
                output_read (channel, (unsigned char *) ch[channel]->buffer,
                             OUTPUT_BUFFER_SIZE (audiocfg));

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
                memcpy (tmp_buf, ch[channel]->buffer, n_read * 2);
                memset (ch[channel]->buffer, 0, n_zeroes * 2);
                memcpy (&ch[channel]->buffer[n_zeroes], tmp_buf, n_read * 2);
            }

            /* perform fft for beat detection */
            if (channel == 0 || channel == 1)
            {
                do_fft(channel, NULL);
            }

            /* if channel is muted, we discard the bytes read */
            if (ch[channel]->mute)
            {
                memset (ch[channel]->buffer, 0, OUTPUT_BUFFER_SIZE (audiocfg));
            }

            if (ch[channel]->fader_dB > -31)
            {

                AUDIOOUTPUT_CalculateVolume(ch[channel]);
                                
                /* attenuate or amplify by db */
                for (sample = 0; sample < OUTPUT_BUFFER_SIZE_SAMPLES (audiocfg); sample++)
                {
                    double tmp;
                    if (ch[channel]->fader_dB < -100.0 || ch[channel]->fader_dB > 100.0)
                        ch[channel]->fader_dB = 0.0;
                    tmp =
                        (double) ch[channel]->buffer[sample] * pow (10,
                                                                    0.05 * (double) ch[channel]->fader_dB);
                    tmp = (int) (tmp);

                    if (tmp > 32767.0)
                    {
                        tmp = 32767.0;
                        ch[channel]->clipping_count++;
                    }
                    if (tmp < -32767.0)
                    {
                        tmp = -32767.0;
                        ch[channel]->clipping_count++;
                    }
                    
                    
                    /* add to master buffer if selected */
                    /* ch 0 and ch1 are added later -> CrossFader !!! */
                    if ((ch[i]->mask & GROUP_MASTER))
                    {
                        ADD_TO_OUTPUT_BUFFER (&group[0]->out_buffer[sample], tmp);
                    }
                    
                    if (ch[i]->mask & GROUP_A)
                    {
                        ADD_TO_OUTPUT_BUFFER (&group[1]->out_buffer[sample], tmp);
                    }
                    
                    if (ch[i]->mask & GROUP_B)
                    {
                        ADD_TO_OUTPUT_BUFFER (&group[2]->out_buffer[sample], tmp);
                    }
                    
                } /* for( 0 to SAMPLES ) */
                
            }
       
        
        }
        /* for( channel <N_CHANNELS ) */


// do the crossfading
#if 1
        if (!((ch[0]->fader_dB <= -31) && (ch[1]->fader_dB <= -31)))
        {
            double ch0   = 0;
            double ch1   = 0;
            double value = 0;
            MIXER_GetFaderValue(&value);
       
            if ((value < 0.00) || (value > 1.00001))
            {
                value = 0.50;
            }

            if (value >= 0.50)
                ch1 = 0.50;
            else
                ch1 = value;

            if (value <= 0.50)
                ch0 = 0.50;
            else
                ch0 = 1.00 - value;

            if (ch0 != 0)
            {
                ch0 = pow (10,0.05 * (double) ((ch0 * 2 - 1) * 30)) * _TO_ATT (ch[0]->fader_dB);
            }

            if (ch1 != 0)
            {
                ch1 = pow (10,0.05 * (double) ((ch1 * 2 - 1) * 30)) * _TO_ATT (ch[1]->fader_dB);
            }




            for (sample = 0; sample < OUTPUT_BUFFER_SIZE_SAMPLES (audiocfg); sample++)
            {
                double ch0_tmp = 0;
                double ch1_tmp = 0;
                double tmp;
                if (!ch[0]->paused)
                    ch0_tmp = (double) ch[0]->buffer[sample] * ch0;
                
                if (!ch[1]->paused)
                    ch1_tmp = (double) ch[1]->buffer[sample] * ch1;
                
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
#endif


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
#define VAL( x )   (c_re((x)))
#define CALC_BPM( _msec )	(60000/_msec)
    if (!(c == 0 || c == 1))
        return -1;

    /* fill the fftw input buffer */
    for (j = 0; j < audiocfg->FragmentSize; j++)
    {
        VAL (fftw_in[j]) =
            (ch[c]->buffer[j * 2] >> 1) + (ch[c]->buffer[j * 2 + 1] >> 1);
    }

    /* Execute the 1d DFT, output is written to fftw_out */
    fftw_execute(fftplan_out);

    /* if the detect_beat  button is toggled i the mixer window */
//    if (ch[c]->detect_beat)
    {
        double beat = 0;
        for (j = 1; j < 5; j++)  //de amplitudes van beneden de 2 * 43 Hz
        {
            beat +=
                /* real = sqrt(im^2 + re^2) */
                sqrt (c_re(fftw_out[j]) * c_re(fftw_out[j]) +
                      c_im(fftw_out[j]) * c_im(fftw_out[j]));

            
        }
        

        beat = beat / 5; //de gemiddelde amplitude

        beat = beat / audiocfg->FragmentSize * 2;  
        beat = beat / 18;


//        printf("BEAT %f %d\n",beat,(int)beat);

        if (beat >= 1.0)
        {
            double newbpm = CALC_BPM (miliseconds);
            //todo g_timer_reset (ch[c]->last_beat);
            if (miliseconds < 200)
            {
                if (ch[c]->beats >= 20)
                    ch[c]->bpm_prescale += 10;
                return 0;
            }
            if (miliseconds > 800)
            {
                ch[c]->bpm_prescale -= 100;
                if (ch[c]->bpm_prescale < 2500)
                    ch[c]->bpm_prescale = 2500;
                printf ("prescale -= 100: %ld\n", ch[c]->bpm_prescale);
                return 0;
            }
            ch[c]->bpm = newbpm;
            ch[c]->beats++;
            printf ("s: %f\n", (float) miliseconds);
            printf ("bpm: %f\n", (float) newbpm);
//                                                                                                                                                                                                                                                              mixer_set_beat_widget( c, 1 );
            printf ("%f\n", (float) beat);
            //todo g_timer_start (ch[c]->last_beat);
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
            printf( "a%lx\n", buf_in[i] );
            buf_out_s16[i] = (gint16) (buf_in[i] >> 16);
            printf( "b%x\n", buf_out_s16[i] );
        }
        printf( "%d\n", convert * sizeof(gint16));
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
        return ERROR_WRONG_CH_NUMBER;

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

            printf( "%x:%x  ", out_buf[i], ptr[i] );
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

    return length;
}




#endif /* ifdef OUTPUT_SIZE_32 */
