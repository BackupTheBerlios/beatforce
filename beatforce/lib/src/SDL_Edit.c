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


void SDL_EditSetCallback(void *widget,int event,void *function,void *data);
void SDL_EditCallback(void *widget,int event);


void* SDL_EditCreate(SDL_Rect* rect)
{
    SDL_Edit *edit;

    edit=(SDL_Edit*)malloc(sizeof(SDL_Edit));

    edit->rect.x  = rect->x;
    edit->rect.y  = rect->y;
    edit->rect.w  = rect->w;
    edit->rect.h  = rect->h;

    edit->Caption = (char*)malloc(1024);
    memset(edit->Caption,0,1024);

    edit->Font    = NULL;
    edit->Redraw  = 1;
    edit->Shift   = 0;

    edit->Focus   = 0;
    edit->bgcolor = 0xfffff7;
    edit->fgcolor = 0x000000;

    /* Reset the callback functions */
    edit->AnyKeyPressCallback = NULL;
    edit->ReturnPressCallback = NULL;
    edit->AnyKeyData    = NULL;
    edit->ReturnKeyData = NULL;

    return edit;
}

void  SDL_EditDraw(void *edit,SDL_Surface *dest)
{
    SDL_Edit *Edit=(SDL_Edit*)edit;
    SDL_Rect cursor;
    SDL_Rect StringPos;
    int StringWidth;
    char *caption;
    
    if(Edit->Font == NULL)
    {
        printf("SDL_Edit ERROR: Can't draw edit without Font set\n");
        return;
    }


    SDL_FontSetColor(Edit->Font,Edit->fgcolor);
    
    SDL_FillRect(dest,&Edit->rect,Edit->bgcolor);
    
    StringPos.y = Edit->rect.y + ((Edit->rect.h - Edit->Font->height)/2);
    StringPos.x = Edit->rect.x;
    StringPos.w = Edit->rect.w;
    StringPos.h = Edit->rect.h;
    
    StringWidth=SDL_FontGetStringWidth(Edit->Font,Edit->Caption);
    
    if(StringWidth <= Edit->rect.w )
    {
        SDL_FontDrawStringRect(dest,Edit->Font,Edit->Caption,&StringPos);
    }
    else
    {
        caption=Edit->Caption;
        while(SDL_FontGetStringWidth(Edit->Font,caption) > Edit->rect.w)
            caption++;
        SDL_FontDrawString(dest,Edit->Font,caption,Edit->rect.x,StringPos.y);
    }
    
    /* draw cursor */
    
    if(SDL_WidgetHasFocus(edit) || Edit->Focus)
    {
        StringWidth=SDL_FontGetStringWidth(Edit->Font,Edit->Caption);
        if(StringWidth > Edit->rect.w)
            cursor.x = Edit->rect.x + Edit->rect.w - 2;
        else
            cursor.x = Edit->rect.x + SDL_FontGetStringWidth(Edit->Font,Edit->Caption)+2;
        cursor.y = StringPos.y;
        cursor.w = 1;
            cursor.h = Edit->Font->height;
            SDL_FillRect(dest,&cursor,0x000007);
    }
    
}

int SDL_EditProperties(void *edit,int feature,va_list list)
{
    SDL_Edit *Edit=(SDL_Edit*)edit;

    switch(feature)
    {
    case SET_FONT:
        Edit->Font   = va_arg(list,SDL_Font*);
        break;
    case SET_CAPTION:
        sprintf(Edit->Caption,"%s",va_arg(list,char*));
        Edit->Redraw = 1;
        break;
    case SET_FG_COLOR:
        Edit->fgcolor = va_arg(list,Uint32);
        Edit->Redraw  = 1;
        break;
    case SET_BG_COLOR:
        Edit->bgcolor = va_arg(list,Uint32);
        Edit->Redraw = 1;
        break;
    case SET_CALLBACK:
    {
        int event=va_arg(list,int);
        void *p    = va_arg(list,void*);
        void *data = va_arg(list,void*);
        SDL_EditSetCallback(edit,event,p,data);
        break;
    }
    case GET_CAPTION:
        sprintf(va_arg(list,char*),"%s",Edit->Caption);
        break;
    case SET_ALWAYS_FOCUS:
        Edit->Focus = va_arg(list,int);
        break;
    case FORCE_REDRAW:
        Edit->Redraw = 1;
        break;
    }
    return 1;
}

void SDL_EditEventHandler(void *edit,SDL_Event *event)
{
    SDL_Edit *Edit=(SDL_Edit*)edit;
    char key;

    if(!SDL_WidgetHasFocus(edit) && !Edit->Focus)
        return;
    
    switch(event->type) 
    {
    case SDL_KEYDOWN:
        switch( event->key.keysym.sym ) 
        {
        case SDLK_BACKSPACE:
            Edit->Caption[strlen(Edit->Caption)-1]='\0';
            Edit->Redraw=1;
            if(Edit->AnyKeyPressCallback)
                Edit->AnyKeyPressCallback(Edit->AnyKeyData);

            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            Edit->Shift=1;
            break;
        case SDLK_RETURN:
            if(Edit->ReturnPressCallback)
                Edit->ReturnPressCallback(Edit->ReturnKeyData);

            break;
        default:
            key=event->key.keysym.sym;
            if(Edit->Shift)
            {
                if(key>=SDLK_a && key <= SDLK_z)
                {
                    key -= 32;
                }
                if(key == SDLK_SEMICOLON)
                {
                    key=SDLK_COLON;
                }
            }
            sprintf(Edit->Caption,"%s%c",Edit->Caption,key);
            Edit->Redraw=1;
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
}

void SDL_EditSetCallback(void *edit,int event,void *function,void *data)
{
    SDL_Edit *Edit=(SDL_Edit*)edit;

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

void SDL_EditCallback(void *edit,int event)
{
    SDL_Edit *Edit=(SDL_Edit*)edit;
    if(event ==  SDL_KEYDOWN_RETURN)
    {
        Edit->ReturnPressCallback(NULL);
    }
}
