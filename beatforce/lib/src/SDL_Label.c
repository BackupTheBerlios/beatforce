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
    SDL_LabelEventHandler,
    NULL,
};


void* SDL_LabelCreate(SDL_Rect* rect)
{
    SDL_Label *label;

    label=(SDL_Label*)malloc(sizeof(SDL_Label));

    label->rect.x  = rect->x;
    label->rect.y  = rect->y;
    label->rect.w  = rect->w;
    label->rect.h  = rect->h;

    label->Caption = NULL;
    label->Font    = NULL;

    label->fgcolor = 0x000000;
    label->bgcolor = TRANSPARANT;
    label->offset   = 0;
    label->increase = 1;

    label->Pattern    = LABEL_BOUNCE;
    label->Background = NULL;

    return label;
}

void SDL_LabelDraw(void *label,SDL_Surface *dest)
{
    SDL_Label *Label=(SDL_Label*)label;
    char string[255];
    SDL_Rect DrawPosititon;

    memset(string ,0,255);

    SDL_FontSetColor(Label->Font,Label->fgcolor);
    
    if(Label->bgcolor == TRANSPARANT)
    {
        if(Label->Background == NULL)
        {
            Label->Background = SDL_WidgetGetBackground(dest,&Label->rect);
        }
        if(SDL_BlitSurface(Label->Background,NULL,dest,&Label->rect)<0)
            fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
    }
    else
    {
        SDL_FillRect(dest,&Label->rect,Label->bgcolor);
    }

    if(Label->Caption)
    {
        Label_CalculatePattern(Label,&DrawPosititon);
        
        SDL_FontDrawStringLimited(dest,Label->Font,Label->Caption,&DrawPosititon,&Label->rect);
    }
   
}

int SDL_LabelProperties(void *label,int feature,va_list list)
{
    SDL_Label *Label=(SDL_Label*)label;

    switch(feature)
    {
    case SET_FONT:
        Label->Font=va_arg(list,SDL_Font*);
        break;
    case SET_CAPTION:
        if(Label->Caption != NULL)
            free(Label->Caption);
        
        Label->Caption=(char*)strdup(va_arg(list,char*));
        break;
    case SET_FG_COLOR:
        Label->fgcolor=va_arg(list,Uint32);
        break;

    case SET_BG_COLOR:
        Label->bgcolor=va_arg(list,Uint32);
        break;

    }
    return 1;
}

void SDL_LabelEventHandler(void *label,SDL_Event *event)
{


}


static void Label_CalculatePattern(SDL_Label *Label,SDL_Rect *Rect)
{
    int StringWidth;

    /* Calculate the total size of the string in pixels */
    StringWidth=SDL_FontGetStringWidth(Label->Font,Label->Caption);

    if(StringWidth > Label->rect.w)
    {
        switch(Label->Pattern)
        {
        case LABEL_NORMAL:
            Rect->x = Label->rect.x;
            Rect->y = Label->rect.y;
            Rect->w = Label->rect.w;
            Rect->h = Label->rect.h;
            break;
        case LABEL_BOUNCE:
            Rect->x = Label->rect.x-Label->offset;
            Rect->y = Label->rect.y;
            Rect->w = Label->rect.w+Label->offset;
            Rect->h = Label->rect.h;
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
            Rect->x = Label->rect.x-Label->offset;
            Rect->y = Label->rect.y;
            Rect->w = Label->rect.w+Label->offset;
            Rect->h = Label->rect.h;
            Label->offset++;
            if(StringWidth + Label->rect.w < Label->rect.w + Label->offset)
            Label->offset = -Label->rect.w;
            break;
        case LABEL_SCROLL_RIGHT:
            Rect->x = Label->rect.x - Label->offset;
            Rect->y = Label->rect.y;
            Rect->w = Label->rect.w + Label->offset;
            Rect->h = Label->rect.h;
            Label->offset--;
            if(Label->offset + Label->rect.w < 0)
                Label->offset = StringWidth;
            break;
        }
    }
    else
    {
        Rect->x = Label->rect.x;
        Rect->y = Label->rect.y;
        Rect->w = Label->rect.w;
        Rect->h = Label->rect.h;
    }
    

}
