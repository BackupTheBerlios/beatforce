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


void* SDL_TreeCreate(SDL_Rect* rect)
{
    SDL_Tree *tree;

    tree=(SDL_Tree*)malloc(sizeof(SDL_Tree));

    tree->rect.x  = rect->x;
    tree->rect.y  = rect->y;
    tree->rect.w  = rect->w;
    tree->rect.h  = rect->h;

    tree->Font    = NULL;

    tree->fgcolor = 0x000000;
    tree->bgcolor = TRANSPARANT;

    tree->Redraw     = 1;
    tree->Background = NULL;

    tree->Visible    = 1;
    tree->Tree       = NULL;
    return tree;
}

void SDL_TreeDraw(void *tree,SDL_Surface *dest)
{
    SDL_Tree *Tree=(SDL_Tree*)tree;
    char string[255];
    
    if(Tree->Visible == 0)
        return;

    memset(string ,0,255);

    SDL_FontSetColor(Tree->Font,Tree->fgcolor);
    
    if(Tree->bgcolor == TRANSPARANT)
    {
        if(Tree->Background == NULL)
        {
            Tree->Background = SDL_WidgetGetBackground(dest,&Tree->rect);
        }
        if(SDL_BlitSurface(Tree->Background,NULL,dest,&Tree->rect)<0)
            fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
    }
    else
    {
        SDL_FillRect(dest,&Tree->rect,Tree->bgcolor);
    }
   
}

int SDL_TreeProperties(void *tree,int feature,va_list list)
{
    SDL_Tree *Tree=(SDL_Tree*)tree;

    switch(feature)
    {
    case SET_FONT:
        Tree->Font=va_arg(list,SDL_Font*);
        break;
    case SET_FG_COLOR:
        Tree->fgcolor=va_arg(list,Uint32);
        break;

    case SET_BG_COLOR:
        Tree->bgcolor=va_arg(list,Uint32);
        break;

    case SET_VISIBLE:
        Tree->Visible=va_arg(list,int);

    }
    return 1;
}

void SDL_TreeEventHandler(void *tree,SDL_Event *event)
{


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
            Tree->Tree->label = strdup(string);
            Tree->Tree->collapsed = 1;
        }
        else 
        {
            temp=Tree->Tree;
            while(temp->next)
                temp=temp->next;
            
            temp->next=(TreeNode*)malloc(sizeof(TreeNode));
            memset(temp->next,0,sizeof(TreeNode));

            temp->next->collapsed = 1;
            temp->next->label=strdup(string);
            temp=temp->next;
        }
        return temp;
    }
    else
    {
        temp=(TreeNode*)root;
        
        if(temp->child == NULL)
        {
            temp->child=(TreeNode*)malloc(sizeof(TreeNode));
            memset(temp->child,0,sizeof(TreeNode));

            temp=temp->child;
            temp->collapsed=1;
            temp->label=strdup(string);
            return temp;
        }
        else
        {
            temp=temp->child;
            while(temp->next)
                temp=temp->next;
            
            temp->next=(TreeNode*)malloc(sizeof(TreeNode));
            memset(temp->next,0,sizeof(TreeNode));

            temp->next->collapsed=1;
            temp->next->label=strdup(string);
            return temp->next;
        }
    }

}
