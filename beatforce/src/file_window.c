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

#define MODULE_ID FILEWINDOW
#include "debug.h"

void *fileedit;
void *TableSubgroup;
void *TableFilesInSubgroup;
void *TableFilesInDirectory;
void *TableDirectories;
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
/* for button */
static void FILEWINDOW_AddSubgroup(void *data);
static void FILEWINDOW_RemoveSubgroup(void *data);
static void FILEWINDOW_RenameSubgroup(void *data);
static void FILEWINDOW_AddSelected(void *data);

/* for table */
static void FILEWINDOW_DirectoryClicked(void *data);
static void FILEWINDOW_RenameSubgroupFinished();

/* local data retreival functions for tables */
static void FILEWINDOW_GetDirectories(int row,int column,char *string);
static void FILEWINDOW_GetFilesInSubgroup(int row,int column,char *string);

Window gFILEWINDOW={ FILEWINDOW_EventHandler };



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
    WNDMGR_Open(&gFILEWINDOW);
}

static void FILEWINDOW_RenameSubgroupFinished()
{
    SDL_TableRow *row=NULL;
    char newlabel[255];
    SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&row);
    if(row)
    {
        SDL_WidgetPropertiesOf(TableSubgroup,GET_CAPTION,newlabel);
        SONGDB_RenameSubgroup(row->index,newlabel);
        SDL_WidgetPropertiesOf(TableSubgroup,CLEAR_SELECTED,0);
    }
} 

static void FILEWINDOW_RenameSubgroup(void *data)
{
    SDL_TableRow *row;
    

    SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&row);
    if(row)
    {
        SDL_WidgetPropertiesOf(TableSubgroup,SET_STATE_EDIT,row->index,1);
    }
}

static void FILEWINDOW_AddSubgroup(void *data)
{
    SONGDB_AddSubgroup(SONGDB_GetActiveGroup(),"<new>");
    SDL_WidgetPropertiesOf(TableSubgroup,ROWS, 100);
}

static void FILEWINDOW_RemoveSubgroup(void *data)
{
    SDL_TableRow *row;
    
    SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&row);
    if(row)
    {
        SONGDB_RemoveSubgroup(row->index);
        SDL_WidgetPropertiesOf(TableSubgroup,CLEAR_SELECTED,0);
    }
    SDL_WidgetPropertiesOf(TableSubgroup,ROWS,100);
}

static void FILEWINDOW_AddSelected(void *data)
{
    SDL_TableRow *row;
    SDL_TableRow *lSubgroup;
    char string[255];

    TRACE("FILEWINDOW_AddSelected");
    memset(string,0,255);

    SDL_WidgetPropertiesOf(TableFilesInDirectory,GET_SELECTED,&row);
    if(row)
    {
        while(row)
        {
            
            FILEWINDOW_GetFilesInDirectory(row->index,0,string);
            if(strlen(string))
            {
                SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&lSubgroup);
                if(lSubgroup)
                {
                    SONGDB_AddFileToSubgroup(lSubgroup->index,string);
////                    return;
                }
//                else
//                    ERROR("No subgroup selected");
            }
            row=row->next;
        }
        SDL_WidgetPropertiesOf(TableFilesInDirectory,CLEAR_SELECTED,0);
    }
    else
    {
//        printf("Nothing selected\n");
    }
}

static void FILEWINDOW_AddAll(void *data)
{
    BFList *l;
    SDL_TableRow *lSubgroup;
    
    TRACE("FILEWINDOW_AddAll");

    SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&lSubgroup);
    if(lSubgroup)
    {
        l=files;
        while(l)
        {   
            SONGDB_AddFileToSubgroup(lSubgroup->index,(char*)l->data);
            l=l->next;
        }
    }
}

int GetDir(int count,char *string)
{
    char *word[20];
    int nw=0;
    char *dircopy;
    char *ptr;
    dircopy=strdup(directory);

    memset(word,0,20*sizeof(char*));
    if(strlen(dircopy) > 1)
        word[nw++]="";
    for(ptr=dircopy; nw < 20; ptr=strchr(ptr,'/'))
    {
        if(ptr==NULL)
            break;
        else if(*ptr == '/')
        {
            while(*ptr == '/')
            {
                *ptr = '\0';
                ptr++;
            }
            if(*ptr)
                word[nw++] = ptr;
        }
        else
        {
            if(*ptr)
                word[nw++]=ptr;
        }
   
    }
    if(word[count])
        sprintf(string,"/%s",word[count]);

    free(dircopy);
    return nw;
    
}

int GetSubDir(int count,char *string)
{
    BFList *dirss;
    char *t;
    int c;

    dirss=OSA_FindDirectories(directory);    

    c=count;
    while(dirss)
    {
        t=strrchr((char*)dirss->data,'/');
        t++;
        if(*t != '.')
        {
            if(count==0)
                break;
            count--;
        }
        dirss=dirss->next;

    }
    if(dirss)
    {
        t=strrchr((char*)dirss->data,'/');
        t++;
        sprintf(string,"/%s",t);
    }
    else
    {
        return 0;
    }
    return LLIST_NoOfEntries(dirss);
}

static void FILEWINDOW_GetDirectories(int row,int column,char *string)
{
    char dir[255];
    int nw=0;
    int count=0;

    nw = GetDir(row,dir);

    if(row < nw)
    {
        memset(string,' ',10);
        sprintf(string+row,"%s",dir);
    }
    else
    {
        count = GetSubDir(row-nw,dir);
        if(count)
        {
            memset(string,' ',10);
            sprintf(string+(row-(row-nw)),"%s",dir);
        }
        
    }
}

