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

#include <stdlib.h>
#include <stdarg.h>

#include <SDLTk.h>


static void SDL_ButtonPaint(SDL_Widget *Widget,SDL_Surface *screen);
void SDL_ButtonMouseButtonDownCB(SDL_Widget *widget,SDL_Event *event);

const struct S_Widget_FunctionList SDL_Button_FunctionList =
{
    SDL_ButtonCreate,
    SDL_ButtonDraw,
    SDL_ButtonProperties,
    SDL_ButtonEventHandler,
    SDL_ButtonClose
};

SDL_Widget* SDL_ButtonCreate(SDL_Rect* rect)
{    
    SDL_Button *newbutton;
    SDL_Widget *widget;

    newbutton=(SDL_Button*)malloc(sizeof(SDL_Button));
    
    widget=(SDL_Widget*)newbutton;
    widget->Type            = SDL_BUTTON;
    widget->Rect.x          = rect->x;
    widget->Rect.y          = rect->y;
    widget->Rect.w          = rect->w;
    widget->Rect.h          = rect->h;
    widget->Focusable       = 0;
    
    
    SDL_SignalConnect(widget,"mousebuttondown",SDL_ButtonMouseButtonDownCB,newbutton);

    newbutton->disabled     = NULL; 
    newbutton->normal       = NULL;
    newbutton->highlighted  = NULL; 
    newbutton->pressed      = NULL; 
    newbutton->State        = SDL_BUTTON_UP;
    newbutton->Label        = NULL;
   
    return (SDL_Widget*)newbutton;
}

void SDL_ButtonDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{
    SDL_Button  *button=(SDL_Button*)widget;
    SDL_Surface *drawbutton=NULL;
    SDL_Rect src;


    if(button->Label)
    {
        if(button->State == SDL_BUTTON_DOWN)
        {
            button->Label->Rect.x = button->Widget.Rect.x + 4;
            button->Label->Rect.y = button->Widget.Rect.y + 4;
            button->Label->Rect.w = button->Widget.Rect.w - 6;
            button->Label->Rect.h = button->Widget.Rect.h - 6;

        }
        else
        {
            button->Label->Rect.x = button->Widget.Rect.x + 3;
            button->Label->Rect.y = button->Widget.Rect.y + 3;
            button->Label->Rect.w = button->Widget.Rect.w - 6;
            button->Label->Rect.h = button->Widget.Rect.h - 6;
        }
    }

    if(button->normal==NULL)
    {
        /* Draw a default button with no text inside */
        src.x=widget->Rect.x;
        src.y=widget->Rect.y;
        src.w=widget->Rect.w - 5;
        src.h=widget->Rect.h;

        //SDL_SetClipRect(dest,&src);
        SDL_ButtonPaint(widget,dest);
        //SDL_SetClipRect(dest,NULL);
        return;
    }
    else
    {
        src.x=0;
        src.y=0;

        if(widget->Rect.w == 0)
            src.w=button->normal->w;
        else
            src.w=widget->Rect.w;

        if(widget->Rect.h == 0)
            src.h=button->normal->h;
        else
            src.h=widget->Rect.h;

    }



    switch(button->State)
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
        drawbutton = button->normal;

    if(drawbutton)
        SDL_BlitSurface(drawbutton,&src,dest,&widget->Rect);

            
}

int SDL_ButtonProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Button *Button=(SDL_Button*)widget;

    switch(feature)
    {
    case SET_IMAGE:
    case SET_NORMAL_IMAGE:
        if(Button->normal==NULL)
        {
            Button->normal = va_arg(list,SDL_Surface*);
        }
        break;
    case SET_HIGHLIGHT_IMAGE:
        if(Button->highlighted==NULL)
        {
            Button->highlighted=va_arg(list,SDL_Surface*);
        }
        break;
    case SET_PRESSED_IMAGE:
        if(Button->pressed==NULL)
        {
            Button->pressed=va_arg(list,SDL_Surface*);
        }
        break;
    case SET_DISABLED_IMAGE:
        if(Button->disabled==NULL)
        {
            Button->disabled=va_arg(list,SDL_Surface*);
        }
        break;
    }
    return 1;
}

