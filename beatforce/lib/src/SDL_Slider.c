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
#include <stdlib.h>

#include "SDL_Slider.h"
#include "SDL_Widget.h"
#include "SDL_WidTool.h" //for SDL_WidgetGetBackground

static void SDL_SliderStep(SDL_Slider *slider,int direction,int step);
static void SDL_SliderPixeloffset(SDL_Slider *Slider);
static void SDL_SliderCurrentValue(SDL_Slider *Slider);
static void SDL_SliderPaint(SDL_Slider *Slider);

const struct S_Widget_FunctionList SDL_Slider_FunctionList =
{
    SDL_SliderCreate,
    SDL_SliderDraw,
    SDL_SliderProperties,
    SDL_SliderEventHandler,
    NULL
};

SDL_Widget* SDL_SliderCreate(SDL_Rect* rect)
{
    SDL_Slider *slider;
    SDL_Widget *widget;

    slider = (SDL_Slider*) malloc (sizeof(SDL_Slider));
    widget = (SDL_Widget*) slider;
    
    widget->Type      = SDL_SLIDER;
    widget->Rect.x    = rect->x;
    widget->Rect.y    = rect->y;
    widget->Rect.w    = rect->w;
    widget->Rect.h    = rect->h;
    widget->Focusable = 0;

    if(rect->w > rect->h)
        slider->orientation = HORIZONTAL;
    else
        slider->orientation = VERTICAL;

    slider->normal=NULL;
    slider->line  =NULL;
    
    slider->percentage   = 0;  // Slider is in start position
    slider->SliderButton = NULL;
    slider->pixeloffset  = 0;
    slider->state        = SLIDER_IDLE;
    slider->changed = 0;

    // default range settings
    slider->MinValue           = 0;
    slider->MaxValue           = 100;
    slider->CurrentValue       = 0.0;
    slider->CurrentPercentage  = 0.0;
    slider->ValueLocked        = 0;

    slider->NormalStepSize    = 1.0;

    // initialize eventhandler
    slider->OnSliderChanged      = NULL;
    slider->OnSliderChangedData  = NULL;
    slider->Visible = 1;

    return (SDL_Widget*)slider;
}


void SDL_SliderDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{
    // drawing order is always background, line , button
    SDL_Slider *Slider=(SDL_Slider*)widget;
    SDL_Rect   button;
    int x_offset=0;
    int y_offset=0;

    if(Slider->Visible == 0)
        return;

    if(Slider->SliderButton == NULL)
        return;
    
    if(Slider->line)
    {
        if(SDL_BlitSurface(Slider->line,NULL,dest,&widget->Rect)<0)
            fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
    }

    if(Slider->orientation == HORIZONTAL)
    {
        x_offset = Slider->pixeloffset;
    }
    else
    {
        y_offset = Slider->pixeloffset;
        x_offset = (widget->Rect.w - Slider->SliderButton->w) / 2 ;
    }

    if(Slider->SliderButton == NULL)
    {
        SDL_SliderPaint(Slider);
    }
    else
    {
        button.x = widget->Rect.x + x_offset;
        button.y = widget->Rect.y + y_offset;
        button.w = Slider->SliderButton->w;
        button.h = Slider->SliderButton->h;
        
        
        if(SDL_BlitSurface(Slider->SliderButton,NULL,dest,&button)<0)
            fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
    }


}

