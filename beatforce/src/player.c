/*
   BeatForce
   player.c  -	player
   
   Copyright (c) 2003, John Beuving (john.beuving@home.nl)

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
#include <sys/stat.h>

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>


#include "osa.h"
#include "player.h"
#include "err.h"
#include "playlist.h"
#include "plugin.h"
#include "mixer.h"
#include "input.h"
#include "output.h"

#define MODULE_ID PLAYER
#include "debug.h"

struct PlayerPrivate *playerdata[3];

int player_load (int);


//prototypes
unsigned int player_timeout (unsigned int interval,void *data);
void player_set_songfield (int, char *, char *);

void player_PlayPauses (int player_nr, int play);

void player_SetData(int player_nr, struct PlayerPrivate *p)
{
    if(player_nr > 2 || player_nr < 0)
        printf("Wrong player_nr\n");

    playerdata[player_nr]=p;
}


struct PlayerPrivate *PLAYER_GetData(int player_nr)
{
    return playerdata[player_nr];
}


int PLAYER_Init(int player_nr, PlayerConfig * cfg)
{
    struct PlayerPrivate *player;

    player = malloc (PLAYER_PRIVATE_LEN);
    memset (player, 0, PLAYER_PRIVATE_LEN);

    player->songdb_id     = SONGDB_ID_UNKNOWN;
    player->ch_id = player_nr;
    player->State = PLAYER_IDLE;
    
    player_SetData(player_nr,player);

    player->ip_plugins=INPUT_Init (player_nr, PLUGIN_GetList(PLUGIN_TYPE_INPUT));
    PLAYLIST_Init (player_nr);
  
    return 0;
}

int PLAYER_Exit(int player_nr)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    if(p==NULL)
        return -1;

    PLAYER_Pause(player_nr);
    OSA_RemoveTimer(p->timeout);
    return 0;
}

int PLAYER_GetPlayingID(int player_nr,long *songid)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    if (p == NULL || songid == NULL)
        return 0;
    
    if(songid)
        *songid=p->songdb_id;

    return 1;
}

int
player_do_songchange (int player_nr)
{
    if (MIXER_FadeInProgress())
    {
        player_next_track (player_nr);
    }
    else if (MIXER_GetAutofade)
    {
        /* we're already at eof, fade instantly !! */
        MIXER_DoFade(1,0);
    }
    else
    {
        player_next_track (player_nr);
        PLAYER_Play(player_nr);
    }
    return 0;
}

int PLAYER_EOF(int player_nr)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    if (p == NULL)
        return -1;

    p->eof = 1;
   
    return 0;
    
}

unsigned int
player_timeout (unsigned int interval,void *data)
{
    struct PlayerPrivate *p = data;
    long songtime;

    if (p != NULL && p->current_plugin)
    {
        struct SongDBEntry *e = SONGDB_GetEntryID(p->songdb_id);
        songtime = INPUT_GetTime (p->current_plugin);
        if (songtime >= 0 || songtime == ERROR_EOF)
        {
            if (e != NULL)
            {
                if (songtime == ERROR_EOF || songtime >= e->time || p->eof)
                {
                    p->eof = 0;
                    player_do_songchange (p->ch_id);
                    p->eof = 0;
                    return 1;
                }

                if (((e->time - songtime) <= (MIXER_GetFadeTime()))
                    && (MIXER_GetAutofade()) && !MIXER_FadeInProgress ())
                {
                    MIXER_DoFade(1,0);
                }
            }
        }

    }
    return interval;
}

void
player_next_track (int player_nr)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    int new_no = 0;

    if(p==NULL)
        return;

    new_no = p->playlist_id + 1;
    player_set_song (player_nr, new_no);
}

