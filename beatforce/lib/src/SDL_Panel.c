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
    SDL_PanelEventHandler,
    NULL,
};


void* SDL_PanelCreate(SDL_Rect* rect)
{
    SDL_Panel *panel;

    panel=(SDL_Panel*) malloc(sizeof(SDL_Panel));

    panel->rect.x  = rect->x;
    panel->rect.y  = rect->y;
    panel->rect.w  = rect->w;
    panel->rect.h  = rect->h;

    panel->color  = 0xfffff7;
    panel->redraw = 1;
    panel->image  = NULL;


    return panel;
}

void SDL_PanelDraw(void *panel,SDL_Surface *dest)
{
    SDL_Panel *Panel=(SDL_Panel*)panel;
    SDL_Rect src;

    src.x=0;
    src.y=0;
    if(Panel->rect.w == 0 && Panel->image)
        src.w=Panel->image->w;
    else
        src.w=Panel->rect.w;
    
    if(Panel->rect.h == 0 && Panel->image)
        src.h=Panel->image->h;
    else
        src.h=Panel->rect.h;
    
    
    if(Panel->redraw || SDL_WidgetNeedsRedraw())
    {
        if(Panel->image)
        {
            if(SDL_BlitSurface(Panel->image,&src,dest,&Panel->rect)<0)
                fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
        }
        else
        {
            SDL_FillRect(dest,&Panel->rect,Panel->color);
        }
        Panel->redraw=0;
    }
    
}

void  SDL_PanelProperties(void *panel,int feature,va_list list)
{
    SDL_Panel *Panel=(SDL_Panel*)panel;

    switch(feature)
    {
    case SET_NORMAL_IMAGE:
        if(Panel->image == NULL)
        {
            char *s = va_arg(list,char*);
            Panel->image = IMG_Load(s);
            if(Panel->image)
                SDL_SetColorKey(Panel->image,SDL_SRCCOLORKEY,TRANSPARANT);
        }
        break;
    case SET_BG_COLOR:
        Panel->color=va_arg(list,Uint32);
        Panel->redraw=1;
        break;
    case FORCE_REDRAW:
        Panel->redraw=1;
        break;
    }
}

void SDL_PanelEventHandler(void *panel,SDL_Event *event)
{


}





