/*
  Beatforce/Config window user interface

  Copyright (C) 2004 John Beuving (john.beuving@beatforce.org)

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
#include <SDL_Window.h>
#include <SDL_Table.h>
#include <malloc.h>

#include "wndmgr.h"
#include "theme.h"
#include "clock.h"

static int CONFIGWINDOW_EventHandler(SDL_Event event);
static SDL_Surface *CONFIGWINDOW_CreateWindow();
static int CONFIGWINDOW_NotifyHandler(SDL_Window *Win);

void *ConfigWindowClock;

SDL_Window CONFIGWINDOW={ CONFIGWINDOW_EventHandler, CONFIGWINDOW_NotifyHandler, NULL, NULL };

void CONFIGWINDOW_Open()
{
    if(CONFIGWINDOW.Surface == NULL)
    {
        CONFIGWINDOW.Surface=CONFIGWINDOW_CreateWindow();
    }
    SDL_WindowOpen(&CONFIGWINDOW);
}

static SDL_Surface *CONFIGWINDOW_CreateWindow()
{
    SDL_Surface       *ConfigWindow;
    ThemeConfig       *tc = THEME_GetActive();
    ThemeConfigWindow *cw = NULL;
    ThemeImage        *Image = NULL;
    ThemeScreen       *s=tc->Screen;
    
    if(tc == NULL)
        return NULL;
    
    cw=tc->ConfigWindow;
    
    if(cw == NULL)
        return NULL;

    Image=cw->Image;


    ConfigWindow = SDL_WidgetNewSurface(s->Width,s->Height,s->BPP);
    SDL_SetColorKey(ConfigWindow,0,0); // disable transparancy
   
#if 0
    while(Image)
    {
        SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_WidgetProperties(SET_IMAGE,IMG_Load(Image->filename));
        Image=Image->next;
    }

    SDL_WidgetCreate(SDL_EDIT,312,20,400,25);
    SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));

    SDL_WidgetCreate(SDL_EDIT,312,50,400,25);
    SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));

    ConfigWindowClock=CLOCK_Create(cw->Clock);
#endif
    
    return ConfigWindow;
}


static int CONFIGWINDOW_EventHandler(SDL_Event event)
{

    switch(event.type)
    {
    case SDL_KEYDOWN:
        switch( event.key.keysym.sym ) 
        {
        case SDLK_ESCAPE:
            SDL_WindowClose();
            break;
        default:
            break;
        }
        break;
    case SDL_KEYUP:
        switch( event.key.keysym.sym ) 
        {

        default:
            break;
        }

    }
    return 0;
}




static int CONFIGWINDOW_NotifyHandler(SDL_Window *Win)
{
    CLOCK_Redraw(ConfigWindowClock);
    return 1;
}
