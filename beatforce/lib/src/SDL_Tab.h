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

#ifndef __SDL_TAB_H
#define __SDL_TAB_H

#include "SDL_Widget.h"
#include "SDL_Button.h"
#include "SDL_Edit.h"
#include "SDL_Font.h"

typedef struct SDL_TabList
{
    char                    *caption;
    int index;
    SDL_Rect                *rect;
    int                     draw;
    struct SDL_TabList      *next;
    struct SDL_TabList      *prev;
}SDL_TabList;


typedef struct SDL_TabImage
{
    SDL_Surface *surface;
    int slopewidth;
}SDL_TabImage;

typedef struct SDL_Tab
{
    SDL_TabImage      *normal;
    SDL_TabImage      *highlighted;

    SDL_Rect          *rect;
    SDL_TabList       *tabs;
    SDL_TabList       *hl;

    SDL_Edit          *edit;
    SDL_Button        *ArrowLeft;
    SDL_Button        *ArrowRight;
    SDL_Font          *font;
    int NoOfTabs;

    int startx;
    int maxstartx;
    int doesntfit;
    int             min_width;  //the minimum width a tab may have
    int             min_height; //the minimum height a tab may have
    int             overlay;   //overlay of tabs in pixels
    Uint32          bgcolor;
    Uint32          fgcolor;

    void (*OnClicked)();
    void (*OnReturn) ();

    struct SDL_Tab       *next;
}SDL_Tab;



void *SDL_TabCreate(SDL_Rect *rect);
void SDL_TabDraw(void *data,SDL_Surface *dest);
int SDL_TabProperties(void *tab,int feature,va_list list);
void SDL_TabEventHandler(void * tab,SDL_Event *event);

#endif //__SDL_TAB_H






