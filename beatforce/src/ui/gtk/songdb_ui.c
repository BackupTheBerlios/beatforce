/*
  Beatforce/ Playlist + Tab user interface

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003-2004 John Beuving (john.beuving@wanadoo.nl)

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
#include <gtk/gtk.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "config.h"
#include "songdb.h"
#include "songdb_ui.h"
#include "player.h"
#include "playlist.h"
#include "configfile.h"
#include "osa.h"

#include "playlist_ui.h"
#include "player_ui.h"   /* for UI_PlayerSetArtistTitle() */




#define MODULE_ID SONGDB_UI
#include "debug.h"


/* Prototypes for functions for buttosn below */
static void SONGDBUI_ChangeGroupClicked(void *data);


void songdbstring(long row,int column,char *dest);

static int activesong[2];
SongdbWidgets *Widgets;

/* If we come here, then the user has selected a row in the list. */
void selection_made( GtkWidget      *clist,
                     gint            row,
                     gint            column,
                     GdkEventButton *event,
                     gpointer        data )
{
    gchar *text;

    struct SongDBEntry * e;

    /* Get the current playlist entry */
    e = SONGDB_GetEntryID(row);
    if(e)
    {
        PLAYER_Load(0,e);
        PLAYER_Play(0);
    }

    /* Get the text that is stored in the selected row and column
     * which was clicked in. We will receive it as a pointer in the
     * argument text.
     */
    gtk_clist_get_text(GTK_CLIST(clist), row, column, &text);

    /* Just prints some information about the selected row */
    g_print("You selected row %d. More specifically you clicked in "
            "column %d, and the text in this cell is %s\n\n",
            row, column, text);

    return;
}

#if 0
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
#endif

void *SONGDBUI_CreateWindow(void *ts)
{
    SongdbWidgets *sw;


    sw=malloc(sizeof(SongdbWidgets));

    SONGDB_Init ();

    /* Create the notebook/tab section below the table*/
    sw->Tabs=gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (sw->Tabs), GTK_POS_BOTTOM);
    gtk_container_add (GTK_CONTAINER (ts), sw->Tabs);
    gtk_widget_show (sw->Tabs);
#if 0
        SDL_SignalConnect(sw->Tabs,"switch-tab",SONGDB_ChangeDatabase,sw->Tabs);
#endif

#if 0
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
#endif
    activesong[0]=-1;
    activesong[1]=-1;

    return sw;
}


void SONGDBUI_Redraw(void *w)
{
    SongdbWidgets *widgets=(SongdbWidgets*)w;
    SongDBSubgroup *sg;
    GtkWidget *table;
    GtkWidget *label;
    gchar *titles[3] = { "ID","Song","Time" };

    if(SONGDB_GroupChanged())
    {
        sg=SONGDB_GetSubgroupList();

        /* Remove all tabs */
        //SDL_NotebookClear(widgets->Tabs);
        
        while(sg)
        {
            table = gtk_clist_new_with_titles( 3, titles);
            
            gtk_clist_set_column_width (GTK_CLIST(table), 0, 30);
            gtk_clist_set_column_width (GTK_CLIST(table), 1, 290);
            gtk_clist_set_column_width (GTK_CLIST(table), 0, 200);
            
            /* When a selection is made, we want to know about it. The callback
             * used is selection_made, and its code can be found further down */
            gtk_signal_connect(GTK_OBJECT(table), "select_row",
                               GTK_SIGNAL_FUNC(selection_made),
                               NULL);
//            SDL_SignalConnect(table,"activate",testplay,table);
            {
                gchar *songinfo[3];
                int r=0,c=0;
                for(c=0;c<3;c++)
                    songinfo[c]=malloc(255);

                SONGDB_SetActiveSubgroup(sg);
                for(r=0;r< sg->Songcount;r++)
                {
                    for(c=0;c<3;c++)
                    {
                        songdbstring(r,c,songinfo[c]);
                    }
                    gtk_clist_append( (GtkCList *)table, songinfo);
                }
                for(c=0;c<3;c++)
                    free(songinfo[c]);
            }
            gtk_widget_show(table);
            label = gtk_label_new (sg->Name);
            gtk_notebook_append_page (GTK_NOTEBOOK (widgets->Tabs), table, label);
            sg=sg->next;
        }
        sg=SONGDB_GetSubgroupList();
        SONGDB_SetActiveSubgroup(sg);
//        SDL_WidgetSetFocus(SDL_NotebookGetChildWidget(Widgets->Tabs));
//        SDL_WidgetRedrawEvent(widgets->Tabs);
    }
    
}

#if 0
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
#endif

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