int SDL_SliderProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Slider *Slider=(SDL_Slider*)widget;
    double val;

    switch(feature)
    {
    case SET_MIN_VALUE:
        Slider->MinValue = va_arg(list,int);
        break;
    case SET_MAX_VALUE:
        Slider->MaxValue = va_arg(list,int);
        if(Slider->CurrentValue > Slider->MaxValue)
            Slider->CurrentValue = (double)Slider->MaxValue;
        break;

    case SET_CUR_VALUE:
        /* The currentvalue is used as an integer and a new pixeloffset */
        /* is calculated but the size of the button is not used yet     */
        /* because it doesn't have to be available at this moment       */
        val= va_arg(list,double);
        Slider->CurrentValue = val;
        SDL_SliderPixeloffset(Slider);
        if(Slider->CurrentValue > Slider->MaxValue)
            Slider->CurrentValue = Slider->MaxValue;
        break;
    case SET_NORMAL_STEP_SIZE:
        val= va_arg(list,double);
        Slider->NormalStepSize = val;
        break;
    case SET_LINE_IMAGE:
        if(Slider->line == NULL)
        {
            Slider->line=va_arg(list,SDL_Surface*);
            if(Slider->line)
                SDL_SetColorKey(Slider->line,SDL_SRCCOLORKEY,TRANSPARANT);
        }
        break;
    case SET_BUTTON_IMAGE:
        if(Slider->SliderButton == NULL)
        {
            Slider->SliderButton=va_arg(list,SDL_Surface*);
            if(Slider->SliderButton)
            {
                SDL_SetColorKey(Slider->SliderButton,SDL_SRCCOLORKEY,TRANSPARANT);
                SDL_SliderPixeloffset(Slider);
            }
        }
        break;
    case GET_CUR_VALUE:
    {
        double *value;
        value=va_arg(list,double*);
        if(value)
        {
            *value=Slider->CurrentValue;
        }
        break;
    }
    case GET_WIDTH:
    {
        int *value;
        value=va_arg(list,int*);
        if(value)
        {
            *value=widget->Rect.w;
        }
        break;
    }
    case GET_STATE:
    {
        int *value;
        value=va_arg(list,int*);
        if(value)
        {
            *value=Slider->state;
        }
        break;
    }
    case SET_CALLBACK:
    {
        int event=va_arg(list,int);
        if(event==SDL_CHANGED)
        {
            Slider->OnSliderChanged=va_arg(list,void*);
            Slider->OnSliderChangedData=va_arg(list,void*);
        }
        break;
    }
    case SET_VISIBLE:
        Slider->Visible = va_arg(list,int);
        break;

    default:
        break;


    }
    return 1;
}