void FILEWINDOW_GetSubgroup(int row,int column,char *string)
{
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
            sprintf(string,"%s",(char*)l->data);

        l=l->next;
        count++;
    }
  
}

static void FILEWINDOW_DirectoryClicked(void *data)
{
    char str[255];
    char newdir[255];
    int i;
    SDL_Table *t=TableDirectories;
    int c;
    int nw=0;
    int nw2=0;

    memset(newdir,0,255);

    /* Get the rownumber which is clicked */
    c=t->CurrentRow;
    
    nw=GetDir(0,str);
    if(c == 0)
    {
        if(!strcmp(directory,"/"))
            sprintf(directory,"%s",str);
        else
            sprintf(directory,"/");
        nw2=GetSubDir(0,str);
    }
    else if(c < nw)
    {
        for(i=1;i<=c;i++)
        {
            GetDir(i,str);
            sprintf(newdir,"%s%s",newdir,str);
        }
        sprintf(directory,"%s",newdir);
    }
    else
    {
        for(i=1;i<nw;i++)
        {
            GetDir(i,str);
            sprintf(newdir,"%s%s",newdir,str);
        }
        nw2=GetSubDir(c-nw,str);
        sprintf(directory,"%s%s",newdir,str);
    }
    files=OSA_FindFiles(directory,".mp3",0);
    
    SDL_WidgetPropertiesOf(t,ROWS,nw2);
    
}

void FILEWINDOW_DeleteSelected(void *data)
{
    struct SongDBSubgroup *list;
    SDL_TableRow *lSubgroup;
    SDL_TableRow *row;
    char string[255];
    
    SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&lSubgroup);
    if(lSubgroup)
    {
        list=SONGDB_GetSubgroup(lSubgroup->index);
    }
    else
    {
//        ERROR("No subgroup selected");
        return;
    }

    SDL_WidgetPropertiesOf(TableFilesInDirectory,GET_SELECTED,&row);
    if(row)    
    {
        while(row)
        {
            FILEWINDOW_GetFilesInDirectory(row->index,0,string);
//            SONGDB_RemoveFileFrom(row->index,string);
            row=row->next;
        }
    }
}

void FILEWINDOW_GetFilesInSubgroup(int row,int column,char *string)
{
    struct SongDBSubgroup *list;
    struct SongDBEntry    *Playlist;
    SDL_TableRow *lSubgroup;
    int count=0;

    SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&lSubgroup);
    if(lSubgroup)
    {
        list=SONGDB_GetSubgroup(lSubgroup->index);
        Playlist=list->Playlist;
    }
    else
    {
//        ERROR("No subgroup selected");
        return;
    }

    while(Playlist)
    {
        if(count == row)
        {
            sprintf(string,"%s",Playlist->filename);
            break;
        }
        Playlist=Playlist->next;
        count++;
    }
  
  
}


SDL_Surface *Window_CreateFileWindow()
{
    SDL_Surface *FileWindow;
    
    ThemeConfig *tc       = THEME_GetActive();
    ThemeFileWindow *fw   = NULL;
    ThemeImage    *Image  = NULL;
    ThemeButton   *Button = NULL;
    ThemeTable    *Table  = NULL;

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
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_RenameSubgroup,NULL);        
            break;
        case BUTTON_ADD:
            //add a empty tab button
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_AddSubgroup,NULL);        
            break;
        case BUTTON_REMOVE:
            //remove the selected subgroup
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_RemoveSubgroup,NULL);        
            break;
        case BUTTON_ADDSELECTED:
            /* add selected files to subgroup */
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_AddSelected,NULL);        
            break;
        case BUTTON_ADDALL:
            /* add all displayed files to selected subgroup */
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_AddAll,NULL);        
            break;
        case BUTTON_DELETESELECTED:
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,Button->normal);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_DeleteSelected,NULL);
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
            TableSubgroup=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetProperties(SET_VISIBLE_ROWS, 19);
            SDL_WidgetProperties(COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetProperties(ROWS, 30);
            SDL_WidgetProperties(SET_SELECTABLE,1);
            SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
            SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
            SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetSubgroup);
            SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN,FILEWINDOW_RenameSubgroupFinished,NULL);
            break;
        case CONTENTS_FILESINDIRECTORY:
            TableFilesInDirectory=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetProperties(SET_VISIBLE_ROWS, 190);
            SDL_WidgetProperties(COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetProperties(ROWS,LLIST_NoOfEntries(files));
            SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
            SDL_WidgetProperties(SET_SELECTABLE,1);
            SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetFilesInDirectory);
            SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
            break;
        case CONTENTS_FILESINSUBGROUP:
            TableFilesInSubgroup=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetProperties(SET_VISIBLE_ROWS, 190);
            SDL_WidgetProperties(COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetProperties(ROWS, 10);
            SDL_WidgetProperties(SET_SELECTABLE,1);
            SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
            SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetFilesInSubgroup);
            SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
            break;
        case CONTENTS_DIRECTORIES:
            TableDirectories=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetProperties(SET_VISIBLE_ROWS, 190);
            SDL_WidgetProperties(COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetPropertiesOf(TableDirectories,ROWS,19);
            SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
            SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetDirectories);
            SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_DirectoryClicked,NULL);

            break;
        }
        Table=Table->next;
    }

    sprintf(directory,"/");
    files=OSA_FindFiles(directory,".mp3",0);

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
            SDL_WidgetPropertiesOf(TableSubgroup,CLEAR_SELECTED,0);
            WNDMGR_CloseWindow();
            SONGDBUI_ChangeDatabase();
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


