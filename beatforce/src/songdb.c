/*
  BeatForce
  songdb.c  -	song database
   
  Copyright (c) 2001, Patrick Prasse (patrick.prasse@gmx.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public Licensse as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#include "configfile.h"
#include "songdb.h"
#include "player.h"
//temp
#include "input.h"
#include "playlist.h"

#define MODULE_ID SONGDB
#include "debug.h"

#define NUMBER_OF 8

/* master index */

struct SongDBEntry **songdb; /* songdb containts all entries */

struct SongDBEntry **subset[NUMBER_OF]; /* subset contains a part of songdb */
unsigned long songcount;
long n_id[NUMBER_OF];
int active;

/* search return */
struct SongDBEntry **search_results;
long n_search_results;
long n_index;
long unid;

SongDBConfig *songdbcfg;

/* local prototypes */
struct SongDBEntry *songdb_AllocEntry (void);
void songdb_FreeEntry (struct SongDBEntry *e);
int songdb_JumpToFileMatch(char* song, char * keys[], int nw);

int SONGDB_Init (SongDBConfig * our_cfg)
{
    int i;

    for(i=0;i<NUMBER_OF;i++)
    {
        subset[i] = NULL;
        n_id[i]   = 0;
    }
    songcount=0;
    active = 0;
    search_results = NULL;
    n_search_results = 0;
    n_index = 0;
    unid=0;
    songdbcfg = our_cfg;

    return 0;
}

struct SongDBEntry *songdb_AllocEntry (void)
{
    struct SongDBEntry *e;

    e = malloc (DBENTRY_LEN);
    if (e == NULL)
    {
        perror ("songdb_alloc_entry");
    }
    memset (e, 0, DBENTRY_LEN);

    return e;
}

void SONGDB_FreeActiveList()
{
    int i=0;
    for(i=0; i < n_id[active]; i++)
    {
        songdb_FreeEntry(subset[active][i]);
        subset[active][i]=NULL;
    }
    n_id[active]=0;

}

void songdb_FreeEntry (struct SongDBEntry *e)
{
    if(e)
    {
        if(e->filename)
            free(e->filename);
        free (e);
    }
}


long SONGDB_GetNoOfEntries (void)
{
    return n_id[active];
}

long SONGDB_GetNoOfSearchResults(void)
{
    return n_search_results;
}

struct SongDBEntry *SONGDB_GetSearchEntry(long id)
{
    if (id >= n_search_results || id == SONGDB_ID_UNKNOWN)
    {
        return NULL;
    }
    return search_results[id];
}

struct SongDBEntry *SONGDB_GetEntry(long id)
{
    struct SongDBEntry *e = NULL;
    
//    TRACE("SONGDB_GetEntry %d",id);

    if (id >= n_id[active] || id == SONGDB_ID_UNKNOWN)
    {
        //ERROR("Returning NULL %d %d",id,n_id[active]);
        return NULL;
    }
    e = subset[active][id];
    if (e != NULL)
    {
        return e;
    }
    return NULL;
}

struct SongDBEntry *SONGDB_GetEntryID(unsigned long id)
{
    if(id >= unid || id == SONGDB_ID_UNKNOWN)
        return NULL;
    return songdb[id];
}


int SONGDB_SetActiveList(int db)
{
    active=db;
    return 1;
}

int SONGDB_AddFile(char *filename)
{
    struct SongDBEntry *e;

    TRACE("SONGDB_AddFile");
    e = songdb_AllocEntry();
    e->id = unid++;
    e->filename = strdup (filename);

    if (input_whose_file (PLAYER_GetData(0)->ip_plugins, filename) != NULL)
    {

//        input_get_tag (0, filename, e);

	/* No valid song if time under 10 ms */
        //    if (e->time < 10)	
        //  return -1;

        /* Add the created entry to the active subset database */
        SONGDB_AddToSubset(e);
        /* Add the entry also to the entire db */
        SONGDB_AddToSongdb(e);
        
    }
    return 0;
}

