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
#include "SDL_Stack.h"

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
    SDL_StackInit();
    return 1;
}

SDL_Surface *SDL_WidgetNewSurface(int width,int height,int bpp)
{
    SDL_Surface *s;
    s=SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,bpp,
                           0xff0000,0x00ff00,0x0000ff,0x000000);
    SDL_StackNewSurface(s);
    return s;

}

int SDL_WidgetUseSurface(SDL_Surface *surface)
{
    SDL_WidgetForceRedraw(surface);
    SDL_SurfaceStack(surface);
    return 1;
}

SDL_Surface *SDL_WidgetGetActiveSurface()
{
    return SDL_GetSurfaceStack();
}

int SDL_WidgetClearSurface(SDL_Surface *surface)
{
    Stack* current_widget;
    
    current_widget=SDL_StackGetStack(surface);
    return 1;
}

void* SDL_WidgetCreate(E_Widget_Type widget,int x,int y, int w, int h)
{
    SDL_Rect dest;

    dest.x = (unsigned short)x;
    dest.y = (unsigned short)y;
    dest.h = (unsigned short)h;
    dest.w = (unsigned short)w;

    return SDL_WidgetCreateR(widget,dest);
}

void* SDL_WidgetCreateR(E_Widget_Type widget,SDL_Rect dest)
{
    T_Widget_Create create;
    void* widgetdata = NULL;

    if(WidgetTable[widget])
    {
        create=WidgetTable[widget]->create;
        
        widgetdata=create(&dest);

        if(widgetdata == NULL)
        {
            printf("Error creating widget\n");
            SDL_Quit();
        }
        SDL_AddToStack(widget,&dest,widgetdata);
        return widgetdata;
    }
    else
    {
        return NULL;
    }
}

int SDL_WidgetProperties(int feature,...)
{
    va_list ap;
    T_Widget_Properties properties;
    Stack* current_widget;

    va_start(ap,feature);
    current_widget=SDL_StackGetLastItem();

    properties=WidgetTable[current_widget->type]->properties;
    properties(current_widget->data,feature,ap);
    current_widget=current_widget->next;
    return 1;
}

int SDL_WidgetPropertiesOf(void *widget,int feature,...)
{
    va_list ap;
    T_Widget_Properties properties;
    Stack* current_widget;
    int retval=0;

    va_start(ap,feature);

    current_widget=SDL_StackGetStack(NULL);

    while(current_widget)
    {
        if(current_widget->data == widget)
            break;
        current_widget=current_widget->next;
    }
    if(current_widget)
    {
        properties=WidgetTable[current_widget->type]->properties;
        retval=properties(current_widget->data,feature,ap);
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
    Stack *current_widget,*prev;
    current_widget=SDL_StackGetStack(NULL);
    prev=NULL;

    while(current_widget)
    {
        if(current_widget->data == widget)
        {
            prev->next = current_widget->next;
            break;
        }
        prev=current_widget;
        current_widget=current_widget->next;
    }
    

    return 1;
}


/*
 * Draws all widgets of the current stack onto the destiation screen
 */
int SDL_DrawAllWidgets(SDL_Surface *screen)
{
    T_Widget_Draw draw;
    Stack* current_widget;
    SDL_Surface *active_surface = NULL;


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

    SDL_WidgetLOCK();

    current_widget=SDL_StackGetStack(NULL);

    
//    if(current_widget == NULL)
//        printf("Nothing to draw\n");

    SDL_mutexP(MyMutex);
    while(current_widget)
    {
        draw=WidgetTable[current_widget->type]->draw;
        draw(current_widget->data,screen);
        current_widget=current_widget->next;
    }
    SDL_UpdateRect(screen,0,0,0,0);                
    SDL_mutexV(MyMutex);


    //   SDL_BlitSurface(active_surface,NULL,screen,NULL);
///    SDL_BlitSurface(last_surface,&src,screen,&dest);

    
    if(previous!=active_surface)
        previous=active_surface;




    return 1;
}

void  SDL_WidgetEvent(SDL_Event *event)
{
    T_Widget_EventHandler eh;
    Stack* current_widget;
    Stack* focus_widget=NULL;

    switch(event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
    {
        focus_widget=SDL_StackGetStack(NULL);
        
        while(focus_widget)
        {
            if(SDL_WidgetIsInside(&focus_widget->dest,event->motion.x,event->motion.y))
            {
                SDL_StackSetFocus(focus_widget);
                // Bug found when there are overlapping widgets the focus is set to the wrong widget
//                break;
            }
            focus_widget=focus_widget->next;
        }

    }    
    break;

    }

    current_widget=SDL_StackGetStack(NULL);

    while(current_widget)
    {
        eh=WidgetTable[current_widget->type]->eventhandler;
        if(eh)
        {
            eh(current_widget->data,event);
        }
        current_widget=current_widget->next;
    }
    
}


int SDL_WidgetHasFocus(void *widget)
{
    Stack* current_widget;

    current_widget=SDL_StackGetFocus();

    if(current_widget != NULL)
    {
        if(widget==current_widget->data)
            return 1;
    }
    return 0;
}

int SDL_WidgetLoseFocus()
{
    SDL_StackSetFocus(NULL);
    return 1;
}

int SDL_WidgetForceRedraw(SDL_Surface *surface)
{
    T_Widget_Properties properties;
    Stack* current_widget;

    current_widget=SDL_StackGetStack(surface);
    
    if(current_widget == NULL)
        return 0;
    
    while(current_widget)
    {
        properties=WidgetTable[current_widget->type]->properties;
        if(properties)
            properties(current_widget->data,FORCE_REDRAW,NULL);
        current_widget=current_widget->next;
    }
    return 1;
}

int SDL_WidgetLOCK()
{
    return 1;
}

int SDL_WidgetUNLOCK()
{
    return 1;
}




