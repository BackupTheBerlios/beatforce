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

#define TAB_DEFAULTHEIGHT    16
#define TAB_LINE_WIDTH       1
#define TAB_OVERLAY          6


/* Prototypes for local tab functions */

static void Tab_AddArrows(SDL_Tab *tab);
static void Tab_AddTab(SDL_Tab *tab,char *string);
static int  Tab_RemoveTab(SDL_Tab *tab);
static void Tab_Recalculate(SDL_Tab *tab);
static int  Tab_DrawTabWithCaption(SDL_Surface *dest,SDL_Tab *tab,
                                   SDL_TabList * tl, int high);


/* Prototypes for editwidget callbacks */
void Tab_EditAnyKeyPressed();
void Tab_EditReturnKeyPressed();

/* Prototypes for button callbacks */
void Tab_ArrowLeftPressed(void *data);
void Tab_ArrowRightPressed(void *data);

extern void DrawPixel(SDL_Surface *screen, int x, int y,unsigned int color2);

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
    newtab->NoOfTabs=0;
    newtab->rect=(SDL_Rect*)malloc(sizeof(SDL_Rect));
    newtab->rect->x    = rect->x;
    newtab->rect->y    = rect->y;
    newtab->rect->w    = rect->w;
    newtab->rect->h    = rect->h;
    newtab->min_width  = 35;
    newtab->min_height = TAB_DEFAULTHEIGHT;
    newtab->tabs  = NULL;
    newtab->next  = NULL;
    newtab->hl    = NULL;
    newtab->bgcolor     = 0xff0000;
    newtab->fgcolor     = WHITE;
    newtab->normal      = NULL;
    newtab->highlighted = NULL;
    newtab->startx      = 0;
    newtab->maxstartx   = 0;
    newtab->doesntfit   = 0;
    newtab->overlay     = TAB_OVERLAY;
    newtab->edit        = NULL;
    newtab->ArrowLeft   = NULL;
    newtab->ArrowRight  = NULL;
    newtab->OnClicked   = NULL;
    newtab->OnReturn    = NULL;
    return newtab;

}

void SDL_TabDraw(void *data,SDL_Surface *dest)
{
    SDL_Tab     *tab=(SDL_Tab*)data;
    SDL_TabList *tablist;

    if(tab->tabs==NULL)
    {
        SDL_FillRect(dest, tab->rect,tab->bgcolor);
        return;
    }
    else
    {
        tablist=tab->tabs;
    }

  
    Tab_Recalculate(tab); /* we can only run this with a valid surface */
    SDL_FillRect(dest,tab->rect,0x222222);        
    while(tablist)
    {
        Tab_DrawTabWithCaption(dest,tab,tablist,0);
        tablist=tablist->next;
    }
    /* Redraw the highlighted tab */
    Tab_DrawTabWithCaption(dest,tab,tab->hl,1);
    if(tab->edit)
    {
        SDL_WidgetPropertiesOf(tab->edit,FORCE_REDRAW,1);
    }
}

int SDL_TabProperties(void *tab,int feature,va_list list)
{
    SDL_Tab *Tab=(SDL_Tab*)tab;
    int retval=1;
    
    switch(feature)
    {
    case SET_FONT:
        Tab->font=va_arg(list,SDL_Font*);
        break;

    case TAB_ADD:
        Tab_AddTab((SDL_Tab *)tab,va_arg(list,char*));
        break;

    case TAB_REMOVE:
        retval=Tab_RemoveTab((SDL_Tab *)tab);
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
    return retval;
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
            if(event->button.button == 1)
            {
                /* Don't handle events when clicked on one of the arrows */
                if(Tab->ArrowLeft && SDL_WidgetIsInside(&Tab->ArrowLeft->rect,event->motion.x,event->motion.y))
                    return;
            
                if(Tab->ArrowRight && SDL_WidgetIsInside(&Tab->ArrowRight->rect,event->motion.x,event->motion.y))
                    return;

                while(tl && event->motion.x > (tl->rect->x+Tab->rect->x - Tab->startx) && 
                      event->motion.x > (Tab->rect->x + tl->rect->x + tl->rect->w - Tab->startx))
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
            if(event->button.button == 4)
            {
                if(Tab->doesntfit)
                    Tab->startx+=5;
            }
            if(event->button.button == 5)
            {
                if(Tab->doesntfit)
                    Tab->startx-=5;
            }
            
        }
    case SDL_MOUSEBUTTONUP:
        break;
    default:
        break;
    }

}

