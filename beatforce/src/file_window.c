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
#include <SDLTk.h>
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
#include "clock.h"

#define MODULE_ID FILEWINDOW
#include "debug.h"

void *fileedit;
void *TableSubgroup;
void *TableFilesInSubgroup;
void *TableFilesInDirectory;
void *TableDirectories;
void *wLabel1;
void *wLabel2;
void *timewidget;

BFList *dirs;
BFList *songs;
BFList *localsongs;
BFList *files; /* a list with all files in the directory `directory[255]` */
BFList *files2;
BFList *subgroupsongs;
BFList *SubDirs;

BFList *TableDirs; /* list of directories in directory[255] */

int change;

char directory[255]; /* Selected directory in directory tree */

/* Local function prototypes */
SDL_Surface *Window_CreateFileWindow();
int FILEWINDOW_EventHandler(SDL_Event event);
void FILEWINDOW_GetFilesInDirectory(int row,int column,char *string);
static int FILEWINDOW_NotifyHandler(SDL_Window *Win);

/* Local callback functions */
void FileWindow_DirSelectClicked(void *data);
/* for button */
static void FILEWINDOW_AddSubgroup(void *data);
static void FILEWINDOW_RemoveSubgroup(void *data);
static void FILEWINDOW_RenameSubgroup(void *data);
static void FILEWINDOW_AddSelected(void *data);

/* for table */
static void FILEWINDOW_DirectoryClicked(void *data);
static void FILEWINDOW_SubgroupClicked(void *data);
static void FILEWINDOW_RenameSubgroupFinished();

/* local data retreival functions for tables */
static void FILEWINDOW_GetFilesInSubgroup(int row,int column,char *string);

static void FILEWINDOW_GetSelectedSubgroup(struct SongDBSubgroup **sel_sg);

SDL_Window gFILEWINDOW={ FILEWINDOW_EventHandler , FILEWINDOW_NotifyHandler, NULL, NULL };


void FILEWINDOW_Init()
{
    gFILEWINDOW.Surface = NULL;
    subgroupsongs = NULL;
}

void FILEWINDOW_Open()
{
    if(gFILEWINDOW.Surface == NULL)
    {
        gFILEWINDOW.Surface=Window_CreateFileWindow();
    }
    SDL_WindowOpen(&gFILEWINDOW);
}
static int FILEWINDOW_NotifyHandler(SDL_Window *Win)
{
    CLOCK_Redraw(timewidget);
    return 1;
}

static void FILEWINDOW_RenameSubgroupFinished()
{
    char newlabel[255];
    struct SongDBSubgroup *sg;

    FILEWINDOW_GetSelectedSubgroup(&sg);
    if(sg)
    {
        SDL_WidgetPropertiesOf(TableSubgroup,GET_CAPTION,newlabel);
        SONGDB_RenameSubgroup(sg,newlabel);
    }
} 

static void FILEWINDOW_DirectoryClicked(void *data)
{
    TreeNode *t;
    BFList *l;
    char diry[255];
    char *r;

    t = SDL_TreeGetSelectedItem(data);
    if(t)
    {
        if(t->Child == NULL)
        {
            TreeNode *a=t;
            sprintf(directory,"/%s",a->Label);
            while(a->Parent)
            {
                a=a->Parent;
                sprintf(diry,"%s",directory);
                sprintf(directory,"/%s%s",a->Label,diry);
            }
//            printf("Got directory %s\n",directory);
            l=OSA_FindDirectories(directory);
            files=OSA_FindFiles(directory,".mp3",0);
            while(l)
            {
                r=(char*)l->data;
                if(r[0] != '.')
                    SDL_TreeInsertItem(TableDirectories,t,(char*)l->data);
                l=l->next;
            }
            t->collapsed=0;

        }
    }

}

static void FILEWINDOW_RenameSubgroup(void *data)
{
    int *row;
   
    SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&row,NULL);
    if(row)
    {
        SDL_WidgetPropertiesOf(TableSubgroup,SET_STATE_EDIT,row[0],1);
    }
}

static void FILEWINDOW_AddSubgroup(void *data)
{

    TRACE("FILEWINDOW_RemoveSubgroup");
    SONGDB_AddSubgroup(SONGDB_GetActiveGroup(),"<new>");
//    SDL_WidgetPropertiesOf(TableSubgroup,ROWS, SONGDB_GetSubgroupCount());
}

static void FILEWINDOW_RemoveSubgroup(void *data)
{
    struct SongDBSubgroup *list;
    int *row;
    int count;
    
    TRACE("FILEWINDOW_RemoveSubgroup");
    SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&row,NULL);

    if(row)
    {
        count=row[0];
        list=SONGDB_GetSubgroupList();
        while(list && count >= 0)
        {
            if(count==0)
            {
                SONGDB_RemoveSubgroup(list);
                SDL_WidgetPropertiesOf(TableSubgroup,CLEAR_SELECTED,0);
//                SDL_WidgetPropertiesOf(TableSubgroup,ROWS,SONGDB_GetSubgroupCount());
                return;
            }
            list=list->next;
            count--;
        }
    }
}

