/*
  Beatforce/SDLTk

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

#include <stdio.h>
#include <malloc.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include "SDL_Window.h"
#include "SDL_Widget.h"

typedef struct RectList
{
    SDL_Rect *Rect;
    struct RecList *Next;
}RectList;

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
RectList  *DrawQueue;

int SDL_WindowInit(SDL_Surface *s)
{
    previous_surface = NULL;
    CurrentFocus     = NULL;
    ScreenList       = NULL;
    CreateOnStack    = NULL;
    ActiveScreen     = NULL;
    DrawQueue        = NULL;
    /* Initialize global variables */
    CurWindow       = NULL;
    WindowManager   = NULL;
    screen          = s;
    SDL_WidgetInit();
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

        if(l->next == NULL)
        {
            l->next=(SDL_Screen*)malloc(sizeof(SDL_Screen));
            memset(l->next,0,sizeof(SDL_Screen));
            l->next->Surface=surface;
            CreateOnStack=l->next;
        }
        else
        {
            printf("ERROR %s %d\n",__FILE__,__LINE__);
        }
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
            CreateOnStack=ActiveScreen;
            SDL_WidgetRedraw(0,NULL);
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
/*
 * A function to calculate the intersection of two rectangles:
 * return true if the rectangles intersect, false otherwise
 */
static 
int SDL_IntersectRect(const SDL_Rect *A, const SDL_Rect *B,SDL_Rect *intersection)
{
    int Amin, Amax, Bmin, Bmax;
    
    /* Horizontal intersection */
    Amin = A->x;
    Amax = Amin + A->w;
    Bmin = B->x;
    Bmax = Bmin + B->w;
    if(Bmin > Amin)
        Amin = Bmin;
    intersection->x = Amin;
    if(Bmax < Amax)
        Amax = Bmax;
    intersection->w = Amax - Amin > 0 ? Amax - Amin : 0;

    /* Vertical intersection */
    Amin = A->y;
    Amax = Amin + A->h;
    Bmin = B->y;
    Bmax = Bmin + B->h;
    if(Bmin > Amin)
        Amin = Bmin;
    intersection->y = Amin;
    if(Bmax < Amax)
        Amax = Bmax;
    intersection->h = Amax - Amin > 0 ? Amax - Amin : 0;
    
    return (intersection->w && intersection->h);
}

