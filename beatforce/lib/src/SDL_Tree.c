/*
  Beatforce/SDLTk

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003-2004 John Beuving (john.beuving@wanadoo.nl)

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
#include "SDL_Tree.h"
#include "SDL_Primitives.h"
#include "SDL_Scrollbar.h"
#include "SDL_Signal.h"

const struct S_Widget_FunctionList SDL_Tree_FunctionList =
{
    SDL_TreeCreate,
    SDL_TreeDraw,
    SDL_TreeProperties,
    SDL_TreeEventHandler,
    NULL,
};

static TreeNode* SDL_TreeGetItem(TreeNode *Tree,int *number);
static void SDL_TreeCollapse(TreeNode *Tree,int number);
static int SDL_TreeCount(TreeNode *Tree,int *number);
static void SDL_TreeDrawExpander(SDL_Surface *screen,
                                 SDL_Tree *Tree,
                                 TreeNode *Item,int row);

static void SDL_TreeDrawLines(SDL_Surface *screen,
                              SDL_Tree *Tree,
                              TreeNode *Item,int row);

SDL_Widget* SDL_TreeCreate(SDL_Rect* rect)
{
    SDL_Widget *Widget;
    SDL_Tree   *Tree;

    Tree=(SDL_Tree*)malloc(sizeof(SDL_Tree));
    Widget=(SDL_Widget*)Tree;
    
    Widget->Type    = SDL_TREE;
    Widget->Rect.x  = rect->x;
    Widget->Rect.y  = rect->y;
    Widget->Rect.w  = rect->w;
    Widget->Rect.h  = rect->h;

    Tree->nItems       = 0; /* Number of items to draw */
    Tree->FirstVisible = 0; /* First item to draw      */

    Tree->Selected = NULL;

    Tree->Font    = NULL;

    Tree->fgcolor = 0x000000;
    Tree->bgcolor = TRANSPARANT;

    Tree->Scrollbar  = NULL;
    Tree->Tree       = NULL;

    Tree->Clicked     = NULL;
    Tree->ClickedData = NULL;
    return (SDL_Widget*)Tree;
}

void SDL_TreeDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{
    SDL_Tree *Tree=(SDL_Tree*)widget;
    TreeNode *Item;
    int w;
    SDL_Rect r;
    int row=0;
    int n=0;

    SDL_FontSetColor(Tree->Font,Tree->fgcolor);
    
    if(Tree->bgcolor != TRANSPARANT)
    {
        SDL_FillRect(dest,&widget->Rect,Tree->bgcolor);
    }

    SDL_TreeCount(Tree->Tree,&n);

    if(n > Tree->nItems)
    {
        n-=Tree->nItems;
        n++;
        if(Tree->Scrollbar == NULL)
        {
            SDL_Rect SliderRect;
                
            /* 
             * Attach the Slider widget 
             */
            SliderRect.x     = widget->Rect.x + widget->Rect.w - 9;
            SliderRect.y     = widget->Rect.y;
            SliderRect.h     = widget->Rect.h;
            SliderRect.w     = 9;
                
            Tree->Scrollbar = SDL_WidgetCreateR(SDL_SCROLLBAR,SliderRect);
            SDL_ScrollbarSetMaxValue(Tree->Scrollbar,n);
        }
        else
        {
            SDL_ScrollbarSetMaxValue(Tree->Scrollbar,n);
        }
    }
    
    if(Tree->Scrollbar)
    {
        int val;
        val=SDL_ScrollbarGetCurrentValue(Tree->Scrollbar);
        row=val;
        Tree->FirstVisible=row;
    }

    for(w=row;w<Tree->nItems+row;w++)
    {
        int j=w;
        int height= SDL_FontGetHeight(Tree->Font)+2;
                
        Item=SDL_TreeGetItem(Tree->Tree,&j);

        if(Item)
        {
            SDL_TreeDrawExpander(dest,Tree,Item,w-row);
            SDL_TreeDrawLines(dest,Tree,Item,w-row);

            if(Tree->Font)
            {
                r.x = widget->Rect.x + 10*Item->Level +11;
                r.h = height;
                r.y = (w-row) * r.h + widget->Rect.y;
                r.w = widget->Rect.w;
               
                if(Item == Tree->Selected)
                    SDL_FontSetColor(Tree->Font,0xff0000);
                else
                    SDL_FontSetColor(Tree->Font,Tree->fgcolor);

                SDL_FontDrawStringRect(dest,Tree->Font,Item->Label,&r);
            }
        }
    }
}

