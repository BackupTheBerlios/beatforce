/*
   BeatForce
   debug.h	- debugging stuff
   
   Copyright (c) 2003, John Beuving (john.beuving@home.nl)

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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#define AUDIO_OUTPUT 1
#define CLOCK        2
#define EFFECT       3
#define FILEWINDOW   4
#define INPUT        5
#define MP3          6
#define OGG          7
#define OSA          8
#define OUTPUT       9
#define PLAYER       10
#define PLAYER_UI    11
#define PLAYLIST     12
#define PLAYLIST_UI  13
#define PLUGIN       14
#define RINGBUFFER   15
#define SAMPLER      16
#define SONGDB       17
#define SONGDB_UI    18
#define THEME        19
#define WNDMGR       20

#define name(x) x
#define module( x ) ( #x )
#define stringer( x ) module(x)
void traceprintf(char *fmt,...);
void noprint(char *fmt,...);
void printid(char *id,int line,char *message);




#ifdef MODULE_ID

#define LOG   printid(__FILE__,__LINE__,"LOG  "),traceprintf
#define ERROR printid(__FILE__,__LINE__,"ERROR"),traceprintf

#define TRACE_ON printid(__FILE__,__LINE__,"TRACE"),traceprintf
#define DEBUG_ON printid(__FILE__,__LINE__,"DEBUG"),traceprintf

#define TRACE_OFF noprint
#define DEBUG_OFF noprint

#if MODULE_ID == AUDIO_OUTPUT
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == CLOCK
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == EFFECT
#define TRACE TRACE_ON
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == FILEWINDOW
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == INPUT
#undef TRACE
#undef DEBUG
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == WNDMGR
#undef TRACE
#undef DEBUG
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == MP3
#undef TRACE
#undef DEBUG
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == OGG
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == OSA
#define TRACE TRACE_ON
#define DEBUG DEBUG_ON
#endif

#if MODULE_ID == OUTPUT
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == PLAYER
#undef TRACE
#undef DEBUG
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == PLAYER_UI
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == PLAYLIST
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == PLAYLIST_UI
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == PLUGIN
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == RINGBUFFER
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == SAMPLER
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == SONGDB
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == SONGDB_UI
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == THEME
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#endif //MODULE_ID


#endif //DEBUG_H







