/*
  Beatforce/ Playlist + Tab user interface

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
#include <malloc.h>

#include "config.h"
#include "SDLTk.h"
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


/* Prototypes for functions for buttosn below */
static void SONGDBUI_ChangeGroupClicked(void *data);


void songdbstring(long row,int column,char *dest);

static int activesong[2];
SongdbWidgets *Widgets;

void eventhandler(SDL_Table *table)
{
    struct SongDBEntry * e;

    /* Get the current playlist entry */
    e = SONGDB_GetEntryID(table->CurrentRow);
    if(e)
    {
        PLAYER_Load(0,e);
        PLAYER_Play(0);
    }
}

void testplay(SDL_Table *table)
{
    struct SongDBEntry * e;

    /* Get the current playlist entry */
    e = SONGDB_GetEntryID(table->HighlightedRow + table->FirstVisibleRow);
    
    if(PLAYER_IsPlaying(0))
        PLAYER_Pause(0);

    if(PLAYER_Load(0,e))
        PLAYER_Play(0);

    
    SDL_TableSetStyle((SDL_Widget*)table,table->HighlightedRow + table->FirstVisibleRow,0xff0000);
    SDL_TableEnsureRowVisible((SDL_Widget*)table,table->HighlightedRow + table->FirstVisibleRow);
    
}

void SONGDB_ChangeDatabase(SDL_Widget *widget)
{
    int w;
    struct SongDBSubgroup *sgl;
    sgl=SONGDB_GetSubgroupList();
    w=SDL_NotebookGetCurrentPage(widget);
    while(w--)
        sgl=sgl->next;
    SONGDB_SetActiveSubgroup(sgl);
}

void *SONGDBUI_CreateWindow(ThemeSongdb *ts)
{
    ThemeButton   *Button = NULL;
    SongdbWidgets *sw;

    if(ts == NULL)
        return NULL;

    sw=malloc(sizeof(SongdbWidgets));
    Button=ts->Button;
    Widgets=sw;
    SONGDB_Init ();

    if(ts)
    {
        /* Create the notebook/tab section below the table*/
        sw->Tabs=SDL_WidgetCreate(SDL_TAB,ts->Table->Rect.x-1,ts->Table->Rect.y,
                                  ts->Table->Rect.w+2,18+ts->Table->Rect.h);
        SDL_WidgetPropertiesOf(sw->Tabs,SET_FONT,THEME_Font("normal"));
        SDL_WidgetPropertiesOf(sw->Tabs,SET_BG_COLOR,0x93c0d5);

        SDL_SignalConnect(sw->Tabs,"switch-tab",SONGDB_ChangeDatabase,sw->Tabs);
    }

    /* Create buttons which change the tabs */
    while(Button)
    {
        switch(Button->action)
        {
            SDL_Widget *w;
        case BUTTON_CHANGE_DIR:
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_SignalConnect(w,"clicked",SONGDBUI_ChangeGroupClicked,NULL);        
            break;
        }
        Button=Button->next;
    }
    activesong[0]=-1;
    activesong[1]=-1;

    return sw;
}

void SONGDBUI_Redraw(void *w)
{
    SongdbWidgets *widgets=(SongdbWidgets*)w;
    SongDBSubgroup *sg;
    ThemeConfig *TC=THEME_GetActive();
    ThemeSongdb *ts=TC->MainWindow->Songdb;
    SDL_Widget *table;

    if(SONGDB_GroupChanged())
    {
        sg=SONGDB_GetSubgroupList();

        /* Remove all tabs */
        SDL_NotebookClear(widgets->Tabs);
        
        while(sg)
        {
            table = SDL_WidgetCreateR(SDL_TABLE,ts->Table->Rect);/*x y w h */
            SDL_WidgetPropertiesOf(table,SET_FONT,THEME_Font("small"));
            SDL_WidgetPropertiesOf(table,SET_VISIBLE_COLUMNS,3);
            
            SDL_TableSetColumnTitle(table,0,"ID");
            SDL_TableSetColumnTitle(table,1,"Song");
            SDL_TableSetColumnTitle(table,2,"Time");
            
            SDL_TableSetColumnWidth(table,0,30);
            SDL_TableSetColumnWidth(table,1,290);
            SDL_TableSetColumnWidth(table,2,200);
            SDL_WidgetPropertiesOf(table,SET_FG_COLOR,WHITE);

            SDL_SignalConnect(table,"select-row",eventhandler,table);
            SDL_SignalConnect(table,"activate",testplay,table);
            {
                char *titles[3];
                int r=0,c=0;
                for(c=0;c<3;c++)
                    titles[c]=malloc(255);

                SONGDB_SetActiveSubgroup(sg);
                for(r=0;r< sg->Songcount;r++)
                {
                    for(c=0;c<3;c++)
                    {
                        songdbstring(r,c,titles[c]);
                    }
                    SDL_TableAddRow(table,titles);
                }
                for(c=0;c<3;c++)
                    free(titles[c]);
            }
            SDL_NotebookAppendTab(widgets->Tabs,table,sg->Name);
            sg=sg->next;
        }
        sg=SONGDB_GetSubgroupList();
        SONGDB_SetActiveSubgroup(sg);
        SDL_WidgetSetFocus(SDL_NotebookGetChildWidget(Widgets->Tabs));
        SDL_WidgetRedrawEvent(widgets->Tabs);
    }
    
}

void SONGDBUI_Play(int player_nr)
{
    int tab;
    struct SongDBEntry *e;
    tab = SDL_NotebookGetCurrentPage(Widgets->Tabs);

    PLAYER_GetPlayingEntry(player_nr,&e);
    tab = SONGDB_FindSubgroup(e);
    SDL_NotebookSetCurrentTab(Widgets->Tabs,tab);
    if(e)
    {
        SDL_TableSetStyle(SDL_NotebookGetChildWidget(Widgets->Tabs),e->id,0xff0000);
        SDL_TableEnsureRowVisible(SDL_NotebookGetChildWidget(Widgets->Tabs),e->id);
        SDL_WidgetSetFocus(SDL_NotebookGetChildWidget(Widgets->Tabs));
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
