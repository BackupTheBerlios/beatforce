/*
  Beatforce/SDLTk

  Copyright (C) 2004 John Beuving (john.beuving@home.nl)

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
    SDL_Font *Font;
    SDL_Rect rect;
    
    Uint32  bgcolor;
    Uint32  fgcolor;

    int nItems;

    int Visible;
    int Redraw;
    int FirstVisible;

    TreeNode *Tree;
    TreeNode *Selected;

    SDL_Surface *Background;
    
    void (*Clicked)(void *data);
    void *ClickedData;

    void* Scrollbar;
    
}SDL_Tree;


void* SDL_TreeCreate(SDL_Rect *rect);
void  SDL_TreeDraw(void *tree,SDL_Surface *dest);
int   SDL_TreeProperties(void *tree,int feature,va_list list);
void  SDL_TreeEventHandler(void *tree,SDL_Event *event);

void *SDL_TreeInsertItem(void *tree,void *root,char *string);


#endif /* __SDL_TREE_H__ */