static void SDL_TreeDrawExpander(SDL_Surface *screen,
                                 SDL_Tree *Tree,
                                 TreeNode *Item,
                                 int row
                                 )
{

    int height = SDL_FontGetHeight(Tree->Font) + 2;
    int x = Tree->Widget.Rect.x + 10 * Item->Level;
    int y = Tree->Widget.Rect.y + height * row + ((height/2)-5);

    if(Item->collapsed == 2)
        return;

    if(x > Tree->Widget.Rect.x + Tree->Widget.Rect.w + 8)
        return;
    
    if(y > Tree->Widget.Rect.y + Tree->Widget.Rect.h + 8)
        return;

    boxColor(screen,x,y, x+8, y+8,0xffffffff);
	rectangleColor (screen,x,y, x+8, y+8,0x000000ff);
    SDL_DrawLine (screen,x + 2, y + 4, x + 6, y + 4,0x000000ff);
    if(Item->collapsed == 1)
        SDL_DrawLine (screen,x + 4, y + 2, x + 4, y + 6,0x000000ff);

}

static void SDL_TreeDrawLines(SDL_Surface *screen,
                              SDL_Tree *Tree,
                              TreeNode *Item,int row)
{
    int height = SDL_FontGetHeight(Tree->Font)+2;
    int x = Tree->Widget.Rect.x + 10 * Item->Level;
    int y = Tree->Widget.Rect.y + height*row -2;
    TreeNode *Parent = Item->Parent;

    /* The small line on the bottom of the expander */
    if(Item->collapsed == 0)
        SDL_DrawLine (screen,x + 4, y + height - 1, x + 4, y + height   ,0x000000ff);

    if(Parent == NULL) /* return when it is the root node */
        return;

    /* draw point of the lines is 10 pixels in front of the expander */
    x -= 10;

    /* Vertical line */
    if(Item->Next )
        SDL_DrawLine (screen,x + 4, y    , x + 4, y + height   ,0x000000ff);
    else
        SDL_DrawLine (screen,x + 4, y    , x + 4, y + height/2,0x000000ff);
    

    /* Horizontal line */
    if(Item->collapsed == 2) /*If there is no expander make the line a little longer */
        SDL_DrawLine (screen,x + 5, y + height/2, x + 15, y + height/2,0x000000ff);
    else
        SDL_DrawLine (screen,x + 5, y + height/2, x + 9, y + height/2,0x000000ff);

    /* Parent line */
    while(Parent)
    {
        x-=10;
        if(Parent->Parent && Parent->Parent->collapsed == 0)
        {
            if(Parent->Next) /* Parent vertical line */
                SDL_DrawLine (screen,x + 4, y    , x + 4, y + height ,0x000000ff);
        }
        Parent=Parent->Parent;
    }

}

int SDL_TreeProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Tree *Tree=(SDL_Tree*)widget;

    switch(feature)
    {
    case SET_FONT:
    {
        int h;
        Tree->Font=va_arg(list,SDL_Font*);
        h=SDL_FontGetHeight(Tree->Font) + 2;
        if(h)
        {
            Tree->nItems=widget->Rect.h / h;
        }
    }
    break;
    
    case SET_FG_COLOR:
        Tree->fgcolor=va_arg(list,Uint32);
        break;

    case SET_BG_COLOR:
        Tree->bgcolor=va_arg(list,Uint32);
        break;

    }
    return 1;
}