void Tab_AddArrows(SDL_Tab *tab)
{
    SDL_Rect LeftArrow;
    SDL_Rect RightArrow;

    LeftArrow.x = tab->rect->x + tab->rect->w - (tab->rect->h *2) - 2 - 1;
    LeftArrow.y = tab->rect->y + 1;
    LeftArrow.w = tab->rect->h - 2;
    LeftArrow.h = tab->rect->h - 2;

    tab->ArrowLeft=SDL_WidgetCreate(SDL_BUTTON,LeftArrow.x,LeftArrow.y,
                                               LeftArrow.w,LeftArrow.h);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,Tab_ArrowLeftPressed, tab);

    RightArrow.x = tab->rect->x + tab->rect->w - tab->rect->h - 2;
    RightArrow.y = tab->rect->y + 1;
    RightArrow.w = tab->rect->h - 2;
    RightArrow.h = tab->rect->h - 2;

    tab->ArrowRight=SDL_WidgetCreate(SDL_BUTTON,RightArrow.x,RightArrow.y,
                                                RightArrow.w,RightArrow.h);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,Tab_ArrowRightPressed, tab);
    
}

static void Tab_AddTab(SDL_Tab *tab,char *caption)
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

static int Tab_DrawSlope(SDL_Surface *dest,SDL_Tab *tab,SDL_TabList * tl, int high,int left)
{
    SDL_Rect rect;
    unsigned int color;
    int i=0,j=0;
    int omde;
    int start;
    int end;
    int mw,w;
    if(left)
        rect.x = tab->rect->x + tl->rect->x - 6;
    else
        rect.x = tab->rect->x + tl->rect->x + tl->rect->w;

    rect.x -= tab->startx;

    rect.y = tab->rect->y + tl->rect->y;
    rect.w = 6;
    rect.h = tl->rect->h;

    if(high)
        color=tab->bgcolor;
    else
        color=0x000099;
    
    if(rect.x > tab->rect->x + tab->rect->w)
        return 0;

    if((rect.x + rect.w) < tab->rect->x)
        return 0;

    w  = tab->rect->x -rect.x;
    mw = tab->rect->x + tab->rect->w - rect.x;

    omde=rect.h / (rect.w-1);
    start = 0;
    end   = rect.w; 

    for(i=0;i<rect.h;i++)
    {
        for(j=w;j<mw;j++)
        {
         

            if(j >= start && j < end)
                DrawPixel(dest,rect.x + j,rect.y + i,color);
            if(j == start && left)
                DrawPixel(dest,rect.x + j,rect.y + i,0xfffefe);

            if(j == end && left == 0)
                DrawPixel(dest,rect.x + j,rect.y + i,0x000101);
        }
        if( (i % omde) == 0)
        {
            if(left)
                start++;    
            else
                end --;
        }

    }

    return 1;

    
}

