/*
  Beatforce/ Playlist/Waitlist user interface

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003-2004 John Beuving (john.beuving@beatforce.org)

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
#include "config.h"

#define PLAYER1 0

//int previous;

#define MODULE_ID PLAYLIST_UI
#include "debug.h"

#if 0
static void PLAYLISTUI_GetEntry(long row,int column,char *dest);
static void PLAYLISTUI_EntryClicked(SDL_Table *table,SDL_Event *event);
#endif

SDL_Widget* PLAYLISTUI_CreateWindow(ThemePlaylist *pl)
{
    SDL_Widget *pui = NULL;

    TRACE("PLAYLISTUI_CreateWindow");
    if(pl == NULL)
        return pui;

    if(pl)
    {
        pui=SDL_WidgetCreate(SDL_TABLE,pl->Table->x,pl->Table->y,pl->Table->w,pl->Table->h);
        SDL_WidgetPropertiesOf(pui,SET_VISIBLE_COLUMNS, 1);
        SDL_WidgetPropertiesOf(pui,SET_FG_COLOR,WHITE);
        SDL_WidgetPropertiesOf(pui,SET_FONT,SDL_FontGet("normal"));
        //SDL_WidgetPropertiesOf(pui,SET_CALLBACK,SDL_CLICKED,PLAYLISTUI_EntryClicked,NULL);
    }
    return pui;
}



void PLAYLISTUI_Redraw(void *widget)
{

}

#if 0
static void PLAYLISTUI_GetEntry(long row,int column,char *dest)
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


static void PLAYLISTUI_EntryClicked(SDL_Table *table,SDL_Event *event)
{
    int player=0;// is playing ?
    struct PlEntry *pl;

    if(event->button.button == 3) //remove fromw aitlist right mousebutton down
    {
        pl=PLAYLIST_GetSong(PLAYER1,table->CurrentRow);
        if(pl && pl->e)
        {
            PLAYLIST_Remove(PLAYER1,pl->e);
            SDL_TableDeleteRow((SDL_Widget*)table,table->CurrentRow);
        }
        return;
    }

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
    {
        if(event->button.button == 1)//play now left mosuebutton down
        {
            PLAYER_SetSong(player,table->CurrentRow);
        }
    }
}
#endif



