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
#include <malloc.h>

#include "songdb_ui.h"
#include "player_ui.h"
#include "playlist_ui.h"
#include "mixer_ui.h"

#include "config.h"
#include "mixer.h"
#include "osa.h"
#include "player.h"
#include "songdb.h"
#include "playlist.h"
#include "player.h"

#include "theme.h"
#include "clock.h"

#include "main_window.h"
#include "wndmgr.h"
#include "search_window.h"

int control_state;
static int MAINWINDOW_EventHandler(SDL_Event event);
static int MAINWINDOW_NotifyHandler(Window *Win);
static SDL_Surface *MAINWINDOW_CreateWindow();
static int MAINWINDOW_KeyHandler(unsigned int key);

Window MAINWINDOW={ MAINWINDOW_EventHandler , MAINWINDOW_NotifyHandler , NULL, NULL };

#define BF_CONTROL         0xf000
#define BF_QUIT            SDLK_ESCAPE
#define BF_SEARCH          SDLK_j
#define BF_NEXTSONG        SDLK_b
#define BF_MAINVOLUMEDOWN  SDLK_PERIOD
#define BF_MAINVOLUMEUP    SDLK_SLASH
#define BF_AUTOFADE        SDLK_f

void MAINWINDOW_Init()
{

}


void MAINWINDOW_Open()
{
    if(MAINWINDOW.Surface == NULL)
    {
        MainwindowWidgets *widgets;
        widgets=malloc(sizeof(MainwindowWidgets));
        MAINWINDOW.Surface=MAINWINDOW_CreateWindow(widgets);
        MAINWINDOW.TransferData=widgets;
    }
    SDL_WidgetUseSurface(MAINWINDOW.Surface);
    WNDMGR_Open(&MAINWINDOW);
}

static SDL_Surface *MAINWINDOW_CreateWindow(MainwindowWidgets *w)
{
    SDL_Surface       *MainWindow;
    ThemeConfig       *tc = THEME_GetActive();
    ThemeMainWindow   *mw = NULL;
    ThemeImage        *Image = NULL;
    ThemeScreen       *s=tc->Screen;
    
    if(tc == NULL)
        return NULL;
    
    mw=tc->MainWindow;
    
    if(mw == NULL)
        return NULL;

    Image=mw->Image;


    control_state=0;
    MainWindow = SDL_CreateRGBSurface(SDL_SWSURFACE,s->Width,s->Height,s->BPP,
                                      0xff0000,0x00ff00,0x0000ff,0x000000);
    SDL_SetColorKey(MainWindow,0,0); // disable transparancy
    
    SDL_WidgetUseSurface(MainWindow);

    while(Image)
    {
        SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_WidgetProperties(SET_IMAGE,IMG_Load(Image->filename));
        Image=Image->next;
    }

    w->Clock=CLOCK_Create(mw->Clock);

    w->Songdb=SONGDBUI_CreateWindow(mw->Songdb);
    w->Playlist=PLAYLISTUI_CreateWindow(mw->Playlist);
    PLAYERUI_CreateWindow(0,mw->Player[0]);
    PLAYERUI_CreateWindow(1,mw->Player[1]);
    w->Mixer=MIXERUI_CreateWindow(mw->Mixer);
    
    return MainWindow;
}



static int MAINWINDOW_EventHandler(SDL_Event event)
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
            MAINWINDOW_KeyHandler(key);
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

int MAINWINDOW_KeyHandler(unsigned int key)
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
        MIXERUI_IncreaseMainVolume();
        break;

    case BF_MAINVOLUMEDOWN:
        MIXERUI_DecreaseMainVolume();
        break;

    case BF_AUTOFADE:
        MIXER_DoFade(1,0);
        break;

    case BF_NEXTSONG:
        if(PLAYLIST_GetSong(0,0))
        {
            if(PLAYER_IsPlaying(1) && PLAYER_IsPlaying(0)==0)
            {
                if(PLAYER_SetSong(0,0))
                {
                    MIXER_DoFade(1,0);
                    SONGDBUI_Play(0);
                }
            }
            else if(PLAYER_IsPlaying(0) && PLAYER_IsPlaying(1)==0)
            {
                if(PLAYER_SetSong(1,0))
                {
                    MIXER_DoFade(1,0);
                    SONGDBUI_Play(1);
                }
            }

        }
        break;

    }

    return 1;


}

static int MAINWINDOW_NotifyHandler(Window *Win)
{
    MainwindowWidgets *w=(MainwindowWidgets*)Win->TransferData;
    CLOCK_Redraw(w->Clock);
    MIXERUI_Redraw(w->Mixer);
    PLAYERUI_Redraw();
    SONGDBUI_Redraw(w->Songdb);
    PLAYLISTUI_Redraw(w->Playlist);

    return 1;
}



