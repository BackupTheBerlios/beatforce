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

#include <config.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>

#include "player.h"
#include "songdb.h"
#include "playlist.h"

#define MODULE_ID PLAYLIST
#include "debug.h"

struct PlEntry *playlist[2];

/*local protypes*/
struct PlEntry *playlist_AllocEntry(struct SongDBEntry *e);
void playlist_FreeEntry (struct PlEntry *pe);

long no_of_entries;
long curr_unique;

int PLAYLIST_Init (int player_nr)
{
    TRACE("PLAYLIST_Init");
    curr_unique = 0;
    no_of_entries=0;
    playlist[0]=NULL;
    playlist[1]=NULL;

    return 0;
}

int PLAYLIST_GetNoOfEntries(int player_nr)
{
//    TRACE("PLAYLIST_GetNoOfEntries");
    return no_of_entries;
}


struct PlEntry *PLAYLIST_GetSong (int player_nr, int no)
{
    struct PlEntry *pe;
    int index=0;
   
    pe = playlist[0];
    
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
    struct PlEntry *newlist = playlist[0];

    TRACE("PLAYLIST_SetEntry %d",player_nr);

    if (e == NULL)
    {
        ERROR("entry == NULL");
        return;
    }
    pe = playlist_AllocEntry(e);

    no_of_entries++;

    if(newlist==NULL)
    {
        playlist[0]=pe;
    }
    else
    {
        while(newlist->next)
        {
            newlist=newlist->next;
        }
        newlist->next=pe;
    }

}

int PLAYLIST_Remove(int player_nr,long songdb_id)
{
    struct PlEntry *newlist = playlist[0];
    struct PlEntry *pe = NULL;

    printf("PLAYLIST_Remove songsb %ld\n",songdb_id);

    while(newlist)
    {
        if(newlist->e && newlist->e->id == songdb_id)
        {
            if(pe)
                pe->next=newlist->next;
            else
            {
                if(newlist->next)
                    newlist=newlist->next;
                else
                    newlist=NULL;
            }
//            playlist_FreeEntry(newlist);
            no_of_entries--;
            break;
        }
        pe=newlist;
        newlist=newlist->next;
    }

    playlist[0]=newlist;
    return 1;
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
  pe->e = e;
  
  pe->next = NULL;
  pe->unique = curr_unique;

  return pe;
}

void playlist_FreeEntry (struct PlEntry *pe)
{
  free (pe);
}

