/*
  Beatforce/ Window manager

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


/************** global variables ****************/

SDL_Surface *screen;
Window *CurWindow;
int WNDMGR_Running;


int gEventsAllowed;
int windowswitch;

/* Local prototypes */
int WNDMGR_Redraw(void *data);


void WNDMGR_CloseWindow()
{
    MAINWINDOW_Open(); /* Always go back to main window */
}

void WNDMGR_Init()
{
    unsigned int flags=0;
    ThemeScreen *s=THEME_GetActive()->Screen;

    TRACE("UI_Init enter");

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
        flags |= SDL_SWSURFACE;
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
    windowswitch=0;
    CurWindow=NULL;
}

void WNDMGR_Open(Window *window)
{
    CurWindow=window;
    windowswitch=1;
}


int WNDMGR_Main()
{
    SDL_Event test_event;
    int timer;

    
    WNDMGR_Running = 1;
    gEventsAllowed = 1;

    timer=OSA_StartTimer(50,WNDMGR_Redraw,NULL);

    while(WNDMGR_Running)
    {
        while(SDL_PollEvent(&test_event)) 
        {
            if(gEventsAllowed)
                CurWindow->EventHandler(test_event);

            switch(test_event.type) 
            {
               case SDL_QUIT:
                WNDMGR_Running=0;
                break;
            default:
                break;
            }
            if(windowswitch)
                windowswitch=0;
            else
                SDL_WidgetEvent(&test_event);
        }   
        SDL_Delay(25); /* To reduce CPU load */
    }

    OSA_RemoveTimer(timer);
    return 1;
}

void WNDMGR_Exit()
{
    WNDMGR_Running=0;
}

int WNDMGR_Redraw(void *data)
{
    if(CurWindow->NotifyRedraw)
        CurWindow->NotifyRedraw();
    SDL_DrawAllWidgets(screen);
    return 50; //redraw every 50ms 
}

/* disable sending events to the event handler of the current window
   this can be used when all events have to be send to a widget
   for example when the widget is in an edit state */
void WNDMGR_DisableEventhandler()
{
    gEventsAllowed=0;
}

void WNDMGR_EnableEventhandler()
{
    gEventsAllowed=1;
}











