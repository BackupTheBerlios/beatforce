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

#define PLAYER1 0

int previous;

extern SDL_Font *LargeBoldFont;

void playliststring(long row,int column,char *dest)
{
    struct PlEntry *pe;
    struct SongDBEntry *e;

    dest[0]='\0';

    if(SONGDB_GetNoOfEntries()<row)
        return;

    if(playlist_get_entries(PLAYER1) < row)
        return;
        
    pe = playlist_GetSong(PLAYER1,row);

    if(pe == NULL)
        return;

    e = pe->e; 
    if(e)
    {
        switch(column)
        {
        case 0:
            if(e->id)
            {
                sprintf(dest,"%ld",(e->id)+1);
            }
            else
            {
                sprintf(dest,"%ld",row+1);
            }
            break;
        case 1:
            if(e->artist && e->title)
            {
                sprintf(dest,"%s - %s",e->artist,e->title);
            }
            else
            {
                sprintf(dest,"%s",strrchr(e->filename,'/')+1);
            }
            break;
        case 2:
            if(e->time)
            {
                sprintf(dest,"%02d:%02d",(int)e->time/60000,
                        (int)((e->time%60000)/1000));
            }
            break;
        }
    }
}

void playlisteventhandler(SDL_Table *table)
{
    player_set_song(0,table->CurrentRow);
}

void PLAYLISTUI_CreateWindow()
{
    return;
    SDL_WidgetCreate(SDL_TABLE,10,150,380,140);

    SDL_WidgetProperties(SET_VISIBLE_ROWS,    10);
    SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
    
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION, playliststring);
    SDL_WidgetEventCallback(playlisteventhandler,SDL_CLICKED);
}



