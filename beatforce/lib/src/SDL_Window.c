/*
  Beatforce/SDLTk

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

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

#include <SDLTk.h>

static void SDL_WindowRedraw();
static void SDL_WindowRedrawEvent();

static SDL_Window *SDL_WindowGetTopVisibleWindow();
static SDL_Window *SDL_WindowGetTopWindow();

static void SDL_WindowAddToWindowList(SDL_Window *Window);
static void SDL_WidgetDraw(SDL_Widget *widget,SDL_Rect *Rect);

int ScreenWidth;
int ScreenHeight;
int ScreenBpp;

SDL_Surface     *VideoSurface;          /* Drawing is done to this surface of the video driver */ 
SDL_WindowList  *WindowList;            /* List of Windows */

/* Init the video surface information and init the global variables*/
int SDL_WindowInit(SDL_Surface *surface,int width,int height,int bpp)
{
    WindowList       = NULL;

    /* Initialize global variables */
    ScreenWidth     = width;
    ScreenHeight    = height;
    ScreenBpp       = bpp;

    VideoSurface    = surface;

    SDL_WidgetInit();
    return 1;
}

/* Create a new window, this has to be inside the video surface */
SDL_Window *SDL_WindowNew(int x,int y,int width,int height)
{
    SDL_Window *Window;

    Window = malloc(sizeof(SDL_Window));
    memset(Window,0,sizeof(SDL_Window));

    Window->Dimensions.x = x;
    Window->Dimensions.y = y;
    if(width == 0)
        Window->Dimensions.w = ScreenWidth - x;
    else
        Window->Dimensions.w = width;

    if(height == 0)
        Window->Dimensions.h = ScreenHeight - y;
    else
        Window->Dimensions.h = height;

    Window->Visible      = 0;
    Window->FocusWidget  = NULL;

    SDL_WindowAddToWindowList(Window);

    return Window;
}

void SDL_WindowOpen(SDL_Window *Window)
{
    if(Window == NULL)
        return;

    /* Add it to the window list it could be closed in
       the mean time */
    SDL_WindowAddToWindowList(Window);

    SDL_WindowRedrawEvent(); /* Post a redraw event */
}

