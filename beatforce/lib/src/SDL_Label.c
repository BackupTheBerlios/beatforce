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
#include <string.h>

#include "SDL_Widget.h"
#include "SDL_WidTool.h"
#include "SDL_Label.h"

static void Label_CalculatePattern(SDL_Label *Label,SDL_Rect *Rect);

const struct S_Widget_FunctionList SDL_Label_FunctionList =
{
    SDL_LabelCreate,
    SDL_LabelDraw,
    SDL_LabelProperties,
    NULL,
    NULL,
};


SDL_Widget* SDL_LabelCreate(SDL_Rect* rect)
{
    SDL_Widget *Widget;
    SDL_Label  *Label;

    Label=(SDL_Label*)malloc(sizeof(SDL_Label));
    
    Widget=(SDL_Widget*)Label;
    Widget->Type      = SDL_LABEL;
    Widget->Rect.x    = rect->x;
    Widget->Rect.y    = rect->y;
    Widget->Rect.w    = rect->w;
    Widget->Rect.h    = rect->h;
    Widget->Focusable = 0;

    Label->Caption = NULL;
    Label->Font    = NULL;

    Label->fgcolor = 0x000000;
    Label->bgcolor = TRANSPARANT;
    Label->offset   = 0;
    Label->increase = 1;

    Label->Pattern    = LABEL_BOUNCE;


    Label->Visible    = 1;
    return (SDL_Widget*)Label;
}

void SDL_LabelDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{
    SDL_Label *Label=(SDL_Label*)widget;
    char string[255];
    SDL_Rect DrawPosititon;
    
    if(Label->Visible == 0)
        return;

    memset(string ,0,255);

    SDL_FontSetColor(Label->Font,Label->fgcolor);
    
    if(Label->bgcolor != TRANSPARANT)
    {
        SDL_FillRect(dest,&widget->Rect,Label->bgcolor);
    }

    if(Label->Caption)
    {
        Label_CalculatePattern(Label,&DrawPosititon);
        
        SDL_FontDrawStringLimited(dest,Label->Font,Label->Caption,&DrawPosititon,&widget->Rect);
    }
   
}

int SDL_LabelProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Label *Label=(SDL_Label*)widget;

    switch(feature)
    {
    case SET_FONT:
        Label->Font=va_arg(list,SDL_Font*);
        break;
    case SET_CAPTION:
        if(Label->Caption != NULL)
            free(Label->Caption);
        
        Label->Caption=(char*)strdup(va_arg(list,char*));
        SDL_WidgetRedrawEvent(widget);
        break;
    case SET_FG_COLOR:
        Label->fgcolor=va_arg(list,Uint32);
        break;

    case SET_BG_COLOR:
        Label->bgcolor=va_arg(list,Uint32);
        break;

    case SET_VISIBLE:
        Label->Visible=va_arg(list,int);

    }
    return 1;
}


static void Label_CalculatePattern(SDL_Label *Label,SDL_Rect *Rect)
{
    SDL_Widget *widget=(SDL_Widget*)Label;
    int StringWidth;

    /* Calculate the total size of the string in pixels */
    StringWidth=SDL_FontGetStringWidth(Label->Font,Label->Caption);

    if(StringWidth > widget->Rect.w)
    {
        switch(Label->Pattern)
        {
        case LABEL_NORMAL:
            Rect->x = widget->Rect.x;
            Rect->y = widget->Rect.y;
            Rect->w = widget->Rect.w;
            Rect->h = widget->Rect.h;
            break;
        case LABEL_BOUNCE:
            Rect->x = widget->Rect.x - Label->offset;
            Rect->y = widget->Rect.y;
            Rect->w = widget->Rect.w + Label->offset;
            Rect->h = widget->Rect.h;
            if(Label->increase == 1)
                Label->offset++;
            else 
                Label->offset--;
            if(StringWidth < Rect->w)
                Label->increase = 0;
            
            if(Label->offset == -1)
                Label->increase=1;
            break;
        case LABEL_SCROLL_LEFT:
            Rect->x = widget->Rect.x - Label->offset;
            Rect->y = widget->Rect.y;
            Rect->w = widget->Rect.w + Label->offset;
            Rect->h = widget->Rect.h;
            Label->offset++;
            if(StringWidth + widget->Rect.w < widget->Rect.w + Label->offset)
                Label->offset = -widget->Rect.w;
            break;
        case LABEL_SCROLL_RIGHT:
            Rect->x = widget->Rect.x - Label->offset;
            Rect->y = widget->Rect.y;
            Rect->w = widget->Rect.w + Label->offset;
            Rect->h = widget->Rect.h;
            Label->offset--;
            if(Label->offset + widget->Rect.w < 0)
                Label->offset = StringWidth;
            break;
        }
    }
    else
    {
        Rect->x = widget->Rect.x;
        Rect->y = widget->Rect.y;
        Rect->w = widget->Rect.w;
        Rect->h = widget->Rect.h;
    }
    

}