static 
int SDL_RectInside(const SDL_Rect *A, const SDL_Rect *B)
{
    if(A->x >= B->x)
        if(A->y >= B->y)
            if(A->y <= B->y + B->h)
                if(A->x <= B->x + B->w)
                    if(A->x + A->w <= B->x + B->w)
                        if(A->y + A->h <= B->y + B->h)
                            return 1;
    return 0;
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
    SDL_Event e;
    int handled;
    int retval =0;
    T_Widget_EventHandler eh;
    SDL_WidgetList* WidgetList;
    SDL_WidgetList* focus_widget=NULL;


    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

    while(CurWindow)
    {
        while(SDL_WaitEvent(&test_event)) 
        {
            handled=0;
            switch(test_event.type) 
            {
            case SDL_QUIT:
                CurWindow=NULL;
                break;
            case SDL_USEREVENT:
                {
                    SDL_Widget *widget;
                    widget = test_event.user.data1;
                    if(test_event.user.code == 2)
                        widget->Visible = 1;

                    if(test_event.user.code == 1)
                        widget->Visible = 0;

                    if(SDL_WidgetActive(widget))
                        SDL_WidgetDraw(widget,&widget->Rect);

                }
                break;
            default:
                break;
            }
     
            if(CurWindow == NULL)
                break;

            /* Remove all mouse motion events from the queue */
            while(SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_MOUSEMOTIONMASK) > 0);


    switch(test_event.type)
    {

    case SDL_MOUSEBUTTONDOWN:
    {
        focus_widget=SDL_StackGetStack(NULL);
        
        while(focus_widget)
        {
            SDL_Widget *w=(SDL_Widget*)focus_widget->Widget;
            if(SDL_WidgetIsInside(w,test_event.motion.x,test_event.motion.y))
            {
                if(focus_widget->Widget->Focusable)
                {
                    SDL_StackSetFocus(focus_widget->Widget);
                }
                // Bug found when there are overlapping widgets the focus is set to the wrong widget
//                break;
            }
            focus_widget=focus_widget->Next;
        }

    }   
    break;

    case SDL_KEYDOWN:
        switch( test_event.key.keysym.sym ) 
        {
        case SDLK_TAB:
        {
            SDL_Widget     *FocusWidget;
            SDL_WidgetList *WidgetList;
            SDL_Widget     *FirstFocusWidget=NULL;
            int store=0;

            WidgetList=SDL_StackGetStack(NULL);
            FocusWidget=SDL_StackGetFocus();
            
            while(WidgetList)
            {
                if(FocusWidget == NULL)
                {
                    if(WidgetList->Widget->Focusable)
                    {
                        SDL_StackSetFocus(WidgetList->Widget);
                        break;
                    }
                }
                else
                {
                    if(WidgetList->Widget->Focusable && FirstFocusWidget == NULL)
                        FirstFocusWidget=WidgetList->Widget;

                    if(store && WidgetList->Widget->Focusable)
                    {
                        SDL_StackSetFocus(WidgetList->Widget);
                        break;
                    }
                    if(WidgetList->Widget == FocusWidget)
                        store=1;

                }
                WidgetList = WidgetList->Next;
            
            }
            if(WidgetList == NULL)
                SDL_StackSetFocus(FirstFocusWidget);
            
        }
        break;

        default:
            break;
        }
        break;
        
    }
            /* If the widgets don't handle the event pass
               the event to the event handler of the window */
            WidgetList=SDL_StackGetStack(NULL);

            while(WidgetList)
            {
                SDL_Widget *Widget=(SDL_Widget*)WidgetList->Widget;
                if(Widget->Visible)
                {
                    if(SDL_WidgetEvent(Widget,&test_event) == 0)
                    {
                        eh = WidgetTable[Widget->Type]->eventhandler;
                        if(eh)
                        {
                            if(eh(Widget,&test_event))
                            {
                                retval=1;
                            }
                        }
                    }
                }
                WidgetList=WidgetList->Next;
            }
            
            {
                if(CurWindow && CurWindow->EventHandler)
                {
                    handled=CurWindow->EventHandler(test_event);
                }
            }
        }   
    }
    return 1;
}

int SDL_WidgetActive(SDL_Widget *widget)
{
    SDL_WidgetList *temp;
    temp=ActiveScreen->WidgetList;

    while(temp)
    {
        if(temp->Widget == widget)
            return 1;
        temp=temp->Next;
    }
    return 0;
}

void SDL_WidgetDraw(SDL_Widget *widget,SDL_Rect *Rect)
{
    SDL_WidgetList *temp;
    T_Widget_Draw draw; /* Draw function prototype */
    SDL_Rect intersection;

    temp=ActiveScreen->WidgetList;

    while(temp)
    {
        if(temp->Widget->Visible)
        {
            if(//(temp->Widget->Type == SDL_PANEL  ) &&
               SDL_IntersectRect(Rect,&temp->Widget->Rect,&intersection))
            {
                draw=WidgetTable[temp->Widget->Type]->draw;
                draw(temp->Widget,screen,&intersection);
            
            }
            else if(SDL_RectInside(Rect,&temp->Widget->Rect))
            {
                draw=WidgetTable[temp->Widget->Type]->draw;
                draw(temp->Widget,screen,Rect);
            }
            else if(SDL_RectInside(&temp->Widget->Rect,Rect))
            {
                draw=WidgetTable[temp->Widget->Type]->draw;
                draw(temp->Widget,screen,Rect);
            }
        }
        temp=temp->Next;
    }
    SDL_UpdateRect(screen,Rect->x,Rect->y,Rect->w,Rect->h);
    
}
