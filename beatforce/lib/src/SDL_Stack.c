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

#include <stdio.h>
#include <malloc.h>
#include "SDL_Stack.h"
#include "SDL_Widget.h"

StackList *stacklist;
Stack     *current_stack;
SDL_Surface  *previous_surface;
Stack     *current_focus;


/* retrun -1 on error
 * return 0 on exits and make active
 * return 1 if a new one is created
 */
int 
SDL_SurfaceStack(SDL_Surface *surface)
{
    StackList *surfaces;

    previous_surface=SDL_GetSurfaceStack();

    if(stacklist==NULL)
    {
        stacklist=malloc(sizeof(StackList));
        if(stacklist==NULL)
            return -1;
        stacklist->surface=surface;
        stacklist->stack=NULL;
        stacklist->parent=NULL;
        stacklist->next=NULL;
        current_stack=stacklist->stack;
    }
    else
    {
        surfaces=stacklist;

        /* If it already exists only make it active */
        if(surfaces->surface == surface)
        {
            current_stack  = surfaces->stack;
            return 0;
        }
        else
        {
            while(surfaces->next)
            {
                surfaces=surfaces->next;
                if(surfaces->surface == surface)
                {
                     current_stack=surfaces->stack;
//                    printf("Found surface %p\n",current_stack);
                    return 0;
                }
            }
        }

        
        /* Add a new one */
        surfaces->next=(StackList*)malloc(sizeof(StackList));
        if(surfaces->next==NULL)
            return -1;
        
        surfaces->next->surface=surface;
        surfaces->next->parent=surfaces->surface;
        surfaces->next->stack=NULL;
        surfaces->next->next=NULL;
         current_stack = surfaces->next->stack;
        
    }
    return 1;
}

SDL_Surface *SDL_GetSurfaceStack()
{
    StackList *surfaces;
    surfaces=stacklist;
    while(surfaces)
    {
        if(surfaces->stack == current_stack)
            return surfaces->surface;
        surfaces=surfaces->next;

    }
    return NULL;

}

SDL_Surface *SDL_GetPreviousStack()
{
    return previous_surface;

}

void SDL_AddToStack(int item,SDL_Rect* dest,void *data)
{
    StackList *list;

    if(current_stack==NULL)
    {
        current_stack=malloc(sizeof(Stack));
        current_stack->dest.x=dest->x;
        current_stack->dest.y=dest->y;
        current_stack->dest.w=dest->w;
        current_stack->dest.h=dest->h;
        current_stack->type=item;
        current_stack->data=data;
        current_stack->next=NULL;

        /* Set the focus to the new widget */
        SDL_StackSetFocus(current_stack);

        /* Add the newly created Stack to the stacklist */
        list=stacklist;

        if(list->stack == NULL)
        {
            list->stack = current_stack;
        }
        else
        {
            while(list->next)
            {
                list=list->next;
                if(list->stack == NULL)
                {
                    list->stack = current_stack;
                    return;
                }
                
            }
        }
    }
    else
    {
        Stack *temp;
        temp=current_stack;
        
        while(temp)
        {
            if(dest->x >= temp->dest.x && dest->x <= (temp->dest.x + temp->dest.w))
            {   
                if(dest->y >= temp->dest.y && dest->y <= (temp->dest.y + temp->dest.h))
                {
//                    printf("Overlapping widgets\n");                    
                }
            }
            temp=temp->next;
        }
        temp=current_stack;
        while(temp->next)
            temp=temp->next;
        temp->next=malloc(sizeof(Stack));
        temp=temp->next;
        temp->dest.x=dest->x;
        temp->dest.y=dest->y;
        temp->dest.w=dest->w;
        temp->dest.h=dest->h;
        temp->type=item;
        temp->data=data;
        temp->next=NULL;
        SDL_StackSetFocus(temp);
    }



}


Stack *SDL_StackGetLastItem()
{
    Stack *tmp=current_stack;
    while(tmp->next)
        tmp=tmp->next;
    return tmp;
}

Stack *SDL_StackGetStack()
{
    return current_stack;
}

Stack *SDL_StackGetSurfaceStack(SDL_Surface *surface)
{
    StackList* surfaces;

    surfaces=stacklist;

    while(surfaces)
    {
        if(surfaces->surface == surface)
            return surfaces->stack;
        surfaces=surfaces->next;

    }
    return NULL;    
}


void SDL_StackSetFocus(Stack *focus_widget)
{
    current_focus=focus_widget;
}

Stack *SDL_StackGetFocus()
{
    return current_focus;
}