static void FILEWINDOW_GetSelectedSubgroup(struct SongDBSubgroup **sel_sg)
{
    struct SongDBSubgroup *sg;
    int *lSubgroup;
    int count;

    if(sel_sg)
    {
        SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&lSubgroup,NULL);
        if(lSubgroup)
        {
            sg=SONGDB_GetSubgroupList();
            count=lSubgroup[0];
            while(count)
            {
                sg=sg->next;
                count--;
            }
            *sel_sg=sg;
        }
        else
        {
            *sel_sg=NULL;
        }
    }
}

static void FILEWINDOW_AddSelected(void *data)
{
    struct SongDBSubgroup *sg;
    int *SelectedFiles;
    int SelectedCount,i;
    char filename[255];
    
    TRACE("FILEWINDOW_AddSelected");
    memset(filename,0,255);

    FILEWINDOW_GetSelectedSubgroup(&sg);

    if(sg)
    {
        SDL_WidgetPropertiesOf(TableFilesInDirectory,GET_SELECTED,&SelectedFiles,&SelectedCount);
        for(i=0;i<SelectedCount;i++)
        {
            FILEWINDOW_GetFilesInDirectory(SelectedFiles[i],0,filename);
            if(strlen(filename))
            {
                SONGDB_AddFileToSubgroup(sg,filename);
            }
        }
        SDL_WidgetPropertiesOf(TableFilesInDirectory,CLEAR_SELECTED,0);
//        SDL_WidgetPropertiesOf(TableFilesInSubgroup,ROWS,sg->Songcount);
    }
    else
    {
//        printf("Nothing selected\n");
    }
}

static void FILEWINDOW_AddAll(void *data)
{
    struct SongDBSubgroup *sg;
    BFList *l;

    TRACE("FILEWINDOW_AddAll");
    FILEWINDOW_GetSelectedSubgroup(&sg);
    if(sg)
    {
        l=files;
        while(l)
        {   
            SONGDB_AddFileToSubgroup(sg,(char*)l->data);
            l=l->next;
        }
    }
}

/* 
 *  Table callback function for the subgroups of th active 
 *  group. 
 */
void FILEWINDOW_GetSubgroup(int row,int column,char *string)
{
    struct SongDBSubgroup *list;
    
    list=SONGDB_GetSubgroupList();
    while(list && row >= 0)
    {
        if(row==0)
            sprintf(string,"%s",list->Name);
       list=list->next;
       row--;
    }
}

/*
 * Table callback function for the files in the selected directory
 */
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
static void FILEWINDOW_SubgroupClicked(void *data)
{
    struct SongDBSubgroup *sg;

    FILEWINDOW_GetSelectedSubgroup(&sg);
//    if(sg)
//        SDL_WidgetPropertiesOf(TableFilesInSubgroup,ROWS,sg->Songcount);
}


void FILEWINDOW_DeleteSelected(void *data)
{
    struct SongDBSubgroup *sg;
    int SelectedCount;
    int *row;
    struct SongDBEntry    *Playlist;
    int song,i;
    
    FILEWINDOW_GetSelectedSubgroup(&sg);

    if(sg)
    {
        SDL_WidgetPropertiesOf(TableFilesInSubgroup,GET_SELECTED,&row,&SelectedCount);
        if(row)    
        {
            for(i=0;i<SelectedCount;i++)
            {
                Playlist=sg->Playlist;
                song=row[i];                
                while(Playlist)
                {
                    if(song==0)
                    {
                        SONGDB_RemovePlaylistEntry(sg,Playlist);
                        break;
                    }
                     
                    song--;
                    Playlist=Playlist->next;
                }
            }
        }
    }
}

