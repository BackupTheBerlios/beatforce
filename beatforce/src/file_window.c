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

char directory[255];

/* Local function prototypes */
SDL_Surface *Window_CreateFileWindow();
int FILEWINDOW_EventHandler(SDL_Event event);
void FILEWINDOW_GetFilesInDirectory(int row,int column,char *string);
static int FILEWINDOW_NotifyHandler(Window *Win);

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
static void FILEWINDOW_GetDirectories(int row,int column,char *string);
static void FILEWINDOW_GetFilesInSubgroup(int row,int column,char *string);

static void FILEWINDOW_GetSelectedSubgroup(struct SongDBSubgroup **sel_sg);

Window gFILEWINDOW={ FILEWINDOW_EventHandler , FILEWINDOW_NotifyHandler };



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
    SDL_WidgetPropertiesOf(TableSubgroup,ROWS, SONGDB_GetSubgroupCount());
    WNDMGR_Open(&gFILEWINDOW);
}
static int FILEWINDOW_NotifyHandler(Window *Win)
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
    SDL_WidgetPropertiesOf(TableSubgroup,ROWS, SONGDB_GetSubgroupCount());
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
                SDL_WidgetPropertiesOf(TableSubgroup,ROWS,SONGDB_GetSubgroupCount());
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
        SDL_WidgetPropertiesOf(TableFilesInSubgroup,ROWS,sg->Songcount);
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
    if(count < 20 && word[count])
        sprintf(string,"%s",word[count]);

    free(dircopy);
    return nw;
    
}

int GetSubDir(int count,char *string)
{
    BFList *dirss;
    char *t;
    int c;
    int entrycount;

    dirss=TableDirs;

    entrycount=LLIST_NoOfEntries(dirss);
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
        if(string)
            sprintf(string,"/%s",t);
    }
    else
    {
        return 0;
    }
    return entrycount;
}

/*
 * Table callback for directories */
static void FILEWINDOW_GetDirectories(int row,int column,char *string)
{
    char dir[255];
    int wordcount=0; /* directory level */
    int count=0;

    memset(dir,0,255);
    wordcount = GetDir(row,dir);

    if(row < wordcount)
    {
        memset(string,' ',10);
        sprintf(string+row,"%s",dir);
    }
    else
    {
        count = GetSubDir(row-wordcount,dir);
        if(count)
        {
            memset(string,' ',10);
            sprintf(string+(row-(row-wordcount)),"%s",dir);
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
    if(sg)
        SDL_WidgetPropertiesOf(TableFilesInSubgroup,ROWS,sg->Songcount);
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
    memset(str,0,255);
    /* Get the rownumber which is clicked */
    c=t->CurrentRow;

//    printf("str %s %d\n",str,__LINE__);
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
//            printf("newdir %s\n",newdir);
        }
        nw2=GetSubDir(c-nw,str);
        if(strlen(str))
            sprintf(directory,"%s%s",newdir,str);
//        printf("directory %s\n",directory);
//        printf("nw2 %d\n",nw2);
    }
    files=OSA_FindFiles(directory,".mp3",0);
    files2=OSA_FindFiles(directory,".ogg",0);
    
    files=LLIST_Combine(files,files2);

    TableDirs=OSA_FindDirectories(directory);
    SDL_WidgetPropertiesOf(t,ROWS,LLIST_NoOfEntries(TableDirs));
    /* Correct the rowcount for the files in directory table (for scrollbar) */
    SDL_WidgetPropertiesOf(TableFilesInDirectory,ROWS,LLIST_NoOfEntries(files));
    
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
    
    ThemeConfig *tc       = THEME_GetActive();
    ThemeFileWindow *fw   = NULL;
    ThemeImage    *Image  = NULL;
    ThemeButton   *Button = NULL;
    ThemeTable    *Table  = NULL;
    ThemeTree     *Tree   = NULL;

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
        SDL_WidgetProperties(SET_IMAGE,IMG_Load(Image->filename));
        Image=Image->next;
    }

    while(Button)
    {
        switch(Button->action)
        {
        case BUTTON_RENAME:
            //rename highlighted tab
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetProperties(SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_RenameSubgroup,NULL);        
            break;
        case BUTTON_ADD:
            //add a empty tab button
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetProperties(SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_AddSubgroup,NULL);        
            break;
        case BUTTON_REMOVE:
            //remove the selected subgroup
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetProperties(SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_RemoveSubgroup,NULL);        
            break;
        case BUTTON_ADDSELECTED:
            /* add selected files to subgroup */
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetProperties(SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_AddSelected,NULL);        
            break;
        case BUTTON_ADDALL:
            /* add all displayed files to selected subgroup */
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetProperties(SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_AddAll,NULL);        
            break;
        case BUTTON_DELETESELECTED:
            SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,IMG_Load(Button->normal));
            SDL_WidgetProperties(SET_PRESSED_IMAGE,IMG_Load(Button->pressed));
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_DeleteSelected,NULL);
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
            SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetProperties(SET_VISIBLE_ROWS, 19);
            SDL_WidgetProperties(COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetProperties(SET_SELECTABLE,1);
            SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
            SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
            SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetSubgroup);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_SubgroupClicked,NULL);
            SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN,FILEWINDOW_RenameSubgroupFinished,NULL);
            SDL_WidgetProperties(SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.jpg"));
            break;
        case CONTENTS_FILESINDIRECTORY:
            TableFilesInDirectory=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetProperties(SET_VISIBLE_ROWS, 190);
            SDL_WidgetProperties(COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetProperties(ROWS,LLIST_NoOfEntries(files));
            SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
            SDL_WidgetProperties(SET_SELECTABLE,-1);
            SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetFilesInDirectory);
            SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
            SDL_WidgetProperties(SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.jpg"));
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
            SDL_WidgetProperties(SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.jpg"));
            break;
        case CONTENTS_DIRECTORIES:
            TableDirectories=SDL_WidgetCreateR(SDL_TABLE,Table->Rect);
            SDL_WidgetProperties(SET_VISIBLE_COLUMNS, 1);
            SDL_WidgetProperties(COLUMN_WIDTH,1,Table->Rect.w);
            SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
            SDL_WidgetProperties(SET_DATA_RETREIVAL_FUNCTION,FILEWINDOW_GetDirectories);
            SDL_WidgetProperties(SET_BG_COLOR,0x93c0d5);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,FILEWINDOW_DirectoryClicked,NULL);
            SDL_WidgetProperties(SET_IMAGE,IMG_Load(THEME_DIR"/beatforce/tablescrollbar.jpg"));

            break;
        }
        Table=Table->next;
    }

    timewidget=CLOCK_Create(fw->Clock);
    
    sprintf(directory,"/");
    
    files=OSA_FindFiles(directory,".mp3",0);
    TableDirs=NULL;

    TableDirs=OSA_FindDirectories(directory);
    if(TableDirectories)
        SDL_WidgetPropertiesOf(TableDirectories,ROWS,LLIST_NoOfEntries(TableDirs));
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


