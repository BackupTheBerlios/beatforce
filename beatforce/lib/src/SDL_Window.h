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
#ifndef __SDL_WINDOW_H__
#define __SDL_WINDOW_H__

#include <SDL/SDL.h> 
#include "SDL_Widget.h"


#define SDLTK_WIDGET_EVENT SDL_USEREVENT

#define SDLTK_WIDGET_HIDE     1
#define SDLTK_WIDGET_MOVE     2
#define SDLTK_WIDGET_SHOW     3
#define SDLTK_WIDGET_REDRAW   4
#define SDLTK_WIDGET_RESIZE   5
#define SDLTK_WINDOW_REDRAW   6

/* Widget list for a specific surface */
typedef struct SDL_WidgetList
{
    SDL_Widget *Widget;
    struct SDL_WidgetList *Parent;

    struct SDL_WidgetList *Child;
    struct SDL_WidgetList *Next;
}SDL_WidgetList;


/* Window specified by the use */
typedef struct SDL_Window
{
    SDL_Rect Dimensions;
    int (*EventHandler)(SDL_Event event);
    int (*NotifyRedraw)();

    void *TransferData;

    int Visible;
    SDL_Widget       *FocusWidget;
    SDL_WidgetList   *WidgetList;
}SDL_Window;


/* Surface with widget settings */
typedef struct SDL_WindowList
{
    SDL_Surface      *parent;

    SDL_Window       *Window;
    struct SDL_WindowList *Next;
}SDL_WindowList;

/* Initialize the main video window */
int SDL_WindowInit(SDL_Surface *s,int width,int height,int bpp);

/* Create a new window */
SDL_Window *SDL_WindowNew(int x,int y,int width,int height);

/* Open the newly created window */
void SDL_WindowOpen(SDL_Window *window);

/* Close the top most window */
void SDL_WindowClose();

/* Main function , a window has to be created before calling this */
int SDLTK_Main();



int SDL_WidgetActive(SDL_Widget *widget);
SDL_WidgetList *SDL_WindowGetWidgetList();
/* Adds a widget to a window, this is done after 
   creating of the widgets */
void SDL_WindowAddWidget(SDL_Widget *widget);

void SDL_WindowSetFocusWidget(SDL_Widget *focus_widget);
SDL_Widget *SDL_WindowGetFocusWidget();

#endif /* __SDL_WINDOW_H__ */
