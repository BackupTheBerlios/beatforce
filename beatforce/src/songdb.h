/*
   BeatForce
   songdb.h  -	song database
   
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
#ifndef __SONGDB_H__
#define __SONGDB_H__

#include "configfile.h"

struct SongAddInfo
{

    char *type;			/* string like MPEG 1, layer 3 */
    
    int n_ch;			/* number of channels */
    
    int vbr;			/* VBR ? */
    long bitrate; 		/* eg. 192000 Bit/s */
    int sample_bits;		/* bits per sample */
    long SampleRate;		/* Sample rate	44100 Samples/s */
    
    int copyright;		/* copyright */
    int original; 		/* original */
    int err_protection;		/* error protection */
    
    long n_frames;		/* frames or samples */
    
    char *add_text;		/* additional description */
};

struct SongDBEntry
{
    unsigned long id;		/* unique ID in song database */
    
    long type;			/* type */
    char *dir;			/* directory */
    long index;			/* index */
    
    char *artist; 		/* artist */
    char *title;			/* song title */
    char *album;			/* album */
    char *comment;		/* comment */
    char *year;			/* year */
    char *genre;			/* genre */
    
    long time;			/* time in msec */
    float bpm;			/* yes, BeatsPerMinute */
    
    char *filename;		/* guess what */
    long filesize;		/* ditto */
    
    struct SongAddInfo *AddInfo; /* additional info */
    
    int played;			/* played ? */
};

typedef struct SongDBTest
{
    int index;
    struct SongDBTest *next;
    struct SongDBTest *prev;

}SongDBTest;

typedef struct SongDBGroup
{
    char *Name;
    int Changed;
    struct SongDBSubgroup *Active;
    struct SongDBSubgroup *Subgroup;
    struct SongDBGroup *next;
    struct SongDBGroup *prev;
}SongDBGroup;

typedef struct SongDBSubgroup
{
    char *Name;
    int Songcount;
    struct SongDBEntry **Playlist;
    struct SongDBSubgroup *next;
    struct SongDBSubgroup *prev;
}SongDBSubgroup;

#define SONG_ADD_INFO_LEN       sizeof( struct SongAddInfo )
#define DBENTRY_PTR_LEN 	sizeof( struct SongDBEntry * )
#define DBENTRY_LEN		sizeof( struct SongDBEntry )

#define SONGDB_ID_UNKNOWN	0xffffff00	/* id not specified in songdb_search */

int SONGDB_Init (SongDBConfig * our_cfg);
int SONGDB_AddFilename(char *filename);
void SONGDB_FreeActiveList();
struct SongDBEntry *SONGDB_GetEntryID(long id);
struct SongDBEntry *SONGDB_GetSearchEntry(long id);
long SONGDB_GetNoOfEntries (void);
long SONGDB_GetNoOfSearchResults(void);
void SONGDB_FindEntry(char *search_string);

/* Group related functions */
int SONGDB_GroupChanged();

int SONGDB_SetActiveSubgroup(int which);

/* Subgroup modifiers */
int SONGDB_AddSubgroup(char *title);
int SONGDB_RemoveSubgroup(int which);
int SONGDB_RenameSubgroup(int which, char *title);
struct SongDBSubgroup *SONGDB_GetSubgroup(int which);
int SONGDB_SubgroupCount();
int SONGDB_AddFileTo(int which,char *filename);
int SONGDB_SetActiveSubgroup(int which);
struct SongDBSubgroup *SONGDB_GetActiveSubgroup();
#endif
