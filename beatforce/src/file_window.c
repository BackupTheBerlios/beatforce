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

#include "osa.h"
#include "songdb.h"
#include <string.h>
#include <malloc.h>

#include "wndmgr.h"
#include "songdb_ui.h"
#include "llist.h"
#include "theme.h"
#include "songdb.h"

void *fileedit;
void *ttable;
void *TableFilesInSubgroup;
void *TableFilesInDirectory;
void *wLabel1;
void *wLabel2;


BFList *dirs;
BFList *songs;
BFList *localsongs;
BFList *files;
BFList *subgroupsongs;
int change;

char directory[255];

/* Local function prototypes */
SDL_Surface *Window_CreateFileWindow();
int FILEWINDOW_EventHandler(SDL_Event event);
void FILEWINDOW_GetFilesInDirectory(int row,int column,char *string);

/* Local callback functions */
void FileWindow_DirSelectClicked(void *data);

void dirselect(SDL_Table *table);
void dirstring(long row,int column,char *dest);
void filewindow_Search(void *data);
Window FILEWINDOW={ FILEWINDOW_EventHandler };



SDL_Surface *FileWindow;


void FILEWINDOW_Init()
{
    FileWindow = NULL;
    subgroupsongs = NULL;
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

void renamefinished()
{
    SDL_TableRow *row;
    char newlabel[255];
    SDL_WidgetPropertiesOf(ttable,GET_SELECTED,&row);
    if(row)
    {
        SDL_WidgetPropertiesOf(ttable,GET_CAPTION,newlabel);
        SONGDB_RenameSubgroup(row->index,newlabel);
        SDL_WidgetPropertiesOf(ttable,CLEAR_SELECTED,0);
    }
} 

void renamecb(void *data)
{
    int i=0;
    SDL_TableRow *row;

    SDL_WidgetPropertiesOf(ttable,GET_SELECTED,&row);
    if(row)
    {
        SDL_WidgetPropertiesOf(ttable,SET_STATE_EDIT,row->index,1);
    }
}

void addcb(void *data)
{
    SONGDB_AddSubgroup("<new>");
}

void removecb(void *data)
{
    int i=0;
    SDL_TableRow *row;

    SDL_WidgetPropertiesOf(ttable,GET_SELECTED,&row);
    if(row)
    {
        SONGDB_RemoveSubgroup(row->index);
        SDL_WidgetPropertiesOf(ttable,CLEAR_SELECTED,0);
    }
}

void addselected(void *data)
{
    int i=0;
    SDL_TableRow *row;
    char string[255];

    memset(string,0,255);

    SDL_WidgetPropertiesOf(TableFilesInDirectory,GET_SELECTED,&row);
    if(row)
    {
        while(row)
        {
            FILEWINDOW_GetFilesInDirectory(row->index,0,string);
            if(strlen(string))
            {
                subgroupsongs=LLIST_Append(subgroupsongs,strdup(string));
            }

            row=row->next;
        }
        SDL_WidgetPropertiesOf(TableFilesInDirectory,CLEAR_SELECTED,0);
    }
    else
    {
        printf("Nothing selected\n");
    }
}

void FILEWINDOW_GetSubgroup(int row,int column,char *string)
{
    int count=0;
    SongDBSubgroup *sg=SONGDB_GetSubgroup(row);

    if(sg)
         sprintf(string,"%s",sg->Name);
    
}

void FILEWINDOW_GetFilesInDirectory(int row,int column,char *string)
{
    int count=0;
    BFList *l;

    l=files;
    while(l)
    {   
        if(count == row)
            sprintf(string,"%s",l->data);

        l=l->next;
        count++;
    }
  
}

void FILEWINDOW_GetFilesInSubgroup(int row,int column,char *string)
{
    int count=0;
    BFList *l;

    l=subgroupsongs;
    while(l)
    {   
        if(count == row)
            sprintf(string,"%s",l->data);

        l=l->next;
        count++;
    }
  
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
    ThemeConfig *tc=THEME_GetActive();
    ThemeFileWindow *fw =NULL;
    ThemeImage    *Image  = NULL;
    ThemeButton   *Button = NULL;
    ThemeTable    *Table  = NULL;

    files=OSA_FindFiles("c:\\winnt\\system32\\",".dll",0);

    if(tc == NULL)
        return NULL;

    fw=tc->FileWindow;
    
    if(fw == NULL)
        return NULL;

    Image  = fw->Image;
    Button = fw->Button;
    Table  = fw->Table;

    FileWindow = SDL_CreateRGBSurface(SDL_SWSURFACE,1024,685,32,0xff0000,0x00ff00,0x0000ff,0x000000);

    SDL_WidgetUseSurface(FileWindow);

    while(Image)
    {
        SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_WidgetProperties(SET_NORMAL_IMAGE,Image->filename);
        Image=Image->next;
    }

    while(Button)
    {
        switch(Button->action)
        {
        case BUTTON_RENAME:
            //rename highlighted tab
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,renamecb,NULL);        
            break;
        case BUTTON_ADD:
            //add a empty tab button
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,addcb,NULL);        
            break;
        case BUTTON_REMOVE:
            //remove the selected subgroup
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,removecb,NULL);        
            break;
        case BUTTON_ADDSELECTED:
            /* add selected files to subgroup */
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,addselected,NULL);        
            break;
        case BUTTON_DELETESELECTED:
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            break;

        }
        Button=Button->next;
    }

    while(Table)
    {
        switch(Table->contents)
        {
        case CONTENTS_SUBGROUPS:
            /* table with the names of the subgroups */
            ttable=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetProperties(SET_VISIBLE_ROWS, 19);
            SDL_WidgetProperties(COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetProperties(ROWS, 10);
            SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
            SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
            SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetSubgroup);
            SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN,renamefinished,NULL);
            break;
        case CONTENTS_FILESINDIRECTORY:
            TableFilesInDirectory=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetProperties(SET_VISIBLE_ROWS, 190);
            SDL_WidgetProperties(COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetProperties(ROWS,LLIST_NoOfEntries(files));
            SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
            SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetFilesInDirectory);
            SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
            break;
        case CONTENTS_FILESINSUBGROUP:
            TableFilesInSubgroup=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetProperties(SET_VISIBLE_ROWS, 190);
            SDL_WidgetProperties(COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetProperties(ROWS, 10);
            SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
            SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetFilesInSubgroup);
            SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
            break;
        }
        Table=Table->next;
    }

    sprintf(directory,"c:\\beatforce\\");
    
    change=0;
    dirs=OSA_FindDirectories("C:\\beatforce\\");
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
    //localsongs=OSA_FindFiles("/mnt/d/",".mp3",0);
    localcount=LLIST_NoOfEntries(localsongs);
    
