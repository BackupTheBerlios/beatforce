/*
  Beatforce/ Search window

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
#include <string.h>

#include <SDL/SDL.h>
#include <SDL_Widget.h>
#include <SDL_Font.h>

#include "songdb_ui.h"
#include "player_ui.h"
#include "mixer_ui.h"

#include "songdb.h"
#include "player.h"
#include "playlist.h"
#include "mixer.h"
#include "osa.h"

#include "search_window.h"
#include "main_ui.h"


extern SDL_Font *LargeBoldFont;
void *editwidget;
void *tablewidget;

void searchfor(void *arg);
void playsong();
void songdb_searchstring(long row,int column,char *dest);

SDL_Surface *SearchWindow;

void SEARCHWINDOW_Init()
{
    SearchWindow = NULL;
}

void SEARCHWINDOW_Open()
{
    if(SearchWindow == NULL)
    {
        SDL_WidgetLOCK();
        SONGDB_FindEntry("");
        SearchWindow=SEARCHWINDOW_Create();
        SDL_WidgetUNLOCK();
    }
    else
    {
        SDL_WidgetUseSurface(SearchWindow);
    }
}

int Window_SearchWindowActive()
{
    return (SDL_WidgetGetActiveSurface() == SearchWindow);
}

SDL_Surface *SEARCHWINDOW_Create()
{
    SDL_Surface *SearchWindow;

    SearchWindow = SDL_CreateRGBSurface(SDL_SWSURFACE,1000,600,32,0xff0000,0x00ff00,0x0000ff,0x000000);

    SDL_WidgetUseSurface(SearchWindow);
 
    SDL_WidgetCreate(SDL_PANEL,0,0,1000,600);
    SDL_WidgetProperties(SET_BG_COLOR,0x000fff);
//    SDL_WidgetProperties(SET_NORMAL_IMAGE,"res/backg.jpg");
    
    tablewidget=SDL_WidgetCreate(SDL_TABLE,5,50,790,540);
    SDL_WidgetProperties(SET_VISIBLE_ROWS,    36);
    SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
    SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
    SDL_WidgetProperties(ROWS,SONGDB_GetNoOfEntries());
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,songdb_searchstring);

    editwidget=SDL_WidgetCreate(SDL_EDIT,200,20,400,25);
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_ALWAYS_FOCUS,1);
    SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_ANY,searchfor);
    SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN,playsong);

    return SearchWindow;
}



void searchfor(void *arg)
{
    char buffer[255];
    SDL_WidgetPropertiesOf(editwidget,GET_CAPTION,&buffer);
    SONGDB_FindEntry(buffer);
    SDL_WidgetPropertiesOf(tablewidget,ROWS,SONGDB_GetNoOfSearchResults());
}

void playsong()
{
    struct SongDBEntry * e;
    int player = 0;// is playing ?
    int autofade = 0;

    /* Reset the edit window */
/* Close this window. main UI can determine to open the correct one again */
    SDL_WidgetPropertiesOf(editwidget,SET_CAPTION,"");
    MAINUI_CloseWindow(SDL_WidgetGetActiveSurface());

    /* Get the current playlist entry */
    e = SONGDB_GetSearchEntry(0);

    if(e)
    {
        if(PLAYER_IsPlaying(0))
        {
            autofade++;
            player=1;
        }
        
        if(PLAYER_IsPlaying(1))
        {
            autofade++;
            if(player!=1)
                player=0;
        }
        
        PLAYLIST_SetEntry(player,e);
        player_set_song(player,0);  // when set_entry is excecuted we only have 1 item thus 0
        
        if(autofade == 1)
            MIXER_DoFade(1,0);
        else
            PLAYER_Play(player);

        /* and the search results */
        SONGDB_FindEntry("");

                

    }
    else
    {
        printf("Can;t find search entry\n");
    }
}

void songdb_searchstring(long row,int column,char *dest)
{
    struct SongDBEntry *e;

    if(row < 0)
        return;

    if(SONGDB_GetNoOfSearchResults() == 0)
        return;

    if(SONGDB_GetNoOfSearchResults() < row)
        return;
    
    e = SONGDB_GetSearchEntry(row);

    dest[0]='\0';
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




