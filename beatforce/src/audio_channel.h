


#ifndef __AUDIO_CHANNEL_H__
#define __AUDIO_CHANNEL_H__

#include "audio_output.h"
#include "input_plugin.h"

typedef struct OutChannel
{
    int id;

    output_word *buffer;
    output_word *buffer2;

    int buffer2_size;

    struct OutRingBuffer *rb;

    int Mute; /* boolean */
    int Paused; /* boolean */
    int Open;      /* boolean */

    long bytes_written;

    unsigned short mask;

    AFormat aformat;
    int n_ch;
    int rate;

    float bpm;
    long beats;
    float bpm_hist[OUTPUT_BPM_HIST];

    unsigned char bands[OUTPUT_FFT_BANDS];
    unsigned char bands_hist[OUTPUT_FFT_BANDS][OUTPUT_FFT_HIST];

    float fader_dB;

    int clipping;
    int clipping_count;
    int readdata;

    int padding_count;

    int detect_beat;
    int display_fft;
    
    void* last_beat; /* Handle to timer */
    long bpm_prescale;

    float speed; /* Playback speed */

    int volumeleft; /* for volume */
    int volumeright;

}OutChannel;


long               AUDIOCHANNEL_BufferFree(int c);
int                AUDIOCHANNEL_Close(int c);
long               AUDIOCHANNEL_GetTimePlayed(int c);
int                AUDIOCHANNEL_GetVolume(int c, float *db);
int                AUDIOCHANNEL_GetVolumeLevel(int channel,int *left,int *right);
int                AUDIOCHANNEL_Mute (int c, int mute);
struct OutChannel *AUDIOCHANNEL_New();
int                AUDIOCHANNEL_Open(int c, AFormat fmt, int rate, int nch, int *max_bytes);
int                AUDIOCHANNEL_Pause (int c, int pause);
int                AUDIOCHANNEL_Read (struct OutChannel *Channel,int len);
int                AUDIOCHANNEL_SetSpeed(int channel, int speed);
int                AUDIOCHANNEL_SetVolume(int c, float db);
int                AUDIOCHANNEL_Write (int c, void* buf, int len);

#endif /* __AUDIO_CHANNEL_H__ */
