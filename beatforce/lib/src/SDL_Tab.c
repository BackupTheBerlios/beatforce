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
#include <stdlib.h>
#include <string.h>

#include "SDL_Widget.h"
#include "SDL_Tab.h"

#define TAB_MAXWIDTH         50
#define TAB_DEFAULTWIDTH     50
#define TAB_DEFAULTHEIGHT    16
#define TAB_LINE_WIDTH       1
#define TAB_OVERLAY          6

static void AddTab(SDL_Tab *tab,char *string);
//static SDL_TabImage *SDL_CreateDefaultTab(SDL_Tab *tab, int hl);
int Tab_DrawTabWithCaption(SDL_Surface *dest,SDL_Tab *tab,SDL_TabList * tl, int high);
int FitsInWiget(SDL_Tab *tab,SDL_TabList *tl);
int Tab_DrawArrows(SDL_Surface *dest,SDL_Tab *tab);
void drawtab(SDL_Surface *dest, int xoffset, int yoffset, int highlighted);
void SDL_TabRecalculate(SDL_Tab *tab);

/* Prototypes for editwidget */
void Tab_EditAnyKeyPressed();
void Tab_EditReturnKeyPressed();

const struct S_Widget_FunctionList SDL_Tab_FunctionList =
{
    SDL_TabCreate,
    SDL_TabDraw,
    SDL_TabProperties,
    SDL_TabEventHandler,
    NULL
};



void *SDL_TabCreate(SDL_Rect *rect)
{
    SDL_Tab *newtab;

    newtab=(SDL_Tab*)malloc(sizeof(SDL_Tab));
    newtab->NoOfTabs=1;
    newtab->rect=(SDL_Rect*)malloc(sizeof(SDL_Rect));
    newtab->rect->x    = rect->x;
    newtab->rect->y    = rect->y;
    newtab->rect->w    = rect->w;
    newtab->rect->h    = rect->h;
    newtab->min_width  = TAB_DEFAULTWIDTH;
    newtab->min_height = TAB_DEFAULTHEIGHT;
    newtab->tabs  = NULL;
    newtab->next  = NULL;
    newtab->bgcolor     = 0xff0000;
    newtab->fgcolor     = WHITE;
    newtab->normal      = NULL;
    newtab->highlighted = NULL;
    newtab->startx      = 0;
    newtab->doesntfit   = 0;
    newtab->overlay     = TAB_OVERLAY;
    newtab->edit        = NULL;
    newtab->OnClicked   = NULL;
    newtab->OnReturn    = NULL;
    return newtab;

}

void SDL_TabDraw(void *data,SDL_Surface *dest)
{
    SDL_Tab     *tab=(SDL_Tab*)data;
    SDL_TabList *tablist;
    //SDL_Surface *drawtab=NULL;


    if(tab->tabs==NULL)
    {
        SDL_FillRect(dest, tab->rect,tab->bgcolor);
        return;
    }
    else
    {
        tablist=tab->tabs;
    }

  
    SDL_TabRecalculate(tab); // we can only run this with a valid surface
    SDL_FillRect(dest,tab->rect,0xff000);        
    while(tablist)
    {
        Tab_DrawTabWithCaption(dest,tab,tablist,0);
        tablist=tablist->next;
    }
    Tab_DrawTabWithCaption(dest,tab,tab->hl,1);
    if(tab->edit)
    {
        SDL_WidgetPropertiesOf(tab->edit,FORCE_REDRAW,1);
    }
}

void SDL_TabProperties(void *tab,int feature,va_list list)
{
    SDL_Tab *Tab=(SDL_Tab*)tab;
    
    switch(feature)
    {
    case SET_FONT:
        Tab->font=va_arg(list,SDL_Font*);
        break;

    case ADD_TAB:
        AddTab((SDL_Tab *)tab,va_arg(list,char*));
        break;

    case SET_BG_COLOR:
        Tab->bgcolor = va_arg(list,Uint32);
        break;

    case SET_FG_COLOR:
        Tab->fgcolor = va_arg(list,Uint32);
        break;

    case SET_CALLBACK:
        {
            int why=va_arg(list,int);
            if(why==SDL_CLICKED)
                Tab->OnClicked = va_arg(list,void*);
            else if(why == SDL_KEYDOWN_RETURN)
            {
                //for the edit/rename widget
                Tab->OnReturn = va_arg(list,void*);
            }
            break;
        }

    case SET_CAPTION:
    {
        (void)va_arg(list,int);
        if(Tab->tabs->caption)
            free(Tab->tabs->caption);
        Tab->tabs->caption=strdup(va_arg(list,char*));
        break;
        
    }
    case SET_STATE_EDIT:
    {
        if(Tab->edit == NULL)
        {
            Tab->edit=SDL_WidgetCreate(SDL_EDIT,Tab->rect->x+Tab->hl->rect->x,Tab->rect->y+Tab->hl->rect->y,
                                       Tab->hl->rect->w,Tab->hl->rect->h);
            SDL_WidgetProperties(SET_FONT,Tab->font);
            SDL_WidgetProperties(SET_ALWAYS_FOCUS,1);
            SDL_WidgetProperties(SET_BG_COLOR,Tab->bgcolor);
            SDL_WidgetProperties(SET_FG_COLOR,Tab->fgcolor);
            SDL_WidgetProperties(SET_CAPTION,Tab->hl->caption);
            SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_ANY    , Tab_EditAnyKeyPressed    , Tab);
            SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN , Tab_EditReturnKeyPressed , Tab);
        }
        break;
    }
    
    }
}