void SDL_ButtonMouseButtonDownCB(SDL_Widget *widget,SDL_Event *event)
{
    SDL_Button *Button=(SDL_Button*)widget;
    if(SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
    {
        Button->State = SDL_BUTTON_DOWN;
        SDL_SignalEmit(widget,"clicked");
        SDL_WidgetDraw(widget,&widget->Rect);
    }
    else
        Button->State = SDL_BUTTON_UP;        
}

int SDL_ButtonEventHandler(SDL_Widget *widget,SDL_Event *event)
{
    SDL_Button *Button=(SDL_Button*)widget;

    switch(event->type)
    {
    case SDL_MOUSEMOTION:
        if(SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
        {
            if(Button->State == SDL_BUTTON_UP)
            {
                Button->State = SDL_BUTTON_HIGHLIGHTED;
                if(Button->highlighted != NULL)
                {
                    SDL_WidgetDraw(widget,&widget->Rect);
                }
            }
        }
        else
            Button->State = SDL_BUTTON_UP;
        
        break;
    case SDL_MOUSEBUTTONUP:
        if(SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
        {
            Button->State = SDL_BUTTON_HIGHLIGHTED;
            if(Button->highlighted != NULL)
            {
                SDL_WidgetDraw(widget,&widget->Rect);
            }
        }
        else
            Button->State = SDL_BUTTON_UP;
        break;
    default:
        break;
    }
    return 0;
}


void SDL_ButtonClose(SDL_Widget *widget)
{
    SDL_Button *Button=(SDL_Button*)widget;
    free(Button);
}



static void SDL_ButtonPaint(SDL_Widget *Widget,SDL_Surface *screen)
{
    SDL_Button *Button=(SDL_Button*)Widget;
    SDL_Rect r;
    int x,y,width,height;
    r.x = Widget->Rect.x;
    r.y = Widget->Rect.y; 
    r.w = Widget->Rect.w;
    r.h = Widget->Rect.h;
    
    x=r.x;
    y=r.y;
    width=r.w;
    height=r.h;
    

    SDL_FillRect(screen,&r,0xd4d0c8);

     
//    if(Button->state == SDL_BUTTON_UP)
    {
        // normal
        SDL_DrawLine(screen,x, y + height - 1, x + width - 1, y + height - 1,0x404040ff);
        SDL_DrawLine (screen,x + width - 1, y, x + width - 1, y + height - 1,0x404040ff);

        SDL_DrawLine (screen,x + 1, y + height - 2, x + width - 2, y + height - 2,0x808080ff);
        SDL_DrawLine (screen,x + width - 2, y + 1, x + width - 2, y + height - 2,0x808080ff);

        SDL_DrawLine (screen,x + 1, y + 1, x + width - 2, y + 1,0xffffffff);
        SDL_DrawLine (screen,x + 1, y + 1, x + 1, y + height - 2,0xffffffff);

        SDL_DrawLine (screen,x, y, x + width - 1, y,0xd4d0c8ff);
        SDL_DrawLine (screen,x, y, x, y + height - 1,0xd4d0c8ff);       
    }
    
    if(Button->State == SDL_BUTTON_DOWN)
    {
        // pressed
        SDL_DrawLine (screen,x, y + height - 1, x + width - 1, y + height - 1, 0xffffffff);
        SDL_DrawLine (screen,x + width - 1, y, x + width - 1, y + height - 1, 0xffffffff);

        SDL_DrawLine (screen,x + 1, y + height - 2, x + width - 2, y + height - 2, 0xd4d0c8ff);
        SDL_DrawLine (screen,x + width - 2, y + 1, x + width - 2, y + height - 2, 0xd4d0c8ff);
      
        SDL_DrawLine (screen,x + 1, y + 1, x + width - 2, y + 1, 0x808080ff);
        SDL_DrawLine (screen,x + 1, y + 1, x + 1, y + height - 2, 0x808080ff);
      
        SDL_DrawLine (screen,x, y, x + width - 1, y, 0x404040ff);
        SDL_DrawLine (screen,x, y, x, y + height - 1, 0x404040ff);

    }
}

void SDL_ButtonSetLabel(SDL_Widget *widget,char *title)
{
    SDL_Button *Button=(SDL_Button*)widget;
    
    if(Button->Label == NULL)
    {
        Button->Label=SDL_WidgetCreate(SDL_LABEL,widget->Rect.x,widget->Rect.y,
                                       widget->Rect.w,widget->Rect.h);
        SDL_WidgetShow(Button->Label);
    }

    SDL_LabelSetText(Button->Label,title);
}
