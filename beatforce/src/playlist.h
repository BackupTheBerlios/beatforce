/*
  Beatforce/ Startup of beatforce

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
#ifndef __PLAYLIST_H__
#define __PLAYLIST_H__


struct PlEntry
{
    struct SongDBEntry *e;
    long unique;
    int  index;
    struct PlEntry *next;
};

#define PL_ENTRY_LEN  ( sizeof(struct PlEntry) )
#define PLAYLIST_UNIQUE_UNKNOWN     0xffffff00


#define GET_PLAYLIST_WIDGET( w )    ((GtkWidget*) gtk_object_get_data( GTK_OBJECT(w), "PlayList" ) )
#define GET_PLAYLIST( w )			(GTK_CLIST( (GtkWidget*) gtk_object_get_data( GTK_OBJECT(w), "PlayList" ) ))

#define GET_PL_ENTRY( clist, row )  ((struct PlEntry *)gtk_clist_get_row_data( clist, row ))


int playlist_init (int);
int playlist_finalize (int);

int __inline playlist_selected_rows (int);

//void playlist_select (GtkWidget *, int);
//void playlist_unselect (GtkWidget *, int);
//void playlist_move (GtkWidget *, int, int);

void playlist_add (int);
//int playlist_remove (GtkWidget *);
//void playlist_selall (GtkWidget *);

int playlist_get_entries (int);
struct PlEntry *playlist_GetSong (int, int);


//int playlist_do_renumber (GtkWidget *);

void playlist_do_add (int,int, struct SongDBEntry *); /* add one entry at the end of the list */
void playlist_set_entry(int player_nr, struct SongDBEntry *e); /* define the playlist as this entry */

//int playlist_get_sel_last (GtkWidget *);


struct PlEntry *alloc_pl_entry (struct SongDBEntry *);
void free_pl_entry (struct PlEntry *);

#endif
