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

#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include <SDLTk.h>

#define TAB_DEFAULTHEIGHT    16
#define TAB_LINE_WIDTH       1
#define TAB_OVERLAY          6


/* Prototypes for local tab functions */

static void Tab_AddArrows(SDL_Tab *tab);


static void Tab_Recalculate(SDL_Tab *tab);
static int  Tab_DrawTabWithCaption(SDL_Surface *dest,SDL_Tab *tab,
                                   SDL_TabList * tl, int high);


/* Prototypes for editwidget callbacks */
void Tab_EditAnyKeyPressed();
void Tab_EditReturnKeyPressed();

/* Prototypes for button callbacks */
void Tab_ArrowLeftPressed(void *data);
void Tab_ArrowRightPressed(void *data);

/* signal handler prototype */
void SDL_TabMouseButtonDownCB(SDL_Widget *widget,SDL_Event *event);

extern void DrawPixel(SDL_Surface *screen, int x, int y,unsigned int color2);

const struct S_Widget_FunctionList SDL_Tab_FunctionList =
{
    SDL_TabCreate,
    SDL_TabDraw,
    SDL_TabProperties,
    NULL,
    NULL
};

SDL_Widget *SDL_TabCreate(SDL_Rect *rect)
{
    SDL_Tab     *newtab;
    SDL_Widget  *Widget;

    newtab=(SDL_Tab*)malloc(sizeof(SDL_Tab));
    newtab->NoOfTabs=0;

    Widget=(SDL_Widget*)newtab;
    Widget->Rect.x = rect->x;
    Widget->Rect.y = rect->y;
    Widget->Rect.w = rect->w;
    Widget->Rect.h = rect->h;
    Widget->Type   = SDL_TAB;


    newtab->min_width  = 35;
    newtab->min_height = TAB_DEFAULTHEIGHT;
    newtab->tabs  = NULL;
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


    SDL_SignalConnect(Widget,"mousebuttondown",SDL_TabMouseButtonDownCB,newtab);

    newtab->font        = &DefaultFont;
    return (SDL_Widget*)newtab;

}

void SDL_TabDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{
    SDL_Tab     *Tab=(SDL_Tab*)widget;
    SDL_TabList *tablist;

    if(Tab->tabs==NULL)
    {
        //SDL_FillRect(dest, &widget->Rect,Tab->bgcolor);
        return;
    }
    else
    {
        tablist=Tab->tabs;
    }

  
    Tab_Recalculate(Tab); /* we can only run this with a valid surface */

    rectangleColor(dest,widget->Rect.x,widget->Rect.y,
                        widget->Rect.x+widget->Rect.w,
                        widget->Rect.y+widget->Rect.h - Tab->tabs->rect->h -1,
                        0x000000ff);

    //SDL_FillRect(dest,&widget->Rect,0x222222);        
    while(tablist)
    {
        Tab_DrawTabWithCaption(dest,Tab,tablist,0);
        tablist=tablist->next;
    }
    /* Redraw the highlighted tab */
    Tab_DrawTabWithCaption(dest,Tab,Tab->hl,1);
    if(Tab->edit)
    {
//        SDL_WidgetPropertiesOf((SDL_Widget*)Tab->edit,FORCE_REDRAW,1);
    }
}

int SDL_TabProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Tab *Tab=(SDL_Tab*)widget;
    int retval=1;
    
    switch(feature)
    {
    case SET_FONT:
        Tab->font=va_arg(list,SDL_Font*);
        break;

    case SET_BG_COLOR:
        Tab->bgcolor = va_arg(list,Uint32);
        break;

    case SET_FG_COLOR:
        Tab->fgcolor = va_arg(list,Uint32);
        break;

    case SET_CAPTION:
    {
        (void)va_arg(list,int);
        if(Tab->tabs->caption)
            free(Tab->tabs->caption);
        Tab->tabs->caption=strdup(va_arg(list,char*));
        break;
        
    }
    }
    return retval;
}

