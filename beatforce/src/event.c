/*
   BeatForce
   event.c  -	Event handler
   
   Copyright (c) 2004, John Beuving (john.beuving@beatforce.org)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public Licensse as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include <malloc.h>

typedef struct CallbackList
{
    void (*callback)(int poster,void *data);
    void *data;
    struct CallbackList *Next;
}CallbackList;

typedef struct SignalList
{
    int Event;
    struct CallbackList *CallList;
    struct SignalList   *Next;
}SignalList;

struct SignalList *EventHandlerList;

void EVENT_Init()
{
    EventHandlerList = NULL;
}

void EVENT_PostEvent(int event_id,int poster)
{
    SignalList *l;

    if(EventHandlerList == NULL)
        return;

    l=EventHandlerList;

    while(l)
    {
        if(l->Event == event_id)
        {
            CallbackList *list;
            list=l->CallList;
            while(list)
            {
                list->callback(poster,list->data);
                list=list->Next;
            }
        }
        l=l->Next;
    }
}

void EVENT_Connect(int event_id,void *callback,void *data)
{
    if(EventHandlerList == NULL)
    {
        EventHandlerList = malloc(sizeof(SignalList));
        memset(EventHandlerList,0,sizeof(SignalList));
        
        EventHandlerList->Event = event_id;
        EventHandlerList->CallList = malloc(sizeof(CallbackList));
        memset(EventHandlerList->CallList,0,sizeof(CallbackList));

        EventHandlerList->CallList->callback = callback;
        EventHandlerList->CallList->data     = data;
    }
    else
    {
        if(EventHandlerList->Event == event_id)
        {
            CallbackList *l;
            
            l=EventHandlerList->CallList;
            while(l->Next)
                l=l->Next;

            l->Next = malloc(sizeof(CallbackList));
            memset(l->Next,0,sizeof(CallbackList));
            
            l->Next->callback = callback;
            l->Next->data     = data;
        }
    }

}
