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


#include <SDL/SDL.h> 


typedef struct Stack
{
    SDL_Rect dest;
    int type;
    void *data;
    struct    Stack *next;
}Stack;


typedef struct StackList
{
    SDL_Surface *surface;
    Stack       *stack;
    SDL_Surface *parent;
    struct StackList *next;
}StackList;

int SDL_SurfaceStack(SDL_Surface *surface);
SDL_Surface* SDL_GetSurfaceStack();
Stack *SDL_StackGetSurfaceStack(SDL_Surface *surface);

void SDL_AddToStack(int item,SDL_Rect* dest,void *data);
Stack *SDL_StackGetStack();
Stack *SDL_StackGetLastItem();
void SDL_StackSetFocus(Stack *focus_widget);
Stack *SDL_StackGetFocus();
