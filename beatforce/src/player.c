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





static void PLAYER_StorePlayerData(int player_nr, struct PlayerPrivate *p)
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
    player->e     = NULL;
    
    PLAYER_StorePlayerData(player_nr,player);

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
    return 0;
}

int PLAYER_GetPlayingEntry(int player_nr,struct SongDBEntry **e)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);

    if (p == NULL || e == NULL)
        return 0;
    
    *e=p->e;

    return 1;
}

int PLAYER_EOF(int player_nr)
{
    struct PlayerPrivate *p = PLAYER_GetData(player_nr);
    if (p == NULL)
        return -1;

    p->eof = 1;
    p->State = PLAYER_PAUSE_EOF;
    return 0;
  
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
    p->e         = pe->e;
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
        return 0;
    }

    if (! (cerr = input_close_file (p->current_plugin)) )
    {
        p->State = PLAYER_PAUSE;
        e   =  SONGDB_GetEntryID (p->songdb_id);
        if(e)
        {
            err =  INPUT_LoadFile (player_nr, p->e);
        }
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

    e=p->e;

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

    e=p->e;
    
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

    e=p->e;
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

    e=p->e;
    
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

    e=p->e;
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

    e=p->e;
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

    e=p->e;

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

    e=p->e;

    if(e && e->AddInfo)
    {
        return e->AddInfo->SampleRate;
    }
    return t;
}

int PLAYER_SetSpeed(int player_nr,double speed)
{
    AUDIOOUTPUT_SetSpeed(player_nr,(float)speed);
    return 1;
}



