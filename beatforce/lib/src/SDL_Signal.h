/*
  Beatforce/ SDLTk

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

#ifndef __SDL_SIGNAL_H__
#define __SDL_SIGNAL_H__

#include "SDL_Widget.h"

typedef struct CallbackList
{
    SDL_Widget *Widget;
    void *callback;
    void *data;
    struct CallbackList *Next;
}CallbackList;

typedef struct SignalList
{
    char *Signal;
    int Type;
    struct CallbackList *CallList;
    struct SignalList   *Next;
}SignalList;

void SDL_SignalNew(char *signal,int type);
void SDL_SignalInit();
int SDL_SignalConnect(SDL_Widget *widget,char *signal,void *callback,void *data);
int SDL_SignalEmit(SDL_Widget *widget,char *signal,...);

#endif /* __SDL_SIGNAL_H__ */