int SONGDB_AddToSubset(struct SongDBEntry *e)
{
    n_id[active]++;
    subset[active] = realloc (subset[active], n_id[active] * DBENTRY_PTR_LEN);
    if(subset[active]==NULL)
    {
        printf("Realloc failed\n");
        return 0;
    }
    subset[active][n_id[active]-1] = e;
    return 1;

}

int SONGDB_AddToSongdb(struct SongDBEntry *e)
{
    songcount++;
    songdb = realloc(songdb,songcount * DBENTRY_PTR_LEN);
    if(songdb == NULL)
    {
        printf("Realloc failed\n");
        return 0;
    }
    songdb[songcount-1] = e;
    return 1;
}


int
songdb_do_list_add (struct SongDBEntry **entries, long id)
{
    char *strings[9];
    char buf[32];
    char *empty;
    struct SongDBEntry *e;

    if (entries == NULL)
        entries = subset[0];

    empty = strdup ("");

    if (id >= n_id[active])
        return -1;
    if (entries[id] == NULL)
        return -1;

    e = entries[id];
    if (e->id != id)
    {
        printf ("strange things are happening!!!\n");
        return -1;
    }

    sprintf (buf, "%ld", e->id);
    strings[0] = strdup (buf);

    strings[1] = ((e->artist == NULL) ? (empty) : (e->artist));
    strings[2] = ((e->title == NULL) ? (empty) : (e->title));
    strings[3] = ((e->genre == NULL) ? (empty) : (e->genre));
    strings[4] = ((e->year == NULL) ? (empty) : (e->year));

    sprintf (buf, "%d:%02d", (int) (e->time / 60000),
             (int) ((e->time % 60000) / 1000));
    strings[5] = strdup (buf);

    if (e->bpm <= 0)
        strings[6] = empty;
    else
    {
        sprintf (buf, "%3.1f", e->bpm);
        strings[6] = strdup (buf);
    }

    strings[7] = ((e->played) ? ("P") : (empty));
    strings[8] = NULL;

    return 0;
}


int songdb_JumpToFileMatch(char* song, char * keys[], int nw)
{
    int i;
    
    for (i = 0; i < nw; i++)
    {
        if (!strstr(song, keys[i]))
            return 0;
    }
    return 1;
}

void SONGDB_FindEntry(char *search_string)
{
    char *ptr;
    int nw=0;
    int entry;
    int i=0;
    char *words[20];

    struct SongDBEntry *e = NULL;

    
    /* lowercase the string */
    for(ptr=search_string;*ptr;ptr++)
        *ptr=tolower(*ptr);

    /* chop the searchstring in pieces */
    for(ptr=search_string; nw < 20; ptr=strchr(ptr,' '))
    {
        if(!ptr)
            break;
        else if(*ptr == ' ')
        {
            while(*ptr == ' ')
            {
                *ptr = '\0';
                ptr++;
            }
            words[nw++] = ptr;
        }
        else
        {
            words[nw++] = ptr;
        }
    }

    n_search_results = 0;
    free(search_results);
    search_results = NULL;

    for(entry=0; entry < n_id[active]; entry++)
    {
        int match = 0; 
        e = subset[active][entry];
        
        if (nw == 0   ||        /* No words yet */
            (nw == 1 && words[0][0] == '\0') || /* empty word */
            (nw == 1 && strlen(words[0]) == 1 && /* only one character */
             ((e->title &&
              strchr(e->title,words[0][0])) || strchr(e->filename,words[0][0]))))
        {

            match=1;
        }
        else
            if (nw == 1 && strlen(words[0]) == 1)
            match=0;
        else
        {
            char song[256];
 
           for(ptr = e->title,i=0   ; ptr && *ptr && i < 254; i++, ptr++)
                song[i]=tolower(*ptr);
            for(ptr= e->filename,i=0 ; ptr && *ptr && i < 254; i++, ptr++)
                song[i]=tolower(*ptr);
            song[i] = '\0';
            match=songdb_JumpToFileMatch(song,words,nw);
        }

        if(match)
        {
            n_search_results++;
            search_results = realloc (search_results, n_search_results * DBENTRY_PTR_LEN);
            search_results[n_search_results - 1] = e;
        }
    }
}


