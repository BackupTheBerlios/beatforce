/*
  Beatforce/ Playlist wiatlist handling

  one line to give the program's name and an idea of what it does.
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>

#include "player.h"
#include "songdb.h"
#include "playlist.h"
#include "osa.h"
#include "input.h"
#include "config.h"

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
    curr_unique    = 0;
    no_of_entries  = 0;
    playlist[0]=NULL;
    playlist[1]=NULL;
    return 1;
}

int PLAYLIST_GetNoOfEntries(int player_nr)
{
    TRACE("PLAYLIST_GetNoOfEntries");
    return no_of_entries;
}


struct PlEntry *PLAYLIST_GetSong (int player_nr, int no)
{
    struct PlEntry *pe;
    int index=0;

    TRACE("PLAYLIST_GetSong");
   
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

void PLAYLIST_AddEntry(int player_nr, struct SongDBEntry *e)
{
    struct PlEntry *pe;
    struct PlEntry *newlist = playlist[0];

    if (e == NULL)
    {
        ERROR("entry == NULL");
        return;
    }
    TRACE("PLAYLIST_SetEntry player:%d filename: %s",player_nr,e->filename);
    pe = playlist_AllocEntry(e);
    
    INPUT_GetTag(e->filename,e);

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

int PLAYLIST_Remove(int player_nr,struct SongDBEntry *e)
{
    struct PlEntry *newlist = playlist[0];
    struct PlEntry *previous = NULL;

    TRACE("PLAYLIST_Remove");

    if(e == NULL)
        return 0;

    while(newlist)
    {
        if(newlist->e == e)
        {
            if(previous)
            {
                previous->next=newlist->next;
                if(newlist->next)
                    newlist=newlist->next;
            }
            else
            {
                if(newlist->next)
                    playlist[0]=newlist->next;
                else
                {
                    playlist_FreeEntry(playlist[0]);
                    playlist[0]=NULL;
                }
            }
            no_of_entries--;
            break;
        }
        previous=newlist;
        newlist=newlist->next;
    }
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

