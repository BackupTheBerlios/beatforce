/*
  Beatforce/SDLTk

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

#ifndef __SDL_PANEL_H__
#define __SDL_PANEL_H__

#include "SDL_Widget.h"
#include "SDL_Font.h"

typedef struct SDL_Panel
{
    SDL_Widget Widget;
    SDL_Surface *Image;

    Uint32  color;
}SDL_Panel;


// prototypes
SDL_Widget*  SDL_PanelCreate(SDL_Rect *rect);
void         SDL_PanelDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area);
int          SDL_PanelProperties(SDL_Widget *widget,int feature,va_list list);


void SDL_PanelSetImage(SDL_Widget *widget,SDL_Surface *image);


#endif /*  __SDL_PANEL_H__ */