void SDL_WindowClose()
{
    SDL_WindowList *l;

    SDL_Window *window = SDL_WindowGetTopVisibleWindow();

    l = WindowList;

    if(WindowList->Window == window)
    {
        WindowList = NULL;
    }
    else
    {
        while(l)
        {
            if(l->Next->Window == window)
            {
                l->Next = NULL;
                SDL_WindowRedrawEvent();
            }
            l=l->Next;
        }
    }
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


void SDL_WindowAddWidget(SDL_Widget *widget)
{
    SDL_WindowList *WindowListItem;

    WindowListItem = WindowList;


    while(WindowListItem->Next)
        WindowListItem=WindowListItem->Next;

    if(WindowListItem->Window->WidgetList==NULL)
    {
        WindowListItem->Window->WidgetList=malloc(sizeof(SDL_WidgetList));
        memset(WindowListItem->Window->WidgetList,0,sizeof(SDL_WidgetList));

        /* Adjust the relative coordinates to absolute ones */
        widget->Rect.x += WindowListItem->Window->Dimensions.x;
        widget->Rect.y += WindowListItem->Window->Dimensions.y;

        WindowListItem->Window->WidgetList->Widget  = widget;
        WindowListItem->Window->WidgetList->Next    = NULL;
        WindowListItem->Window->WidgetList->Parent  = NULL;

        /* Set the focus to the new widget if edit widget */
        if(widget->Focusable && SDL_WindowGetFocusWidget() == NULL)
        {
            WindowListItem->Window->FocusWidget=widget;
        }
    }
    else
    {
        SDL_WidgetList *temp;
        temp=WindowListItem->Window->WidgetList;
        

        while(temp->Next)
            temp=temp->Next;

        temp->Next=malloc(sizeof(SDL_WidgetList));
        memset(temp->Next,0,sizeof(SDL_WidgetList));
        temp=temp->Next;

        /* Adjust the relative coordinates to absolute ones */
        widget->Rect.x += WindowListItem->Window->Dimensions.x;
        widget->Rect.y += WindowListItem->Window->Dimensions.y;

        temp->Widget=widget;
        temp->Next=NULL;

        if(widget->Focusable && SDL_WindowGetFocusWidget() == NULL)
        {
            WindowListItem->Window->FocusWidget=widget;
        }
    }


}

/* Return the widget list of the top visible (active) window */
SDL_WidgetList *SDL_WindowGetWidgetList()
{
    SDL_WindowList *surfaces;
    SDL_Window     *Win;
    surfaces=WindowList;

    Win=SDL_WindowGetTopVisibleWindow();
    if(Win)
    {
        return Win->WidgetList;
    }
    else
    {
        return NULL;
    }
}

void SDL_WindowSetFocusWidget(SDL_Widget *focus_widget)
{
    SDL_WindowGetTopVisibleWindow()->FocusWidget=focus_widget;
}

SDL_Widget *SDL_WindowGetFocusWidget()
{
    return SDL_WindowGetTopVisibleWindow()->FocusWidget;
}




int SDLTK_Main()
{
    SDL_Event Event;
    SDL_Event e;
    int handled;
    int retval =0;
    T_Widget_EventHandler eh;
    SDL_WidgetList* WidgetList;
    SDL_WidgetList* focus_widget=NULL;


    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

    while(WindowList)
    {
        while(SDL_WaitEvent(&Event)) 
        {
            handled=0;
            switch(Event.type) 
            {
            case SDL_QUIT:
                while(WindowList)
                {
                    SDL_WindowClose();
                }
                break;
            case SDLTK_WIDGET_EVENT:
            {
                SDL_Widget *widget;
                SDL_Rect   *rect;

                switch(Event.user.code)
                {
                case SDLTK_WIDGET_HIDE:
                    widget = Event.user.data1;
                    if(SDL_WidgetActive(widget))
                    {
                        if(widget->Visible == 1)
                        {
                            widget->Visible = 0;
                            SDL_SignalEmit(widget,"hide");
                            SDL_WidgetDraw(widget,&widget->Rect);
                        }
                    }
                    break;

                case SDLTK_WIDGET_SHOW:
                    widget = Event.user.data1;
                    if(SDL_WidgetActive(widget))
                    {
                        if(widget->Visible == 0)
                        {
                            widget->Visible = 1;
                            SDL_SignalEmit(widget,"show");
                            if(SDL_WindowGetTopWindow()->Visible)
                                SDL_WidgetDraw(widget,&widget->Rect);
                        }
                    }
                    break;
                case SDLTK_WIDGET_RESIZE:
                case SDLTK_WIDGET_MOVE:
                    widget = Event.user.data1;
                    rect   = Event.user.data2;
                    if(SDL_WidgetActive(widget))
                    {
                        SDL_Rect   r;

                        r.x = widget->Rect.x;
                        r.y = widget->Rect.y;
                        r.w = widget->Rect.w;
                        r.h = widget->Rect.h;
                        
                        widget->Rect.x = rect->x;
                        widget->Rect.y = rect->y;
                        widget->Rect.w = rect->w;
                        widget->Rect.h = rect->h;
                        SDL_WidgetDraw(widget,rect);
                        SDL_WidgetDraw(widget,&r);
                        free(rect);
                    }
                    break;

                case SDLTK_WIDGET_REDRAW:
                    widget = Event.user.data1;
                    if(SDL_WidgetActive(widget))
                        SDL_WidgetDraw(widget,&widget->Rect);
                    break;
                case SDLTK_WINDOW_REDRAW:
                    SDL_WindowRedraw();
                    break;

                
                }
                
            }
            break;
            default:
                break;
            }
     
            if(WindowList == NULL)
                break;

            /* Remove all mouse motion events from the queue */
            while(SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_MOUSEMOTIONMASK) > 0);


            switch(Event.type)
            {
            case SDL_MOUSEBUTTONDOWN:
            {
                focus_widget = SDL_WindowGetWidgetList();
        
                while(focus_widget)
                {
                    SDL_Widget *w=(SDL_Widget*)focus_widget->Widget;
                    if(SDL_WidgetIsInside(w,Event.motion.x,Event.motion.y))
                    {
                        if(focus_widget->Widget->Focusable)
                        {
                            SDL_WindowSetFocusWidget(focus_widget->Widget);
                        }
                        // Bug found when there are overlapping widgets the focus is set to the wrong widget
//                break;
                    }
                    focus_widget=focus_widget->Next;
                }

            }   
            break;

            case SDL_KEYDOWN:
                switch( Event.key.keysym.sym ) 
                {
                case SDLK_TAB:
                {
                    SDL_Widget     *FocusWidget;
                    SDL_WidgetList *WidgetList;
                    SDL_Widget     *FirstFocusWidget=NULL;
                    int store=0;

                    WidgetList=SDL_WindowGetWidgetList();
                    FocusWidget=SDL_WindowGetFocusWidget();
            
                    while(WidgetList)
                    {
                        if(FocusWidget == NULL)
                        {
                            if(WidgetList->Widget->Focusable)
                            {
                                SDL_WindowSetFocusWidget(WidgetList->Widget);
                                break;
                            }
                        }
                        else
                        {
                            if(WidgetList->Widget->Focusable && FirstFocusWidget == NULL)
                                FirstFocusWidget=WidgetList->Widget;

                            if(store && WidgetList->Widget->Focusable)
                            {
                                SDL_WindowSetFocusWidget(WidgetList->Widget);
                                break;
                            }
                            if(WidgetList->Widget == FocusWidget)
                                store=1;

                        }
                        WidgetList = WidgetList->Next;
            
                    }
                    if(WidgetList == NULL)
                    {
                        SDL_WindowSetFocusWidget(FirstFocusWidget);
                    }
            
                }
                break;

                default:
                    break;
                }
                break;
        
            }
    
            WidgetList=SDL_WindowGetWidgetList();

            while(WidgetList)
            {
                SDL_Widget *Widget=(SDL_Widget*)WidgetList->Widget;
                if(Widget->Visible)
                {
                    if(SDL_WidgetEvent(Widget,&Event) == 1)
                    {
                        eh = WidgetTable[Widget->Type]->eventhandler;
                        if(eh)
                        {
                            if(eh(Widget,&Event))
                            {
                                retval=1;
                            }
                        }
                    }
                }
                WidgetList=WidgetList->Next;
            }
            /* If the widgets don't handle the event pass
               the event to the event handler of the window */
            {
                SDL_Window *Win;

                Win=SDL_WindowGetTopVisibleWindow();
                
                if(Win && Win->EventHandler)
                {
                    handled=Win->EventHandler(Event);
                }
            }
        }   
    }
    return 1;
}

