/*
  Beatforce/ Startup of beatforce

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
#include <string.h>

#include <SDL/SDL.h>
#include "SDL_Widget.h"
#include "SDL_Signal.h"

SignalList *SList;

void SDL_SignalInit()
{
    SList = NULL;
}

void SDL_SignalNew(char *signal,int type)
{
    if(SList == NULL)
    {
        SList=(SignalList*)malloc(sizeof(SignalList));
        memset(SList,0,sizeof(SignalList));
        SList->Signal   = strdup(signal);
        SList->Type     = type;
        SList->CallList = NULL;
        SList->Next     = NULL;
    }
    else
    {
        SignalList *l;
   
        l=SList;
        while(l->Next)
        {
            if(!strcmp(l->Signal,signal))
                printf("%s:%d: Signal already exists\n",__FILE__,__LINE__);
            l=l->Next;
        }

        l->Next = (SignalList*)malloc(sizeof(SignalList));
        memset(l->Next,0,sizeof(SignalList));
        l->Next->Signal   = strdup(signal);
        l->Next->Type     = type;
        l->Next->CallList = NULL;
        l->Next->Next     = NULL;
    }
}

int SDL_SignalConnect(SDL_Widget *widget,char *signal,void *callback,void *data)
{
    if(SList == NULL)
    { 
        printf("%s:%d: Signal doesn't exists \"%s\"\n",__FILE__,__LINE__,signal);
        return 0;
    }
    else
    {
        SignalList *l;
   
        l=SList;
        while(l)
        {
            if(!strcmp(signal,l->Signal))
                break;
            l=l->Next;
        }

        if(l == NULL)
        { 
            printf("%s:%d: Signal doesn't exists \"%s\"\n",__FILE__,__LINE__,signal);
            return 0;
        }

        if(l->CallList == NULL)
        {
            l->CallList = (CallbackList*)malloc(sizeof(CallbackList));
            memset(l->CallList,0,sizeof(CallbackList));
            l->CallList->Widget   = widget;
            l->CallList->callback = callback;
            l->CallList->data     = data;
            l->CallList->Next     = NULL;
            return 1;
        }
        else
        {
            CallbackList *cbl;

            cbl=l->CallList;
            while(cbl->Next)
                cbl=cbl->Next;

            cbl->Next = (CallbackList*)malloc(sizeof(CallbackList));
            memset(cbl->Next,0,sizeof(CallbackList));
            cbl->Next->Widget   = widget;
            cbl->Next->callback = callback;
            cbl->Next->data     = data;
            cbl->Next->Next     = NULL;
            return 1;

        }
    }
    return 0;
}


int SDL_SignalEmit(SDL_Widget *widget,char *signal,...)
{
    SignalList *l;
    CallbackList *cbl;
    va_list ap;
    void (*cb0)(void *data);
    void (*cb1)(void *data,SDL_Event *e);

    va_start(ap,signal);

    l=SList;
    while(l)
    {
        if(!strcmp(l->Signal,signal))
        {
            if(l->CallList)
            {
                cbl=l->CallList;

                if(l->Type == 0)
                {
                    while(cbl)
                    {
                        if(widget == cbl->Widget)
                        {
                            cb0=cbl->callback;
                            cb0(cbl->data);
                        }
                        cbl=cbl->Next;
                    }
                }
                else if(l->Type == 1)
                {
                    SDL_Event *event=va_arg(ap,SDL_Event*);

                    while(cbl)
                    {
                        if(widget == cbl->Widget)
                        {
                            cb1 = cbl->callback;
                            cb1(cbl->data,event);
                        }
                        cbl=cbl->Next;
                    }
                }
            }
            else
            {
                /* printf("No handlers for signal %s\n",signal); */
            }
        }
        l=l->Next;
    }
    va_end(ap);
    return 1;
}
