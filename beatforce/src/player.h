/*
   BeatForce
   player.h  - player
   
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

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "input_plugin.h"
#include "llist.h"
#include "songdb.h"

#define PLAYER1 0 
#define PLAYER2 1

typedef enum ePlayerState
{
    PLAYER_IDLE = 1,
    PLAYER_PAUSE,
    PLAYER_PLAY,
    PLAYER_PAUSE_EOF,
}ePlayerState;


typedef int PlayerState; 

struct PlayerPrivate
{
    int playlist_id;             /* no of song playing ( in playlist ) 0, if not in playlist */
    unsigned long songdb_id;     /* id of song playing  SONGDB_ID_UNKNOWN, if no entry */
    
    struct SongDBEntry *e;
    ePlayerState State;           /* State of the player */
    
    float pitch;                  /* current pitch */
    int pitch_on;                 /* 1, if pitch on */
    int pitch_range;              /* pitch range: 1, 2, 3 */
    
    int shuffle;                  /* shuffle after song ? */
    int repeat;                   /* repeat playlist */

    int auto_remove;              /* auto remove played songs */

    BFList *ip_plugins;            /* linked list of initialised input plugins */
    InputPluginData *current_plugin;        /* pointer to current plugin in list */

    int ch_id;
    
    int eof;

};


#define PLAYER_PRIVATE_LEN		(sizeof( struct PlayerPrivate ))

struct PlayerPrivate *PLAYER_GetData(int player_nr);

int PLAYER_Init (int, PlayerConfig *);
int player_finalize (int);

int PLAYER_EOF(int);


int player_set_callback(void*);


void player_rev  (int, int);
void player_forw (int, int);



int PLAYER_IsPlaying(int);

int PLAYER_GetPlayingID(int player_nr,long *songid);
void player_set_song (int player_nr, int no);

int PLAYER_SetTimePlayed(int player_nr,long seconds);
int PLAYER_SetSpeed(int player_nr,double speed);

/* Functions for user interface information */
int PLAYER_GetArtist(int player_nr,char *artist);
int PLAYER_GetTitle(int player_nr,char *title);
int PLAYER_GetFilename(int player_nr,char *filename);

int PLAYER_GetState(int player_nr);

long PLAYER_GetTimeTotal(int player_nr);
long PLAYER_GetTimeLeft(int player_nr);
long PLAYER_GetTimePlayed(int player_nr);

int PLAYER_GetBitrate(int player_nr);
int PLAYER_GetSamplerate(int player_nr);

/* Change file information */
int PLAYER_SetTitle(int player_nr,char *title);




int PLAYER_Cue(int player_nr);
int PLAYER_Play(int player_nr);
int PLAYER_Pause(int player_nr);

#endif
