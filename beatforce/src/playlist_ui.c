/*
  Beatforce/ Playlist/Waitlist user interface

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003 John Beuving (john.beuving@home.nl)

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

#include <stdio.h>
#include <string.h>
#include "SDL_Widget.h"
#include "SDL_Table.h"
#include "SDL_Font.h"

#include "songdb.h"
#include "playlist.h"
#include "player.h"
#include "theme.h"

#define PLAYER1 0

int previous;



void playliststring(long row,int column,char *dest)
{
    struct PlEntry *pe;
    struct SongDBEntry *e;


    dest[0]='\0';

    if(PLAYLIST_GetNoOfEntries(PLAYER1) < row)
        return;
        
    pe = PLAYLIST_GetSong(PLAYER1,row);

    if(pe == NULL)
        return;

    e = pe->e; 
    if(e)
    {
        switch(column)
        {
        case 0:
            if(e->artist && e->title)
            {
                sprintf(dest,"%s - %s",e->artist,e->title);
            }
            else
            {
                sprintf(dest,"%s",strrchr(e->filename,'/')+1);
            }
            break;
        }
    }
}

void playlisteventhandler(SDL_Table *table)
{
    int player=0;// is playing ?

    if(PLAYER_IsPlaying(0))
        player=1;

    if(PLAYER_IsPlaying(1))
    {
        if(player!=1)
            player=0;
        else
            player=-1;
    }

    if(player == -1)
        printf("Can't set song to one of the players\n");
    else
        player_set_song(player,table->CurrentRow);

}


void PLAYLISTUI_CreateWindow(ThemePlaylist *pl)
{
    if(pl == NULL)
        return;

    if(pl)
    {
        SDL_WidgetCreateR(SDL_TABLE,pl->Table->Rect);

        SDL_WidgetProperties(SET_VISIBLE_ROWS,    16);
        SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
        SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
        SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
        SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION, playliststring);
        SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,playlisteventhandler,NULL);
    }
}



