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
#include <stdarg.h>

#include <SDL/SDL.h>
#include "SDL_Widget.h"

typedef enum
{
    SDL_BUTTON_DOWN,
    SDL_BUTTON_UP,
    SDL_BUTTON_HIGHLIGHTED
}SDL_ButtonStates;

typedef struct SDL_Button
{
    SDL_Widget       Widget;
    SDL_Surface      *disabled;
    SDL_Surface      *normal;
    SDL_Surface      *highlighted;
    SDL_Surface      *pressed;
    SDL_ButtonStates state;
    int              Visible;

    void (*Clicked)(void* data);
    void *ClickedData;
    char             *caption;

    struct SDL_Button       *next;
}SDL_Button;

/*
void UpdateButtons(SDL_Surface *dest,int x, int y, int pressed);
void CreateButton(int x, int y);
*/

SDL_Widget* SDL_ButtonCreate(SDL_Rect *rect);
void        SDL_ButtonDraw(SDL_Widget *widget,SDL_Surface *dest);
int         SDL_ButtonProperties(SDL_Widget *widget,int feature,va_list list);
void        SDL_ButtonEventHandler(SDL_Widget *widget,SDL_Event *event);
void        SDL_ButtonClose(SDL_Widget *widget);
