/*
  Beatforce/SDLTk

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

#include <stdio.h>
#include <malloc.h>
#include "SDL_Window.h"
#include "SDL_Widget.h"

typedef struct WindowList
{
    SDL_Window *Window;
    struct WindowList *Next;
    struct WindowList *Prev;
}WindowList;

unsigned int SDL_WidgetRedraw(unsigned int interval,void *data);

SDL_Surface *screen;
SDL_Window *CurWindow;

WindowList *WindowManager;

SDL_Surface  *previous_surface; /* Previous active surface */

SDL_Widget *CurrentFocus;       /* Current widget focus    */
SDL_Screen *ScreenList;         /* List of surfaces/screens */
SDL_Screen *CreateOnStack;      /* This is the screen where the create functions
                                   add widgets to */
SDL_Screen *ActiveScreen;

int SDL_WindowInit(SDL_Surface *s)
{
    previous_surface = NULL;
    CurrentFocus     = NULL;
    ScreenList      = NULL;
    CreateOnStack    = NULL;
    ActiveScreen     = NULL;

    /* Initialize global variables */
    CurWindow       = NULL;
    WindowManager   = NULL;
    screen          = s;
    return 1;
}

/* Allocate a new surface and initialize the widget list */
int SDL_StackNewScreen(SDL_Surface *surface)
{
    if(ScreenList == NULL)
    {
        ScreenList=(SDL_Screen*)malloc(sizeof(SDL_Screen));
        memset(ScreenList,0,sizeof(SDL_Screen));
        ScreenList->Surface=surface;
        CreateOnStack=ScreenList;
    }
    else
    {
        SDL_Screen *l;
        l=ScreenList;
        while(l->next)
            l=l->next;

        l->next=(SDL_Screen*)malloc(sizeof(SDL_Screen));
        memset(l->next,0,sizeof(SDL_Screen));
        l->next->Surface=surface;
        CreateOnStack=l->next;
        return 0;
    }
    return 1;
}


int 
SDL_ActiveSurface(SDL_Surface *surface)
{
    SDL_Screen *surfaces;
    surfaces=ScreenList;

    while(surfaces)
    {
        if(surfaces->Surface == surface)
        {
            ActiveScreen=surfaces;
            return 1;
        }
        surfaces=surfaces->next;

    }
    return 0;
}

SDL_Surface *SDL_GetSurfaceStack()
{
    SDL_Screen *surfaces;
    surfaces=ScreenList;
    while(surfaces)
    {
        if(surfaces == ActiveScreen)
        {
            return surfaces->Surface;
        }
        surfaces=surfaces->next;

    }
    return NULL;

}

SDL_Surface *SDL_GetPreviousStack()
{
    return previous_surface;

}

void SDL_StoreWidget(SDL_Widget *widget)
{
    if(CreateOnStack->WidgetList==NULL)
    {
        CreateOnStack->WidgetList=malloc(sizeof(SDL_WidgetList));
        memset(CreateOnStack->WidgetList,0,sizeof(SDL_WidgetList));

        CreateOnStack->WidgetList->Widget  = widget;
        CreateOnStack->WidgetList->Next    = NULL;
        CreateOnStack->WidgetList->Parent  = NULL;

        /* Set the focus to the new widget if edit widget */
        if(widget->Focusable && SDL_StackGetFocus() == NULL)
            SDL_StackSetFocus(widget);
        
    }
    else
    {
        SDL_WidgetList *temp;
        temp=CreateOnStack->WidgetList;
        
        while(temp->Next)
            temp=temp->Next;

        temp->Next=malloc(sizeof(SDL_WidgetList));
        memset(temp->Next,0,sizeof(SDL_WidgetList));
        temp=temp->Next;
        temp->Widget=widget;
        temp->Next=NULL;

        if(widget->Focusable && SDL_StackGetFocus() == NULL)
            SDL_StackSetFocus(widget);
    }


}

SDL_WidgetList *SDL_StackGetLastItem()
{
    SDL_WidgetList *tmp=CreateOnStack->WidgetList;
    while(tmp->Next)
        tmp=tmp->Next;
    return tmp;
}


SDL_WidgetList *SDL_StackGetStack(SDL_Surface *surface)
{
    SDL_Screen *surfaces;
    surfaces=ScreenList;

    if(surface == NULL)
    {
        return ActiveScreen->WidgetList;
    }
    else
    {
        while(surfaces)
        {
            if(surfaces->Surface == surface)
                return surfaces->WidgetList;
            
            surfaces=surfaces->next;
        }

    }
    return NULL;
}

void SDL_StackSetFocus(SDL_Widget *focus_widget)
{
    CurrentFocus=focus_widget;
}

SDL_Widget *SDL_StackGetFocus()
{
    return CurrentFocus;
}


void SDL_WindowClose()
{
    WindowList *l;

    l=WindowManager;

    if(l->Next)
    {
        CurWindow=l->Window;
        free(l->Next);
        l->Next=NULL;
    }
    else
    {
        free(WindowManager);
        WindowManager=NULL;
        CurWindow=NULL;
    }
    if(CurWindow && CurWindow->Surface)
        SDL_WidgetUseSurface(CurWindow->Surface);
}


void SDL_WindowOpen(SDL_Window *window)
{
    WindowList *l;

    if(WindowManager == NULL)
    {
        WindowManager = malloc(sizeof(WindowList));
        memset(WindowManager,0,sizeof(WindowList));
        WindowManager->Window=window;
    }
    else
    {
        l=WindowManager;
        while(l->Next)
            l=l->Next;
     
        l->Next=malloc(sizeof(WindowList));
        memset(l->Next,0,sizeof(WindowList));
        
        l->Next->Window=window;
    }
    CurWindow=window;
    SDL_WidgetUseSurface(CurWindow->Surface);
}

unsigned int SDL_WidgetRedraw(unsigned int interval,void *data)
{
    if(CurWindow)
    {
        if(CurWindow->NotifyRedraw)
            CurWindow->NotifyRedraw(CurWindow);

        SDL_DrawAllWidgets(screen);
        return 60; //redraw every 50ms 
    }
    else
    {
        /* WindowManager is finished, no windows to draw */
        return 0; 
    }

}

int SDLTK_Main()
{
    SDL_Event test_event;
    SDL_TimerID timer;
    int handled;

    timer=SDL_AddTimer(1,SDL_WidgetRedraw,NULL);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

    while(CurWindow)
    {
        while(SDL_PollEvent(&test_event)) 
        {
            handled=0;
            switch(test_event.type) 
            {
            case SDL_QUIT:
                CurWindow=NULL;
                break;
//            case SDL_VIDEOEXPOSE:
//                SDL_WidgetRedraw(10,NULL);
                break;
            default:
                break;
            }
     
            /* If the widgets don't handle the event pass
               the event to the event handler of the window */
            if(SDL_WidgetEvent(&test_event) == 0)
            {
                if(CurWindow && CurWindow->EventHandler)
                {
                    handled=CurWindow->EventHandler(test_event);
                }
            }
        
        }   
        SDL_Delay(25); /* To reduce CPU load */
    }
    SDL_RemoveTimer(timer);
    return 1;
}
