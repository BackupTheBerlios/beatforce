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

SDL_Surface  *previous_surface; /* Previous active surface */

Stack     *current_focus;       /* Current widget focus    */
static SDL_sem *StackSem;
StackList *SurfaceList;         /* List of surfaces (NEW)  */
StackList *CreateOnStack;
StackList *ActiveScreen;

int SDL_StackInit()
{
    previous_surface=NULL;
    current_focus=NULL;
    SurfaceList   = NULL;
    CreateOnStack = NULL;
    ActiveScreen  = NULL;
    StackSem=SDL_CreateSemaphore(1);
    return 1;
}

int SDL_StackNewSurface(SDL_Surface *surface)
{
    if(SurfaceList == NULL)
    {
        SurfaceList=(StackList*)malloc(sizeof(StackList));
        memset(SurfaceList,0,sizeof(StackList));
        SurfaceList->surface=surface;
        CreateOnStack=SurfaceList;
    }
    else
    {
        StackList *l;
        l=SurfaceList;
        while(l->next)
            l=l->next;

        l->next=(StackList*)malloc(sizeof(StackList));
        memset(l->next,0,sizeof(StackList));
        l->next->surface=surface;
        CreateOnStack=l->next;
        return 0;
    }
    return 1;
}


int 
SDL_SurfaceStack(SDL_Surface *surface)
{
    StackList *surfaces;
    surfaces=SurfaceList;

    while(surfaces)
    {
        if(surfaces->surface == surface)
        {
            ActiveScreen=surfaces;
            return 1;
        }
        surfaces=surfaces->next;

    }
    return 0;
}

SDL_Surface *SDL_GetSurfaceStack()
{
    StackList *surfaces;
    surfaces=SurfaceList;
    while(surfaces)
    {
        if(surfaces == ActiveScreen)
        {
            return surfaces->surface;
        }
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
    if(CreateOnStack->stack==NULL)
    {
        CreateOnStack->stack=malloc(sizeof(Stack));
        memset(CreateOnStack->stack,0,sizeof(Stack));

        CreateOnStack->stack->dest.x=dest->x;
        CreateOnStack->stack->dest.y=dest->y;
        CreateOnStack->stack->dest.w=dest->w;
        CreateOnStack->stack->dest.h=dest->h;
        CreateOnStack->stack->data=data;
        CreateOnStack->stack->next=NULL;

        /* Set the focus to the new widget */
        SDL_StackSetFocus(CreateOnStack->stack);
    }
    else
    {
        Stack *temp;
        temp=CreateOnStack->stack;
        
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
        temp=CreateOnStack->stack;
        while(temp->next)
            temp=temp->next;
        temp->next=malloc(sizeof(Stack));
        memset(temp->next,0,sizeof(Stack));
        temp=temp->next;
        temp->dest.x=dest->x;
        temp->dest.y=dest->y;
        temp->dest.w=dest->w;
        temp->dest.h=dest->h;
        temp->data=data;
        temp->next=NULL;
        SDL_StackSetFocus(temp);
    }



}

Stack *SDL_StackGetLastItem()
{
    Stack *tmp=CreateOnStack->stack;
    while(tmp->next)
        tmp=tmp->next;
    return tmp;
}


Stack *SDL_StackGetStack(SDL_Surface *surface)
{
    StackList *surfaces;
    surfaces=SurfaceList;

    if(surface == NULL)
    {
        return ActiveScreen->stack;
    }
    else
    {
        while(surfaces)
        {
            if(surfaces->surface == surface)
                return surfaces->stack;
            
            surfaces=surfaces->next;
        }

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




