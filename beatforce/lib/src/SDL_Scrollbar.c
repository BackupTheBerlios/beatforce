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
#include <stdlib.h>

#include <SDLTk.h>


const struct S_Widget_FunctionList SDL_Scrollbar_FunctionList =
{
    SDL_ScrollbarCreate,
    SDL_ScrollbarDraw,
    NULL,
    SDL_ScrollbarEventHandler,
    NULL
};

SDL_Widget* SDL_ScrollbarCreate(SDL_Rect* rect)
{
    SDL_Scrollbar *scrollbar;
    SDL_Widget *widget;

    scrollbar = (SDL_Scrollbar*) malloc (sizeof(SDL_Scrollbar));
    widget    = (SDL_Widget*) scrollbar;
    
    widget->Type      = SDL_SCROLLBAR;
    widget->Rect.x    = rect->x;
    widget->Rect.y    = rect->y;
    widget->Rect.w    = rect->w;
    widget->Rect.h    = rect->h;
    widget->Focusable = 0;

    if(rect->w > rect->h)
        scrollbar->Orientation = HORIZONTAL;
    else
        scrollbar->Orientation = VERTICAL;

    scrollbar->State        = SCROLLBAR_IDLE;

    /* default range settings */
    scrollbar->MinValue           = 0;
    scrollbar->MaxValue           = 100;
    scrollbar->CurrentValue       = 0;
    scrollbar->PixelValue         = 0;

    scrollbar->Callback           = NULL;
    scrollbar->CallbackWidget     = NULL;
    return (SDL_Widget*)scrollbar;
}


void SDL_ScrollbarDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{
    // drawing order is always background, line , button
    SDL_Scrollbar *Scrollbar=(SDL_Scrollbar*)widget;
 
    {   
        SDL_Rect R;
        int x,y,height,width;

        SDL_FillRect(dest,&widget->Rect,0xf4f0e8);
            
        R.x = widget->Rect.x;
        R.y = widget->Rect.y + Scrollbar->PixelValue;
        R.w = widget->Rect.w;
        R.h = widget->Rect.h/Scrollbar->MaxValue;
        SDL_FillRect(dest,&R,0xd4d0c8);

        x      = widget->Rect.x;
        y      = widget->Rect.y + Scrollbar->PixelValue;
        width  = widget->Rect.w;
        height = widget->Rect.h/Scrollbar->MaxValue;

        if(height < 6)
            height = 6;

            
        {
            SDL_DrawLine (dest,x, y + height - 1, x + width - 1, y + height - 1,0x404040ff);
            SDL_DrawLine (dest,x + width - 1, y, x + width - 1, y + height - 1,0x404040ff);
                
            SDL_DrawLine (dest,x + 1, y + height - 2, x + width - 2, y + height - 2,0x808080ff);
            SDL_DrawLine (dest,x + width - 2, y + 1, x + width - 2, y + height - 2,0x808080ff);
                
            SDL_DrawLine (dest,x + 1, y + 1, x + width - 2, y + 1,0xffffffff);
            SDL_DrawLine (dest,x + 1, y + 1, x + 1, y + height - 2,0xffffffff);
                
            SDL_DrawLine (dest,x, y, x + width - 1, y,0xd4d0c8ff);
            SDL_DrawLine (dest,x, y, x, y + height - 1,0xd4d0c8ff);       
        }
    }
}