void
player_track_rf (int player_nr, int rev_forw)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    int new_no = 0;

    if(p==NULL)
        return;

    if (rev_forw)
    {
        if (PLAYER_IsPlaying(player_nr))
        {
            new_no = p->playlist_id + 1;
            player_set_song (player_nr, new_no);
            PLAYER_Play(p->ch_id);
            return;
        }
        else
        {
            new_no = p->playlist_id + 1;
        }
    }
    else
    {
        if (PLAYER_IsPlaying(player_nr))
        {
            long songtime = INPUT_GetTime (p->current_plugin);

            if (songtime <= 1000)
            {
                /* go to previous song */
                new_no = p->playlist_id - 1;
                if (new_no <= 0)
                {
                    new_no = p->playlist_id;
                }
                player_set_song (p->ch_id, new_no);
                PLAYER_Play(p->ch_id);
                return;
            }
            else
            {
                /* just restart song from beginning */
                player_set_song (p->ch_id, p->playlist_id);
                PLAYER_Play(p->ch_id);
                return;
            }

        }
        else
        {
            new_no = p->playlist_id - 1;
            if (new_no <= 0)
            {
                new_no = p->playlist_id;
            }
        }
    }

    player_set_song (p->ch_id, new_no);
}

int PLAYER_Play(int player_nr)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    if(p == NULL)
    {
        ERROR("No player data available");
        return 0;
    }
    if(p->State == PLAYER_PLAY)
    {
        ERROR("Wrong play state");
        return 0;
    }

    if(p->songdb_id != SONGDB_ID_UNKNOWN)
    {
        if(INPUT_Play (p->current_plugin))
        {
            PLAYLIST_Remove(player_nr,p->songdb_id);
            p->State = PLAYER_PLAY;
        }
        else
        {
            ERROR("Invalid data for decoder");
            return 0; /* Invalid data for decoder */
        }
    }
    else
    {
        ERROR("No song loaded");
        return 0; /* No song loaded */
    }
    return 1;
}

int PLAYER_Pause(int player_nr)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    if(p == NULL)
        return 0;
    TRACE("PLAYER_Pause %d",player_nr);


    if(p->State == PLAYER_PAUSE)
    {
        return 0;
    }

    if(INPUT_Pause (p->current_plugin))
    {
        p->State = PLAYER_PAUSE;
    }
    return 1;
}

int PLAYER_Cue(int player_nr)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    if(p == NULL)
        return 0;
    TRACE("PLAYER_Cue %d",player_nr);

    return 1;

}

int PLAYER_IsPlaying (int player_nr)
{
    struct PlayerPrivate *p=PLAYER_GetData(player_nr);
   
    if(p==NULL)
        return 0;

    if(p->State == PLAYER_PLAY)
        return TRUE;
    else
        return FALSE;
}

/* set song with playing id no as actrive (load it) */
void player_set_song (int player_nr, int no)
{
    struct PlEntry *pe;
    struct PlayerPrivate *p;
    int ent, err = 0;

    TRACE("PLAYER_SetSong enter %d,%d",player_nr,no);
    p = PLAYER_GetData(player_nr);
    if(p==NULL)
        return;

    ent =  PLAYLIST_GetNoOfEntries(player_nr);
    if (ent == 0)
    {
        printf ("playlist has no entries.\n");
        p->songdb_id     = SONGDB_ID_UNKNOWN;
        p->playlist_id     = 0;
        player_load (player_nr);
        return;
    }

    if (ent < no)
    {
        player_set_song (player_nr, 1);
        return;
    }

    pe = PLAYLIST_GetSong (player_nr, no);
    if(pe == NULL)
    {
        ERROR("Nothing loaded");
        exit(1);
        return;
    }
    p->playlist_id     = no;
    
   
    INPUT_GetTag(p->ip_plugins,pe->e->filename,pe->e);
    p->songdb_id = pe->e->id;
    err = player_load (player_nr);
    if (err)
    {
        printf ("player_set_song: error loading song ID %ld. Retrying...\n",
                pe->e->id);
        printf ("player_set_song: error loading song ID %ld. Retrying...\n",
                pe->e->id);
        printf ("player_set_song: error loading song ID %ld. Retrying...\n",
                pe->e->id);
        printf ("player_set_song: error loading song ID %ld. Retrying...\n",
                pe->e->id);
        printf ("player_set_song: error loading song ID %ld. Retrying...\n",
                pe->e->id);
        err = player_load (player_nr);	/* 2nd try */
    }

    if (err)
    {
        printf ("player_set_song: error loading song ID %ld.", pe->e->id);

        if (PLAYLIST_GetNoOfEntries (player_nr) > 1)
        {
            player_set_song (player_nr, no + 1);
            printf ("trying next!\n");
        }
        else
            printf ("no more songs\n");

    }
//    else
//        playlist_do_renumber (p->PlayerWindow);
    TRACE("PLAYER_SetSong <- %d",player_nr);

}

