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
#include "SDL_WidTool.h"
#include "SDL_ProgressBar.h"
#include "SDL_Signal.h"


/* Prototypes for Internal functions */
static void ProgressBar_DrawStripe(SDL_Surface *dest,SDL_ProgressBar *ProgressBar,int line,Uint32 color);
//static void ProgressBar_CurrentLine(SDL_ProgressBar * ProgressBar);
static void ProgressBar_CurrentValue(SDL_ProgressBar * ProgressBar);

/* Exported data */
const struct S_Widget_FunctionList SDL_ProgressBar_FunctionList =
{
    SDL_ProgressBarCreate,
    SDL_ProgressBarDraw,
    SDL_ProgressBarProperties,
    SDL_ProgressBarEventHandler,
    NULL,
};


SDL_Widget* SDL_ProgressBarCreate(SDL_Rect* rect)
{
    SDL_Widget      *widget;
    SDL_ProgressBar *progressbar;
    
    progressbar=(SDL_ProgressBar*)malloc(sizeof(SDL_ProgressBar));
    widget=(SDL_Widget*)progressbar;

    widget->Type    = SDL_PROGRESSBAR;
    widget->Rect.x  = rect->x;
    widget->Rect.y  = rect->y;
    widget->Rect.w  = rect->w;
    widget->Rect.h  = rect->h;

    if(rect->w > rect->h)
        progressbar->Orientation = HORIZONTAL;
    else
        progressbar->Orientation = VERTICAL;

    progressbar->color1  = 0x007f80;
    progressbar->color2  = 0xa8cbf1;

    /* Default range settings */
    progressbar->MaxValue      = 100;
    progressbar->MinValue      = 0;
    progressbar->CurrentValue  = 0;

    /* Used for drawing */
    progressbar->CurrentLine   = 0;
    progressbar->State         = PROGRESSBAR_NORMAL;

    return (SDL_Widget*)progressbar;
}

void SDL_ProgressBarDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{
    SDL_ProgressBar *ProgressBar=(SDL_ProgressBar*)widget;
    int line;
    int highlight=0;
    int Maxline=0;
    
    if(ProgressBar->Orientation == VERTICAL)
        Maxline = widget->Rect.h;
    else if(ProgressBar->Orientation == HORIZONTAL)
        Maxline = widget->Rect.w;

    SDL_FillRect(dest,&widget->Rect,BLACK);

    for( line=0; line < Maxline ; line+=2)
    {
        highlight= (line < ProgressBar->CurrentLine);
        if(highlight)
            ProgressBar_DrawStripe(dest,ProgressBar,line,ProgressBar->color2);
        else
            ProgressBar_DrawStripe(dest,ProgressBar,line,ProgressBar->color1);
    }
}

int  SDL_ProgressBarProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_ProgressBar *ProgressBar=(SDL_ProgressBar*)widget;

    switch(feature)
    {
    case SET_MAX_VALUE:
        ProgressBar->MaxValue = va_arg(list,int);
        break;
    case SET_MIN_VALUE:
        break;
     case GET_STATE:
    {
        int *value;
        value=va_arg(list,int*);
        if(value)
        {
            *value=ProgressBar->State;
        }
        break;
    }
    default:
        return 0;
        printf("%s%d\n",__FILE__,__LINE__);
        break;          
    
    }
    return 1;
}

int SDL_ProgressBarSetCurrentValue(SDL_Widget *widget,int value)
{
    SDL_ProgressBar *ProgressBar=(SDL_ProgressBar*)widget;

    if(ProgressBar->State == PROGRESSBAR_DRAG)
        return 0;
    if(ProgressBar->CurrentValue != value )
    {
        ProgressBar->CurrentValue = value; 
        if(ProgressBar->Orientation == VERTICAL)
        {
            ProgressBar->CurrentLine  = (value * widget->Rect.h) / 
                (ProgressBar->MaxValue - ProgressBar->MinValue); 
        }
        else
        {
            ProgressBar->CurrentLine  = (value * widget->Rect.w) 
                / (ProgressBar->MaxValue - ProgressBar->MinValue); 
        }
        
        SDL_WidgetRedrawEvent(widget);
    }
    return 0;
}

