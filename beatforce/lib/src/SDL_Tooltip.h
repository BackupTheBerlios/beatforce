/*
  Beatforce/SDLTk

  one line to give the program's name and an idea of what it does.
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

#ifndef __SDL_TOOLTIP_H__
#define __SDL_TOOLTIP_H__

#include <SDL/SDL.h>
#include "SDL_Widget.h"
#include "SDL_Font.h"

typedef struct SDL_Tooltip
{
    SDL_Widget Widget;
    SDL_Widget *Parent;

    SDL_TimerID  Timer;

    SDL_Font     *Font;

    int Lines;

    int x;
    int y;

    char *string;
}SDL_Tooltip;


SDL_Widget* SDL_TooltipCreate(SDL_Widget *parent, char *text);
void SDL_TooltipDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area);
void SDL_TooltipSetFont(SDL_Widget *Widget,SDL_Font *Font);

#endif /* __SDL_TOOLTIP_H__ */
