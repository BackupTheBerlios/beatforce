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
#include "SDL_ProgressBar.h"


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


void* SDL_ProgressBarCreate(SDL_Rect* rect)
{
    SDL_ProgressBar *progressbar;

    progressbar=(SDL_ProgressBar*)malloc(sizeof(SDL_ProgressBar));

    progressbar->rect.x  = rect->x;
    progressbar->rect.y  = rect->y;
    progressbar->rect.w  = rect->w;
    progressbar->rect.h  = rect->h;

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

    /* Event callback SDL_CLICKED */
    progressbar->State         = PROGRESSBAR_NORMAL;
    progressbar->OnClicked     = NULL;
    progressbar->OnClickedData = NULL;

    /* Force initial redraw */
    progressbar->Redraw  = 1;

    return progressbar;
}

void SDL_ProgressBarDraw(void *progressbar,SDL_Surface *dest)
{
    SDL_ProgressBar *ProgressBar=(SDL_ProgressBar*)progressbar;
    int line;
    int highlight=0;
    int Maxline=0;
    
    if(ProgressBar->Orientation == VERTICAL)
        Maxline = ProgressBar->rect.h;
    else if(ProgressBar->Orientation == HORIZONTAL)
        Maxline = ProgressBar->rect.w;

    if(ProgressBar->Redraw || SDL_WidgetNeedsRedraw())
    {
        SDL_FillRect(dest,&ProgressBar->rect,BLACK);

        for( line=0; line < Maxline ; line+=2)
        {
            highlight= (line < ProgressBar->CurrentLine);
            if(highlight)
                ProgressBar_DrawStripe(dest,ProgressBar,line,ProgressBar->color2);
            else
                ProgressBar_DrawStripe(dest,ProgressBar,line,ProgressBar->color1);
        }
        ProgressBar->Redraw = 0;
    }
}

int  SDL_ProgressBarProperties(void *progressbar,int feature,va_list list)
{
    SDL_ProgressBar *ProgressBar=(SDL_ProgressBar*)progressbar;
    double val;

    switch(feature)
    {
    case SET_MAX_VALUE:
        ProgressBar->MaxValue = va_arg(list,int);
        break;
    case SET_MIN_VALUE:
        break;
    case SET_CUR_VALUE:
        val=va_arg(list,double);
        if(ProgressBar->State == PROGRESSBAR_DRAG)
            return 0;
        ProgressBar->CurrentValue = val; 
        if(ProgressBar->Orientation == VERTICAL)
            ProgressBar->CurrentLine  = (val * ProgressBar->rect.h) / (ProgressBar->MaxValue - ProgressBar->MinValue); 
        else
            ProgressBar->CurrentLine  = (val * ProgressBar->rect.w) / (ProgressBar->MaxValue - ProgressBar->MinValue); 
        ProgressBar->Redraw       = 1;
        break;
    case SET_CALLBACK:
    {
        int event=va_arg(list,int);
        if(event== SDL_CLICKED)
        {
            ProgressBar->OnClicked     = va_arg(list,void*);
            ProgressBar->OnClickedData = va_arg(list,void*);
        }
        break;
    }
    case GET_CUR_VALUE:
    {
        double *value;
        value=va_arg(list,double*);
        if(value)
        {
            *value=ProgressBar->CurrentValue;
        }
    }
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
    break;

    }
    return 1;
}

void SDL_ProgressBarEventHandler(void *progressbar, SDL_Event *event)
{
    SDL_ProgressBar *ProgressBar=(SDL_ProgressBar*)progressbar;
    
    switch(event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
        if(SDL_WidgetIsInside(&ProgressBar->rect,event->motion.x,event->motion.y))
        {
            if(ProgressBar->Orientation == HORIZONTAL)
            {
                ProgressBar->State=PROGRESSBAR_DRAG;
                ProgressBar->CurrentLine = event->motion.x - ProgressBar->rect.x;
                ProgressBar_CurrentValue(ProgressBar);
            }
        }
        break;
    case SDL_MOUSEMOTION:
        if(ProgressBar->Orientation == HORIZONTAL)
        {
            if(ProgressBar->State==PROGRESSBAR_DRAG)
            {
                ProgressBar->CurrentLine = event->motion.x - ProgressBar->rect.x;
                ProgressBar_CurrentValue(ProgressBar);
            }
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if(ProgressBar->State == PROGRESSBAR_DRAG ||
           SDL_WidgetIsInside(&ProgressBar->rect,event->motion.x,event->motion.y))
        {
            if(ProgressBar->Orientation == VERTICAL)
            {
                ProgressBar->CurrentLine = event->motion.y - ProgressBar->rect.y;
                ProgressBar_CurrentValue(ProgressBar);
            }
            else if(ProgressBar->Orientation == HORIZONTAL)
            {
                ProgressBar->CurrentLine = event->motion.x - ProgressBar->rect.x;
                ProgressBar_CurrentValue(ProgressBar);
            }
            ProgressBar->State=PROGRESSBAR_NORMAL;
            if(ProgressBar->OnClicked)
                ProgressBar->OnClicked(ProgressBar->OnClickedData);
        }
        break;
    default:
        break;
    }


}


/* Internal Helper functions */


void ProgressBar_DrawStripe(SDL_Surface *dest,SDL_ProgressBar *ProgressBar,int line,Uint32 color)
{
    SDL_Rect rect;

    if(ProgressBar->Orientation == VERTICAL)
    {
        rect.x = ProgressBar->rect.x;
        rect.y = ProgressBar->rect.y + ProgressBar->rect.h - line;
        rect.w = ProgressBar->rect.w;
        rect.h = 1;
    }
    else if(ProgressBar->Orientation == HORIZONTAL)
    {
        rect.x = ProgressBar->rect.x + line;
        rect.y = ProgressBar->rect.y;
        rect.w = 1;
        rect.h = ProgressBar->rect.h;
    }
    SDL_FillRect(dest,&rect,color);
}


static void ProgressBar_CurrentValue(SDL_ProgressBar * ProgressBar)
{
    int BarRange=1; // in pixels

    if(ProgressBar->CurrentLine < 0)
        ProgressBar->CurrentLine = 0;

    if(ProgressBar->Orientation == HORIZONTAL)
    {
        BarRange = ProgressBar->rect.w;
    }
    else if(ProgressBar->Orientation == VERTICAL)
    {
        BarRange = ProgressBar->rect.h;
    }
    if(ProgressBar->CurrentLine > BarRange)
        ProgressBar->CurrentLine = BarRange;
    
    ProgressBar->CurrentValue  = (ProgressBar->MaxValue - ProgressBar->MinValue)*ProgressBar->CurrentLine;
    ProgressBar->CurrentValue /= BarRange;
    ProgressBar->CurrentValue += ProgressBar->MinValue;
    ProgressBar->Redraw=1;
}