void FILEWINDOW_GetFilesInSubgroup(int row,int column,char *string)
{
    struct SongDBSubgroup *list;
    struct SongDBEntry    *Playlist;
    int count=0;
    
    FILEWINDOW_GetSelectedSubgroup(&list);
    if(list)
    {
        Playlist=list->Playlist;
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
  
  
}


SDL_Surface *Window_CreateFileWindow()
{
    SDL_Surface *FileWindow;
    SDL_Widget *w; /* Temporary widget */
    
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

    FileWindow = SDL_WidgetNewSurface(1024,685,32);


    while(Image)
    {
        w=SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_WidgetPropertiesOf(w,SET_IMAGE,IMG_Load(Image->filename));
        Image=Image->next;
    }


    while(Button)
    {
        switch(Button->action)
        {
        case BUTTON_RENAME:
            //rename highlighted tab
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetPropertiesOf(w,SET_CALLBACK,SDL_CLICKED,FILEWINDOW_RenameSubgroup,NULL);        
            break;
        case BUTTON_ADD:
            //add a empty tab button
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetPropertiesOf(w,SET_CALLBACK,SDL_CLICKED,FILEWINDOW_AddSubgroup,NULL);        
            break;
        case BUTTON_REMOVE:
            //remove the selected subgroup
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetPropertiesOf(w,SET_CALLBACK,SDL_CLICKED,FILEWINDOW_RemoveSubgroup,NULL);        
            break;
        case BUTTON_ADDSELECTED:
            /* add selected files to subgroup */
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetPropertiesOf(w,SET_CALLBACK,SDL_CLICKED,FILEWINDOW_AddSelected,NULL);        
            break;
        case BUTTON_ADDALL:
            /* add all displayed files to selected subgroup */
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetPropertiesOf(w,SET_CALLBACK,SDL_CLICKED,FILEWINDOW_AddAll,NULL);        
            break;
        case BUTTON_DELETESELECTED:
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetPropertiesOf(w,SET_CALLBACK,SDL_CLICKED,FILEWINDOW_DeleteSelected,NULL);
            break;

        }
        Button=Button->next;
    }

    while(Table)
    {
        switch(Table->ContentType)
        {

        case CONTENTS_SUBGROUPS:
            /* table with the names of the subgroups */
            TableSubgroup=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetPropertiesOf(TableSubgroup,SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetPropertiesOf(TableSubgroup,SET_VISIBLE_ROWS, 19);
            SDL_WidgetPropertiesOf(TableSubgroup,COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetPropertiesOf(TableSubgroup,SET_SELECTION_MODE,TABLE_MODE_BROWSE);
            SDL_WidgetPropertiesOf(TableSubgroup,SET_FONT,THEME_Font("normal"));
            SDL_WidgetPropertiesOf(TableSubgroup,SET_BG_COLOR,0x93c0d5);
            //SDL_WidgetPropertiesOf(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetSubgroup);
            SDL_WidgetPropertiesOf(TableSubgroup,SET_CALLBACK,SDL_CLICKED,FILEWINDOW_SubgroupClicked,NULL);
            SDL_WidgetPropertiesOf(TableSubgroup,SET_CALLBACK,SDL_KEYDOWN_RETURN,FILEWINDOW_RenameSubgroupFinished,NULL);
            SDL_WidgetPropertiesOf(TableSubgroup,SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.jpg"));
            break;
        case CONTENTS_FILESINDIRECTORY:
            TableFilesInDirectory=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_VISIBLE_ROWS, 190);
            SDL_WidgetPropertiesOf(TableFilesInDirectory,COLUMN_WIDTH,1,Table->Rect.w);
//            SDL_WidgetPropertiesOf(TableFilesInDirectory,ROWS,LLIST_NoOfEntries(files));
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_FONT,THEME_Font("normal"));
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_SELECTION_MODE,TABLE_MODE_MULTIPLE);
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetFilesInDirectory);
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_BG_COLOR,0x93c0d5);
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.jpg"));
            break;
        case CONTENTS_FILESINSUBGROUP:
            TableFilesInSubgroup=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,SET_VISIBLE_ROWS, 190);
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,COLUMN_WIDTH,1,Table->Rect.w);
//            SDL_WidgetPropertiesOf(TableFilesInSubgroup,ROWS, 10);
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,SET_SELECTION_MODE,TABLE_MODE_MULTIPLE);
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,SET_FONT,THEME_Font("normal"));
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetFilesInSubgroup);
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,SET_BG_COLOR,0x93c0d5);
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.jpg"));
            break;

        }
        Table=Table->next;
    }


    timewidget=CLOCK_Create(fw->Clock);
    
    sprintf(directory,"/");
    
    files=OSA_FindFiles(directory,".mp3",0);

    {
        BFList *l;
        char *r;

        TableDirectories=SDL_WidgetCreate(SDL_TREE,10,200,200,170);
        SDL_WidgetPropertiesOf(TableDirectories,SET_FONT,THEME_Font("normal"));
        SDL_WidgetPropertiesOf(TableDirectories,SET_BG_COLOR,0x93c0d5);
        SDL_WidgetPropertiesOf(TableDirectories,SET_CALLBACK,SDL_CLICKED,FILEWINDOW_DirectoryClicked,TableDirectories);
        
        l=OSA_FindDirectories(directory);
        while(l)
        {
            r=(char*)l->data;
            if(r[0] != '.')
                SDL_TreeInsertItem(TableDirectories,NULL,(char*)l->data);
            l=l->next;
        }
    }

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
            SDL_WindowClose();
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


