/*
  Beatforce/ Window manager

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
#include "config.h"
#include <stdlib.h>


//SDL gui specific
#include <SDL/SDL.h>
#include <SDL_Widget.h>

#include "main_window.h"
#include "wndmgr.h"
#include "osa.h"
#include "theme.h"

#define MODULE_ID WNDMGR
#include "debug.h" 


void WNDMGR_Init()
{
    SDL_Surface *screen;
    unsigned int flags=0;
    ThemeConfig *c=THEME_GetActive();
    ThemeScreen *s;

    TRACE("UI_Init enter");
    
    if(c)
        s=c->Screen;
    else
    {
        ERROR("No active theme found");
        exit(1);
    }


    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_CDROM) < 0 ) 
    {
        ERROR("Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    if(s->FullScreen)
    {
        flags |= SDL_FULLSCREEN;
    }
    else
    {
        flags |= SDL_HWSURFACE;
        flags |= SDL_DOUBLEBUF;
    }

    if(s->NoFrame)
    {
        flags |= SDL_NOFRAME;
    }
    screen = SDL_SetVideoMode(s->Width,s->Height,s->BPP,flags);
    
    if ( screen == NULL) 
    {
        fprintf(stderr, "Unable to set %dx%d video: %s\n", s->Width,s->Height,SDL_GetError());
        exit(1);
    }

    SDL_WM_SetCaption("Beatforce",NULL);
    SDL_WindowInit(screen);

}



















