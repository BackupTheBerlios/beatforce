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
#include "SDL_Font.h"

typedef struct SDL_ProgressBar
{
    SDL_Font *Font;
    SDL_Rect rect;
    
    Uint32  color1;
    Uint32  color2;
    Uint32  color3;
    Uint32  color4;
    
    int Redraw;
    int State;

    int Orientation;

    int MinValue;
    int MaxValue;
    int CurrentValue;

    /* Event handler for SDL_CLICKED */
    void (*OnClicked)();
    void *OnClickedData;

    /* Settings for internal draw functions */
    int CurrentLine;   /* line where the volume is at */

}SDL_ProgressBar;

#define HORIZONTAL 1
#define VERTICAL   2

#define PROGRESSBAR_NORMAL 3
#define PROGRESSBAR_DRAG   4

// prototypes
void* SDL_ProgressBarCreate(SDL_Rect *rect);
void  SDL_ProgressBarDraw(void *progressbar,SDL_Surface *dest);
void  SDL_ProgressBarEventHandler(void * progressbar,SDL_Event *event);
int   SDL_ProgressBarProperties(void *progressbar,int feature,va_list list);


