/*
  Beatforce/ Playlist/Waitlist user interface

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

#include "songdb.h"
#include "playlist.h"
#include "player.h"
#include "config.h"

#define PLAYER1 0

//int previous;

#define MODULE_ID PLAYLIST_UI
#include "debug.h"

#if 0
static void PLAYLISTUI_GetEntry(long row,int column,char *dest);
static void PLAYLISTUI_EntryClicked(SDL_Table *table,SDL_Event *event);
#endif

/* If we come here, then the user has selected a row in the list. */
void selection_made( GtkWidget      *clist,
                     gint            row,
                     gint            column,
                     GdkEventButton *event,
                     gpointer        data )
{
    gchar *text;

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

GtkWidget* PLAYLISTUI_CreateWindow(void *pl)
{
    GtkWidget *pui = NULL;

    TRACE("PLAYLISTUI_CreateWindow");

    pui = gtk_clist_new(2);

    gtk_clist_set_column_width (GTK_CLIST(pui), 0, 150);

    /* Add the CList widget to the vertical box and show it. */
    gtk_container_add(GTK_CONTAINER(pl), pui);
    {
        int indx;
        
        /* Something silly to add to the list. 4 rows of 2 columns each */
        gchar *drink[4][2] = { { "Milk",    "3 Oz" },
                               { "Water",   "6 l" },
                               { "Carrots", "2" },
                               { "Snakes",  "55" } };
        
        /* Here we do the actual adding of the text. It's done once for
         * each row.
         */
        for ( indx=0 ; indx < 4 ; indx++ )
            gtk_clist_append( (GtkCList *) pui, drink[indx]);
    }
        
        gtk_widget_show(pui);

    return pui;
}



void PLAYLISTUI_Redraw(void *widget)
{

}

#if 0
static void PLAYLISTUI_GetEntry(long row,int column,char *dest)
{
    struct PlEntry *pe;
    struct SongDBEntry *e;

    dest[0]='\0';

    if(PLAYLIST_GetNoOfEntries(PLAYER1) < row)
        return;
        
    pe = PLAYLIST_GetSong(PLAYER1,row);

    if(pe == NULL)
        return;

    e = pe->e; 
    if(e)
    {
        switch(column)
        {
        case 0:
            if(e->artist && e->title)
            {
                sprintf(dest,"%s - %s",e->artist,e->title);
            }
            else
            {
                sprintf(dest,"%s",strrchr(e->filename,'/')+1);
            }
            break;
        }
    }
}


static void PLAYLISTUI_EntryClicked(SDL_Table *table,SDL_Event *event)
{
    int player=0;// is playing ?
    struct PlEntry *pl;

    if(event->button.button == 3) //remove fromw aitlist right mousebutton down
    {
        pl=PLAYLIST_GetSong(PLAYER1,table->CurrentRow);
        if(pl && pl->e)
        {
            PLAYLIST_Remove(PLAYER1,pl->e);
            SDL_TableDeleteRow((SDL_Widget*)table,table->CurrentRow);
        }
        return;
    }

    if(PLAYER_IsPlaying(0))
        player=1;

    if(PLAYER_IsPlaying(1))
    {
        if(player!=1)
            player=0;
        else
            player=-1;
    }

    if(player == -1)
        printf("Can't set song to one of the players\n");
    else
    {
        if(event->button.button == 1)//play now left mosuebutton down
        {
            PLAYER_SetSong(player,table->CurrentRow);
        }
    }
}
#endif



