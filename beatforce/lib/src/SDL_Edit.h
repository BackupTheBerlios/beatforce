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
#ifndef __SDL_EDIT_H__
#define __SDL_EDIT_H__

#include "SDL_Widget.h"
#include "SDL_Font.h"

typedef struct SDL_Edit
{
    SDL_Widget  Widget;
    SDL_Font   *Font;

    char    *Caption;
    int      Redraw;
    int      Shift;
    int      Focus;

    void (*AnyKeyPressCallback)();
    void *AnyKeyData;

    void (*ReturnPressCallback)();
    void *ReturnKeyData;

    Uint32   fgcolor;
    Uint32   bgcolor;

}SDL_Edit;


/* prototypes */
SDL_Widget* SDL_EditCreate(SDL_Rect *rect);
void        SDL_EditDraw(SDL_Widget *widget,SDL_Surface *dest);
int         SDL_EditProperties(SDL_Widget *widget,int feature,va_list list);
int         SDL_EditEventHandler(SDL_Widget *widget,SDL_Event *event);


#endif /* __SDL_EDIT_H__ */
