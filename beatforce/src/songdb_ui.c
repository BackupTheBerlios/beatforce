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

#define MODULE_ID SONGDB_UI
#include "debug.h"

extern SDL_Font *LargeBoldFont;
extern SongDBConfig   *songdbcfg;

/* Prototypes for functions for buttosn below */
void UI_SongdbChangeDirClicked(void *data);
void UI_SongdbRenameClicked(void *data);
void UI_SongdbAddTabClicked(void *data);
void UI_SongdbRenameFinished(void *data);
void songdbui_RemoveTab(void *data);

void UI_SongdbChangeDatabase();

void songdbstring(long row,int column,char *dest);
void *table;
void *tabwidget;

void eventhandler(SDL_Table *table)
{
    struct SongDBEntry * e;
    int player=0;// is playing ?

    /* Get the current playlist entry */
    e = SONGDB_GetEntry(table->CurrentRow);
    if(PLAYER_IsPlaying(0))
        player=1;

    if(PLAYER_IsPlaying(1))
    {
        if(player!=1)
            player=0;
    }
    PLAYLIST_SetEntry(player,e);
    player_set_song(player,0);  // when set_entry is excecuted we only have 1 item thus 0
    UI_PlayerSetArtistTitle(player);
    
}

void SONGDBUI_CreateWindow()
{
    
    /* Create the large table (songdb)*/
    table=SDL_WidgetCreate(SDL_TABLE,30,340,950,240);
    SDL_WidgetProperties(SET_VISIBLE_ROWS,    20);
    SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 3);
    SDL_WidgetProperties(COLUMN_WIDTH, 1, 30  );
    SDL_WidgetProperties(COLUMN_WIDTH, 2, 490 );
    SDL_WidgetProperties(COLUMN_WIDTH, 3, 40  );
    SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
    SDL_WidgetProperties(ROWS,SONGDB_GetNoOfEntries());
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,songdbstring);
    
    SDL_WidgetEventCallback(eventhandler,SDL_CLICKED);

    /* Craete the tab section below the table*/
    tabwidget=SDL_WidgetCreate(SDL_TAB,30,580,500,20);
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
    {
        int i;
        for(i=0;i<songdbcfg->Tabs;i++)
        {
            SDL_WidgetProperties(ADD_TAB,songdbcfg->TabTitle[i]);
            if(songdbcfg->TabString[i])
            {
                BFList *mp3;
                SONGDB_SetActiveList(i);
                SONGDB_FreeActiveList();
                mp3  = OSA_FindFiles(songdbcfg->TabString[i],".mp3",1); //recursive search
                while(mp3)
                {
                    SONGDB_AddFilename((char*)mp3->data);
                    mp3=mp3->next;
                }
            }
        }
        SONGDBUI_ChangeDatabase(songdbcfg->TabString[0]);
    }
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,SONGDBUI_ChangeDatabase,NULL);
    SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN,UI_SongdbRenameFinished);

    /* Create buttons which change the tabs */
    //add dir to highlighted tab
    SDL_WidgetCreate(SDL_BUTTON,30,610,20,20);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,UI_SongdbChangeDirClicked,NULL);        

    //rename highlighted tab
    SDL_WidgetCreate(SDL_BUTTON,60,610,20,20);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,UI_SongdbRenameClicked,NULL);        

    //add a empty tab button
    SDL_WidgetCreate(SDL_BUTTON,90,610,20,20);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,UI_SongdbAddTabClicked,NULL);        

    //remove the active tab
    SDL_WidgetCreate(SDL_BUTTON,120,610,20,20);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,songdbui_RemoveTab,NULL);        
    
}

void SONGDBUI_ChangeDatabase(char *string)
{
    SDL_Tab *t;
    TRACE("SONGDBUI_ChangeDabase enter %s",string);
    
    t=(SDL_Tab *)tabwidget;
    SONGDB_SetActiveList(t->hl->index);
    SDL_WidgetPropertiesOf(table,ROWS,SONGDB_GetNoOfEntries());

    if(string)
    {
        if(t->hl->index < songdbcfg->Tabs)
        {

            sprintf(songdbcfg->TabString[t->hl->index],"%s",string);
        }

    }
}

void UI_SongdbChangeDirClicked(void *data)
{
    /* Will call void UI_SongdbChangeDatabase(char *string) when finished */
    FILEWINDOW_Open();
}

void UI_SongdbRenameFinished(void *data)
{
    SDL_Tab *t;
    t=(SDL_Tab *)tabwidget;
    
    if(songdbcfg->TabTitle[t->hl->index-1] == NULL)
        printf("Can't rename empty tab\n");
    
    if(t->hl->caption == NULL)
        printf("Nothing to copy from\n");
    
    strcpy(songdbcfg->TabTitle[t->hl->index],t->hl->caption);
}

void UI_SongdbRenameClicked(void *data)
{
    SDL_WidgetPropertiesOf(tabwidget,SET_STATE_EDIT,0);
}



void UI_SongdbAddTabClicked(void *data)
{
    int i;
    printf("Adding tab %d\n",songdbcfg->Tabs);
    for(i=0;i<songdbcfg->Tabs;i++)
        printf("songdbcfg %s\n",songdbcfg->TabTitle[i]);
    songdbcfg->Tabs++;
    songdbcfg->TabString = (char**)realloc( songdbcfg->TabString, songdbcfg->Tabs * sizeof(char*));
    songdbcfg->TabTitle  = realloc( songdbcfg->TabTitle , songdbcfg->Tabs * sizeof(char*));
    songdbcfg->TabTitle[songdbcfg->Tabs - 1] = malloc(255 *sizeof(char));
    songdbcfg->TabString[songdbcfg->Tabs - 1] = malloc(255 *sizeof(char));
    
    memset(songdbcfg->TabTitle[songdbcfg->Tabs - 1],0,255);
    memset(songdbcfg->TabString[songdbcfg->Tabs - 1],0,255);

    for(i=0;i<songdbcfg->Tabs;i++)
        printf("2:songdbcfg %s\n",songdbcfg->TabTitle[i]);
    
    SDL_WidgetPropertiesOf(tabwidget,ADD_TAB,NULL);
}

void songdbui_RemoveTab(void *data)
{
    TRACE("Remove tab:Not implemented");
}

void songdbstring(long row,int column,char *dest)
{
    struct SongDBEntry *e;
    
    if(row < 0)
        return;

    if(SONGDB_GetNoOfEntries()<row)
        return;

    e = SONGDB_GetEntry(row);

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
