/*
  Beatforce/Main windows user interface

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

#include <malloc.h>

#include "config.h"
#include "mixer.h"
#include "osa.h"
#include "player.h"
#include "songdb.h"
#include "playlist.h"
#include "player.h"

#include "main_window.h"


int control_state;
//static int MAINWINDOW_EventHandler(SDL_Event event);
static int MAINWINDOW_CreateWindow();
//static int MAINWINDOW_KeyHandler(unsigned int key);
//static int MAINWINDOW_RedrawTimeout();


MainwindowWidgets *widgets;

//#define BF_CONTROL         0xf000
//#define BF_QUIT            SDLK_ESCAPE
//#define BF_SEARCH          SDLK_j
//#define BF_NEXTSONG        SDLK_b
//#define BF_MAINVOLUMEDOWN  SDLK_PERIOD
//#define BF_MAINVOLUMEUP    SDLK_SLASH
//#define BF_AUTOFADE        SDLK_f

int MAINWINDOW_Open()
{

    widgets=malloc(sizeof(MainwindowWidgets));
    memset(widgets,0,sizeof(MainwindowWidgets));
    MAINWINDOW_CreateWindow();
/*
    if(MAINWINDOW.Surface == NULL)
    {

    MAINWINDOW.Surface=MAINWINDOW_CreateWindow(widgets);
    MAINWINDOW.TransferData=widgets;
    }
    SDL_WindowOpen(&MAINWINDOW);
*/
}

void configopen(void *d)
{
    CONFIGWINDOW_Open();
}

static int MAINWINDOW_CreateWindow()
{
    GtkWidget         *MainWindow;
    GtkWidget         *fixed;
//    ThemeConfig       *tc = THEME_GetActive();
//    ThemeMainWindow   *mw = NULL;
//    ThemeImage        *Image = NULL;
//    ThemeScreen       *s=tc->Screen;

 /* create a new window */
    MainWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_usize(MainWindow,1024,685);
    fixed = gtk_fixed_new ();
    gtk_widget_show (fixed);
    gtk_container_add (GTK_CONTAINER (MainWindow), fixed);


#if 0    
    if(tc == NULL)
        return NULL;
    
    mw=tc->MainWindow;
    
    if(mw == NULL)
        return NULL;

    Image=mw->Image;


    control_state=0;
    MainWindow = SDL_WidgetNewSurface(s->Width,s->Height,s->BPP);

    SDL_SetColorKey(MainWindow,0,0); // disable transparancy
   

    while(Image)
    {
        SDL_Widget *image;
        image=SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_PanelSetImage(image,IMG_Load(Image->filename));
        Image=Image->next;
    }


    w->Clock=CLOCK_Create(mw->Clock);


#endif 
//    printf("Creating playlist widget\n");
//    widgets->Playlist=PLAYLISTUI_CreateWindow(MainWindow);
    PLAYERUI_CreateWindow(0,fixed);
#if 0
    PLAYERUI_CreateWindow(1,mw->Player[1]);

    w->Mixer = MIXERUI_CreateWindow(mw->Mixer);
#endif
    widgets->Songdb = SONGDBUI_CreateWindow(fixed);    
    SONGDBUI_Redraw(widgets->Songdb);
#if 0
    OSA_StartTimer(60,MAINWINDOW_RedrawTimeout,NULL);

    return MainWindow;
#endif

    gtk_widget_show(MainWindow);
}


#if 0
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
                    SONGDBUI_Play(0);
                    Handled=1;
                }
            }
            else if(PLAYER_IsPlaying(0) && PLAYER_IsPlaying(1)==0)
            {
                if(PLAYER_SetSong(1,0))
                {
                    MIXER_DoFade(1,0);
                    SONGDBUI_Play(1);
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
    if(SDL_GetSurfaceStack() == MAINWINDOW.Surface)
    {
        CLOCK_Redraw(widgets->Clock);
        MIXERUI_Redraw(widgets->Mixer);
        PLAYERUI_Redraw();
        SONGDBUI_Redraw(widgets->Songdb);
        PLAYLISTUI_Redraw(widgets->Playlist);
    }
    return 60;
}
#endif


