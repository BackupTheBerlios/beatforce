/*
  Beatforce/Main windows user interface

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
#include <SDL/SDL.h>
#include <SDL_Widget.h>
#include <SDL_Table.h>

#include "songdb_ui.h"
#include "player_ui.h"
#include "playlist_ui.h"
#include "mixer_ui.h"

#include "config.h"
#include "mixer.h"
#include "osa.h"
#include "player.h"
#include "songdb.h"




#include "wndmgr.h"
#include "search_window.h"

extern SDL_Font *DigitsFont;
extern SDL_Font *LargeBoldFont;

void songend_callback(int player_nr)
{

}

int control_state;
int mainwindow_EventHandler(SDL_Event event);
int mainwindow_NotifyHandler();
SDL_Surface *mainwindow_CreateWindow();
int mainwindow_HandleKeys(unsigned int key);

SDL_Surface *MainWindow;
void *timewidget;

Window MAINWINDOW={ mainwindow_EventHandler , mainwindow_NotifyHandler };

#define BF_CONTROL         0xf000
#define BF_QUIT            SDLK_ESCAPE
#define BF_SEARCH          SDLK_j
#define BF_NEXTSONG        SDLK_b
#define BF_MAINVOLUMEDOWN  SDLK_PERIOD
#define BF_MAINVOLUMEUP    SDLK_SLASH


void MAINWINDOW_Init()
{
    MainWindow = NULL;
}


void MAINWINDOW_Open()
{
    if(MainWindow == NULL)
    {
        MainWindow=mainwindow_CreateWindow();
    }
    SDL_WidgetUseSurface(MainWindow);
    WNDMGR_Open(&MAINWINDOW);
}

SDL_Surface *mainwindow_CreateWindow()
{
    SDL_Surface *MainWindow;

    control_state=0;
    MainWindow = SDL_CreateRGBSurface(SDL_SWSURFACE,1024,685,32,0xff0000,0x00ff00,0x0000ff,0x000000);
    SDL_SetColorKey(MainWindow,0,0); // disable transparancy

    SDL_WidgetUseSurface(MainWindow);

    SDL_WidgetCreate(SDL_PANEL,0,0,1024,685);
    SDL_WidgetProperties(SET_NORMAL_IMAGE,THEME_DIR"/beatforce/sbackground.bmp");
    
    SDL_WidgetCreate(SDL_PANEL,0,230,1024,3);
    SDL_WidgetProperties(SET_NORMAL_IMAGE,THEME_DIR"/beatforce/seperator.bmp");

    SDL_WidgetCreate(SDL_PANEL,0,30,1024,3);
    SDL_WidgetProperties(SET_NORMAL_IMAGE,THEME_DIR"/beatforce/seperator.bmp");

    timewidget=SDL_WidgetCreate(SDL_LABEL,4,4,65,22);
    SDL_WidgetProperties(SET_BG_COLOR,BLACK);
    SDL_WidgetProperties(SET_FG_COLOR,0x00e1ff);
    SDL_WidgetProperties(SET_FONT,DigitsFont);

    SDL_WidgetCreate(SDL_LABEL,80,5,65,17);
    SDL_WidgetProperties(SET_FG_COLOR,0x00e1ff);
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_CAPTION,"Beatforce");


    SONGDBUI_CreateWindow();
    PLAYLISTUI_CreateWindow();
    PLAYERUI_CreateWindow(0,10);
    PLAYERUI_CreateWindow(1,610);
    MIXERUI_CreateWindow();


    player_set_callback(songend_callback);



    return MainWindow;
}



int mainwindow_EventHandler(SDL_Event event)
{
    unsigned int key = 0;
    switch(event.type)
    {
    case SDL_KEYDOWN:
        switch( event.key.keysym.sym ) 
        {

        case SDLK_RCTRL:
        case SDLK_LCTRL:
            control_state=1;
            break;

        default:
            key=event.key.keysym.sym;
            if(control_state)
                key |= BF_CONTROL;
            mainwindow_HandleKeys(key);
            break;
            
        }
        break;
    case SDL_KEYUP:
        switch( event.key.keysym.sym ) 
        {
        case SDLK_RCTRL:
        case SDLK_LCTRL:
            control_state=0;
            break;

        default:
            break;
        }

    }
    return 0;
}

int mainwindow_HandleKeys(unsigned int key)
{
    switch(key)
    {
    case BF_QUIT:
        WNDMGR_Exit();
        break;
        
    case BF_SEARCH:
        SEARCHWINDOW_Open();
        break;
        
    case BF_MAINVOLUMEUP:
        MIXER_IncreaseMainVolume();
        break;

    case BF_MAINVOLUMEDOWN:
        MIXER_DecreaseMainVolume();
        break;

    case BF_NEXTSONG:
        break;

    }

    return 1;


}

int mainwindow_NotifyHandler()
{
    char time[6];
    int min=0,hour=0;
    OSA_GetTime(&hour,&min);
    sprintf(time,"%02d:%02d",hour,min);
    SDL_WidgetPropertiesOf(timewidget,SET_CAPTION,time);

    MIXERUI_Redraw();
    PLAYERUI_Redraw();
    return 1;
}
