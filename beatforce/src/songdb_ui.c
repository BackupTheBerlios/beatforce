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

static void SONGDBUI_ChangeDatabase(void *table,void *tabwidget);
static int SONGDBUI_GetHighlightedTab(void *tabwidget);
static int SONGDBUI_SetHighlightedTab(int which);

void songdbstring(long row,int column,char *dest);

static int activesong[2];
SongdbWidgets *Widgets;

void eventhandler(SDL_Table *table)
{
    struct SongDBEntry * e;

    /* Get the current playlist entry */
    e = SONGDB_GetEntryID(table->CurrentRow);
    PLAYLIST_AddEntry(0,e);
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

    if(ts)
    {
        /* Create the large table (songdb)*/
        sw->SongArchive=SDL_WidgetCreateR(SDL_TABLE,ts->Table->Rect);/*x y w h */
        SDL_WidgetPropertiesOf(sw->SongArchive,
                               SET_FONT,THEME_Font("small"));
//        SDL_WidgetProperties(SET_VISIBLE_ROWS,    ts->Table->Rows);
        SDL_WidgetPropertiesOf(sw->SongArchive,SET_VISIBLE_COLUMNS, ts->Table->Columns);
        SDL_WidgetPropertiesOf(sw->SongArchive,COLUMN_WIDTH, 1, 30  );
        SDL_WidgetPropertiesOf(sw->SongArchive,COLUMN_WIDTH, 2, 290 );
        SDL_WidgetPropertiesOf(sw->SongArchive,COLUMN_WIDTH, 3, 60  );
        SDL_WidgetPropertiesOf(sw->SongArchive,SET_FG_COLOR,WHITE);
        SDL_WidgetPropertiesOf(sw->SongArchive,SET_CALLBACK,SDL_CLICKED,eventhandler,sw->SongArchive);
        SDL_WidgetPropertiesOf(sw->SongArchive,SET_CALLBACK,SDL_KEYDOWN_RETURN,testplay,sw->SongArchive);
        SDL_WidgetPropertiesOf(sw->SongArchive,SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.bmp"));

        
        {
            char *titles[3];
            int r=0,c=0;
            for(c=0;c<3;c++)
                titles[c]=malloc(255);

            for(r=0;r< SONGDB_GetNoOfEntries();r++)
            {
                for(c=0;c<3;c++)
                {
                    songdbstring(r,c,titles[c]);
                }
                SDL_TableAddRow(sw->SongArchive,titles);
            }
            for(c=0;c<3;c++)
                free(titles[c]);
        }

        /* Craete the tab section below the table*/
        sw->Tabs=SDL_WidgetCreate(SDL_TAB,ts->Table->Rect.x,ts->Table->Rect.y + ts->Table->Rect.h,
                                           ts->Table->Rect.w,20);
        SDL_WidgetPropertiesOf(sw->Tabs,SET_FONT,THEME_Font("normal"));
        SDL_WidgetPropertiesOf(sw->Tabs,SET_BG_COLOR,0x93c0d5);
        SDL_WidgetPropertiesOf(sw->Tabs,SET_CALLBACK,SDL_CLICKED,SONGDBUI_ChangeDatabase,NULL);

    }
    /* Create buttons which change the tabs */
    while(Button)
    {
        switch(Button->action)
        {
            SDL_Widget *w;
        case BUTTON_CHANGE_DIR:
            //add dir to highlighted tab
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetPropertiesOf(w,SET_CALLBACK,SDL_CLICKED,SONGDBUI_ChangeGroupClicked,NULL);        
            break;
        }
        Button=Button->next;
    }
    activesong[0]=-1;
    activesong[1]=-1;

    return sw;
}

static void SONGDBUI_ChangeDatabase(void *table,void *tabwidget)
{
    int count;
    struct SongDBSubgroup *sg;

    count=SONGDBUI_GetHighlightedTab(tabwidget);
      
    if(activesong[0]==count)
        SDL_WidgetPropertiesOf(table,SET_HIGHLIGHTED,activesong[1]);                  
    else
        SDL_WidgetPropertiesOf(table,SET_HIGHLIGHTED,-1);    

    if(count >=0)
    {
        sg=SONGDB_GetSubgroupList();
        while(count)
        {
            sg=sg->next;
            count--;
        }
        SONGDB_SetActiveSubgroup(sg);
    }
}

void SONGDBUI_Redraw(void *w)
{
    SongdbWidgets *widgets=(SongdbWidgets*)w;
    SongDBSubgroup *sg;
    
    if(SONGDB_GroupChanged())
    {
        sg=SONGDB_GetSubgroupList();
        
        while(SDL_WidgetPropertiesOf(widgets->Tabs,TAB_REMOVE,NULL))
        {
        }
        
        while(sg)
        {
            SDL_WidgetPropertiesOf(widgets->Tabs,TAB_ADD,sg->Name);
            sg=sg->next;
        }
        SONGDBUI_ChangeDatabase(widgets->SongArchive,widgets->Tabs);
    }
 
}

void SONGDBUI_Play(int player_nr)
{
    int tab;
    struct SongDBEntry *e;
    tab=SONGDBUI_GetHighlightedTab(Widgets->Tabs);

    if(tab>=0)
    {
        PLAYER_GetPlayingEntry(player_nr,&e);
        tab=SONGDB_FindSubgroup(e);
        if(e)
        {
            activesong[0]=tab;
            activesong[1]=e->id;
            SONGDBUI_SetHighlightedTab(tab);
            SDL_WidgetPropertiesOf(Widgets->SongArchive,SET_HIGHLIGHTED,e->id);    
            SONGDBUI_ChangeDatabase(Widgets->SongArchive,Widgets->Tabs);
        }
    }
}
       

static int SONGDBUI_GetHighlightedTab(void *tabwidget)
{
    SDL_Tab *t;
    t=(SDL_Tab *)Widgets->Tabs;
    if(t && t->hl)
    {
        return t->hl->index;
    }
    return -1;

}

static int SONGDBUI_SetHighlightedTab(int which)
{
    SDL_Tab *t;
    SDL_TabList *tl;
    t=(SDL_Tab *)Widgets->Tabs;
    tl=t->tabs;

    while(tl)
    {
        if(which == 0)
        {
            t->hl=tl;
            return 1;
        }
        which--;
        tl=tl->next;
    }
    return 0;
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
