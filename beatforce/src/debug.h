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
#define CONFIGWINDOW 3
#define EFFECT       4
#define FILEWINDOW   5
#define INPUT        6
#define MP3          7
#define OGG          8
#define OSA          9
#define OUTPUT       10
#define PLAYER       11
#define PLAYER_UI    12
#define PLAYLIST     13
#define PLAYLIST_UI  14
#define PLUGIN       15
#define RINGBUFFER   16
#define SAMPLER      17
#define SONGDB       18
#define SONGDB_UI    19
#define THEME        20
#define WNDMGR       21

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

#if MODULE_ID == CONFIGWINDOW
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
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
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


#endif /* __DEBUG_H__ */







