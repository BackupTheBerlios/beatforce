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
#include "SDL_Tree.h"


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

    
    Tree->Redraw     = 1;
    Tree->Background = NULL;

    
    Tree->Scrollbar  = NULL;
    Tree->Visible    = 1;
    Tree->Tree       = NULL;
    return (SDL_Widget*)Tree;
}

void SDL_TreeDraw(SDL_Widget *widget,SDL_Surface *dest)
{
    SDL_Tree *Tree=(SDL_Tree*)widget;
    TreeNode *Item;
    int w;
    SDL_Rect r;
    int row=0;
    int n=0;

    if(Tree->Visible == 0)
        return;

    SDL_FontSetColor(Tree->Font,Tree->fgcolor);
    
    if(Tree->bgcolor == TRANSPARANT)
    {
        if(Tree->Background == NULL)
        {
            Tree->Background = SDL_WidgetGetBackground(dest,&widget->Rect);
        }
        if(SDL_BlitSurface(Tree->Background,NULL,dest,&widget->Rect)<0)
            fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
    }
    else
    {
        SDL_FillRect(dest,&widget->Rect,Tree->bgcolor);
    }

    {


        SDL_TreeCount(Tree->Tree,&n);

        if(n > Tree->nItems)
        {
            n-=Tree->nItems;
            if(Tree->Scrollbar == NULL)
            {
                SDL_Rect SliderRect;
                
                /* 
                 * Attach the Slider widget 
                 */
                SliderRect.x     = widget->Rect.x + widget->Rect.w - 45;
                SliderRect.y     = widget->Rect.y;
                SliderRect.h     = widget->Rect.h;
                SliderRect.w     = 45;
                
                Tree->Scrollbar = SDL_WidgetCreateR(SDL_SLIDER,SliderRect);
            
                SDL_WidgetProperties(SET_MAX_VALUE,n);
                SDL_WidgetProperties(SET_MIN_VALUE,0);
                /* Use the background of the tree */
                SDL_WidgetProperties(STOREBACKGROUND,0);

            }
            else
            {
                SDL_WidgetProperties(SET_MAX_VALUE,n);
            }
        }
        if(Tree->Scrollbar)
        {
            double val;
            SDL_WidgetPropertiesOf(Tree->Scrollbar,GET_CUR_VALUE,&val);
            row=(int)val;
            Tree->FirstVisible=row;
        }

    }


    for(w=row;w<Tree->nItems+row;w++)
    {
        int j=w;
        char string[255];
        Item=SDL_TreeGetItem(Tree->Tree,&j);
        
        if(Item)
        {
            if(Tree->Font)
            {
                int i;
                string[0]=0;
                for(i=0;i<Item->Level;i++)
                    sprintf(string,"%s ",string);

                if(Item->collapsed == 1)
                    sprintf(string,"%s+%s",string,Item->Label);
                else if(Item->collapsed == 2)
                    sprintf(string,"%s %s",string,Item->Label);
                else
                    sprintf(string,"%s-%s",string,Item->Label);

                r.x = widget->Rect.x;
                r.h = SDL_FontGetHeight(Tree->Font);
                r.y = (w-row) * r.h + widget->Rect.y;
                r.w = widget->Rect.w;
                
                if(Item == Tree->Selected)
                    SDL_FontSetColor(Tree->Font,0xff0000);
                else
                    SDL_FontSetColor(Tree->Font,Tree->fgcolor);
                SDL_FontDrawStringRect(dest,Tree->Font,string,&r);
            }
        }
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
        h=SDL_FontGetHeight(Tree->Font);
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

    case SET_VISIBLE:
        Tree->Visible=va_arg(list,int);
        break;

    case SET_CALLBACK:
    {
        int t=va_arg(list,int);
        Tree->Clicked=va_arg(list,void*);
        Tree->ClickedData=va_arg(list,void*);
    }
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
        if(SDL_WidgetIsInside(&widget->Rect,event->motion.x,event->motion.y))
        {
            if(event->button.button == 1)
            {
                int row=SDL_FontGetHeight(Tree->Font);
                int y=event->motion.y - widget->Rect.y;
                
                if(event->motion.x < widget->Rect.x + widget->Rect.w)
                    if(event->motion.x > (widget->Rect.x + (widget->Rect.w -45)))
                        return;
                    
                
                row=y/row;
                
                row=row+Tree->FirstVisible;
                SDL_TreeCollapse(Tree->Tree,row);
                Tree->Selected=SDL_TreeGetItem(Tree->Tree,&row);
                if(Tree->Clicked)
                    Tree->Clicked(Tree->ClickedData);
            }
            if(event->button.button == 4) //mousehweel down
            {
                double row;
                if(Tree->Scrollbar)
                {
                    SDL_WidgetPropertiesOf(Tree->Scrollbar,GET_CUR_VALUE,&row);
                    row-=1;
                    SDL_WidgetPropertiesOf(Tree->Scrollbar,SET_CUR_VALUE,row);
                }

            }
            if(event->button.button == 5)
            {
                double row;
                if(Tree->Scrollbar)
                {
                    SDL_WidgetPropertiesOf(Tree->Scrollbar,GET_CUR_VALUE,&row);
                    row+=1;
                    SDL_WidgetPropertiesOf(Tree->Scrollbar,SET_CUR_VALUE,row);
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

void *SDL_TreeInsertItem(void *tree,void *root,char *string)
{
    SDL_Tree *Tree=(SDL_Tree*)tree;
    TreeNode *temp=NULL;

    if(string == NULL)
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
           
            return temp;
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