int SDL_TreeEventHandler(SDL_Widget *widget,SDL_Event *event)
{
    SDL_Tree *Tree=(SDL_Tree*)widget;
    
    switch(event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
        if(SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
        {
            if(event->button.button == 1)
            {
                int row=SDL_FontGetHeight(Tree->Font)+2;
                int y=event->motion.y - widget->Rect.y;
                
                if(event->motion.x < widget->Rect.x + widget->Rect.w)
                    if(event->motion.x > (widget->Rect.x + (widget->Rect.w -5)))
                        return 0;
                    
                
                row=y/row;
                
                row=row+Tree->FirstVisible;
                SDL_TreeCollapse(Tree->Tree,row);
                Tree->Selected=SDL_TreeGetItem(Tree->Tree,&row);
                SDL_SignalEmit(widget,"clicked");
                SDL_WidgetDraw(widget,&widget->Rect);
            }
            if(event->button.button == 4) /* mousehweel down */
            {
                int row;
                if(Tree->Scrollbar)
                {
                    row=SDL_ScrollbarGetCurrentValue(Tree->Scrollbar);
                    row-=1;
                    SDL_ScrollbarSetCurrentValue(Tree->Scrollbar,row);
                    SDL_WidgetDraw(widget,&widget->Rect);
                }

            }
            if(event->button.button == 5) /* Mousewheel up */
            {
                int row;
                if(Tree->Scrollbar)
                {
                    row=SDL_ScrollbarGetCurrentValue(Tree->Scrollbar);
                    row+=1;
                    SDL_ScrollbarSetCurrentValue(Tree->Scrollbar,row);
                    SDL_WidgetDraw(widget,&widget->Rect);
                }
            }
            
        }
        break;
    }
    
    return 0;
}


TreeNode *SDL_TreeGetSelectedItem(void *tree)
{
    SDL_Tree *Tree=(SDL_Tree*)tree;
    
    if(Tree)
        return Tree->Selected;
    else
        return NULL;

}

TreeNode *SDL_TreeInsertItem(SDL_Widget *widget,TreeNode *root,char *string)
{
    SDL_Tree *Tree=(SDL_Tree*)widget;
    TreeNode *temp=NULL;

    if(Tree == NULL || string == NULL)
        return NULL;
    
    if(root == NULL)
    {
        if(Tree->Tree  == NULL)
        {
            Tree->Tree = (TreeNode*)malloc(sizeof(TreeNode));
            memset(Tree->Tree,0,sizeof(TreeNode));
            Tree->Tree->Label = strdup(string);
            Tree->Tree->collapsed = 1;
            Tree->Tree->Level = 0;
            Tree->Tree->Parent = NULL;
            return Tree->Tree;
        }
        else 
        {
            temp=Tree->Tree;
            while(temp->Next)
                temp=temp->Next;
            
            temp->Next=(TreeNode*)malloc(sizeof(TreeNode));
            memset(temp->Next,0,sizeof(TreeNode));

            temp->Next->collapsed = 1;
            temp->Next->Label = strdup(string);
            temp->Next->Level = 0;
            temp->Next->Parent = NULL;
            temp=temp->Next;
        }
        return temp;
    }
    else
    {
        temp=(TreeNode*)root;
        
        if(temp->Child == NULL)
        {
            temp->Child=(TreeNode*)malloc(sizeof(TreeNode));
            memset(temp->Child,0,sizeof(TreeNode));

            temp->Child->Parent    = temp;
            temp->Child->Level     = temp->Level+1;
            temp->Child->collapsed = 1;
            temp->Child->Label     = strdup(string);
           
            return temp->Child;
        }
        else
        {
            int level   = temp->Level;
            TreeNode *p = temp;

            temp=temp->Child;
            while(temp->Next)
                temp=temp->Next;
            
            temp->Next=(TreeNode*)malloc(sizeof(TreeNode));
            memset(temp->Next,0,sizeof(TreeNode));

            temp->Next->collapsed=1;
            temp->Next->Label  = strdup(string);
            temp->Next->Level  = level+1;
            temp->Next->Parent = p;
            return temp->Next;
        }
    }

}





static TreeNode* SDL_TreeGetItem(TreeNode *Tree,int *number)
{
    TreeNode *b;

    while(Tree)
    {
        if((*number) == 0)
            return Tree;

        if((*number)>0)
            (*number)--;

        if(Tree->Child && Tree->collapsed == 0)
        {
            b=SDL_TreeGetItem(Tree->Child,number);
            if(b)
                return b;
        }
        Tree=Tree->Next;
    }
    return NULL;
}

static int SDL_TreeCount(TreeNode *Tree,int *number)
{
    while(Tree)
    {
        if(Tree->Label)
            (*number)++;

        if(Tree->Child && Tree->collapsed == 0)
        {
            SDL_TreeCount(Tree->Child,number);
        }
        Tree=Tree->Next;
    }
    return 0;
}

static void SDL_TreeCollapse(TreeNode *Tree,int number)
{
    TreeNode *node;
    node=SDL_TreeGetItem(Tree,&number);
    if(node)
    {
        if(node->collapsed == 1)
        {
            if(node->Child || node->Level == 0)
            {
                node->collapsed=0;
            }
            else
            {
                node->collapsed=2;
            }
        }
        else if(node->collapsed == 0)
        {
            node->collapsed=1;
        }
    }
}


