/*
  Beatforce/ Playlist wiatlist handling

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
/*
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>

#include "player.h"
#include "songdb.h"
#include "playlist.h"

void playlist_set_playing (void *, int, unsigned long);

long no_of_entries;
long curr_unique;
int selectall;

int
playlist_init (int player_nr)
{
    object_get_data(player_nr)->playlist=NULL;
    selectall=0;
    curr_unique = 0;
    no_of_entries=0;
    return 0;
}

int
playlist_finalize (int player_nr)
{
    return 0;
}


void
playlist_select(void * win, int row)
{
}

void
playlist_unselect (void * win, int row)
{
}

void
playlist_move (void * win, int src_row, int dst_row)
{

}

void
playlist_add (int player_nr)
{

}


int
playlist_remove(void * win)
{
    /*
  int row = -1;
  //GtkCList *clist = GET_PLAYLIST (win);
  //BFList *glist = (BFList *)clist->selection;
  struct PlayerPrivate *p = NULL;

  
  while (glist != NULL)
  {
    struct PlEntry *pe = NULL;

    row = (int) glist->data;
    glist = glist->next;

    // check if we are going to remove the currently playing song 
    pe = GET_PL_ENTRY (clist, row);
    p = object_get_data(0);//TODO PLAYER_PRIVATE (win);
    if (p->playing_unique == pe->unique)
    {
#ifdef DEBUG_PLAYLIST
      printf ("removed currently playing song!\n");
#endif
      p->playing_no = 0;
      player_update_trackno (p->ch_id);
    }

    free_pl_entry (pe);
  //gtk_clist_remove (clist, row);
  }

  playlist_do_renumber (win);*/
  return 0;
}


void
playlist_selall (void* win)
{
    selectall=1;
}


void
playlist_set_playing (void *win, int no, unsigned long id)
{
  /*GdkColor color;

  color.pixel = color.blue = color.green = 0;
  color.red = 65535;

  gtk_clist_set_foreground (GET_PLAYLIST (win), no - 1, &color);*/

}


int
playlist_get_entries (int player_nr)
{
    return no_of_entries;
}


struct PlEntry *
playlist_GetSong (int player_nr, int no)
{
    struct PlEntry *pe;
    int index=0;

    printf("Player get song %d-%d\n",player_nr,no);

    pe=object_get_data(player_nr)->playlist;
    
    if(pe == NULL)
        return NULL;
   
    while(pe->next && no > index)
    {
        index++;
        pe=pe->next;
    }
    if(index < no)
        return NULL;

    return pe;
}

void playlist_set_entry(int player_nr, struct SongDBEntry *e)
{
    struct PlEntry *pe;
    struct PlEntry *newlist = object_get_data(player_nr)->playlist;

    printf("playlist+set_entry\n");
#define STR( x )  ((x == NULL) ? ("") : (x))

    if (e == NULL)
    {
        printf ("ERROR: entry == NULL\n");
        return;
    }
    else
    {
        printf("Adding %s to player %d\n",e->artist,player_nr);
        
    }
    pe = alloc_pl_entry (e);

    no_of_entries=1;
    if(newlist==NULL)
    {
        printf("Creating new list\n");
        object_get_data(player_nr)->playlist=pe;
    }
    else
    {
        free(object_get_data(player_nr)->playlist);
        printf("Creating new list and removing old one\n");
        object_get_data(player_nr)->playlist=pe;
    }

}

void
playlist_do_add (int player_nr, int row, struct SongDBEntry *e)
{
    struct PlEntry *pe;
    struct PlEntry *newlist = object_get_data(player_nr)->playlist;

#define STR( x )  ((x == NULL) ? ("") : (x))

    if (e == NULL)
    {
        printf ("ERROR: entry == NULL\n");
        return;
    }
    else
    {
        printf("Adding %s to player %d\n",e->artist,player_nr);
        
    }
    pe = alloc_pl_entry (e);
    no_of_entries++;
    if(newlist==NULL)
    {
        printf("Creating new list\n");
        object_get_data(player_nr)->playlist=pe;
    }
    else
    {
        while(newlist->next)
        {
            newlist=newlist->next;
        }
        printf("Add to the end %p\n",pe);
        newlist->next=pe;
    }
}

int
playlist_do_renumber (void * win)
{
  return 0;
}


int
playlist_get_sel_last (void * win)
{
  int row = -1;
  return (row);
}

struct PlEntry *
alloc_pl_entry (struct SongDBEntry *e)
{
  struct PlEntry *pe;

  pe = malloc (PL_ENTRY_LEN);
  if (pe == NULL)
  {
      perror ("alloc_pl_entry:");
      return NULL;
  }
  memset (pe, 0, PL_ENTRY_LEN);
  
  curr_unique++;
  pe->e = e;//(struct SongDBEntry*)malloc(sizeof(struct SongDBEntry));
//  if(e->filename)
//  {
//      pe->e->filename=(char*)malloc(sizeof(struct 
//  }
  
  pe->next = NULL;
  pe->unique = curr_unique;

  return pe;
}

void
free_pl_entry (struct PlEntry *pe)
{
  free (pe);
}

