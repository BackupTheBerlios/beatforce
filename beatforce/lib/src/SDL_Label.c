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
    label->Redraw  = 1;

    label->fgcolor = 0x000000;
    label->bgcolor = TRANSPARANT;
    
    label->Background = NULL;

    return label;
}

void SDL_LabelDraw(void *label,SDL_Surface *dest)
{
    SDL_Label *Label=(SDL_Label*)label;
    int StringWidth;
    char string[255];

    int counter=0;

    memset(string ,0,255);


    if(Label->Redraw || SDL_WidgetNeedsRedraw())
    {
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

        StringWidth=SDL_FontGetStringWidth(Label->Font,Label->Caption);

        if(Label->Caption)
            counter=strlen(Label->Caption);

        strncpy(string,Label->Caption,counter);

        while(StringWidth > Label->rect.w && counter) 
        {
            StringWidth=SDL_FontGetStringWidth(Label->Font,string);
            memset(string,0,255);
            strncpy(string,Label->Caption,counter--);
        }

        SDL_FontDrawStringRect(dest,Label->Font,string,&Label->rect);
        
        SDL_UpdateRect(dest,
                       Label->rect.x,
                       Label->rect.y,
                       Label->rect.w,
                       Label->rect.h);

        Label->Redraw = 0;
    }
}

void  SDL_LabelProperties(void *label,int feature,va_list list)
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
        Label->Redraw = 1;
        break;
    case SET_FG_COLOR:
        Label->fgcolor=va_arg(list,Uint32);
        break;

    case SET_BG_COLOR:
        Label->bgcolor=va_arg(list,Uint32);
        break;

    }
}

void SDL_LabelEventHandler(void *label,SDL_Event *event)
{


}

