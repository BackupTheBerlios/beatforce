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

/*local protypes*/
struct PlEntry *playlist_AllocEntry(struct SongDBEntry *e);
void playlist_FreeEntry (struct PlEntry *pe);
long no_of_entries;
long curr_unique;

int PLAYLIST_Init (int player_nr)
{
    curr_unique = 0;
    no_of_entries=0;
    return 0;
}

int PLAYLIST_GetNoOfEntries(int player_nr)
{
    return no_of_entries;
}


struct PlEntry *PLAYLIST_GetSong (int player_nr, int no)
{
    struct PlEntry *pe;
    int index=0;

    pe= PLAYER_GetData(player_nr)->playlist;
    
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

void PLAYLIST_SetEntry(int player_nr, struct SongDBEntry *e)
{
    struct PlEntry *pe;
    struct PlEntry *newlist = PLAYER_GetData(player_nr)->playlist;

#define STR( x )  ((x == NULL) ? ("") : (x))

    if (e == NULL)
    {
        printf ("ERROR: entry == NULL\n");
        return;
    }
    pe = playlist_AllocEntry (e);

    no_of_entries=1;
    if(newlist==NULL)
    {
        PLAYER_GetData(player_nr)->playlist=pe;
    }
    else
    {
        free(PLAYER_GetData(player_nr)->playlist);
        PLAYER_GetData(player_nr)->playlist=pe;
    }

}

void PLAYLIST_DoAdd(int player_nr, int row, struct SongDBEntry *e)
{
    struct PlEntry *pe;
    struct PlEntry *newlist = PLAYER_GetData(player_nr)->playlist;

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
    pe = playlist_AllocEntry (e);
    no_of_entries++;
    if(newlist==NULL)
    {
        PLAYER_GetData(player_nr)->playlist=pe;
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

struct PlEntry *
playlist_AllocEntry(struct SongDBEntry *e)
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

void playlist_FreeEntry (struct PlEntry *pe)
{
  free (pe);
}

