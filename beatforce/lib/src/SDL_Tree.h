/*
  Beatforce/SDLTk

  Copyright (C) 2004 John Beuving (john.beuving@beatforce.org)

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

#ifndef __SDL_TREE_H__
#define __SDL_TREE_H__

#include "SDL_Widget.h"
#include "SDL_Font.h"

typedef struct TreeNode
{
    struct TreeNode *Next;
    struct TreeNode *Child;
    struct TreeNode *Parent;
    
    int collapsed;
    char *Label;
    int Level;
}TreeNode;

typedef struct SDL_Tree
{
    SDL_Widget Widget;
    SDL_Font  *Font;
    
    Uint32  bgcolor;
    Uint32  fgcolor;

    int nItems;

    int FirstVisible;

    TreeNode *Tree;
    TreeNode *Selected;

   
    void (*Clicked)(void *data);
    void *ClickedData;

    SDL_Widget* Scrollbar;
    
}SDL_Tree;


SDL_Widget* SDL_TreeCreate(SDL_Rect *rect);
void        SDL_TreeDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area);
int         SDL_TreeProperties(SDL_Widget *widget,int feature,va_list list);
int         SDL_TreeEventHandler(SDL_Widget *widget,SDL_Event *event);

TreeNode *SDL_TreeInsertItem(SDL_Widget *widget,TreeNode *root,char *string);
TreeNode *SDL_TreeGetSelectedItem(void *tree);

#endif /* __SDL_TREE_H__ */
