/*
  Beatforce/SDLTk

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003-2004 John Beuving (john.beuving@wanadoo.nl)

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
#include <string.h>

#include "SDL_Widget.h"
#include "SDL_WidTool.h"
#include "SDL_Tooltip.h"
#include "SDL_Font.h"
#include "SDL_Signal.h"

static void SDL_TooltipMouseMotionCB(SDL_Widget *Widget,SDL_Event *event);
static void SDL_TooltipHideCB(SDL_Widget *Widget);

const struct S_Widget_FunctionList SDL_Tooltip_FunctionList =
{
    NULL,
    SDL_TooltipDraw,
    NULL,
    NULL,
    NULL,
};


SDL_Widget* SDL_TooltipCreate(SDL_Widget *parent, char *text)
{
    SDL_Widget    *widget;
    SDL_Tooltip   *tooltip;

    tooltip=(SDL_Tooltip*) malloc(sizeof(SDL_Tooltip));
    widget=(SDL_Widget*)tooltip;

    widget->Type      = SDL_TOOLTIP;
    widget->Rect.x    = 0;
    widget->Rect.y    = 0;
    widget->Rect.w    = 10;
    widget->Rect.h    = 10;
    widget->Focusable = 0;
    widget->Visible   = 0;
  
    tooltip->string = strdup(text);
    tooltip->Parent = parent;
    tooltip->Font   = &DefaultFont;

    tooltip->Lines  = 1;

    SDL_SignalConnect(parent,"mousemotion",SDL_TooltipMouseMotionCB,widget);
    SDL_SignalConnect(parent,"hide",SDL_TooltipHideCB,widget);

    SDL_StoreWidget(widget);
    return widget;
}

void SDL_TooltipDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{ 
    SDL_Tooltip *Tooltip=(SDL_Tooltip*)widget;
    SDL_Rect r;

    r.x = widget->Rect.x ;
    r.y = widget->Rect.y;
    r.w = widget->Rect.w;
    r.h = widget->Rect.h;


    boxColor(dest,r.x,r.y,r.x + r.w-1,r.y + r.h-1,0xf1ff89ff);
    rectangleColor(dest,r.x,r.y,r.x + r.w-1,r.y + r.h-1,0x000000ff);
      
    if(Tooltip->Lines > 1)
    {
        char *str2;
        r.x = widget->Rect.x + 2;
        r.y = widget->Rect.y;
        r.w = widget->Rect.w;
        r.h = widget->Rect.h / 2;
        SDL_FontDrawStringRect(dest,Tooltip->Font,Tooltip->string,&r);
        r.x = widget->Rect.x + 4;
        r.y = widget->Rect.y + r.h;
        r.w = widget->Rect.w;
        r.h = widget->Rect.h / 2;
        str2 = Tooltip->string + strlen(Tooltip->string)/2;
        SDL_FontDrawStringRect(dest,Tooltip->Font,str2,&r);

    }
    else
    {
        r.x = widget->Rect.x + 2;
        r.y = widget->Rect.y;
        r.w = widget->Rect.w;
        r.h = widget->Rect.h;
        SDL_FontDrawStringRect(dest,Tooltip->Font,Tooltip->string,&r);
    }

}

Uint32 TimerCallback(Uint32 interval, void *param)
{
    SDL_Tooltip *Tooltip = (SDL_Tooltip*)param;
    SDL_Widget  *Widget  = (SDL_Widget*)param;
    SDL_Surface *VideoSurface;

    if(Widget->Visible == 0)
    {
        Widget->Rect.x = Tooltip->x;
        Widget->Rect.y = Tooltip->y;

        Widget->Rect.h = SDL_FontGetHeight(Tooltip->Font) + 4;
        Widget->Rect.w = SDL_FontGetStringWidth(Tooltip->Font,Tooltip->string) + 4;

        VideoSurface = SDL_GetVideoSurface();
        while(Widget->Rect.x + Widget->Rect.w > VideoSurface->clip_rect.x + VideoSurface->clip_rect.w)
        {
            Widget->Rect.h *= 2;
            Widget->Rect.w /= 2;
            Tooltip->Lines ++;
        }
        SDL_WidgetShow(Widget);
    }
    return 0;
}

static void SDL_TooltipHideCB(SDL_Widget *Widget)
{
    SDL_Tooltip *Tooltip=(SDL_Tooltip*)Widget;

    if(Widget->Visible)
    {
        Tooltip->Lines=1;
        SDL_WidgetHide(Widget);
        SDL_RemoveTimer(Tooltip->Timer);
    }
}

static void SDL_TooltipMouseMotionCB(SDL_Widget *Widget,SDL_Event *event)
{
    SDL_Tooltip *Tooltip=(SDL_Tooltip*)Widget;
    if(SDL_WidgetIsInside(Tooltip->Parent,event->motion.x,event->motion.y))
    {
        Tooltip->x = event->motion.x + 10;
        Tooltip->y = event->motion.y + 10;

        SDL_RemoveTimer(Tooltip->Timer);
        Tooltip->Timer=SDL_AddTimer(1000,TimerCallback,Tooltip);
        
    }
    else
    {
        Tooltip->Lines=1;
        SDL_WidgetHide(Widget);
        SDL_RemoveTimer(Tooltip->Timer);
    }
}

void SDL_TooltipSetFont(SDL_Widget *Widget,SDL_Font *Font)
{
    SDL_Tooltip *Tooltip=(SDL_Tooltip*)Widget;

    Tooltip->Font = Font;
}
