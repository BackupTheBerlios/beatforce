/*
  Beatforce/ Search window

  This window is a jump to file type of window
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

#include <config.h>
#include <string.h>
#include <malloc.h>

#include "songdb.h"
#include "player.h"
#include "playlist.h"
#include "mixer.h"
#include "osa.h"

#include "search_window.h"
#include "theme.h"


SDL_Widget *editwidget;
SDL_Widget *tablewidget;

static SDL_Surface *SEARCHWINDOW_Create();
static void SEARCHWINDOW_Search(void *data); /* This function is called when any key is pressed */
void searchwindow_PlayClicked(void *data);
void searchwindow_Play(void *data);
void songdb_searchstring(long row,char *dest);
int SEARCHWINDOW_EventHandler(SDL_Event event);

SDL_Window SEARCHWINDOW={ SEARCHWINDOW_EventHandler , NULL, NULL,NULL};


void SEARCHWINDOW_Open()
{
    if(SEARCHWINDOW.Surface == NULL)
    {
        SEARCHWINDOW.Surface=SEARCHWINDOW_Create();
    }
    SDL_WindowOpen(&SEARCHWINDOW);                

    SONGDB_FindEntry("");
    /* Fill the table with the entire search database */
    {
        char *string[1];
        int i;

        string[0]=malloc(255);
        
        for(i=0;i<SONGDB_GetNoOfSearchResults();i++)
        {
            songdb_searchstring(i,string[0]);
            SDL_TableAddRow(tablewidget,string);
        }
        SDL_WidgetRedrawEvent(tablewidget);
        free(string[0]);
    }
    SDL_WidgetSetFocus(editwidget);
}


static SDL_Surface *SEARCHWINDOW_Create()
{
    SDL_Surface *SearchWindow;
    SDL_Widget  *w;
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
        w=SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_PanelSetImage(w,IMG_Load(Image->filename));
        Image=Image->next;
    }


    tablewidget=SDL_WidgetCreate(SDL_TABLE,12,50,1000,540);
    SDL_WidgetPropertiesOf(tablewidget,SET_VISIBLE_COLUMNS, 1);
    SDL_WidgetPropertiesOf(tablewidget,SET_BG_COLOR,0x93c0d5);
    SDL_WidgetPropertiesOf(tablewidget,SET_FONT,THEME_Font("normal"));
    SDL_SignalConnect(tablewidget,"select-row",searchwindow_PlayClicked,tablewidget);
    SDL_SignalConnect(tablewidget,"activate",searchwindow_PlayClicked,tablewidget);

    editwidget=SDL_WidgetCreate(SDL_EDIT,312,20,400,25);
    SDL_WidgetPropertiesOf(editwidget,SET_FONT,THEME_Font("normal"));
    SDL_SignalConnect(editwidget,"changed",SEARCHWINDOW_Search,NULL);
    SDL_SignalConnect(editwidget,"activate",searchwindow_Play,NULL);
  
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
            SDL_WidgetPropertiesOf(editwidget,SET_CAPTION,"");
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

    strcpy(buffer,SDL_EditGetText(editwidget));

    SONGDB_FindEntry(buffer);

    SDL_TableDeleteAllRows(tablewidget);
    {
        char *string[1];
        int i;

        string[0]=malloc(255);
        for(i=0;i<SONGDB_GetNoOfSearchResults();i++)
        {
            songdb_searchstring(i,string[0]);
            SDL_TableAddRow(tablewidget,string);
        }
        SDL_WidgetRedrawEvent(tablewidget);
        free(string[0]);
    }
   
}

void searchwindow_PlayClicked(void *data)
{
    SDL_Table *table=(SDL_Table*)data;
    struct SongDBEntry * e;
    int player = 0;// is playing ?
    int autofade = 0;
    
    SDL_WidgetPropertiesOf(editwidget,SET_CAPTION,"");
    printf("PlayClicked\n");
    SDL_WindowClose();

    /* Get the current playlist entry */
    e = SONGDB_GetSearchEntry(table->HighlightedRow);

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
    printf("Play\n");
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
//        PLAYLIST_AddEntry(player,e);
        PLAYER_Load(0,e);
        PLAYER_Play(0);
        SONGDBUI_Play(0);
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

void songdb_searchstring(long row,char *dest)
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
        if(e->artist && e->title)
        {
            sprintf(dest,"%s - %s",e->artist,e->title);
        }
        else
        {
            sprintf(dest,"%s",strrchr(e->filename,'/')+1);
        }
    }
}