void SDL_TabEventHandler(void * tab,SDL_Event *event)
{
    SDL_Tab *Tab=(SDL_Tab*)tab;
    SDL_TabList *tl = Tab->tabs;

    switch(event->type)
    {
    case SDL_MOUSEMOTION:
        break;
    case SDL_MOUSEBUTTONDOWN:
        if(SDL_WidgetIsInside(Tab->rect,event->motion.x,event->motion.y))
        {
            while(tl && event->motion.x > (tl->rect->x+Tab->rect->x) && 
                  event->motion.x > (Tab->rect->x + tl->rect->x + tl->rect->w))
                tl=tl->next;

            if(tl)
            {
                Tab->hl=tl;

                if(Tab->OnClicked)
                {
                    Tab->OnClicked(NULL);
                }
            }
        }
    case SDL_MOUSEBUTTONUP:
        break;
    default:
        break;
    }
    


}

#if 0
static SDL_TabImage *SDL_CreateDefaultTab(SDL_Tab *tab, int hl)
{
    SDL_TabImage *tabimage;
    SDL_Surface *surface;
    Uint32 *data;
    Uint32 rmask, gmask, bmask, amask;
    int x=0;
    int y=0;
    int lineoffset=0;
    int linex;
    int entireline=0;
    int line=0;
    int width=tab->min_width;

    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;

    tabimage= (SDL_TabImage*)malloc(sizeof(SDL_TabImage));
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, tab->min_width, tab->min_height, 32,
                                   rmask, gmask, bmask, amask);

    if(surface == NULL) 
    {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }



    data=surface->pixels;
    for(y=0;y<tab->min_height;y++)
    {
        linex=lineoffset;
        for(x=0;x<width;x++)     
        {
            if(linex==x)
            {
                if(hl)
                    *data=0xC8C8F8;
                else
                    *data=0x585858;

                data++;
                
                linex++;
                line++;
                
                if(!entireline)
                {
                    if(line >= TAB_LINE_WIDTH)
                    {
                        if(linex <width-lineoffset-TAB_LINE_WIDTH)
                            linex = width-lineoffset-TAB_LINE_WIDTH;
                    }
                    else
                    {
                        data--;
                        /*    if(hl)
                         *data=0xC8C8C8;
                         else
                         *data=0xA8A8A8;*/
                        *data=0xFFEFFF;
                        data++;
                    }
                }         
                

                if(x >= (width-1)-lineoffset)
                    linex=0;

            }
            else if((x < linex) && ( x > lineoffset))
            {
                *data=0x808080;
                data++;
            }
            else
            {
                *data=0xFFFFFF;
                data++;
            }
        }
        line=0;
        entireline=( ((tab->min_height - y -1) > TAB_LINE_WIDTH) ? 0 : 1);
        if(y%2 == 0)
            lineoffset++;
    }


    tabimage->slopewidth = surface->h /2;
    tabimage->surface = SDL_DisplayFormat(surface);
    SDL_FreeSurface(surface);

    return tabimage;
}
#endif

