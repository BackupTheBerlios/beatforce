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

#include <SDL/SDL_image.h>
#include "SDL_Button.h"
#include "SDL_Widget.h"

#include <stdlib.h>
#include <stdarg.h>


const struct S_Widget_FunctionList SDL_Button_FunctionList =
{
    SDL_ButtonCreate,
    SDL_ButtonDraw,
    SDL_ButtonProperties,
    SDL_ButtonEventHandler,
    SDL_ButtonClose
};

void* SDL_ButtonCreate(SDL_Rect* rect)
{    
    SDL_Button *newbutton;

    newbutton=(SDL_Button*)malloc(sizeof(SDL_Button));
    newbutton->disabled     = NULL; 
    newbutton->normal       = NULL;
    newbutton->highlighted  = NULL; 
    newbutton->pressed      = NULL; 
    newbutton->rect.x=rect->x;
    newbutton->rect.y=rect->y;
    newbutton->rect.w=rect->w;
    newbutton->rect.h=rect->h;
    newbutton->Clicked      = NULL;
    newbutton->ClickedData  = NULL;
    newbutton->state = SDL_BUTTON_UP;
    newbutton->next  = NULL;
    return newbutton;
}

void SDL_ButtonDraw(void *data,SDL_Surface *dest)
{
    
    SDL_Button  *button=(SDL_Button*)data;
    SDL_Surface *drawbutton=NULL;
    SDL_Rect src;

    if(button->normal==NULL)
    {
//        printf("SDL_Button: Can't draw without an image\n");
        SDL_FillRect(dest,&button->rect,0xfff000);
        return;
    }
    else
    {

        src.x=0;
        src.y=0;

        if(button->rect.w == 0)
            src.w=button->normal->w;
        else
            src.w=button->rect.w;

        if(button->rect.h == 0)
            src.h=button->normal->h;
        else
            src.h=button->rect.h;

    }

    switch(button->state)
    {
    case SDL_BUTTON_HIGHLIGHTED:
        drawbutton=button->highlighted;
        break;
    case SDL_BUTTON_DOWN:
        drawbutton=button->pressed;
        break;
    default:
        drawbutton=button->normal;
        break;
    }

    if(drawbutton == NULL)
        drawbutton=button->normal;

    
    if(SDL_BlitSurface(drawbutton,&src,dest,&button->rect)<0)
        fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
            
    SDL_UpdateRect(dest, button->rect.x,button->rect.y,
                         button->rect.w,button->rect.h);
}

int SDL_ButtonProperties(void *button,int feature,va_list list)
{
    SDL_Button *but=(SDL_Button*)button;

    switch(feature)
    {

    case SET_NORMAL_IMAGE:
        if(but->normal==NULL)
        {
            char *image=va_arg(list,char*);
            
            if(image)
                but->normal = IMG_Load(image);
        }
        break;
    case SET_HIGHLIGHT_IMAGE:
        if(but->highlighted==NULL)
        {
            char *image=va_arg(list,char*);
            
            if(image)
                but->highlighted=IMG_Load(image);
        }
        break;
    case SET_PRESSED_IMAGE:
        if(but->pressed==NULL)
        {
            char *image=va_arg(list,char*);

            if(image)
                but->pressed=IMG_Load(image);
        }
        break;
    case SET_DISABLED_IMAGE:
        if(but->disabled==NULL)
        {
            but->disabled=IMG_Load(va_arg(list,char*));
        }
        break;
    case SET_CALLBACK:
    {
        int event=va_arg(list,int);
        if(event== SDL_CLICKED)
        {
            but->Clicked=va_arg(list,void*);
            but->ClickedData=va_arg(list,void*);
        }
        break;
    }
    }
    return 1;
}

void SDL_ButtonEventHandler(void * button,SDL_Event *event)
{
    SDL_Button *but=(SDL_Button*)button;

    switch(event->type)
    {
    case SDL_MOUSEMOTION:
        if(SDL_WidgetIsInside(&but->rect,event->motion.x,event->motion.y))
        {
            if(but->state == SDL_BUTTON_UP)
                but->state = SDL_BUTTON_HIGHLIGHTED;
        }
        else
            but->state = SDL_BUTTON_UP;
        break;
    case SDL_MOUSEBUTTONDOWN:
        if(SDL_WidgetIsInside(&but->rect,event->motion.x,event->motion.y))
        {
            but->state = SDL_BUTTON_DOWN;
            if(but->Clicked)
                but->Clicked(but->ClickedData);
        }
        else
            but->state = SDL_BUTTON_UP;
        break;
    case SDL_MOUSEBUTTONUP:
        if(SDL_WidgetIsInside(&but->rect,event->motion.x,event->motion.y))
            but->state = SDL_BUTTON_HIGHLIGHTED;
        else
            but->state = SDL_BUTTON_UP;
        break;
    default:
        break;
    }
}


void SDL_ButtonClose(void *button)
{
    SDL_Button *Button=(SDL_Button*)button;


    free(Button);
}