static int Tab_DrawTabWithCaption(SDL_Surface *dest,SDL_Tab *tab,SDL_TabList * tl, int high)
{
    SDL_Rect dst,set;
    int xoffset=0;
    int yoffset=0;
    int ml = tab->startx;

    xoffset = tab->rect->x;
    yoffset = tab->rect->y;

    dst.x = tl->rect->x + xoffset -ml;
    dst.y = tl->rect->y + yoffset;

    dst.w = tl->rect->w;
    dst.h = tl->rect->h;

    set.x = tl->rect->x + xoffset - ml;
    set.y = dst.y;
    set.w = dst.w;
    set.h = dst.h;

    /* Check if the tab doesn't start on the left side of the tab area */
    if(dst.x < tab->rect->x)
    {
        /* If the entire tab is on the left side of the area return and don't draw */
        if(((tl->rect->x + tl->rect->w) - ml) < 0)
            return 0; 
        dst.w = dst.w - (tab->rect->x - dst.x);
        dst.x = tab->rect->x;
    }

    /* Check if the width doesn't go beyong the tab area */
    if((dst.x + dst.w) > (tab->rect->x + tab->rect->w))
        dst.w = (tab->rect->x + tab->rect->w) - dst.x;
    
    /* Check if the tab starts outside the area */
    if(dst.x > (tab->rect->x + tab->rect->w))
        return 0;

    if(high)
    {
        Tab_DrawSlope(dest,tab,tl,high,1);
        SDL_FillRect(dest,&dst,tab->bgcolor);
        Tab_DrawSlope(dest,tab,tl,high,0);
    }
    else
    {
        Tab_DrawSlope(dest,tab,tl,high,1);
        SDL_FillRect(dest,&dst,0x000099);
        Tab_DrawSlope(dest,tab,tl,high,0);
    }

    SDL_FontSetColor(tab->font,0xffffee);

    if(tl->caption)
    {
        SDL_FontDrawStringLimited(dest,tab->font,tl->caption,&set,&dst);
    }
    
    return 1;
}


static void Tab_Recalculate(SDL_Tab *tab)
{
    SDL_TabList *tablist;
    int width=0;
    int maxstartx=0;
   
    tablist=tab->tabs;
    
    while(tablist)
    {
        width = 0;
        /*just take a value of 10 for additional space */
        if(tablist->caption)
            width = SDL_FontGetStringWidth(tab->font,tablist->caption) + 2;
        if(width < 35)
            width = 35;

        tablist->rect->w = width;
        if(tab->font)
            tablist->rect->h = tab->font->height;
        else
            tablist->rect->h = 14;
        
        if(tablist->prev == NULL)
        {
            /* this is the first item in the tabtablist */
            tablist->rect->x = 10;
            tablist->rect->y = 0;
        }
        else
        {
            /*just take a value of 10 for additional space */
            tablist->rect->x = tablist->prev->rect->x + tablist->prev->rect->w + 10;
            tablist->rect->y = tablist->prev->rect->y;
            if( (tablist->rect->x + tablist->rect->w) > tab->rect->w)
            {
                if(tab->ArrowLeft == NULL && tab->ArrowRight == NULL)
                    Tab_AddArrows(tab);
                tab->doesntfit = 1;
            }

        }
        maxstartx = maxstartx + tablist->rect->w + 10;
        tablist=tablist->next;
    }
    maxstartx -= tab->rect->w;
    if(tab->startx > maxstartx)
        tab->startx = maxstartx;
    if(tab->startx < 0)
        tab->startx = 0;
}

static int Tab_RemoveTab(SDL_Tab *tab)
{
    SDL_TabList *remove;

    if(tab->NoOfTabs > 0)
    {
        if(tab->hl)
            remove=tab->hl;    
        else
            return 0;

        tab->hl=tab->hl->prev;        
        
            
        if(remove->next)
            remove->next->prev=remove->prev;
        if(remove->prev)
        {
            remove->prev->next=remove->next;
        }
        else
        {
            tab->tabs=remove->next; /* move the root node */
            tab->hl=tab->tabs;      /* highlight the first node */
        }

        if(remove->caption)
            free(remove->caption);

        free(remove->rect);
        free(remove);
        tab->NoOfTabs--; 
        return 1;
    }
    return 0;
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
        Tab->hl->caption = strdup(string);

    SDL_WidgetClose(Tab->edit);
    Tab->edit=NULL;

    if(Tab->OnReturn)
        Tab->OnReturn();

}

void Tab_ArrowLeftPressed(void *data)
{
    SDL_Tab *Tab = (SDL_Tab*)data;
    Tab->startx -=25;
}

void Tab_ArrowRightPressed(void *data)
{
    SDL_Tab *Tab = (SDL_Tab*)data;

    Tab->startx +=25;

}
