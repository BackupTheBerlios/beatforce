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

#include "songdb.h"
#include "player.h"
#include "osa.h"

#include "config.h"

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

SDL_Surface *MainWindow;
void *timewidget;

Window MAINWINDOW={ mainwindow_EventHandler , mainwindow_NotifyHandler };

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
    switch(event.type)
    {
    case SDL_KEYDOWN:
        switch( event.key.keysym.sym ) 
        {
        case SDLK_RCTRL:
        case SDLK_LCTRL:
            control_state=1;
            break;

        case SDLK_j:
            if(control_state == 0)// && !SONGDBUI_IsRenaming())
            {
                SEARCHWINDOW_Open();
                return 1; //Don't give this event to widgets
            }
            break;

        case SDLK_k:
            if(control_state)
            {
                printf("Control k pressed\n");
            }
            break;

        case SDLK_b:
            if(control_state == 0)
            {
                
                
            }
            break;

        case SDLK_c:
            if(control_state == 0)
            {
                PLAYER_Pause (0);
                PLAYER_Pause (1);
            }
            break;

        case SDLK_ESCAPE:
            WNDMGR_Exit();
            break;
                
        
        default:
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



int mainwindow_NotifyHandler()
{
    char time[6];
    int min=0,hour=0;
    OSA_GetTime(&hour,&min);
    sprintf(time,"%02d:%02d",hour,min);
    SDL_WidgetPropertiesOf(timewidget,SET_CAPTION,time);
    PLAYERUI_Redraw();
    return 1;
}
