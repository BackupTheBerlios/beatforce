/*
  Beatforce/SDLTk

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003 John Beuving (john.beuving@wanadoo.nl)

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
#include <malloc.h>

#include "SDL_Widget.h"
#include "SDL_Menu.h"

const struct S_Widget_FunctionList SDL_Menu_FunctionList =
{
    SDL_MenuCreate,
    SDL_MenuDraw,
    NULL,
    NULL,
    NULL,
};


SDL_Widget* SDL_MenuCreate(SDL_Rect* rect)
{
    SDL_Widget *widget;
    SDL_Menu   *menu;

    menu=(SDL_Menu*) malloc(sizeof(SDL_Menu));
    widget=(SDL_Widget*)menu;

    widget->Type      = SDL_MENU;
    widget->Rect.x    = rect->x;
    widget->Rect.y    = rect->y;
    widget->Rect.w    = rect->w;
    widget->Rect.h    = rect->h;
    widget->Focusable = 0;
    widget->Visible   = 0;
  
    menu->string[0] = NULL;
    menu->string[1] = NULL;
    menu->OldWidget = NULL;
    return (SDL_Widget*)menu;
}

void SDL_MenuDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{ 
    SDL_Menu *menu=(SDL_Menu*)widget;

    SDL_FillRect(dest,&widget->Rect,0x00ffff);

}




void SDL_MenuAppend(SDL_Widget *widget,char *txt)
{
    SDL_Menu *menu=(SDL_Menu*)widget;
    if(menu->string[0] == NULL)
        menu->string[0]=strdup(txt);
    else
        menu->string[1]=strdup(txt);
    
}

void SDL_MenuPopup(SDL_Widget *widget)
{
    SDL_Menu *menu=(SDL_Menu*)widget;
    int x,y;

    if(menu->OldWidget == NULL)
        menu->OldWidget = malloc(sizeof(SDL_Menu));
    
    menu->OldWidget->Rect.x = widget->Rect.x;
    menu->OldWidget->Rect.y = widget->Rect.y;
    menu->OldWidget->Rect.w = widget->Rect.w;
    menu->OldWidget->Rect.h = widget->Rect.h;

    SDL_GetMouseState(&x,&y);
    widget->Rect.x = x;
    widget->Rect.y = y;

    
    SDL_WidgetHide(menu->OldWidget);
    SDL_WidgetShow(widget);
}