/*
   BeatForce
   event.h  -	Event handler
   
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

#ifndef __EVENT_H__
#define __EVENT_H__

#define EVENT_PLAYER_PAUSE   8
#define EVENT_PLAYER_PLAY    9

void EVENT_Init();
void EVENT_PostEvent(int event_id,int poster);
void EVENT_Connect(int event_id,void *callback,void *data);


#endif /* __EVENT_H__ */