void SDL_TabMouseButtonDownCB(SDL_Widget *widget,SDL_Event *event)
{
    SDL_Tab *Tab=(SDL_Tab*)widget;
    SDL_TabList *tl = Tab->tabs;

    if(SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
    {
        if(event->button.button == 1)
        {
            if(tl == NULL)
                return;

            /* Dont process the event when clicked above the tabs */
            if(event->motion.y < widget->Rect.y + widget->Rect.h - tl->rect->h)
                return;

            /* Don't handle events when clicked on one of the arrows */
            if(Tab->ArrowLeft && SDL_WidgetIsInside(Tab->ArrowLeft,event->motion.x,event->motion.y))
                return;
            
            if(Tab->ArrowRight && SDL_WidgetIsInside(Tab->ArrowRight,event->motion.x,event->motion.y))
                return;

              
            while(tl && event->motion.x > (tl->rect->x + widget->Rect.x - Tab->startx) && 
                    event->motion.x > (widget->Rect.x + tl->rect->x + tl->rect->w - Tab->startx))
                    tl=tl->next;

            if(tl && tl != Tab->hl)
            {   
                SDL_WidgetHide(Tab->hl->Child);
                SDL_WidgetShow(tl->Child);
                Tab->hl=tl;
                    
                SDL_SignalEmit(widget,"switch-tab");
                SDL_WidgetDraw(widget,&widget->Rect);
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
}



void Tab_AddArrows(SDL_Tab *tab)
{
    SDL_Widget *widget=(SDL_Widget*)tab;
    SDL_Rect LeftArrow;
    SDL_Rect RightArrow;

    LeftArrow.x = widget->Rect.x + widget->Rect.w - (widget->Rect.h *2) - 2 - 1;
    LeftArrow.y = widget->Rect.y + 1;
    LeftArrow.w = widget->Rect.h - 2;
    LeftArrow.h = widget->Rect.h - 2;

    tab->ArrowLeft=SDL_WidgetCreate(SDL_BUTTON,LeftArrow.x,LeftArrow.y,
                                    LeftArrow.w,LeftArrow.h);
    SDL_SignalConnect(tab->ArrowLeft,"clicked",Tab_ArrowLeftPressed,tab);
    
    RightArrow.x = widget->Rect.x + widget->Rect.w - widget->Rect.h - 2;
    RightArrow.y = widget->Rect.y + 1;
    RightArrow.w = widget->Rect.h - 2;
    RightArrow.h = widget->Rect.h - 2;

    tab->ArrowRight=SDL_WidgetCreate(SDL_BUTTON,RightArrow.x,RightArrow.y,
                                     RightArrow.w,RightArrow.h);
    SDL_SignalConnect(tab->ArrowRight,"clicked",Tab_ArrowRightPressed,tab);
    
}



static int Tab_DrawSlope(SDL_Surface *dest,SDL_Tab *tab,SDL_TabList * tl, int high,int left)
{
    SDL_Widget *widget=(SDL_Widget*)tab;
    SDL_Rect rect;
    unsigned int color;
    int i=0,j=0;
    int omde;
    int start;
    int end;
    int mw,w;
    if(left)
        rect.x = widget->Rect.x + tl->rect->x - 6;
    else
        rect.x = widget->Rect.x + tl->rect->x + tl->rect->w;

    rect.x -= tab->startx;

    rect.y = widget->Rect.y + tl->rect->y;
    rect.w = 6;
    rect.h = tl->rect->h;

    if(high)
        color=tab->bgcolor;
    else
        color=0x000099;
    
    if(rect.x > widget->Rect.x + widget->Rect.w)
        return 0;

    if((rect.x + rect.w) < widget->Rect.x)
        return 0;

    w  = widget->Rect.x - rect.x;
    mw = widget->Rect.x + widget->Rect.w - rect.x;

    omde=rect.h / (rect.w-1);
    start = 0;
    end   = rect.w; 

    for(i=0;i<rect.h;i++)
    {
        for(j=w;j<mw;j++)
        {
         

            if(j >= start && j < end)
                fastPixelColorNolockNoclip(dest,rect.x + j,rect.y + i,color);
            if(j == start && left)
                fastPixelColorNolockNoclip(dest,rect.x + j,rect.y + i,0xfffefe);

            if(j == end && left == 0)
                fastPixelColorNolockNoclip(dest,rect.x + j,rect.y + i,0x000101);
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
    SDL_Widget *widget=(SDL_Widget*)tab;
    SDL_Rect dst,set;
    int xoffset=0;
    int yoffset=0;
    int ml = tab->startx;

    xoffset = widget->Rect.x;
    yoffset = widget->Rect.y;

    dst.x = tl->rect->x + xoffset -ml;
    dst.y = tl->rect->y + yoffset;

    dst.w = tl->rect->w;
    dst.h = tl->rect->h;

    set.x = tl->rect->x + xoffset - ml;
    set.y = dst.y;
    set.w = dst.w;
    set.h = dst.h;

    /* Check if the tab doesn't start on the left side of the tab area */
    if(dst.x < widget->Rect.x)
    {
        /* If the entire tab is on the left side of the area return and don't draw */
        if(((tl->rect->x + tl->rect->w) - ml) < 0)
            return 0; 
        dst.w = dst.w - (widget->Rect.x - dst.x);
        dst.x = widget->Rect.x;
    }

    /* Check if the width doesn't go beyong the tab area */
    if((dst.x + dst.w) > (widget->Rect.x + widget->Rect.w))
        dst.w = (widget->Rect.x + widget->Rect.w) - dst.x;
    
    /* Check if the tab starts outside the area */
    if(dst.x > (widget->Rect.x + widget->Rect.w))
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

    if(tl->caption)
    {
        SDL_FontDrawStringLimited(dest,tab->font,tl->caption,0xfffffee,&set,&dst);
    }
    
    return 1;
}


static void Tab_Recalculate(SDL_Tab *tab)
{
    SDL_TabList *tablist;
    SDL_Widget  *widget  = (SDL_Widget*)tab;
    int width=0;
    int maxstartx=0;
   
    tablist=tab->tabs;
    
    while(tablist)
    {
        width = 0;
        /*just take a value of 10 for additional space */
        if(tablist->caption && tab->font)
            width = SDL_FontGetStringWidth(tab->font,tablist->caption) + 2;
        if(width < 35)
            width = 35;

        tablist->rect->w = width;
        if(tab->font)
            tablist->rect->h = SDL_FontGetHeight(tab->font);
        else
            tablist->rect->h = 14;
        
        if(tablist->prev == NULL)
        {
            /* this is the first item in the tabtablist */
            tablist->rect->x = 10;
            tablist->rect->y = widget->Rect.h - tablist->rect->h - 1;
        }
        else
        {
            /*just take a value of 10 for additional space */
            tablist->rect->x = tablist->prev->rect->x + tablist->prev->rect->w + 10;
            tablist->rect->y = tablist->prev->rect->y;
            if( (tablist->rect->x + tablist->rect->w) > widget->Rect.w)
            {
                if(tab->ArrowLeft == NULL && tab->ArrowRight == NULL)
                    Tab_AddArrows(tab);
                tab->doesntfit = 1;
            }

        }
        maxstartx = maxstartx + tablist->rect->w + 10;
        tablist=tablist->next;
    }
    maxstartx -= widget->Rect.w;
    if(tab->startx > maxstartx)
        tab->startx = maxstartx;
    if(tab->startx < 0)
        tab->startx = 0;
}


/*
 * Functions used for the edit widget when renaming a tab
 */
void Tab_EditAnyKeyPressed(void *data)
{


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

int SDL_NotebookAppendTab(SDL_Widget *parent,SDL_Widget *child,char *caption)
{
    SDL_Tab *tab=(SDL_Tab*)parent;
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
        tab->tabs->rect->h = parent->Rect.h;
        tab->tabs->rect->w = 0;
        tab->tabs->Child = child;
        tab->tabs->next = NULL;
        tab->tabs->prev = NULL;
        tab->tabs->index = tab->NoOfTabs;
        if(child)
            child->Visible = 1;

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
        l->next->rect->h = parent->Rect.h;
        l->next->rect->x = 0;
        l->next->rect->w = 0;
        l->next->Child   = child;
        l->next->index = tab->NoOfTabs;
        l->next->next = NULL;
        l->next->prev = l;

        if(child)
        {
            child->Visible = 0;
            SDL_WidgetHide(child);
        }
                   
    }
    tab->NoOfTabs++;
 
    return 1;
}


int SDL_NotebookRemoveTab(SDL_Widget *widget,int tabnr)
{
    SDL_Tab *tab=(SDL_Tab*)widget;
    SDL_TabList *remove;

    if(tab->NoOfTabs > 0 && tabnr < tab->NoOfTabs)
    {
        remove=tab->tabs;
        if(tabnr == 0)
        {
            tab->tabs       = remove->next;
            tab->tabs->prev = NULL;
        }
        else
        {
            while(tabnr--)
            {
                remove=remove->next;
            }
            remove->prev->next=remove->next;
            remove->next->prev=remove->prev;

        }
        if(remove->caption)
            free(remove->caption);

        free(remove->rect);
        free(remove);
        tab->NoOfTabs--; 
        SDL_WidgetRedrawEvent(widget);
        return 1;
    }
    return 0;
}

int SDL_NotebookSetCurrentTab(SDL_Widget *widget,int tabnr)
{
    SDL_Tab *tab=(SDL_Tab*)widget;
    SDL_TabList *page;

    if(tab->NoOfTabs > 0 && tabnr < tab->NoOfTabs)
    {
        page=tab->tabs;
        while(tabnr--)
            page=page->next;

        SDL_WidgetHide(tab->hl->Child);
        SDL_WidgetShow(page->Child);
        tab->hl=page;
        SDL_WidgetRedrawEvent(widget);

    }
    return 1;

}

int SDL_NotebookGetCurrentPage(SDL_Widget *widget)
{
    SDL_Tab *tab=(SDL_Tab*)widget;
    
    return tab->hl->index;
}

SDL_Widget *SDL_NotebookGetChildWidget(SDL_Widget *widget)
{
    SDL_Tab *tab=(SDL_Tab*)widget;

    if(tab->hl)
        return tab->hl->Child;
    else
        return NULL;
}

void SDL_NotebookClear(SDL_Widget *widget)
{
    SDL_Tab *Tab=(SDL_Tab*)widget;
    SDL_TabList *page;
    if(Tab->hl)
    {
        SDL_WidgetHide(Tab->hl->Child);
        SDL_WidgetClose(Tab->hl->Child);
    }
    Tab->hl       = NULL;
    
    page=Tab->tabs;
    while(page)
    {
        page->Child->Visible = 0;
        page=page->next;
    }

    Tab->tabs     = NULL;
    Tab->NoOfTabs = 0;
}
