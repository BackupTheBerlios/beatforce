/*
  Beatforce/ Player user interface

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003-2004 John Beuving (john.beuving@wanadoo.nl)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __PLAYER_UI_H__
#define __PLAYER_UI_H__

#include <gtk/gtk.h>
#include "llist.h"

/* Public routines */

/* 
 * Create a player
 */
void PLAYERUI_CreateWindow(int nr,void *window);


void PLAYERUI_Redraw();

enum 
{
    PLAYERUI_STATE_NORMAL,
    PLAYERUI_STATE_INFO
};


typedef struct PlayerDisplayNormal
{
    GtkWidget *ButtonPause;
    GtkWidget *ButtonPlay;
    GtkWidget *ButtonInfo;
    
    GtkWidget *TimeElapsed;
    GtkWidget *TimeRemaining;

    GtkWidget *Title;
    GtkWidget *Artist;
    char       sArtist[255];
    GtkWidget *PlayerState;
    GtkWidget *Samplerate;
    GtkWidget *Bitrate;
    GtkWidget *SongProgress; 
    GtkWidget *VolumeLeft;
    GtkWidget *VolumeRight;

    GtkWidget *Pitch;
}PlayerDisplayNormal;

typedef struct PlayerDisplayInfo
{
   GtkWidget *EditTitle;
}PlayerDisplayInfo;

/* Player ui data structures */
typedef struct PlayerDisplay
{
    int PlayerNr;
    int State;

    PlayerDisplayNormal Normal;
    PlayerDisplayInfo   Info;
    BFList *Images;
    
}PlayerDisplay;



#endif /* __PLAYER_UI_H__ */
