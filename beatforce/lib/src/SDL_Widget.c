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

#include <malloc.h>
#include <stdarg.h>


#include "SDL_Widget.h"
#include "SDL_Window.h"
#include "SDL_WidTool.h"
#include "SDL_Panel.h"

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
    MyMutex=SDL_CreateMutex();
    return 1;
}

SDL_Surface *SDL_WidgetNewSurface(int width,int height,int bpp)
{
    SDL_Surface *s;

    s=SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,bpp,
                           0xff0000,0x00ff00,0x0000ff,0x000000);
    SDL_StackNewScreen(s);
    return s;

}

int SDL_WidgetUseSurface(SDL_Surface *surface)
{
    SDL_ActiveSurface(surface);
    return 1;
}

SDL_Surface *SDL_WidgetGetActiveSurface()
{
    return SDL_GetSurfaceStack();
}

int SDL_WidgetClearSurface(SDL_Surface *surface)
{
    SDL_WidgetList* current_widget;
    
    current_widget=SDL_StackGetStack(surface);
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
        
        NewWidget=create(&dest);

        SDL_StoreWidget(NewWidget);
        return NewWidget;
    }
    else
    {
        return NULL;
    }
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
        retval=properties(widget,feature,ap);
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
int SDL_WidgetClose(void *widget)
{
    SDL_WidgetList *current_widget,*prev;
    return 0;
    current_widget=SDL_StackGetStack(NULL);
    prev=NULL;

    while(current_widget)
    {
        if(current_widget->Widget == widget)
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

    SDL_Rect dest;
    SDL_Rect src;

    if(target_surface == NULL && screen)
        target_surface = screen;

    active_surface=SDL_GetSurfaceStack();
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

    WidgetList=SDL_StackGetStack(NULL);

    
//    if(current_widget == NULL)
//        printf("Nothing to draw\n");

    SDL_mutexP(MyMutex);
    while(WidgetList)
    {
        Widget=(SDL_Widget*)WidgetList->Widget;
        draw=WidgetTable[Widget->Type]->draw;
        draw(Widget,screen,NULL);
        WidgetList=WidgetList->Next;
    }
    
    
    SDL_Flip(screen);
//    SDL_UpdateRect(screen,0,0,0,0);                
    SDL_mutexV(MyMutex);


    //   SDL_BlitSurface(active_surface,NULL,screen,NULL);
///    SDL_BlitSurface(last_surface,&src,screen,&dest);

    
    if(previous!=active_surface)
        previous=active_surface;




    return 1;
}

int SDL_WidgetEvent(SDL_Event *event)
{
    int retval =0;
    T_Widget_EventHandler eh;
    SDL_WidgetList* WidgetList;
    SDL_WidgetList* focus_widget=NULL;

    switch(event->type)
    {

    case SDL_MOUSEBUTTONDOWN:
    {
        focus_widget=SDL_StackGetStack(NULL);
        
        while(focus_widget)
        {
            SDL_Widget *w=(SDL_Widget*)focus_widget->Widget;
            if(SDL_WidgetIsInside(w,event->motion.x,event->motion.y))
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
        switch( event->key.keysym.sym ) 
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

    WidgetList=SDL_StackGetStack(NULL);

    while(WidgetList)
    {
        SDL_Widget *Widget=(SDL_Widget*)WidgetList->Widget;
        eh = WidgetTable[Widget->Type]->eventhandler;
        if(eh)
        {
            if(eh(Widget,event))
            {
                retval=1;
            }
        }
        WidgetList=WidgetList->Next;
    }

    return retval;
}

int SDL_WidgetSetFocus(SDL_Widget *widget)
{
    if(widget && widget->Focusable)
    {
        SDL_StackSetFocus(widget);
        return 1;
    }
    return 0;
}

int SDL_WidgetHasFocus(SDL_Widget *widget)
{
    if(SDL_StackGetFocus() == widget)
        return 1;
    return 0;
}

int SDL_WidgetLoseFocus()
{
    SDL_StackSetFocus(NULL);
    return 1;
}

void SDL_WidgetRedrawEvent(SDL_Widget *widget)
{
    SDL_Event event;
    
    event.type = SDL_USEREVENT;
    event.user.code = 0;
    event.user.data1 = widget;
    event.user.data2 = 0;
    SDL_PushEvent(&event);
}

