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

#ifndef __SDL_SCROLLBAR_H__
#define __SDL_SCROLLBAR_H__

#include "SDL_Widget.h"
#include "SDL_WidTool.h"

#define SCROLLBAR_IDLE      0x00
#define SCROLLBAR_DRAG      0x01

#define HORIZONTAL 1
#define VERTICAL   2

typedef struct SDL_Scrollbar
{
    SDL_Widget  Widget; /* Parent object */
   
    int Orientation;  // hor or vert
   
    int State;      // states of the widget, used for eventhandler

    
    int    MaxValue;        // maximum value (percentage = 100%)
    int    MinValue;        // minimum valie (percentage = 0%)
    int    CurrentValue;    // value of current position
    
    int    PixelValue;

    void (*Callback)();
    SDL_Widget *CallbackWidget;
}SDL_Scrollbar;

SDL_Widget* SDL_ScrollbarCreate(SDL_Rect* rect);
void SDL_ScrollbarDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area);
int SDL_ScrollbarEventHandler(SDL_Widget *widget,SDL_Event *event);

int SDL_ScrollbarGetCurrentValue(SDL_Widget *widget);

void SDL_ScrollbarSetCurrentValue(SDL_Widget *widget,int CurrentValue);
void SDL_ScrollbarSetMaxValue(SDL_Widget *widget,int MaxValue);
void SDL_ScrollbarSetCallback(SDL_Widget *widget,void *callback,SDL_Widget *returnwidget);

#endif /* __SDL_SCROLLBAR_H__ */
