/*
  Beatforce/SDLTk

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

#include "SDL_Widget.h"
#include "SDL_Font.h"

typedef struct SDL_Panel
{
    SDL_Rect rect;
    SDL_Surface *image;
    int redraw;
    Uint32  color;

}SDL_Panel;


// prototypes
void*  SDL_PanelCreate(SDL_Rect *rect);
void   SDL_PanelDraw(void *panel,SDL_Surface *dest);
void   SDL_PanelHandler(void * panel,SDL_Event *event);
void   SDL_PanelProperties(void *panel,int feature,va_list list);
void   SDL_PanelEventHandler(void *panel,SDL_Event *event);

