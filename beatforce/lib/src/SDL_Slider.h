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

#include "SDL_Widget.h"


#define SLIDER_IDLE      0x00
#define SLIDER_DRAG      0x01

#define HORIZONTAL 1
#define VERTICAL   2

typedef struct SDL_Slider
{
    SDL_Surface *normal;
    SDL_Surface *line;
    SDL_Surface *SliderButton;
    SDL_Surface *background;

    int orientation;  // hor or vert
    int percentage;   // percentage on scale
    int pixeloffset;  // pixeloffset runs from 0 to (Slider height - SliderButton height) left nd top
    
    int state;      // states of the widget, used for eventhandler
    int changed;    // if changed is true a redraw is performed


    // Slider range (Can be queried by event callback functions
    int    MaxValue;        // maximum value (percentage = 100%)
    int    MinValue;        // minimum valie (percentage = 0%)
    double CurrentValue;    // value of current position
    double CurrentPercentage;
    
    double NormalStepSize;
    
    //event handler functions
    void (*OnSliderChanged)(void*);
    void (*OnSliderChangedData)(void*);

    SDL_Rect rect;  //dimensions of the widget area
}SDL_Slider;


// prototypes
void* SDL_SliderCreate(SDL_Rect *rect);
void SDL_SliderDraw(void *slider,SDL_Surface *dest);
void SDL_SliderEventHandler (void * slider,SDL_Event *event);
int  SDL_SliderProperties(void *slider,int feature,va_list list);

// internal enums
enum
{
    RIGHT,
    LEFT,
    UP,
    DOWN
};

enum
{
    SMALL_STEP,
    NORMAL_STEP,
    LARGE_STEP
};