#if 0
    fileedit=SDL_WidgetCreate(SDL_EDIT,10,5,500,25);
    SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
    SDL_WidgetProperties(SET_ALWAYS_FOCUS,1);
    SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN,filewindow_Search,NULL);
    SDL_WidgetProperties(SET_CAPTION,directory);

    ttable=SDL_WidgetCreate(SDL_TABLE,10,30,980,400);
    SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
    SDL_WidgetProperties(SET_VISIBLE_ROWS, 27);
    SDL_WidgetProperties(ROWS,count);
    SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
    SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,dirstring);
    SDL_WidgetEventCallback(dirselect,SDL_CLICKED);

    sprintf(label,"Songcount local dir %d",songcount);
    sprintf(label2,"Songcount all dirs  %d",localcount);

    wLabel1=SDL_WidgetCreate(SDL_LABEL,10,440,450,23);
    SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
    SDL_WidgetProperties(SET_FG_COLOR,WHITE);
    SDL_WidgetProperties(SET_CAPTION,label);

    wLabel2=SDL_WidgetCreate(SDL_LABEL,10,470,450,23);
    SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
    SDL_WidgetProperties(SET_FG_COLOR,WHITE);
    SDL_WidgetProperties(SET_CAPTION,"Sub groups");


    SDL_WidgetCreate(SDL_BUTTON,20,500,40,40);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FileWindow_DirSelectClicked,NULL);
#endif


    return FileWindow;

}

void filewindow_Search(void *data)
{
    

}


int FILEWINDOW_EventHandler(SDL_Event event)
{
    switch(event.type)
    {
    case SDL_KEYDOWN:
        switch( event.key.keysym.sym ) 
        {
        case SDLK_ESCAPE:
            SDL_WidgetPropertiesOf(ttable,CLEAR_SELECTED,0);
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
        SONGDB_AddFile((char*)mp3->data);
        free(mp3->data);
        mp3=mp3->next;
    }
    WNDMGR_CloseWindow(FileWindow);
    SONGDBUI_ChangeDatabase(directory);
    

    
}

