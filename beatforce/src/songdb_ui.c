/*
  Beatforce/ Playlist + Tab user interface

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
#include <malloc.h>

#include "wndmgr.h"
#include "SDL_Widget.h"
#include "SDL_Font.h"
#include "SDL_Table.h"
#include "SDL_Tab.h"
#include "songdb.h"
#include "songdb_ui.h"
#include "player.h"
#include "playlist.h"
#include "configfile.h"
#include "osa.h"

#include "playlist_ui.h"
#include "player_ui.h"   /* for UI_PlayerSetArtistTitle() */

#include "file_window.h"
#include "theme.h"

#define MODULE_ID SONGDB_UI
#include "debug.h"

extern SongDBGroup *MainGroup;

/* Prototypes for functions for buttosn below */
static void SONGDBUI_ChangeGroupClicked(void *data);

void UI_SongdbRenameClicked(void *data);
void UI_SongdbAddTabClicked(void *data);
void songdbui_RemoveTab(void *data);

static void SONGDBUI_ChangeDatabase();

void songdbstring(long row,int column,char *dest);
void *table;
void *tabwidget;

void eventhandler(SDL_Table *table)
{
    struct SongDBEntry * e;

    /* Get the current playlist entry */
    e = SONGDB_GetEntryID(table->CurrentRow);
    PLAYLIST_SetEntry(0,e);
}

void SONGDBUI_CreateWindow(ThemeSongdb *ts)
{
    ThemeButton *Button = NULL;

    if(ts == NULL)
        return;

    Button=ts->Button;

    if(ts)
    {
        /* Create the large table (songdb)*/
        table=SDL_WidgetCreateR(SDL_TABLE,ts->Table->Rect);/*x y w h */
        SDL_WidgetProperties(SET_VISIBLE_ROWS,    ts->Table->Rows);
        SDL_WidgetProperties(SET_VISIBLE_COLUMNS, ts->Table->Columns);
        SDL_WidgetProperties(COLUMN_WIDTH, 1, 30  );
        SDL_WidgetProperties(COLUMN_WIDTH, 2, 290 );
        SDL_WidgetProperties(COLUMN_WIDTH, 3, 60  );
        SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
        SDL_WidgetProperties(ROWS,SONGDB_GetNoOfEntries());
        SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
        SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,songdbstring);
        SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,eventhandler,table);
    }

    /* Craete the tab section below the table*/
    tabwidget=SDL_WidgetCreate(SDL_TAB,30,580,500,20);
    SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
    SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,SONGDBUI_ChangeDatabase,NULL);


    /* Create buttons which change the tabs */
    while(Button)
    {
        switch(Button->action)
        {
        case BUTTON_CHANGE_DIR:
            //add dir to highlighted tab
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,SONGDBUI_ChangeGroupClicked,NULL);        
            break;
        }
        Button=Button->next;

    }
}

static void SONGDBUI_ChangeDatabase()
{
    SDL_Tab *t;
    
    t=(SDL_Tab *)tabwidget;
    if(t && t->hl)
    {
        SONGDB_SetActiveSubgroup(t->hl->index);
    }
    SDL_WidgetPropertiesOf(table,ROWS,SONGDB_GetNoOfEntries());
    
    
}

void SONGDBUI_Redraw()
{
        
    SongDBSubgroup *sg;
    
    if(SONGDB_GroupChanged())
    {
        sg=SONGDB_GetSubgroup(0);
        
        while(SDL_WidgetPropertiesOf(tabwidget,TAB_REMOVE,NULL))
        {
        }
        
        while(sg)
        {
            SDL_WidgetPropertiesOf(tabwidget,TAB_ADD,sg->Name);
            sg=sg->next;
        }
        SONGDBUI_ChangeDatabase();
    }
    


}

static void SONGDBUI_ChangeGroupClicked(void *data)
{
    FILEWINDOW_Open();
}

void songdbstring(long row,int column,char *dest)
{
    struct SongDBEntry *e = NULL;
    
    if(row < 0)
        return;
    
    if(row < SONGDB_GetNoOfEntries())
    {
        e = SONGDB_GetEntryID(row);
    }
    
    dest[0]='\0';
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
