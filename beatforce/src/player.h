/*
   BeatForce
   player.h  - player
   
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

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "input_plugin.h"
#include "llist.h"


#define PLAYER1 0 
#define PLAYER2 1

typedef enum ePlayerState
{
    PLAYER_IDLE,
    PLAYER_PAUSE,
    PLAYER_PLAY
}ePlayerState;


typedef int PlayerState; 

struct PlayerPrivate
{
    int playing_no;               /* no of song playing ( in playlist ) 0, if not in playlist */

    unsigned long playing_id;     /* id of song playing  SONGDB_ID_UNKNOWN, if no entry */
    unsigned int  songdb_index;

    long playing_unique;
    
    int play;
    
    ePlayerState State;
    
    
    float pitch;                  /* current pitch */
    int pitch_on;                 /* 1, if pitch on */
    int pitch_range;              /* pitch range: 1, 2, 3 */
    
    int shuffle;                  /* shuffle after song ? */
    int repeat;                   /* repeat playlist */

    int auto_remove;              /* auto remove played songs */

    BFList *ip_plugins;            /* linked list of input plugins */
    InputPluginData *current_plugin;        /* pointer to current plugin in list */

    void *eventcallback;
    
    struct PlEntry *playlist;

    int ch_id;

    unsigned int timeout;                /* timeout for various things */
    
    int eof;

};


#define PLAYER_PRIVATE_LEN		(sizeof( struct PlayerPrivate ))

struct PlayerPrivate *object_get_data(int player_nr);

int PLAYER_Init (int, PlayerConfig *);
int player_finalize (int);

int player_eof (int);


int player_set_callback(void*);

void player_next_track (int);

void player_track_rf (int,int);
void player_rev  (int, int);
void player_forw (int, int);



int PLAYER_IsPlaying(int);
int player_load (int);

int player_get_song(int player_nr,long *songid);
void player_set_song (int player_nr, int no);

void player_set_playing (int, int, unsigned long);
void player_update_trackno (int);


void player_get_songfield(int,char *);
long PLAYER_GetTimeTotal(int player_nr);
long PLAYER_GetTimeLeft(int player_nr);
long PLAYER_GetTimePlayed(int player_nr);
int PLAYER_SetTimePlayed(int player_nr,long seconds);




void PLAYER_Play(int player_nr);
void PLAYER_Pause(int player_nr);

#endif
