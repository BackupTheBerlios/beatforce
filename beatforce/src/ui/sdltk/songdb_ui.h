
/*
  Beatforce/ Song db user interface prototypes

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

#ifndef __SONGDB_UI_H__
#define __SONGDB_UI_H__

#include "SDL_Widget.h"
#include "theme.h"

typedef struct SongdbWidgets
{
    SDL_Widget *Tabs;
}SongdbWidgets;

void SONGDBUI_GroupChanged(void *w);
void* SONGDBUI_CreateWindow(ThemeSongdb *ts);



#endif /* __SONGDB_UI_H__ */