/* loads a song set by playing_id */
int player_load (int player_nr)
{
    int err = 0;
    int cerr = 0;
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    struct SongDBEntry *e;

    TRACE("player_load enter");
    if(p==NULL)
        return 0;
    
    if (p->playlist_id == 0 && p->songdb_id == SONGDB_ID_UNKNOWN)
    {
        printf ("no song to play\n");
        return 0;
    }

    if (! (cerr = input_close_file (p->current_plugin)) )
    {
        p->State = PLAYER_PAUSE;
        e   =  SONGDB_GetEntryID (p->songdb_id);
        if(e)
            err =  INPUT_LoadFile (player_nr, e);
        else
            ERROR("No such entry");
        if (err)
        {
            fprintf (stderr, "Error 0x%x(%d) loading song id %ld: %s\n", err, err,
                     p->songdb_id, e->filename);
            return -1;
        }
    }
    else
    {
        if (cerr == ERROR_NOT_OPEN || cerr == ERROR_NO_FILE_LOADED || cerr == ERROR_EOF)
        {
            printf("No file opened\n");
            return 0;
        }

        fprintf (stderr,
                 "Error stopping and closing currently playing song: 0x%x!\n",
                 -cerr);
        return -2;
    }
    TRACE("player_load leave");
    return 0;

}
int PLAYER_GetState(int player_nr)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    if(p == NULL)
        return 0;
    
    return p->State;
}

int PLAYER_GetArtist(int player_nr,char *label)
{
    struct SongDBEntry *e;
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    e=SONGDB_GetEntryID(p->songdb_id);

    if(e && e->artist)
    {
        sprintf(label,"%s",e->artist);
        return 1;
    }
    return 0;
}

int PLAYER_GetTitle(int player_nr,char *label)
{
    struct SongDBEntry *e;
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    e=SONGDB_GetEntryID(p->songdb_id);
    
    if(e && e->title)
    {
        sprintf(label,"%s",e->title);
        return 1;
    }
    return 0;
}

int PLAYER_SetTitle(int player_nr,char *title)
{
    struct SongDBEntry *e;
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    e = SONGDB_GetEntryID(p->songdb_id);
    
    if(e)
    {
        if(e->title)
            free(e->title);
        e->title=strdup(title);
        return 1;
    }
    return 0;

}

/* This functions return the filename with the filepath */
int PLAYER_GetFilename(int player_nr,char *filename)
{
    struct SongDBEntry *e;
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    e = SONGDB_GetEntryID(p->songdb_id);
    
    if(e && e->filename)
    {
        sprintf(filename,"%s",e->filename);
        return 1;
    }
    return 0;
}

long PLAYER_GetTimeTotal(int player_nr)
{
    struct SongDBEntry *e;
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    e=SONGDB_GetEntryID(p->songdb_id);

    if(e)
        return e->time;
    else
        return 0;
}

long PLAYER_GetTimePlayed(int player_nr)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    if(p->current_plugin)
    {
        return INPUT_GetTime (p->current_plugin);
    }
    return 0;
}

long PLAYER_GetTimeLeft(int player_nr)
{
    long t=0;
    struct SongDBEntry *e;
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    e=SONGDB_GetEntryID(p->songdb_id);
    if(e)
    {
        t = e->time - INPUT_GetTime(p->current_plugin);
    }
    return t;
}

int PLAYER_SetTimePlayed(int player_nr,long seconds)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    INPUT_Seek(p->current_plugin, seconds*1000);
    return 0;
}

int PLAYER_GetBitrate(int player_nr)
{
    struct SongDBEntry *e;
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    e=SONGDB_GetEntryID(p->songdb_id);    

    if(e && e->AddInfo)
    {
        return e->AddInfo->bitrate;
    }
    
    return 0;
}

int PLAYER_GetSamplerate(int player_nr)
{
    long t=0;
    struct SongDBEntry *e;
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    e=SONGDB_GetEntryID(p->songdb_id);

    if(e && e->AddInfo)
    {
        return e->AddInfo->SampleRate;
    }
    return t;
}

int PLAYER_SetSpeed(double speed)
{
    return 0;
}



