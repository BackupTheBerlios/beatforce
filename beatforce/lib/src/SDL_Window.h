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
#ifndef __SDL_WINDOW_H__
#define __SDL_WINDOW_H__

#include <SDL/SDL.h> 
#include "SDL_Widget.h"

/* Window specified by the use */
typedef struct SDL_Window
{
    int (*EventHandler)(SDL_Event event);
    int (*NotifyRedraw)();

    SDL_Surface *Surface;
    void *TransferData;
}SDL_Window;

/* Widget list for a specific surface */
typedef struct SDL_WidgetList
{
    SDL_Widget *Widget;
    struct SDL_WidgetList *Parent;
    struct SDL_WidgetList *Child;
    struct SDL_WidgetList *Next;
}SDL_WidgetList;


/* Surface with widget settings */
typedef struct SDL_Screen
{
    SDL_Surface      *Surface;
    SDL_WidgetList   *WidgetList;
    SDL_Surface      *parent;
    struct SDL_Screen *next;
}SDL_Screen;

int SDLTK_Main();

int SDL_WindowInit(SDL_Surface *s);
int SDL_StackNewScreen(SDL_Surface *surface);
int SDL_ActiveSurface(SDL_Surface *surface);

int SDL_NewSurfaceStack(SDL_Surface *surface);
SDL_Surface* SDL_GetSurfaceStack();
SDL_WidgetList *SDL_StackGetSurfaceStack(SDL_Surface *surface);

void SDL_StoreWidget(SDL_Widget *widget);
SDL_WidgetList *SDL_StackGetStack(SDL_Surface *surface);
SDL_WidgetList *SDL_StackGetLastItem();
void SDL_StackSetFocus(SDL_Widget *focus_widget);
SDL_Widget *SDL_StackGetFocus();


void SDL_WindowOpen(SDL_Window *window);
void SDL_WindowClose();

void SDL_WidgetDraw(SDL_Widget *widget,SDL_Rect *Rect);

#endif /* __SDL_WINDOW_H__ */
