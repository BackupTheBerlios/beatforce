/*
  Beatforce/Main windows user interface

  one line to give the program's name and an idea of what it does.
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
#include <SDL_Widget.h>
#include <SDL_Window.h>
#include <SDL_Table.h>
#include <malloc.h>

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
#include "search_window.h"
#include "config_window.h"

#include "mixer_ui.h"
#include "player_ui.h"
#include "playlist_ui.h"
#include "songdb_ui.h"

int control_state;
static int MAINWINDOW_EventHandler(SDL_Event event);
static void MAINWINDOW_CreateWindow();
static int MAINWINDOW_KeyHandler(unsigned int key);
static int MAINWINDOW_RedrawTimeout();

SDL_Window *MAINWINDOW;
MainwindowWidgets *widgets;

#define BF_CONTROL         0xf000
#define BF_QUIT            SDLK_ESCAPE
#define BF_SEARCH          SDLK_j
#define BF_NEXTSONG        SDLK_b
#define BF_MAINVOLUMEDOWN  SDLK_PERIOD
#define BF_MAINVOLUMEUP    SDLK_SLASH
#define BF_AUTOFADE        SDLK_f

void MAINWINDOW_Open()
{
    if(MAINWINDOW == NULL)
    {
        MAINWINDOW_CreateWindow();
    }

    SDL_WindowOpen(MAINWINDOW);
}

void configopen(void *d)
{
    CONFIGWINDOW_Open();
}

static void MAINWINDOW_CreateWindow()
{
    ThemeConfig       *tc = THEME_GetActive();
    ThemeMainWindow   *mw = NULL;
    ThemeImage        *Image = NULL;
    ThemeScreen       *s=tc->Screen;
    ThemeFont         *f=tc->Font;
    
    widgets=malloc(sizeof(MainwindowWidgets));

    if(tc == NULL)
        return;
    
    mw=tc->MainWindow;
    
    if(mw == NULL)
        return;

    Image=mw->Image;

    while(f)
    {
        SDL_FontLoad(f->id,f->filename);
        f=f->Next;
    }

    control_state=0;
    MAINWINDOW = SDL_WindowNew(0,0,s->Width,s->Height);

    while(Image)
    {
        SDL_Widget *image;
        image=SDL_WidgetCreate(SDL_PANEL,Image->x,Image->y,Image->w,Image->h);
        SDL_PanelSetImage(image,IMG_Load(Image->filename));
        SDL_WidgetShow(image);
        Image=Image->next;
    }


    widgets->Clock=CLOCK_Create(mw->Clock);

    widgets->Playlist = PLAYLISTUI_CreateWindow(mw->Playlist);

    PLAYERUI_CreateWindow(0,mw->Player[0]);
    PLAYERUI_CreateWindow(1,mw->Player[1]);

    
    widgets->Mixer  = MIXERUI_CreateWindow(mw->Mixer);
    widgets->Songdb = SONGDBUI_CreateWindow(mw->Songdb);    

    OSA_StartTimer(10,MAINWINDOW_RedrawTimeout,NULL);

    MAINWINDOW->TransferData = widgets;
    MAINWINDOW->EventHandler = MAINWINDOW_EventHandler;
}



static int MAINWINDOW_EventHandler(SDL_Event event)
{
    int handled=0;
    unsigned int key = 0;

    switch(event.type)
    {
    case SDL_KEYDOWN:
    {
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
            handled=MAINWINDOW_KeyHandler(key);
            break;
            
        }
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
    return handled;
}

int MAINWINDOW_KeyHandler(unsigned int key)
{
    int Handled=0;

    switch(key)
    {
    case BF_QUIT:
        SDL_WindowClose();
        Handled=1;
        break;
        
    case BF_SEARCH:
        SEARCHWINDOW_Open();
        Handled=1;
        break;
        
    case BF_MAINVOLUMEUP:
        MIXERUI_IncreaseMainVolume();
        Handled=1;
        break;

    case BF_MAINVOLUMEDOWN:
        MIXERUI_DecreaseMainVolume();
        Handled=1;
        break;

    case BF_AUTOFADE:
        MIXER_DoFade(1,0);
        Handled=1;
        break;

    case BF_NEXTSONG:
        if(PLAYLIST_GetSong(0,0))
        {
            if(PLAYER_IsPlaying(1) && PLAYER_IsPlaying(0)==0)
            {
                if(PLAYER_SetSong(0,0))
                {
                    MIXER_DoFade(1,0);
                    Handled=1;
                }
            }
            else if(PLAYER_IsPlaying(0) && PLAYER_IsPlaying(1)==0)
            {
                if(PLAYER_SetSong(1,0))
                {
                    MIXER_DoFade(1,0);
                    Handled=1;
                }
            }
        }
        break;

    }

    return Handled;
}

static int MAINWINDOW_RedrawTimeout()
{
    CLOCK_Redraw(widgets->Clock);
    MIXERUI_Redraw(widgets->Mixer);
    PLAYERUI_Redraw();
    SONGDBUI_Redraw(widgets->Songdb);
    PLAYLISTUI_Redraw(widgets->Playlist);
    return 60;
}



