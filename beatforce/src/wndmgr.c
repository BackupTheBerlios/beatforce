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
#include <SDL/SDL_image.h>
#include <SDL/SDL_endian.h>	/* Used for the endian-dependent 24 bpp mode */

#include <SDL_Widget.h>
#include <SDL_Font.h>

#include <SDL_Table.h>

#include "wndmgr.h"
#include "osa.h"

#include "main_window.h"
#include "file_window.h"
#include "search_window.h"

#define MODULE_ID WNDMGR
#include "debug.h" 

SDL_Surface *screen;
Window *CurWindow;
int WNDMGR_Running;

SDL_Font *SmallFont;
SDL_Font *LargeBoldFont;
SDL_Font *DigitsFont;


/* global variables */
int gEventsAllowed;
int windowswitch;

/* Local prototypes */
int wndmgr_Redraw(void *data);


void WNDMGR_CloseWindow()
{
    MAINWINDOW_Open();//Always go back to main window
}

void WNDMGR_Init()
{
    TRACE("WNDMGR_Init enter");

    CurWindow=NULL;
    if(SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) 
    {
        ERROR("Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
 
    screen = SDL_SetVideoMode(1024,685 , 32, SDL_SWSURFACE | SDL_NOFRAME);
    
    if ( screen == NULL) 
    {
        fprintf(stderr, "Unable to set 800x600 video: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_WM_SetCaption("Beatforce",NULL);
    MAINWINDOW_Init();
    SEARCHWINDOW_Init();
    FILEWINDOW_Init();

    LargeBoldFont=SDL_FontInit(THEME_DIR"/beatforce/digital.fnt");
    SmallFont=SDL_FontInit(THEME_DIR"/beatforce/yysmallfont.fnt");
//  LargeBoldFont=SDL_FontInit(THEME_DIR"/beatforce/arial12.bdf");
//    LargeBoldFont=SDL_FontInit("./res/cour10.bdf");
    DigitsFont=SDL_FontInit(THEME_DIR"/beatforce/digits.fnt");
    windowswitch=0;
}

void WNDMGR_Open(Window *window)
{
    CurWindow=window;
    windowswitch=1;
}


int WNDMGR_Main(void * data)
{
    SDL_Event test_event;
    int timer;

    
    WNDMGR_Running = 1;
    gEventsAllowed = 1;

    
    timer=OSA_StartTimer(50,wndmgr_Redraw,NULL);

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
    SDL_Quit();
    return 1;
}

void WNDMGR_Exit()
{
    WNDMGR_Running=0;
}

int wndmgr_Redraw(void *data)
{
    if(CurWindow->NotifyRedraw)
        CurWindow->NotifyRedraw();
    SDL_DrawAllWidgets(screen);
    return 50; //redraw every 50ms 
}

void WNDMGR_DisableEventhandler()
{
    gEventsAllowed=0;
}

void WNDMGR_EnableEventhandler()
{
    gEventsAllowed=1;
}