int SDL_ScrollbarEventHandler(SDL_Widget *widget,SDL_Event *event)
{
    SDL_Scrollbar *Scrollbar=(SDL_Scrollbar*)widget;
    int changed=0;

    switch(event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
        {
            int motion;
            int height;

            motion = event->motion.y;
            height = widget->Rect.h/Scrollbar->MaxValue;   /* Height is the button height */
            if(height < 6)
                height = 6;

            if(event->button.button == SDL_BUTTON_LEFT)
            {
                /* if the button click is below the button */
                if(motion > (widget->Rect.y + Scrollbar->PixelValue + height))
                {
                    if(event->motion.x >= widget->Rect.x)
                        if(event->motion.x <= (widget->Rect.x + widget->Rect.w ))
                        {
                            Scrollbar->CurrentValue++;
                            Scrollbar->PixelValue=Scrollbar->CurrentValue * widget->Rect.h / Scrollbar->MaxValue;
                            changed=1;
                        }
                }
                else if (motion > widget->Rect.y + Scrollbar->PixelValue)
                {
                    if(event->motion.x >= widget->Rect.x)
                        if(event->motion.x <= (widget->Rect.x + widget->Rect.w ))
                            Scrollbar->State=SCROLLBAR_DRAG;
                }
                else
                {
                    if(event->motion.x >= widget->Rect.x)
                        if(event->motion.x <= (widget->Rect.x + widget->Rect.w ))
                        {
                            Scrollbar->CurrentValue--;
                            Scrollbar->PixelValue=Scrollbar->CurrentValue * widget->Rect.h / Scrollbar->MaxValue;
                            changed=1;
                        }
                }
                
            }

        }
        break;
    case SDL_MOUSEBUTTONUP:
        if(Scrollbar->State == SCROLLBAR_DRAG)
        {
            Scrollbar->State=SCROLLBAR_IDLE;
        }
        break;
    case SDL_MOUSEMOTION:
        if(Scrollbar->State == SCROLLBAR_DRAG)
        {
            int height;
            height = widget->Rect.h
            /Scrollbar->MaxValue;   /* Height is the button height */
            if(height < 6)
                height = 6;

            Scrollbar->PixelValue   = event->motion.y - widget->Rect.y;
            if(Scrollbar->PixelValue >  widget->Rect.h - height)
                Scrollbar->PixelValue = widget->Rect.h - height;
            if(Scrollbar->PixelValue < 0)
                Scrollbar->PixelValue = 0;
            Scrollbar->CurrentValue = ((Scrollbar->MaxValue - Scrollbar->MinValue)* Scrollbar->PixelValue) 
                / widget->Rect.h;
            
            changed=1;
        }
        break;
    default:
        break;
    }

    if(changed)
    {
        SDL_WidgetRedrawEvent(widget);
        if(Scrollbar->Callback)
            Scrollbar->Callback(Scrollbar->CallbackWidget);
    }
    return 0;
}

int SDL_ScrollbarGetCurrentValue(SDL_Widget *widget)
{
    SDL_Scrollbar *Scrollbar=(SDL_Scrollbar*)widget;

    return Scrollbar->CurrentValue;
    
}

void SDL_ScrollbarSetMaxValue(SDL_Widget *widget,int MaxValue)
{
    SDL_Scrollbar *Scrollbar=(SDL_Scrollbar*)widget;

    if(MaxValue < Scrollbar->CurrentValue)
        Scrollbar->CurrentValue = MaxValue;

    Scrollbar->MaxValue=MaxValue;

    Scrollbar->PixelValue   = Scrollbar->CurrentValue * widget->Rect.h / Scrollbar->MaxValue;
}

void SDL_ScrollbarSetCurrentValue(SDL_Widget *widget,int CurrentValue)
{
    SDL_Scrollbar *Scrollbar=(SDL_Scrollbar*)widget;

    if(CurrentValue < 0)
        CurrentValue = 0;

    if(CurrentValue >= Scrollbar->MaxValue)
        CurrentValue = Scrollbar->MaxValue - 1;

    Scrollbar->CurrentValue = CurrentValue;
    Scrollbar->PixelValue   = Scrollbar->CurrentValue * widget->Rect.h / Scrollbar->MaxValue;
}

void SDL_ScrollbarSetCallback(SDL_Widget *widget,void *callback,SDL_Widget *returnwidget)
{
    SDL_Scrollbar *Scrollbar=(SDL_Scrollbar*)widget;
    
    Scrollbar->Callback       = callback;
    Scrollbar->CallbackWidget = returnwidget;
}
