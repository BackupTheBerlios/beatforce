/*
   BeatForce
   debug.h	- debugging stuff
   
   Copyright (c) 2003-2004, John Beuving (john.beuving@beatforce.org)

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

#define AUDIO_CHANNEL 1
#define AUDIO_OUTPUT  2
#define AUDIOCD       3
#define CLOCK         4
#define CONFIGFILE    5
#define CONFIGWINDOW  6
#define EFFECT        7
#define EVENT         8
#define FILEWINDOW    9
#define INPUT         10
#define INPUTPLUGIN   11
#define MIXER         12 
#define MP3           13
#define OGG           14
#define OSA           15
#define OUTPUT        16
#define PLAYER        17
#define PLAYER_UI     18
#define PLAYLIST      19
#define PLAYLIST_UI   20
#define PLUGIN        21
#define RINGBUFFER    22
#define SAMPLER       23
#define SONGDB        24
#define SONGDB_UI     25
#define THEME         26
#define WNDMGR        27

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
//#define TRACE_OFF TRACE_ON
#define DEBUG_OFF noprint

#if MODULE_ID == AUDIO_CHANNEL
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == AUDIO_OUTPUT
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == AUDIOCD
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == CLOCK
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == CONFIGFILE
#define TRACE TRACE_ON
#define DEBUG DEBUG_ON
#endif

#if MODULE_ID == CONFIGWINDOW
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == EFFECT
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == EVENT
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == FILEWINDOW
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == INPUT
#define TRACE TRACE_ON
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == INPUTPLUGIN
#define TRACE TRACE_ON
#define DEBUG DEBUG_ON
#endif

#if MODULE_ID == WNDMGR
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == MIXER
#define TRACE TRACE_OFF
#define DEBUG DEBUG_OFF
#endif

#if MODULE_ID == MP3
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
#define TRACE TRACE_ON
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
#define TRACE TRACE_ON
#define DEBUG DEBUG_ON
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







