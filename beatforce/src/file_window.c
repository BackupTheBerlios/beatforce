/*
  Beatforce/ File window

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
#include <SDL/SDL.h>
#include <SDLTk.h>
#include "SDL_Signal.h"
#include <config.h>

#include "osa.h"
#include "songdb.h"
#include <string.h>
#include <malloc.h>


#include "llist.h"
#include "theme.h"
#include "songdb.h"
#include "clock.h"

#define MODULE_ID FILEWINDOW
#include "debug.h"

void *fileedit;
SDL_Widget *TableSubgroup;
SDL_Widget *TableFilesInSubgroup;
SDL_Widget *TableFilesInDirectory;
SDL_Widget *TableDirectories;
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
#if 0
static void FILEWINDOW_SubgroupClicked(void *data);
#endif
static void FILEWINDOW_RenameSubgroupFinished();

/* local data retreival functions for tables */
static void FILEWINDOW_LoadSubgroups(SDL_Widget *widget);
#if 0
static void FILEWINDOW_GetFilesInSubgroup(int row,int column,char *string);
#endif
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
    TRACE("FILEWINDOW_NotifyHandler");
    CLOCK_Redraw(timewidget);
    return 1;
}

static void FILEWINDOW_RenameSubgroupFinished()
{
    char newlabel[255];
    struct SongDBSubgroup *sg;
    int *row;
    SDL_TableCell *tbc;

    TRACE("FILEWINDOW_RenameSubgroupFinished");

    FILEWINDOW_GetSelectedSubgroup(&sg);
    if(sg)
    {
        SDL_WidgetPropertiesOf(TableSubgroup,GET_SELECTED,&row,NULL);
        if(row)
        {
            tbc=SDL_TableGetCell(TableSubgroup,row[0],0);
            sprintf(newlabel,"%s",tbc->String);
            SONGDB_RenameSubgroup(sg,newlabel);
        }
    }
} 

static void FILEWINDOW_DirectoryClicked(void *data)
{
    TreeNode *t;
    BFList *DirList;
    BFList *FileList;
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
            DirList=OSA_FindDirectories(directory);
            {
                files=OSA_FindFiles(directory,".mp3",0);
                SDL_TableDeleteAllRows(TableFilesInDirectory);
                FileList=files;

                while(FileList)
                {
                    SDL_TableAddRow(TableFilesInDirectory,(char**)&FileList->data);
                    FileList=FileList->next;
                }
            }
            while(DirList)
            {
                r=(char*)DirList->data;
                if(r[0] != '.')
                    SDL_TreeInsertItem(TableDirectories,t,r);
                DirList=DirList->next;
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
        SDL_TableSetCursor(TableSubgroup,row[0],0);
    }
}

static void FILEWINDOW_AddSubgroup(void *data)
{
    char *new[1];
    new[0]=malloc(255);
    sprintf(new[0],"<new>");
    TRACE("FILEWINDOW_AddSubgroup");
    SONGDB_AddSubgroup(SONGDB_GetActiveGroup(),"<new>");
    SDL_TableAddRow(TableSubgroup,new);
    SDL_WidgetRedrawEvent(TableSubgroup);
    free(new[0]);
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
                FILEWINDOW_LoadSubgroups(TableSubgroup);
                SDL_WidgetPropertiesOf(TableSubgroup,SET_SELECTION_MODE,TABLE_MODE_SINGLE);
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
    printf("Add to subgroup %p\n",sg);
    if(sg)
    {
        l=files;
        while(l)
        {   
            printf("Add file %s\n",(char*)l->data);
            SONGDB_AddFileToSubgroup(sg,(char*)l->data);
            l=l->next;
        }
    }
}

/* 
 *  Table callback function for the subgroups of th active 
 *  group. 
 */
