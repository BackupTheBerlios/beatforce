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
#include <stdarg.h>


#include "SDL_Widget.h"
#include "SDL_Window.h"
#include "SDL_WidTool.h"
#include "SDL_Panel.h"
#include "SDL_Signal.h"

T_Widget_EventHandler user_eventhandler;
int StackLock;
SDL_Surface *last_surface = NULL;
SDL_Surface *previous;
SDL_Surface *target_surface;
int fadex;
int fadey;
int fadew;
int fadeh;
int fadeon;
static SDL_mutex *MyMutex;

void EnableFade()
{
    fadey=0;
    fadex=0;
    fadew=0;
    fadeh=0;
    fadeon=0;
}

int SDL_WidgetInit()
{
    target_surface = NULL;
    StackLock = 0;
    SDL_SignalInit();
    

    /* Widget */
    SDL_SignalNew("mousebuttondown",1);
    SDL_SignalNew("mousemotion",1);
    SDL_SignalNew("keydown",1);
    SDL_SignalNew("hide",0);
    SDL_SignalNew("show",0);

    /* Button */
    SDL_SignalNew("clicked",0); 
    /* Tab */
    SDL_SignalNew("switch-tab",0);
    /* Table */
    SDL_SignalNew("select-row",0);
    SDL_SignalNew("edited",0);
    /* Slider */
    SDL_SignalNew("value-changed",0);
    /* Edit */
    SDL_SignalNew("changed",0);
    SDL_SignalNew("activate",0);

    MyMutex=SDL_CreateMutex();
    return 1;
}

SDL_Widget* SDL_WidgetCreate(E_Widget_Type widget,int x,int y, int w, int h)
{
    SDL_Rect dest;

    dest.x = (unsigned short)x;
    dest.y = (unsigned short)y;
    dest.h = (unsigned short)h;
    dest.w = (unsigned short)w;

    return SDL_WidgetCreateR(widget,dest);
}

SDL_Widget* SDL_WidgetCreateR(E_Widget_Type WidgetType,SDL_Rect dest)
{
    T_Widget_Create create;
    SDL_Widget *NewWidget;

    if(WidgetTable[WidgetType])
    {
        create=WidgetTable[WidgetType]->create;
        if(create)
        {
            NewWidget=create(&dest);

            NewWidget->Visible = 0;
            SDL_WindowAddWidget(NewWidget);
            return NewWidget;
        }
    }
    return NULL;
}

int SDL_WidgetPropertiesOf(SDL_Widget *widget,int feature,...)
{
    va_list ap;
    T_Widget_Properties properties;
    int retval=0;

    va_start(ap,feature);

    if(widget)
    {
        properties=WidgetTable[widget->Type]->properties;
        if(properties)
        {
            retval=properties(widget,feature,ap);
        }
    }
    else
    {
//        printf("SDL_WidgetPropertiesOf not found\n");
    }
    return retval;
}

/*
 * Closes the widget
 */
int SDL_WidgetClose(SDL_Widget *Widget)
{
    SDL_WidgetList *current_widget,*prev;

    current_widget=SDL_WindowGetWidgetList();
    prev=NULL;

    while(current_widget)
    {
        if(current_widget->Widget == Widget)
        {
            prev->Next = current_widget->Next;
            break;
        }
        prev=current_widget;
        current_widget=current_widget->Next;
    }
    return 1;
}


/*
 * Draws all widgets of the current stack onto the destiation screen
 */
int SDL_DrawAllWidgets(SDL_Surface *screen)
{
    T_Widget_Draw draw; /* Draw function prototype */
    SDL_WidgetList *WidgetList;
    SDL_Surface    *active_surface = NULL;
    SDL_Widget     *Widget;

#if 0
    SDL_Rect dest;
    SDL_Rect src;

    if(target_surface == NULL && screen)
        target_surface = screen;

    active_surface = SDL_GetSurfaceStack();
    if(active_surface == NULL)
        return 0;

    if(previous != active_surface)
    {
        last_surface=previous;
        fadeon=1;
    }

    dest.x=0;
    dest.y=0;
    dest.w=0;
    dest.h=0;

    if(active_surface)
    {
        src.x=0;
        src.y=0;
        src.w=active_surface->w;
        src.h=active_surface->h;
    }
#endif
    WidgetList=SDL_WindowGetWidgetList();

    
//    if(current_widget == NULL)
//        printf("Nothing to draw\n");

    SDL_mutexP(MyMutex);
    while(WidgetList)
    {
        Widget=(SDL_Widget*)WidgetList->Widget;
        if(Widget->Visible)
        {
            draw=WidgetTable[Widget->Type]->draw;
            draw(Widget,screen,NULL);
        }
        WidgetList=WidgetList->Next;
    }
    
    
    
    SDL_UpdateRect(screen,0,0,0,0);                
    SDL_mutexV(MyMutex);


    //   SDL_BlitSurface(active_surface,NULL,screen,NULL);
///    SDL_BlitSurface(last_surface,&src,screen,&dest);

    
    if(previous!=active_surface)
        previous=active_surface;




    return 1;
}

