/*
  Beatforce/ Operating System Abstraction layer

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
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>
#include <time.h>

#include "osa.h"


#define MODULE_ID OSA
#include "debug.h"


#define NO_OF_THREADS 20
#define NO_OF_TIMERS  20

SDL_Thread *threads[NO_OF_THREADS];
SDL_TimerID timers[NO_OF_TIMERS];

static char configdir[255];

void OSA_Init()
{
    int i=0;
    for(i=0;i<NO_OF_THREADS;i++)
        threads[i]=NULL;
    for(i=0;i<NO_OF_TIMERS;i++)
        timers[i]=0;
}

char *OSA_GetSharedLibExtension()
{
    return (char*)".so";
}

char *OSA_GetConfigDir()
{
    sprintf(configdir,"%s/%s/",getenv("HOME"),".beatforce");
    return configdir;
}

BFList *OSA_FindDirectories(char *dir)
{
    DIR *d;
    struct dirent* dent;
    BFList *dirs = NULL;
    char dirname[255];
    struct stat buf;

    TRACE("OSA_FindDirectories enter %s",dir);

    if(dir == NULL)
    {
        ERROR("Unable to execute");
        return NULL;
    }


    d=opendir(dir);
    if(d==NULL)
    {
        ERROR("Can't open directory %s",dir);
        return NULL;
    }
    dent=readdir(d);
    if(dent == NULL)
    {
        ERROR("readdir");
    }
    while(dent)
    {
        sprintf(dirname,"%s/%s",dir,dent->d_name);
        stat(dirname,&buf);
        if(S_ISDIR(buf.st_mode))
        {
            if(strlen(dirname))
                dirs=LLIST_Append(dirs,strdup(dirname));
        }
        dent=readdir(d);
    }
    closedir(d);
    return dirs;
}


BFList *OSA_FindFiles(char *dir,char *extension,int recursive)
{
    DIR *d;
    struct dirent* dent;
    BFList *files=NULL;
    int offset=0;
    char filename[255];
    struct stat buf;

    if(dir == NULL)
    {
        return NULL;
    }
    
    d=opendir(dir+offset);
    if(d==NULL)
    {
        DEBUG("DoDir %s",dir);
        return NULL;
    }
    dent=readdir(d);
    while(dent)
    {
        sprintf(filename,"%s/%s",dir,dent->d_name);
        stat(filename,&buf);
        
        if(S_ISREG(buf.st_mode) && strlen(dent->d_name)>4)
        {
            char *ext;

            ext=strrchr(dent->d_name,'.');
            if(ext!=NULL)
            {
                if(!strcmp(ext,extension))
                {
                    files=LLIST_Append(files,strdup(filename));
                }
            }
        }
        if(S_ISDIR(buf.st_mode) && dent->d_name[0]!='.')
        {
            char newdir[255];

            memset(newdir,0,255);
            if(dir[strlen(dir)-1]=='/')
                sprintf(newdir,"%s%s/",dir,dent->d_name);
            else
                sprintf(newdir,"%s/%s/",dir,dent->d_name);
            if(recursive)
            {
                BFList *newfiles;
                BFList *last;
                newfiles=OSA_FindFiles(newdir,extension,1);
                if(files)
                {
                    last=LLIST_Last(files);
                    last->next=newfiles;
                }
                else
                {
                    files=newfiles;
                }
            }
            
        }
        dent=readdir(d);
    }
    closedir(d);
    return files;
}


void *OSA_LoadLibrary(char *filename)
{
    return dlopen(filename,RTLD_NOW | RTLD_GLOBAL);
}

void *OSA_GetFunctionAddress(void *h,char *function)
{
    return dlsym (h, function);

}

void OSA_CloseLibrary(void *h)
{
    dlclose (h);
}

unsigned int OSA_StartTimer(unsigned int interval,void *function,void *data)
{
    int i;
    for(i=0;i<NO_OF_TIMERS;i++)
    {
        if(timers[i]==0)
        {
            timers[i]=SDL_AddTimer(interval,function,data);
            return i;
        }
    }
    return NO_OF_TIMERS;
}

void OSA_RemoveTimer(unsigned int timer)
{
    if(timers[timer]!=0)
    {
        SDL_RemoveTimer(timers[timer]);
        timers[timer]=0;
    }
}


int OSA_CreateThread(int (*fn)(void *), void *data)
{
    int i;

    for(i=0;i<NO_OF_THREADS;i++)
    {
        if( threads[i] == NULL)
        {
            threads[i] = SDL_CreateThread(fn,data);
            return i;
        }
    }
    return NO_OF_THREADS;
}

void OSA_RemoveThread(int thread)
{
    if(thread > NO_OF_THREADS)
    {
        exit(1);
    }
    if(threads[thread])
    {
        SDL_WaitThread(threads[thread], NULL);
        threads[thread]=NULL;
    }
}


char *OSA_SearchFilename(char *filepath)
{
    char *p=strrchr(filepath,'/');
    char *m;

    if(p)
    {
        m=strrchr(p,'.');
        if(m)
          *m=0;
        return p+1;
    }
    else
        return p;
    
}

int OSA_GetTime(int *hours,int *minutes)
{
  time_t rawtime;
  struct tm * timeinfo;

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  if(hours)
      *hours=timeinfo->tm_hour;
  if(minutes)
      *minutes=timeinfo->tm_min;
  
  return 0;


}
