/*
  Beatforce/ Search window

  This window is a jump to file type of window
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

#include <config.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL_Window.h>
#include <SDL_Font.h>
#include <SDL_Table.h>

#include "songdb_ui.h"
#include "player_ui.h"
#include "mixer_ui.h"

#include "songdb.h"
#include "player.h"
#include "playlist.h"
#include "mixer.h"
#include "osa.h"

#include "search_window.h"
#include "wndmgr.h"
#include "theme.h"



void *editwidget;
void *tablewidget;

static SDL_Surface *SEARCHWINDOW_Create();
static void SEARCHWINDOW_Search(void *data); /* This function is called when any key is pressed */
void searchwindow_PlayClicked(void *data);
void searchwindow_Play(void *data);
void songdb_searchstring(long row,int column,char *dest);
int SEARCHWINDOW_EventHandler(SDL_Event event);

SDL_Window SEARCHWINDOW={ SEARCHWINDOW_EventHandler , NULL, NULL,NULL};


void SEARCHWINDOW_Open()
{
    if(SEARCHWINDOW.Surface == NULL)
    {
        SONGDB_FindEntry("");
        SEARCHWINDOW.Surface=SEARCHWINDOW_Create();
    }
    SDL_WindowOpen(&SEARCHWINDOW);                
}


static SDL_Surface *SEARCHWINDOW_Create()
{
    SDL_Surface *SearchWindow;
    ThemeConfig *tc=THEME_GetActive();
    ThemeSearchWindow *sw =NULL;
    ThemeImage    *Image = NULL;
    if(tc == NULL)
        return NULL;

    sw=tc->SearchWindow;
    
    if(sw == NULL)
        return NULL;
    Image=sw->Image;


    SearchWindow = SDL_WidgetNewSurface(1024,685,32);

    while(Image)
    {
        SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_WidgetProperties(SET_IMAGE,IMG_Load(Image->filename));
        Image=Image->next;
    }

    tablewidget=SDL_WidgetCreate(SDL_TABLE,12,50,1000,540);
    SDL_WidgetProperties(SET_VISIBLE_ROWS,    36);
    SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
    SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
    SDL_WidgetProperties(ROWS,SONGDB_GetNoOfEntries());
    SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
    SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,songdb_searchstring);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,searchwindow_PlayClicked,NULL);
    SDL_WidgetProperties(SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.jpg"));

    editwidget=SDL_WidgetCreate(SDL_EDIT,312,20,400,25);
    SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
    SDL_WidgetProperties(SET_ALWAYS_FOCUS,1);
    SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_ANY,SEARCHWINDOW_Search);
    SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN,searchwindow_Play);

    return SearchWindow;
}


int SEARCHWINDOW_EventHandler(SDL_Event event)
{
    switch(event.type)
    {
    case SDL_KEYDOWN:
        switch( event.key.keysym.sym ) 
        {
        case SDLK_ESCAPE:
            SDL_WindowClose();
            break;
            
        default:
            break;
            
        }
        break;

    }
    return 0;
}



static void SEARCHWINDOW_Search(void *data)
{
    char buffer[255];
    SDL_WidgetPropertiesOf(editwidget,GET_CAPTION,&buffer);
    SONGDB_FindEntry(buffer);
    SDL_WidgetPropertiesOf(tablewidget,ROWS,SONGDB_GetNoOfSearchResults());
}

void searchwindow_PlayClicked(void *data)
{
    SDL_Table *table=(SDL_Table*)data;
    struct SongDBEntry * e;
    int player = 0;// is playing ?
    int autofade = 0;
    
    SDL_WidgetPropertiesOf(editwidget,SET_CAPTION,"");
    SDL_WindowClose();

    /* Get the current playlist entry */
    e = SONGDB_GetSearchEntry(table->CurrentRow);

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
        
        PLAYLIST_AddEntry(player,e);
        PLAYER_SetSong(player,0);  // when set_entry is excecuted we only have 1 item thus 0
        
        if(autofade == 1)
            MIXER_DoFade(1,0);
        else
            PLAYER_Play(player);

        SONGDBUI_Play(player);
        /* and the search results */
        SONGDB_FindEntry("");
    }
}

void searchwindow_Play(void *data)
{
    struct SongDBEntry * e;
    int player = 0;// is playing ?
    int autofade = 0;

    /* Reset the edit window */
    /* Close this window. main UI can determine to open the correct one again */
    SDL_WidgetPropertiesOf(editwidget,SET_CAPTION,"");
    SDL_WindowClose();

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

        /* Add the song to the bottom of the playlist */
        PLAYLIST_AddEntry(player,e);
        
#if 0
        player_set_song(player,0);  // when set_entry is excecuted we only have 1 item thus 0
        
        if(autofade == 1)
            MIXER_DoFade(1,0);
        else
            PLAYER_Play(player);
#endif
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




