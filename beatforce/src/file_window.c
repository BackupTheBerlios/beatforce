/*
  Beatforce/ File window

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
#include <SDL/SDL.h>
#include <SDL_Table.h>
#include <config.h>
#include <llist.h>
#include <osa.h>
#include <songdb.h>
#include <string.h>
#include <malloc.h>

#include "wndmgr.h"
#include "songdb_ui.h"



void *ttable;
void *wLabel1;
void *wLabel2;


BFList *dirs;
BFList *songs;
BFList *localsongs;
int change;
extern SDL_Font *LargeBoldFont;
char directory[255];

/* Local function prototypes */
SDL_Surface *Window_CreateFileWindow();
int FILEWINDOW_EventHandler(SDL_Event event);

/* Local callback functions */
void FileWindow_DirSelectClicked(void *data);

void dirselect(SDL_Table *table);
void dirstring(long row,int column,char *dest);

Window FILEWINDOW={ FILEWINDOW_EventHandler };

SDL_Surface *FileWindow;

void FILEWINDOW_Init()
{
    FileWindow = NULL;
}

void FILEWINDOW_Open()
{
    if(FileWindow == NULL)
    {
        SDL_WidgetLOCK();
        FileWindow=Window_CreateFileWindow();
        SDL_WidgetUNLOCK();
    }
    else
    {
        SDL_WidgetUseSurface(FileWindow);
    }
    WNDMGR_Open(&FILEWINDOW);
}

SDL_Surface *Window_CreateFileWindow()
{
    SDL_Surface *FileWindow;
    BFList *tmp;
    int count=0;
    int songcount=0;
    int localcount=0;
    char label[255];
    char label2[255];

    FileWindow = SDL_CreateRGBSurface(SDL_SWSURFACE,1024,685,32,0xff0000,0x00ff00,0x0000ff,0x000000);

    SDL_WidgetUseSurface(FileWindow);

    SDL_WidgetCreate(SDL_PANEL,0,0,1024,685);
    SDL_WidgetProperties(SET_NORMAL_IMAGE,THEME_DIR"/beatforce/sbackground.bmp");

    
    change=0;
    dirs=OSA_FindDirectories("/mnt/d/");
    if(dirs)
    {
        tmp=dirs;
        count++;
        while(tmp->next)
        {
            count++;
            tmp=tmp->next;
        }
    }
    songs=OSA_FindFiles("/mnt/d/",".mp3",1);
    songcount=LLIST_NoOfEntries(songs);
    localsongs=OSA_FindFiles("/mnt/d/",".mp3",0);
    localcount=LLIST_NoOfEntries(localsongs);
    

    ttable=SDL_WidgetCreate(SDL_TABLE,10,10,980,400);
    SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
    SDL_WidgetProperties(SET_VISIBLE_ROWS, 27);
    SDL_WidgetProperties(ROWS,count);
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,dirstring);
    SDL_WidgetEventCallback(dirselect,SDL_CLICKED);

    sprintf(label,"Songcount local dir %d",songcount);
    sprintf(label2,"Songcount all dirs  %d",localcount);

    wLabel1=SDL_WidgetCreate(SDL_LABEL,10,440,450,23);
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_FG_COLOR,WHITE);
    SDL_WidgetProperties(SET_CAPTION,label);

    wLabel2=SDL_WidgetCreate(SDL_LABEL,10,470,450,23);
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_FG_COLOR,WHITE);
    SDL_WidgetProperties(SET_CAPTION,label2);


    SDL_WidgetCreate(SDL_BUTTON,20,500,40,40);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FileWindow_DirSelectClicked,NULL);

    return FileWindow;
}

int FILEWINDOW_EventHandler(SDL_Event event)
{
    switch(event.type)
    {
    case SDL_KEYDOWN:
        switch( event.key.keysym.sym ) 
        {
        case SDLK_ESCAPE:
            WNDMGR_CloseWindow();
            break;
      
        default:
            break;
            
        }
        break;

    }
    return 0;
}

void dirstring(long row,int column,char *dest)
{
    int count=row;
    BFList *tmp;
    if(change)
        return;

    if(dirs)
    {
        tmp=dirs;
        
        while(count && tmp->next)
        {
            tmp=tmp->next;
            count--;
        }

        if(tmp && tmp->data && count==0)
        {
            sprintf(dest,"%s",(char*)tmp->data);
        }
        else if(count == 1)
        {
            sprintf(dest," ");
        }
        else
        {
            count-=2;
            tmp=localsongs;
            while(count && tmp && tmp->next)
            {
                tmp=tmp->next;
                count--;
            }
            if(tmp && tmp->data && count == 0)
            {
                sprintf(dest,"%s",(char*)tmp->data);
            }
            else
            {
                sprintf(dest," ");
            }
        }
    }

    
}


void dirselect(SDL_Table *table)
{
    int count=table->CurrentRow;
    BFList *new_dirs;
    BFList *new_songs;
    BFList *new_localsongs;
    BFList *tmp;
    char label[255];
    char label2[255];
    int songcount=0;
    int localcount=0;
    char *d;

    tmp=dirs;
    while(count && tmp->next)
    {
        tmp=tmp->next;
        count--;
    }
    
    if(tmp->data && count == 0 && strlen(tmp->data))
    {
        change=1;
        d=strrchr(tmp->data,'/');
        d++;

        if(!strcmp(d,".."))
        {
            d=strrchr(tmp->data,'/');
            d[0]=0;
            d=strrchr(tmp->data,'/');
            if(d)
            {
                d[1]=0;
                    
            }
            
        }
        memset(directory,0,255);
        sprintf(directory,"%s",(char*)tmp->data);
        new_dirs=OSA_FindDirectories(directory);
        new_songs=OSA_FindFiles(directory,".mp3",1);
        new_localsongs=OSA_FindFiles(directory,".mp3",0);

        count=LLIST_NoOfEntries(new_dirs);
        localcount=LLIST_NoOfEntries(new_localsongs);
        songcount=LLIST_NoOfEntries(new_songs);

        SDL_WidgetPropertiesOf(ttable,ROWS,count);
        sprintf(label,"Songcount local dir %d",localcount);
        sprintf(label2,"Songcount all dirs  %d",songcount);
        SDL_WidgetPropertiesOf(wLabel1,SET_CAPTION,label);
        SDL_WidgetPropertiesOf(wLabel2,SET_CAPTION,label2);
        dirs=new_dirs;
        songs=new_songs;
        localsongs=new_localsongs;
        change=0;
    }
}


void FileWindow_DirSelectClicked(void *data)
{
    BFList *mp3;
    SONGDB_FreeActiveList();
    mp3  = OSA_FindFiles(directory,".mp3",0);
    
    while(mp3)
    {
        SONGDB_AddFilename((char*)mp3->data);
        free(mp3->data);
        mp3=mp3->next;
    }
    WNDMGR_CloseWindow(FileWindow);
    SONGDBUI_ChangeDatabase(directory);
    

    
}

