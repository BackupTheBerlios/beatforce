/*
  Beatforce/ Startup of beatforce

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
#include <SDL/SDL.h>
#include "SDL_Widget.h"


#if 0
SDL_Surface* SDL_WidgetGetBackground(SDL_Surface *dest,SDL_Rect *srcrect)
{
    Uint32 rmask, gmask, bmask, amask;
    SDL_Surface *newsurface;
    SDL_Surface *surface;
    SDL_Rect rect;

    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;

    rect.x = srcrect->x;
    rect.y = srcrect->y;
    rect.w = srcrect->w;
    rect.h = srcrect->h;
    

    newsurface = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32,
                                      rmask, gmask, bmask, amask);

    if(newsurface == NULL) 
    {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
    surface = SDL_DisplayFormat(newsurface);
    SDL_FreeSurface(newsurface);

    if(SDL_BlitSurface(dest,&rect,surface,NULL)<0)
        fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());

    return surface;

}
#endif

int SDL_WidgetIsInside(SDL_Widget *widget,int x, int y)
{
    if( x > widget->Rect.x)
    {
        if(y > widget->Rect.y)
        {
            if(x < (widget->Rect.x + widget->Rect.w))
            {
                if(y < (widget->Rect.y + widget->Rect.h))
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}