int SDL_ProgressBarEventHandler(SDL_Widget *widget, SDL_Event *event)
{

    SDL_ProgressBar *ProgressBar=(SDL_ProgressBar*)widget;
    
    switch(event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
        if(SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
        {
            if(ProgressBar->Orientation == HORIZONTAL)
            {
                ProgressBar->State=PROGRESSBAR_DRAG;
                ProgressBar->CurrentLine = event->motion.x - widget->Rect.x;
                ProgressBar_CurrentValue(ProgressBar);
                SDL_WidgetDraw(widget,&widget->Rect);
            }
        }
        break;
    case SDL_MOUSEMOTION:
        if(ProgressBar->Orientation == HORIZONTAL)
        {
            if(ProgressBar->State==PROGRESSBAR_DRAG)
            {
                ProgressBar->CurrentLine = event->motion.x - widget->Rect.x;
                ProgressBar_CurrentValue(ProgressBar);
                SDL_WidgetDraw(widget,&widget->Rect);
            }
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if(ProgressBar->State == PROGRESSBAR_DRAG ||
           SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
        {
            if(ProgressBar->Orientation == VERTICAL)
            {
                ProgressBar->CurrentLine = event->motion.y - widget->Rect.y;
                ProgressBar_CurrentValue(ProgressBar);
                SDL_WidgetDraw(widget,&widget->Rect);
            }
            else if(ProgressBar->Orientation == HORIZONTAL)
            {
                ProgressBar->CurrentLine = event->motion.x - widget->Rect.x;
                ProgressBar_CurrentValue(ProgressBar);
                SDL_WidgetDraw(widget,&widget->Rect);
            }
            ProgressBar->State=PROGRESSBAR_NORMAL;
            SDL_SignalEmit(widget,"clicked");
        }
        break;
    default:
        break;
    }
    return 0;

}


/* Internal Helper functions */


void ProgressBar_DrawStripe(SDL_Surface *dest,SDL_ProgressBar *ProgressBar,int line,Uint32 color)
{
    SDL_Widget *widget=(SDL_Widget*)ProgressBar;
    SDL_Rect rect;

    if(ProgressBar->Orientation == VERTICAL)
    {
        rect.x = widget->Rect.x;
        rect.y = widget->Rect.y + widget->Rect.h - line;
        rect.w = widget->Rect.w;
        rect.h = 1;
    }
    else if(ProgressBar->Orientation == HORIZONTAL)
    {
        rect.x = widget->Rect.x + line;
        rect.y = widget->Rect.y;
        rect.w = 1;
        rect.h = widget->Rect.h;
    }
    SDL_FillRect(dest,&rect,color);
}


static void ProgressBar_CurrentValue(SDL_ProgressBar * ProgressBar)
{
    SDL_Widget *widget=(SDL_Widget*)ProgressBar;
    int BarRange=1; // in pixels

    if(ProgressBar->CurrentLine < 0)
        ProgressBar->CurrentLine = 0;

    if(ProgressBar->Orientation == HORIZONTAL)
    {
        BarRange = widget->Rect.w;
    }
    else if(ProgressBar->Orientation == VERTICAL)
    {
        BarRange = widget->Rect.h;
    }
    if(ProgressBar->CurrentLine > BarRange)
        ProgressBar->CurrentLine = BarRange;
    
    ProgressBar->CurrentValue  = (ProgressBar->MaxValue - ProgressBar->MinValue)*ProgressBar->CurrentLine;
    ProgressBar->CurrentValue /= BarRange;
    ProgressBar->CurrentValue += ProgressBar->MinValue;
}

double SDL_ProgressBarGetCurrentValue(SDL_Widget *widget)
{
    SDL_ProgressBar *ProgressBar=(SDL_ProgressBar*)widget;
    return ProgressBar->CurrentValue;
}