int SDL_WidgetEvent(SDL_Widget *widget,SDL_Event *event)
{
   switch(event->type)
   {
   case SDL_KEYDOWN:
       SDL_SignalEmit(widget,"keydown",event);
       break;
   case SDL_KEYUP:
       SDL_SignalEmit(widget,"keyup",event);
       break;
   case SDL_MOUSEMOTION:
       SDL_SignalEmit(widget,"mousemotion",event);
       break;
   case SDL_MOUSEBUTTONDOWN:
       SDL_SignalEmit(widget,"mousebuttondown",event);
       break;
   case SDL_MOUSEBUTTONUP:
       SDL_SignalEmit(widget,"mousebuttonup",event);
       break;
   default:
       break;
   }
   return 1;
}

int SDL_WidgetSetFocus(SDL_Widget *widget)
{
    if(widget && widget->Focusable)
    {
        SDL_WindowSetFocusWidget(widget);
        return 1;
    }
    return 0;
}

int SDL_WidgetHasFocus(SDL_Widget *widget)
{
    if(SDL_WindowGetFocusWidget() == widget)
        return 1;
    return 0;
}

int SDL_WidgetLoseFocus()
{
    SDL_WindowSetFocusWidget(NULL);
    return 1;
}

void SDL_WidgetRedrawEvent(SDL_Widget *widget)
{
    SDL_Event event;
    
    if(widget == NULL)
        return;

    event.type = SDLTK_WIDGET_EVENT;
    event.user.code  = SDLTK_WIDGET_REDRAW;
    event.user.data1 = widget;
    event.user.data2 = 0;
    SDL_PushEvent(&event);
}

void SDL_WidgetHide(SDL_Widget *widget)
{
    SDL_Event event;
    
    if(widget == NULL)
        return;

    event.type = SDLTK_WIDGET_EVENT;
    event.user.code = SDLTK_WIDGET_HIDE;
    event.user.data1 = widget;
    event.user.data2 = 0;
    SDL_PushEvent(&event);
}


void SDL_WidgetShow(SDL_Widget *widget)
{
    SDL_Event event;
    
    if(widget == NULL)
        return;
    
    event.type = SDLTK_WIDGET_EVENT;
    event.user.code = SDLTK_WIDGET_SHOW;
    event.user.data1 = widget;
    event.user.data2 = 0;
    SDL_PushEvent(&event);

}

void SDL_WidgetMove(SDL_Widget *widget,int x, int y)
{
    SDL_Event event;
    SDL_Rect  *rect;

    if(widget == NULL)
        return;
    
    rect = malloc(sizeof(SDL_Rect));
    
    rect->x = x;
    rect->y = y;
    rect->w = widget->Rect.w;
    rect->h = widget->Rect.h;

    event.type = SDLTK_WIDGET_EVENT;
    event.user.code = SDLTK_WIDGET_MOVE;
    event.user.data1 = widget;
    event.user.data2 = rect;
    SDL_PushEvent(&event);
}

void SDL_WidgetResize(SDL_Widget *widget,int w,int h)
{
    SDL_Event event;
    SDL_Rect  *rect;

    if(widget == NULL)
        return;
    
    rect = malloc(sizeof(SDL_Rect));
    
    rect->x = widget->Rect.x;
    rect->y = widget->Rect.y;
    rect->w = w;
    rect->h = h;

    event.type = SDLTK_WIDGET_EVENT;
    event.user.code = SDLTK_WIDGET_RESIZE;
    event.user.data1 = widget;
    event.user.data2 = rect;
    SDL_PushEvent(&event);
}
