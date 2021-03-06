/*
  Beatforce/ Playlist header file

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
#ifndef __PLAYLIST_H__
#define __PLAYLIST_H__

struct PlEntry
{
    struct SongDBEntry *e;
    long unique;
    int  index;
    struct PlEntry *next;
};

enum
{
    MODE_SINGLE,
    MODE_DOUBLE,
}E_PlaylistMode;

#define PL_ENTRY_LEN  ( sizeof(struct PlEntry) )
#define PLAYLIST_UNIQUE_UNKNOWN     0xffffff00


int PLAYLIST_Init (int player_nr);
int PLAYLIST_GetNoOfEntries(int player_nr);
struct PlEntry *PLAYLIST_GetSong (int player_nr, int no);
void PLAYLIST_AddEntry(int player_nr, struct SongDBEntry *e);
int PLAYLIST_Remove(int player_nr,struct SongDBEntry *e);

#endif /* __PLAYLIST_H__ */


