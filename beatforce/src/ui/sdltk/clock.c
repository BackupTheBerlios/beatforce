/*
  Beatforce/ Clock widget

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

#include <SDLTk.h>
#include "theme.h"
#include "osa.h"

#define MODULE_ID CLOCK
#include "debug.h"

SDL_Widget *CLOCK_Create(ThemeClock *Clock)
{
    SDL_Widget *widget=NULL;
    
    TRACE("CLOCK_Create");
    
    if(Clock && Clock->font)
    {
        widget=SDL_WidgetCreateR(SDL_LABEL,Clock->Rect);
        SDL_WidgetPropertiesOf(widget,SET_BG_COLOR,Clock->bgcolor);
        SDL_WidgetPropertiesOf(widget,SET_FG_COLOR,Clock->fgcolor);
        SDL_WidgetPropertiesOf(widget,SET_FONT,THEME_Font(Clock->font));
    }
    return widget;
}


void CLOCK_Redraw(SDL_Widget *clock)
{
    static char time[6];
    int min=0,hour=0;

    TRACE("CLOCK_Redraw");
    if(clock)
    {
        OSA_GetTime(&hour,&min);
        sprintf(time,"%02d:%02d",hour,min);
        SDL_LabelSetText(clock,time);
    }
}


