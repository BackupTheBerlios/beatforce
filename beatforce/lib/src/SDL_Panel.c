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

    widget->Type    = SDL_PANEL;
    widget->Rect.x  = rect->x;
    widget->Rect.y  = rect->y;
    widget->Rect.w  = rect->w;
    widget->Rect.h  = rect->h;

    panel->color  = 0xfffff7;
    panel->Redraw = 1;
    panel->Image  = NULL;

    panel->Visible = 1;
    return panel;
}

void SDL_PanelDraw(SDL_Widget *widget,SDL_Surface *dest)
{
    SDL_Panel *Panel=(SDL_Panel*)widget;
    SDL_Rect src;

    if(Panel->Visible == 0)
        return;

    src.x=0;
    src.y=0;
    if(widget->Rect.w == 0 && Panel->Image)
        src.w=Panel->Image->w;
    else
        src.w=widget->Rect.w;
    
    if(widget->Rect.h == 0 && Panel->Image)
        src.h=Panel->Image->h;
    else
        src.h=widget->Rect.h;
    
    
    if(Panel->Redraw)
    {
        if(Panel->Image)
        {
            if(SDL_BlitSurface(Panel->Image,&src,dest,&widget->Rect)<0)
                ;
        }
        else
        {
            SDL_FillRect(dest,&widget->Rect,Panel->color);
        }
        Panel->Redraw=0;
    }
    
}

int SDL_PanelProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Panel *Panel=(SDL_Panel*)widget;

    switch(feature)
    {
    case SET_IMAGE:
        if(Panel->Image == NULL)
        {
            Panel->Image = va_arg(list,SDL_Surface*);
            if(Panel->Image)
                SDL_SetColorKey(Panel->Image,SDL_SRCCOLORKEY,TRANSPARANT);
        }
        break;
    case SET_BG_COLOR:
        Panel->color=va_arg(list,Uint32);
        Panel->Redraw=1;
        break;
    case FORCE_REDRAW:
        Panel->Redraw=1;
        break;
    case SET_VISIBLE:
        Panel->Visible=va_arg(list,int);
        Panel->Redraw=1;
        break;

    default:
        break;
    }
    return 1;
}







