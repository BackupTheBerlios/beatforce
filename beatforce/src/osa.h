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
#include "llist.h"

//init
void OSA_Init();

// path settings
char *OSA_GetConfigDir();
char *OSA_GetSharedLibExtension();

// directory functions
BFList *OSA_FindFiles(char *dir,char *extension,int recursive);
BFList *OSA_FindDirectories(char *dir);

// shared library functions
void   *OSA_LibraryLoad(char *filename);
void   *OSA_LibraryGetSym(void *h,char *function);
void    OSA_LibraryClose(void *h);

// file functions
int OSA_FileExists(char *filename);

// timer functions
unsigned int OSA_TimerStart(unsigned int interval,void *function,void *data);
void         OSA_TimerRemove(unsigned int timer);

//thread functions
int OSA_ThreadCreate(int (*fn)(void *), void *data);
void OSA_ThreadWait(int thread);


//File functions (string)
char *OSA_SearchFilename(char *filepath);

//time function
int OSA_TimeGet(int *hours,int *minutes);


void OSA_Sleep(int us);
char *OSA_GetError();
