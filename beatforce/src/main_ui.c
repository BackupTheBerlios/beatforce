/*
  Beatforce/ Main user interface

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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>


//SDL gui specific
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_endian.h>	/* Used for the endian-dependent 24 bpp mode */

#include <SDL_Widget.h>
#include <SDL_Font.h>

#include <SDL_Table.h>

#include "osa.h"

#include "main_window.h"
#include "file_window.h"
#include "search_window.h"

#define MODULE_ID MAIN_UI
#include "debug.h" 

SDL_Surface *screen;

SDL_Surface *MainWindow;


 
SDL_Font *LargeBoldFont;
SDL_Font *DigitsFont;

void MAINUI_CloseWindow(SDL_Surface *surface)
{
    SDL_WidgetUseSurface(MainWindow);
}

void MAINUI_Init()
{
    TRACE("UI_Init enter");

    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) 
    {
        ERROR("Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
 
    screen = SDL_SetVideoMode(1000, 650, 32, SDL_SWSURFACE);
    
    if ( screen == NULL) 
    {
        fprintf(stderr, "Unable to set 800x600 video: %s\n", SDL_GetError());
        exit(1);
    }

    MainWindow   = NULL;
    SEARCHWINDOW_Init();
    FILEWINDOW_Init();

    LargeBoldFont=SDL_FontInit(THEME_DIR"/beatforce/digital.fnt");
//    LargeBoldFont=SDL_FontInit("./res/arial10.bdf");
//    LargeBoldFont=SDL_FontInit("./res/cour10.bdf");
    DigitsFont=SDL_FontInit(THEME_DIR"/beatforce/digits.fnt");

}

void MAINUI_CreateWindow()
{
    MainWindow=MAINWINDOW_CreateWindow();
}


int timerredraw(void *data)
{
    SDL_DrawAllWidgets(screen);
    return 50;
}

int MAINUI_Main(void * data)
{
    int quit = 0;
    SDL_Event test_event;
    int timer;
    int retval=0;

    timer=OSA_StartTimer(50,timerredraw,NULL);

    while(!quit)
    {
        while(SDL_PollEvent(&test_event)) 
        {
            if(SDL_WidgetGetActiveSurface() == MainWindow)
                retval=MAINWINDOW_EventHandler(test_event);

            switch(test_event.type) 
            {

            case SDL_KEYDOWN:
                switch( test_event.key.keysym.sym ) 
                {
                case SDLK_ESCAPE:
                    if(SDL_WidgetGetActiveSurface() == MainWindow)
                        quit=1;
                    else //if(Window_SearchWindowActive())
                    {
                        SDL_WidgetUseSurface(MainWindow);
                    }

                    break;
                
                default:
                    ;
                }
                break;

            case SDL_QUIT:
                quit=1;
                break;
            default:
                break;
            }
            if(retval==0)
                SDL_WidgetEvent(&test_event);
            else
                retval=0;
        }   
        SDL_Delay(25);
    }

    OSA_RemoveTimer(timer);
    SDL_Quit();
    return 1;
}
















