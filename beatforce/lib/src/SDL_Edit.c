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
#include <stdarg.h>
#include <malloc.h>

#include "SDL_Widget.h"
#include "SDL_Edit.h"

const struct S_Widget_FunctionList SDL_Edit_FunctionList =
{
    SDL_EditCreate,
    SDL_EditDraw,
    SDL_EditProperties,
    SDL_EditEventHandler,
    NULL,
};


static void SDL_EditSetCallback(SDL_Widget *widget,int event,void *function,void *data);
void SDL_EditCallback(SDL_Widget *widget,int event);


SDL_Widget *SDL_EditCreate(SDL_Rect* rect)
{
    SDL_Widget *Widget;
    SDL_Edit   *Edit;


    Edit=(SDL_Edit*)malloc(sizeof(SDL_Edit));
    Widget=(SDL_Widget*)Edit;

    Widget->Type    = SDL_EDIT;
    Widget->Rect.x  = rect->x;
    Widget->Rect.y  = rect->y;
    Widget->Rect.w  = rect->w;
    Widget->Rect.h  = rect->h;

    Edit->Caption = (char*)malloc(1024);
    memset(Edit->Caption,0,1024);

    Edit->Font    = NULL;
    Edit->Shift   = 0;

    Edit->bgcolor = 0xfffff7;
    Edit->fgcolor = 0x000000;

    /* Reset the callback functions */
    Edit->AnyKeyPressCallback = NULL;
    Edit->ReturnPressCallback = NULL;
    Edit->AnyKeyData    = NULL;
    Edit->ReturnKeyData = NULL;

    return (SDL_Widget*)Edit;
}

void  SDL_EditDraw(SDL_Widget *widget,SDL_Surface *dest)
{
    SDL_Edit *Edit=(SDL_Edit*)widget;
    SDL_Rect cursor;
    SDL_Rect StringPos;
    int StringWidth;
    char *caption;
    
    if(Edit->Font == NULL)
    {
        return;
    }


    SDL_FontSetColor(Edit->Font,Edit->fgcolor);
    
    SDL_FillRect(dest,&widget->Rect,BLACK);
    {
        SDL_Rect r;
        r.x = widget->Rect.x+1;
        r.y = widget->Rect.y+1;
        r.w = widget->Rect.w-2;
        r.h = widget->Rect.h-2;

        SDL_FillRect(dest,&r,Edit->bgcolor);
    }
    
    StringPos.y = widget->Rect.y + ((widget->Rect.h - SDL_FontGetHeight(Edit->Font))/2);
    StringPos.x = widget->Rect.x;
    StringPos.w = widget->Rect.w;
    StringPos.h = widget->Rect.h;
    
    StringWidth=SDL_FontGetStringWidth(Edit->Font,Edit->Caption);
    
    if(StringWidth <= widget->Rect.w )
    {
        SDL_FontDrawStringRect(dest,Edit->Font,Edit->Caption,&StringPos);
    }
    else
    {
        caption=Edit->Caption;

        while(SDL_FontGetStringWidth(Edit->Font,caption) > widget->Rect.w)
            caption++;

        SDL_FontDrawStringRect(dest,Edit->Font,caption,&StringPos);
    }
    
    /* draw cursor */
    
    if(SDL_WidgetHasFocus(widget))
    {
        StringWidth=SDL_FontGetStringWidth(Edit->Font,Edit->Caption);
        if(StringWidth > widget->Rect.w)
            cursor.x = widget->Rect.x + widget->Rect.w - 2;
        else
            cursor.x = widget->Rect.x + SDL_FontGetStringWidth(Edit->Font,Edit->Caption)+2;
        cursor.y = StringPos.y;
        cursor.w = 1;
        cursor.h = SDL_FontGetHeight(Edit->Font);
        SDL_FillRect(dest,&cursor,0x000007);
    }
    
}

int SDL_EditProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Edit *Edit=(SDL_Edit*)widget;

    switch(feature)
    {
    case SET_FONT:
        Edit->Font   = va_arg(list,SDL_Font*);
        break;
    case SET_CAPTION:
        sprintf(Edit->Caption,"%s",va_arg(list,char*));
        break;
    case SET_FG_COLOR:
        Edit->fgcolor = va_arg(list,Uint32);
        break;
    case SET_BG_COLOR:
        Edit->bgcolor = va_arg(list,Uint32);
        break;
    case SET_CALLBACK:
    {
        int event=va_arg(list,int);
        void *p    = va_arg(list,void*);
        void *data = va_arg(list,void*);
        SDL_EditSetCallback(widget,event,p,data);
        break;
    }
    case GET_CAPTION:
        sprintf(va_arg(list,char*),"%s",Edit->Caption);
        break;
    }
    return 1;
}

int SDL_EditEventHandler(SDL_Widget *widget,SDL_Event *event)
{
    int retval = 0;
    SDL_Edit *Edit=(SDL_Edit*)widget;
    char key;

    if(!SDL_WidgetHasFocus(widget))
        return 0;
    
    switch(event->type) 
    {
    case SDL_KEYDOWN:
        switch( event->key.keysym.sym ) 
        {
        case SDLK_BACKSPACE:
            Edit->Caption[strlen(Edit->Caption)-1]='\0';
            if(Edit->AnyKeyPressCallback)
                Edit->AnyKeyPressCallback(Edit->AnyKeyData);

            break;
        case SDLK_ESCAPE:
            SDL_WidgetLoseFocus();
            retval=1;
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            Edit->Shift=1;
            break;
        case SDLK_RETURN:
            if(Edit->ReturnPressCallback)
                Edit->ReturnPressCallback(Edit->ReturnKeyData);
            
            retval=1;
            SDL_WidgetLoseFocus();
            break;
        default:
            key=event->key.keysym.sym;
            if(Edit->Shift)
            {
                if(key>=SDLK_a && key <= SDLK_z)
                {
                    retval = 1;
                    key -= 32;
                }
                if(key == SDLK_SEMICOLON)
                {
                    key=SDLK_COLON;
                }
            }
            sprintf(Edit->Caption,"%s%c",Edit->Caption,key);
            if(Edit->AnyKeyPressCallback)
                Edit->AnyKeyPressCallback();
            break;
        }
        break;
    case SDL_KEYUP:
        switch( event->key.keysym.sym ) 
        {
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            Edit->Shift=0;
            break;
        default:
            break;
        }
        break;
        
    }
    return retval;
}

static void SDL_EditSetCallback(SDL_Widget *widget,int event,void *function,void *data)
{
    SDL_Edit *Edit=(SDL_Edit*)widget;

    if(event ==  SDL_KEYDOWN_RETURN)
    {
        Edit->ReturnPressCallback = function;
        Edit->ReturnKeyData = data;
    }
    else if(event == SDL_KEYDOWN_ANY)
    {
        Edit->AnyKeyPressCallback=function;
        Edit->AnyKeyData = data;
    }

}

void SDL_EditCallback(SDL_Widget *widget,int event)
{
    SDL_Edit *Edit=(SDL_Edit*)widget;
    if(event ==  SDL_KEYDOWN_RETURN)
    {
        Edit->ReturnPressCallback(NULL);
    }
}