static void AddTab(SDL_Tab *tab,char *caption)
{
    SDL_TabList *l;

    if(tab->tabs == NULL)
    { 
        tab->tabs   = (SDL_TabList*)malloc(sizeof(SDL_TabList));
        tab->tabs->rect= (SDL_Rect *)malloc(sizeof(SDL_Rect));
        if(caption)
        {
            tab->tabs->caption = strdup(caption);
        }
        else
        {
            tab->tabs->caption = malloc(255 * sizeof(char));
            memset(tab->tabs->caption,0,255);
        }
        tab->NoOfTabs=0;
        tab->hl=tab->tabs;
        tab->tabs->draw    = 1;
        tab->tabs->rect->x = 0;
        tab->tabs->rect->y = 0;
        tab->tabs->rect->h = tab->rect->h;
        tab->tabs->rect->w = 0;
        tab->tabs->next = NULL;
        tab->tabs->prev = NULL;
        tab->tabs->index = tab->NoOfTabs;
    }
    else
    {
          
        l=tab->tabs;

        while(l->next)
        {
            l=l->next;
        }
        l->next= (SDL_TabList*) malloc(sizeof(SDL_TabList));
        if(caption)
            l->next->caption = strdup(caption);
        else
            l->next->caption = NULL;
        l->next->rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
        l->next->rect->y = 0;
        l->next->rect->h = tab->rect->h;
        l->next->rect->x = 0;
        l->next->rect->w = 0;
        l->next->index = tab->NoOfTabs;
        l->next->next = NULL;
        l->next->prev = l;
        
    }
    tab->NoOfTabs++;
}

int FitsInWiget(SDL_Tab *tab,SDL_TabList *tl)
{
    int width=SDL_FontGetStringWidth(tab->font,tl->caption);
    
    
    while(tl->next)
    {
        tl=tl->next;
        width+=SDL_FontGetStringWidth(tab->font,tl->caption);
    }


    if(width > tab->rect->w)
        return 0;
    else
        return 1;


}

int Tab_DrawTabWithCaption(SDL_Surface *dest,SDL_Tab *tab,SDL_TabList * tl, int high)
{
    //int width       = tl->rect->w;
//    int howmany     = 0;
//    int where       = 0;
//    int stretch     = 0;
//    int slopewidth  = tab->highlighted->slopewidth;
//    SDL_Rect src,dst;
    SDL_Rect dst;
//    SDL_TabImage *TabSurface;

    int xoffset=0;
    int yoffset=0;


    xoffset=tab->rect->x;
    yoffset=tab->rect->y;

    dst.x = tl->rect->x + xoffset;
    dst.y = tl->rect->y + yoffset;

    dst.w = tl->rect->w;
    dst.h = tl->rect->h;

    if(high)
        SDL_FillRect(dest,&dst,tab->bgcolor);
    else
        SDL_FillRect(dest,&dst,0xff00ff);

    SDL_FontSetColor(tab->font,0xffffee);

    if(tl->caption)
    {
        SDL_FontDrawString(dest,tab->font,tl->caption,
                           xoffset + tl->rect->x + 10,
                           yoffset + tl->rect->y);
    }
    
    return 1;
}

void tabarrowclicked()
{
    printf("Tab to left\n");
}

int Tab_DrawArrows(SDL_Surface *dest,SDL_Tab *tab)
{
    SDL_Rect rect;

    rect.x = tab->rect->x;
    rect.y = tab->rect->y;
    rect.w = 30;
    rect.h = 20;

    SDL_WidgetCreate(SDL_BUTTON,tab->rect->x,tab->rect->y,20,30);

    //SDL_FillRect(dest,&rect,SDL_MapRGB(dest->format,255,0,0));
    return 0;
}


void SDL_TabRecalculate(SDL_Tab *tab)
{
    SDL_TabList *tablist;
    int width;

    tablist=tab->tabs;

    while(tablist)
    {
        /*just take a value of 10 for additional space */
        if(tablist->caption)
            width = SDL_FontGetStringWidth(tab->font,tablist->caption) + 10;
        else
            width = 35;
        tablist->rect->w = width;
        tablist->rect->h = tab->font->height;
        if(tablist->prev == NULL)
        {
            /* this is the first item in the tabtablist */
            tablist->rect->x = 0;
            tablist->rect->y = 0;
        }
        else
        {
            tablist->rect->x = tablist->prev->rect->x + tablist->prev->rect->w + 10;
            tablist->rect->y = tablist->prev->rect->y;
        }
        tablist=tablist->next;
    }
}




/*
 * Functions used for the edit widget when renaming a tab
 */
void Tab_EditAnyKeyPressed(void *data)
{


}

void Tab_EditReturnKeyPressed(void *data)
{
    SDL_Tab *Tab=(SDL_Tab*)data;
    char string[255];

    SDL_WidgetPropertiesOf(Tab->edit,GET_CAPTION,&string);

    if(Tab->hl->caption == NULL)
        Tab->hl->caption = malloc(sizeof(char)*255);

    sprintf(Tab->hl->caption,"%s",string);
    
    SDL_WidgetClose(Tab->edit);
    Tab->edit=NULL;

    if(Tab->OnReturn)
        Tab->OnReturn();

}
