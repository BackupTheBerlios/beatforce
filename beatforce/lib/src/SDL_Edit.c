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
#include <stdarg.h>
#include <malloc.h>
#include <string.h>

#include <SDLTk.h>

void SDL_EditKeyDownCB(SDL_Widget *widget,SDL_Event *event);

const struct S_Widget_FunctionList SDL_Edit_FunctionList =
{
    SDL_EditCreate,
    SDL_EditDraw,
    SDL_EditProperties,
    NULL,
    NULL,
};


SDL_Widget *SDL_EditCreate(SDL_Rect* rect)
{
    SDL_Widget *Widget;
    SDL_Edit   *Edit;


    Edit=(SDL_Edit*)malloc(sizeof(SDL_Edit));
    Widget=(SDL_Widget*)Edit;

    Widget->Type      = SDL_EDIT;
    Widget->Rect.x    = rect->x;
    Widget->Rect.y    = rect->y;
    Widget->Rect.w    = rect->w;
    Widget->Rect.h    = rect->h;
    Widget->Focusable = 1;
    
    
    SDL_SignalConnect(Widget,"keydown",SDL_EditKeyDownCB,Widget);
    
    Edit->Caption = (char*)malloc(1024);
    memset(Edit->Caption,0,1024);

    Edit->Font    = &DefaultFont;

    Edit->CursorPosition = 0;
    Edit->bgcolor = 0xfffff7;
    Edit->fgcolor = 0x000000;


    return (SDL_Widget*)Edit;
}

void  SDL_EditDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{
    SDL_Edit *Edit=(SDL_Edit*)widget;
    SDL_Rect cursor;
    SDL_Rect StringPos;
    int StringWidth;
    
    if(Edit->Font == NULL)
    {
        return;
    }

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
    StringPos.x = widget->Rect.x + 2;
    StringPos.w = widget->Rect.w - 2;
    StringPos.h = widget->Rect.h;
    
    StringWidth=SDL_FontGetStringWidth(Edit->Font,Edit->Caption);
    
    {
        char tmp[255];
        unsigned int size;
        int size2;

        size=strlen(Edit->Caption);
        {
            size = size - (size - Edit->CursorPosition);
            if(size >= 0)
            {
                int w;
                char *caption;

                memset(tmp,0,255);
                strncpy(tmp,Edit->Caption,size);
                tmp[size]=0;

                w = SDL_FontGetStringWidth(Edit->Font,tmp);

                caption=(char*)&tmp;
                while(w > widget->Rect.w)
                {
                    w=SDL_FontGetStringWidth(Edit->Font,caption);
                    caption++;
                }
                SDL_FontDrawStringRect(dest,Edit->Font,caption,Edit->fgcolor,&StringPos);

                if(SDL_WidgetHasFocus(widget))
                {
                    StringWidth = SDL_FontGetStringWidth(Edit->Font,caption);
                    if(StringWidth > widget->Rect.w)
                        cursor.x = widget->Rect.x + widget->Rect.w - 4;
                    else
                        cursor.x = widget->Rect.x + SDL_FontGetStringWidth(Edit->Font,caption)+3;
                    cursor.y = StringPos.y;
                    cursor.w = 1;
                    cursor.h = SDL_FontGetHeight(Edit->Font);
                    
                    SDL_DrawLine(dest,cursor.x,cursor.y,cursor.x,cursor.y+cursor.h,0x000007ff);
                }

                /* Draw the part behind the cursor */
                size2=strlen(Edit->Caption) - size;
                if(size2 > 0)
                {
                    memset(tmp,0,255);
                    strncpy(tmp,Edit->Caption+size,size2);
                    StringPos.x = cursor.x + 1;
                    SDL_FontDrawStringRect(dest,Edit->Font,tmp,Edit->fgcolor,&StringPos);
                }
            }

        }
    }
    
   
}

int SDL_EditProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Edit *Edit=(SDL_Edit*)widget;

    switch(feature)
    {
    case SET_CAPTION:
        sprintf(Edit->Caption,"%s",va_arg(list,char*));
        break;
    case SET_FG_COLOR:
        Edit->fgcolor = va_arg(list,Uint32);
        break;
    case SET_BG_COLOR:
        Edit->bgcolor = va_arg(list,Uint32);
        break;
    default:
        return 0;
    }
    return 1;
}

void SDL_EditKeyDownCB(SDL_Widget *widget,SDL_Event *event)
{
    SDL_Edit *Edit=(SDL_Edit*)widget;
    SDLMod mod;
    int key;

    if(!SDL_WidgetHasFocus(widget))
        return;

    mod = SDL_GetModState();

    switch(event->key.keysym.sym)
    {
        case SDLK_DELETE:
            {
                if(Edit->CursorPosition >= 0)
                {
                    strcpy(Edit->Caption+Edit->CursorPosition,Edit->Caption+Edit->CursorPosition+1);
                    SDL_WidgetRedrawEvent(widget);
                    SDL_SignalEmit(widget,"changed");
                }
            }
            break;
        case SDLK_BACKSPACE:
            {
                if(Edit->CursorPosition > 0)
                {
                    Edit->CursorPosition--;
                    strcpy(Edit->Caption+Edit->CursorPosition,Edit->Caption+Edit->CursorPosition+1);
                    SDL_WidgetRedrawEvent(widget);
                    SDL_SignalEmit(widget,"changed");
                }
            }
            break;
        case SDLK_HOME:
            Edit->CursorPosition = 0; 
            SDL_WidgetRedrawEvent(widget);
            break;
        case SDLK_END:
            Edit->CursorPosition = strlen(Edit->Caption)+1; 
            SDL_WidgetRedrawEvent(widget);
            break;
         case SDLK_LEFT:
             if(Edit->CursorPosition > 0)
                Edit->CursorPosition--;
             SDL_WidgetRedrawEvent(widget);
             break;
         case SDLK_RIGHT:
             if(Edit->CursorPosition < strlen(Edit->Caption))
                 Edit->CursorPosition++;
             SDL_WidgetRedrawEvent(widget);
             break;
         case SDLK_RETURN:
             if(SDL_WidgetHasFocus(widget))
                 SDL_SignalEmit(widget,"activate");
             break;
         default:
            key=event->key.keysym.sym;
            if ((key >= 0x20) && (key <= 0xFF))
            {
                if(mod & KMOD_SHIFT)
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
                {   
                    char tmp[255];
                    memset(tmp,0,255);
                    sprintf(tmp,"%c%s",key,Edit->Caption+Edit->CursorPosition);
                    Edit->Caption[Edit->CursorPosition+1]=0;
                    sprintf(Edit->Caption,"%s%s",Edit->Caption,tmp);
                }
                Edit->CursorPosition++;
                SDL_WidgetRedrawEvent(widget);
                SDL_SignalEmit(widget,"changed");
            }
            break;
    }
}




char *SDL_EditGetText(SDL_Widget *widget)
{
    SDL_Edit *Edit=(SDL_Edit*)widget;
    return Edit->Caption;
}

int SDL_EditSetFont(SDL_Widget *widget,SDL_Font *font)
{
    SDL_Edit *Edit=(SDL_Edit*)widget;
    
    if(font == NULL)
        return 0;
    
    Edit->Font = font;
    
    return 1;
}

void SDL_EditSetText(SDL_Widget *widget,const char *text)
{
    SDL_Edit *Edit=(SDL_Edit*)widget;
    strcpy(Edit->Caption,text);
    Edit->CursorPosition=strlen(Edit->Caption)+1;

}