void FILEWINDOW_LoadSubgroups(SDL_Widget *widget)
{
    struct SongDBSubgroup *list;
    char *string[1];

    string[0]=malloc(255);
    list=SONGDB_GetSubgroupList();
    SDL_TableDeleteAllRows(widget);
    while(list)
    {
        sprintf(string[0],list->Name);
        SDL_TableAddRow(widget,string);
        list=list->next;
    }
    free(string[0]);
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

#if 0 /* Not used */
static void FILEWINDOW_SubgroupClicked(void *data)
{
    struct SongDBSubgroup *sg;

    FILEWINDOW_GetSelectedSubgroup(&sg);
    
    if(sg)
    {
        

    }
//    if(sg)
//        SDL_WidgetPropertiesOf(TableFilesInSubgroup,ROWS,sg->Songcount);
}
#endif


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

#if 0
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
#endif


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
        SDL_PanelSetImage(w,IMG_Load(Image->filename));
        Image=Image->next;
    }


    while(Button)
    {
        switch(Button->action)
        {
        case BUTTON_RENAME:
            /* Rename the selected subgroup */
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_SignalConnect(w,"clicked",FILEWINDOW_RenameSubgroup,NULL);
            break;
        case BUTTON_CREATE:
            /* Add a subgroup */
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_SignalConnect(w,"clicked",FILEWINDOW_AddSubgroup,NULL);
            break;
        case BUTTON_REMOVE:
            //remove the selected subgroup
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_SignalConnect(w,"clicked",FILEWINDOW_RemoveSubgroup,NULL);
            break;
        case BUTTON_ADDSELECTED:
            /* add selected files to subgroup */
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_SignalConnect(w,"clicked",FILEWINDOW_AddSelected,NULL);
            break;
        case BUTTON_ADDALL:
            /* add all displayed files to selected subgroup */
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_SignalConnect(w,"clicked",FILEWINDOW_AddAll,NULL);
            break;
        case BUTTON_DELETESELECTED:
            w=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetPropertiesOf(w,SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetPropertiesOf(w,SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_SignalConnect(w,"clicked",FILEWINDOW_DeleteSelected,NULL);
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
            SDL_TableSetColumnWidth(TableSubgroup,0,Table->Rect.w);
            SDL_WidgetPropertiesOf(TableSubgroup,SET_SELECTION_MODE,TABLE_MODE_SINGLE);
            SDL_WidgetPropertiesOf(TableSubgroup,SET_FONT,THEME_Font("normal"));
            SDL_WidgetPropertiesOf(TableSubgroup,SET_BG_COLOR,0x93c0d5);
            SDL_TableSetEditable(TableSubgroup,1);
            SDL_SignalConnect(TableSubgroup,"edited",FILEWINDOW_RenameSubgroupFinished,NULL);
//            SDL_WidgetPropertiesOf(TableSubgroup,SET_CALLBACK,SDL_CLICKED,FILEWINDOW_SubgroupClicked,NULL);
            SDL_WidgetPropertiesOf(TableSubgroup,SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.jpg"));
            FILEWINDOW_LoadSubgroups(TableSubgroup);
            break;
        case CONTENTS_FILESINDIRECTORY:
            TableFilesInDirectory=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_VISIBLE_COLUMNS, 1);
//            SDL_WidgetPropertiesOf(TableFilesInDirectory,COLUMN_WIDTH,1,Table->Rect.w);
//            SDL_WidgetPropertiesOf(TableFilesInDirectory,ROWS,LLIST_NoOfEntries(files));
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_FONT,THEME_Font("normal"));
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_SELECTION_MODE,TABLE_MODE_MULTIPLE);
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_BG_COLOR,0x93c0d5);
            SDL_WidgetPropertiesOf(TableFilesInDirectory,SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.jpg"));
            break;
        case CONTENTS_FILESINSUBGROUP:
            TableFilesInSubgroup=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,SET_VISIBLE_COLUMNS, 1);
            //SDL_WidgetPropertiesOf(TableFilesInSubgroup,COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,SET_SELECTION_MODE,TABLE_MODE_MULTIPLE);
            SDL_WidgetPropertiesOf(TableFilesInSubgroup,SET_FONT,THEME_Font("normal"));
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
        SDL_SignalConnect(TableDirectories,"clicked",FILEWINDOW_DirectoryClicked,TableDirectories);

        
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


