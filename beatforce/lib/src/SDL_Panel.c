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
#include <stdarg.h>
#include <malloc.h>

#include "SDL_Widget.h"
#include "SDL_Panel.h"

const struct S_Widget_FunctionList SDL_Panel_FunctionList =
{
    SDL_PanelCreate,
    SDL_PanelDraw,
    SDL_PanelProperties,
    NULL,
    NULL,
};


SDL_Widget* SDL_PanelCreate(SDL_Rect* rect)
{
    SDL_Widget *widget;
    SDL_Panel  *panel;

    panel=(SDL_Panel*) malloc(sizeof(SDL_Panel));
    widget=(SDL_Widget*)panel;

    widget->Type      = SDL_PANEL;
    widget->Rect.x    = rect->x;
    widget->Rect.y    = rect->y;
    widget->Rect.w    = rect->w;
    widget->Rect.h    = rect->h;
    widget->Focusable = 0;
    
    panel->color  = WHITE;
    panel->Image  = NULL;

    return (SDL_Widget*)panel;
}

void SDL_PanelDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{
    SDL_Panel *Panel=(SDL_Panel*)widget;
    SDL_Rect src;

    if(Panel->Image)
    {
        

        if(Area != NULL)
        {
            src.x = Area->x - widget->Rect.x;
            src.y = Area->y - widget->Rect.y;
            src.w = Area->w;
            src.h = Area->h;

            SDL_BlitSurface(Panel->Image,&src,dest,Area);
        }
        else
        {
            src.x = 0;
            src.y = 0;
            src.w = widget->Rect.w;
            src.h = widget->Rect.h;

            SDL_BlitSurface(Panel->Image,&src,dest,&widget->Rect);            
        }

           
    }
    else
    {
        SDL_FillRect(dest,&widget->Rect,Panel->color);
    }
    
}

int SDL_PanelProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Panel *Panel=(SDL_Panel*)widget;

    switch(feature)
    {
    case SET_BG_COLOR:
        Panel->color=va_arg(list,Uint32);
        break;
    default:
        return 0;
    }
    return 1;
}




void SDL_PanelSetImage(SDL_Widget *widget,SDL_Surface *image)
{
    SDL_Panel *Panel=(SDL_Panel*)widget;
    
    if(image == NULL)
        return;

    if(Panel->Image == NULL)
    {
        Panel->Image = image;
        SDL_SetColorKey(Panel->Image,SDL_SRCCOLORKEY,TRANSPARANT);

        if(widget->Rect.w == 0 && Panel->Image)
            widget->Rect.w=Panel->Image->w;
        
        if(widget->Rect.h == 0 && Panel->Image)
            widget->Rect.h=Panel->Image->h;

    }
}