int SDL_SliderEventHandler(SDL_Widget *widget,SDL_Event *event)
{
    SDL_Slider *Slider=(SDL_Slider*)widget;

    switch(event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
        if(Slider->SliderButton && SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
        {
            int motion;

            if(Slider->orientation == HORIZONTAL)
            {
                motion=event->motion.x;
                
                if(event->button.button == SDL_BUTTON_LEFT)
                {
                    /* if the button click is on the left side of the button */
                    if(motion < (widget->Rect.x + Slider->pixeloffset))
                    {
                        SDL_SliderStep(Slider,LEFT,NORMAL_STEP);
                    }
                    /* the button click is on the right side off the button */
                    else if (motion > widget->Rect.x + Slider->pixeloffset + Slider->SliderButton->w)
                    {
                        SDL_SliderStep(Slider,RIGHT,NORMAL_STEP);
                    }
                    /* if the button click is on the button */
                    else
                    {
                        Slider->state=SLIDER_DRAG;
                    }
                }
                if(event->button.button == 4)
                {
                    SDL_SliderStep(Slider,RIGHT,NORMAL_STEP);
                }
                if(event->button.button == 5)
                {
                    SDL_SliderStep(Slider,LEFT,NORMAL_STEP);
                }
            }
            else /* Vertical orientation */
            {
                motion=event->motion.y;
                
                if(event->button.button == SDL_BUTTON_LEFT)
                {
                    /* if the button click is below the button */
                    if(motion > (widget->Rect.y + Slider->pixeloffset + Slider->SliderButton->h))
                    {
                        SDL_SliderStep(Slider,DOWN,NORMAL_STEP);
                    }
                    else if (motion > widget->Rect.y + Slider->pixeloffset)
                    {
                        Slider->state=SLIDER_DRAG;
                    }
                    else
                    {
                        SDL_SliderStep(Slider,UP,NORMAL_STEP);
                    }
                }
                if(event->button.button == 4)
                {
                    SDL_SliderStep(Slider,UP,NORMAL_STEP);
                }
                if(event->button.button == 5)
                {
                    SDL_SliderStep(Slider,DOWN,NORMAL_STEP);
                }
            }
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if(Slider->state == SLIDER_DRAG)
        {
            Slider->state=SLIDER_IDLE;
        }
        break;
    case SDL_MOUSEMOTION:
        if(Slider->state == SLIDER_DRAG)
        {
            if(Slider->orientation == HORIZONTAL)
            {
                Slider->pixeloffset = event->motion.x - widget->Rect.x - (Slider->SliderButton->w/2);
            }
            else
            {
                Slider->pixeloffset = event->motion.y - widget->Rect.y - (Slider->SliderButton->h/2);
            }
            SDL_SliderCurrentValue(Slider);
            Slider->changed=1;
        }
        break;
    default:
        break;
    }

    if(Slider->changed)
    {
        // Run event handler if available (SDL_CHANGED event)
        if(Slider->OnSliderChanged != NULL)
            Slider->OnSliderChanged(Slider->OnSliderChangedData);
        Slider->changed=0;
    }
    return 0;
}


/* Internal helper functions */

static void SDL_SliderStep(SDL_Slider *Slider,int direction,int step)
{
    switch(direction)
    {
    case UP:
    case LEFT:
        Slider->CurrentValue -= Slider->NormalStepSize;
        SDL_SliderPixeloffset(Slider);
        Slider->changed=1;
        break;
    case DOWN:
    case RIGHT:
        Slider->CurrentValue += Slider->NormalStepSize;
        SDL_SliderPixeloffset(Slider);
        Slider->changed=1;
        break;
    }

}

static void SDL_SliderPixeloffset(SDL_Slider *Slider)
{
    SDL_Widget *widget=(SDL_Widget*)Slider;

    if(Slider->CurrentValue < Slider->MinValue)
        Slider->CurrentValue = Slider->MinValue;

    if(Slider->CurrentValue > Slider->MaxValue)
        Slider->CurrentValue = Slider->MaxValue;

    if(Slider->orientation == HORIZONTAL)
    {
        if(Slider->SliderButton)
        {
            Slider->pixeloffset = ((widget->Rect.w - Slider->SliderButton->w) * Slider->CurrentValue) 
                / (Slider->MaxValue - Slider->MinValue);
        }
    }
    else
    {
        if(Slider->SliderButton)
        {
            Slider->pixeloffset = ((widget->Rect.h - Slider->SliderButton->h) * Slider->CurrentValue) 
                / (Slider->MaxValue - Slider->MinValue);
           
        }
    }
}



static void SDL_SliderCurrentValue(SDL_Slider *Slider)
{
    SDL_Widget *widget=(SDL_Widget*)Slider;
    /* First do a check on the current value of pixeloffset */

    if(Slider->pixeloffset < 0)
        Slider->pixeloffset = 0;
    
    /* then recalculate CurrentValue */
    if(Slider->SliderButton)
    {
        if(Slider->orientation == HORIZONTAL)
        {
            if(Slider->pixeloffset > (widget->Rect.w - Slider->SliderButton->w))
                Slider->pixeloffset = (widget->Rect.w - Slider->SliderButton->w);

            Slider->CurrentValue = (double)((Slider->MaxValue -Slider->MinValue)* Slider->pixeloffset) 
                /(double)(widget->Rect.w - Slider->SliderButton->w);
        }
        else
        {
            if(Slider->pixeloffset > (widget->Rect.h - Slider->SliderButton->h))
                Slider->pixeloffset = (widget->Rect.h - Slider->SliderButton->h);

            Slider->CurrentValue = (double)((Slider->MaxValue -Slider->MinValue)* Slider->pixeloffset) 
                / (double)(widget->Rect.h - Slider->SliderButton->h);
        }
    }
}

static void SDL_SliderPaint(SDL_Slider *Slider)
{


}