/* Get the top most visible window.
   On this window the drawing of widgets is done (window has the focus) */
static SDL_Window *SDL_WindowGetTopVisibleWindow()
{
    SDL_WindowList *WindowListItem = WindowList;
    while(WindowListItem && WindowListItem->Next && WindowListItem->Window->Visible)
        WindowListItem=WindowListItem->Next;
   
    if(WindowListItem)
        return WindowListItem->Window;
    else
        return NULL;
}

/* Get the top most window, regardless if it is visible or not 
   On this window new widgets are created */
static SDL_Window *SDL_WindowGetTopWindow()
{
    SDL_WindowList *WindowListItem = WindowList;

    while(WindowListItem->Next)
        WindowListItem=WindowListItem->Next;
    
    if(WindowListItem)
        return WindowListItem->Window;
    else
        return NULL;
}

/* Post a redraw event, this event will trigger a redraw of the top most visible window */
static void SDL_WindowRedrawEvent()
{
    SDL_Event event;

    event.type       = SDLTK_WIDGET_EVENT;
    event.user.code  = SDLTK_WINDOW_REDRAW;
    event.user.data1 = 0;
    event.user.data2 = 0;
    SDL_PushEvent(&event);
}

/* Redraw the entire top most visible window */
static void SDL_WindowRedraw()
{
    SDL_Window *Window;

    Window=SDL_WindowGetTopVisibleWindow();
        
    Window->Visible = 1;
    if(Window->NotifyRedraw)
    {
        Window->NotifyRedraw(Window);
    }
    SDL_DrawAllWidgets(VideoSurface);
}

int SDL_WidgetActive(SDL_Widget *widget)
{
    SDL_Window *Win;
    SDL_WidgetList *temp;

    Win=SDL_WindowGetTopVisibleWindow();
    if(Win)
    {
        temp=Win->WidgetList;
        
        while(temp)
        {
            if(temp->Widget == widget)
                return 1;
            temp=temp->Next;
        }
    }
    return 0;
}

static void SDL_WidgetDraw(SDL_Widget *widget,SDL_Rect *Rect)
{
    SDL_WidgetList *temp;
    T_Widget_Draw  draw; /* Draw function prototype */
    SDL_Rect intersection;

    temp= SDL_WindowGetWidgetList();

    while(temp)
    {
        if(temp->Widget->Visible)
        {
            /* If the redraw area is intersecting with the widget from 
               the list redraw it */
            if(SDL_IntersectRect(Rect,&temp->Widget->Rect,&intersection)) /* //(temp->Widget->Type == SDL_PANEL  ) && */
            {
                draw=WidgetTable[temp->Widget->Type]->draw;
                draw(temp->Widget,VideoSurface,&intersection);
            }
            else if(SDL_RectInside(Rect,&temp->Widget->Rect))
            {
                draw=WidgetTable[temp->Widget->Type]->draw;
                draw(temp->Widget,VideoSurface,Rect);
            }
            else if(SDL_RectInside(&temp->Widget->Rect,Rect))
            {
                draw=WidgetTable[temp->Widget->Type]->draw;
                draw(temp->Widget,VideoSurface,Rect);
            }
        }
        temp=temp->Next;
    }
    SDL_UpdateRect(VideoSurface,Rect->x,Rect->y,Rect->w,Rect->h);
    
}


static void SDL_WindowAddToWindowList(SDL_Window *Window)
{
    if(WindowList == NULL)
    {
        WindowList=(SDL_WindowList*)malloc(sizeof(SDL_WindowList));
        memset(WindowList,0,sizeof(SDL_WindowList));
        
        WindowList->Window=Window;
    }
    else
    {
        SDL_WindowList *l;
        l=WindowList;

        while(l)
        {
            /* If the window is already in the list 
               than don't add it */
            if(l->Window == Window)
                return;
            l=l->Next;
        }

        l=WindowList;

        while(l->Next)
            l=l->Next;

        /* Add the window to the end of the list*/
        if(l->Next == NULL)
        {
            l->Next=(SDL_WindowList*)malloc(sizeof(SDL_WindowList));
            memset(l->Next,0,sizeof(SDL_WindowList));

            l->Next->Window=Window;
        }
    }
}
