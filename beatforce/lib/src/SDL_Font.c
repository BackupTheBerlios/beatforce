/*
  Beatforce/SDLTk

  one line to give the program's name and an idea of what it does.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "SDL_Font.h"
#include "SDL_FontBDF.h"
#include "SDL_FontFNT.h"

void SDL_ReadFont(FILE *fp,SDL_Font *font);

int SDL_FontDrawChar(SDL_Surface *dest,SDL_Font *font,
                     char character,SDL_Rect *pos,SDL_Rect *clip);
void DrawPixel(SDL_Surface *screen, int x, int y,unsigned int color2);
int SDL_FontGetCharWidth(SDL_Font* font,char character);

extern SDL_Surface *screen;


SDL_FontList *LoadedFonts = NULL;


static SDL_Font *SDL_FontInit(char *filename)
{
    SDL_Font *font=NULL;
    int i;        
    T_Font_IsFormat isformat;
    T_Font_Read     readfont;

    if(filename == NULL)
        return NULL;
    
    font=malloc(sizeof(SDL_Font));
    memset(font,0,sizeof(SDL_Font));
    
    for(i=0;i<LAST_FONT;i++)
    {
        isformat = FontTable[i]->isformat;
        if(isformat(filename))
        {
            readfont = FontTable[i]->read;
            if(readfont(filename,font) == 0)
                return NULL;
                
            if(font)
                font->color=0xfefefe;
            break;
        }
    }
    return font;     
}

void SDL_FontLoad(char *fontid,char *filename)
{
    if(LoadedFonts == NULL)
    {
        LoadedFonts=(SDL_FontList*)malloc(sizeof(SDL_FontList));
        LoadedFonts->FontId   =(char*)strdup(fontid);
        LoadedFonts->Filename =(char*)strdup(filename);
        LoadedFonts->Font     = NULL;
        LoadedFonts->Next     = NULL;
    }
    else
    {
        SDL_FontList *l;

        l=LoadedFonts;

        while(l->Next)
            l=l->Next;
        
        l->Next=(SDL_FontList*)malloc(sizeof(SDL_FontList));
        l->Next->FontId    =(char*)strdup(fontid);
        l->Next->Filename  =(char*)strdup(filename);
        l->Next->Font      = NULL;
        l->Next->Next      = NULL;
    }

}

void SDL_FontDrawStringRect(SDL_Surface *dest,SDL_Font *font,
                            char *string,unsigned int color,SDL_Rect *rect)
{
    char *p;
    int xoffset=0;
    int height;
    SDL_Rect pos;
    
    if(font == NULL || string == NULL)
        return;

    font->color=color;
    height=SDL_FontGetHeight(font);

    p=string;
    
    while(*p != 0)
    {
        if(SDL_FontGetCharWidth(font,*p) + xoffset > rect->w)
            return;

        pos.x=xoffset+rect->x;
        pos.y=rect->y + ((rect->h - height)/2);
        pos.w=rect->w;
        pos.h=rect->h;
        xoffset+=SDL_FontDrawChar(dest,font,*p,&pos,NULL);
        p++;
    }
}

int SDL_FontDrawString(SDL_Surface *dest,SDL_Font *font,
                       char *string,unsigned int color,int x, int y)
{
    char *p;
    int xoffset=x;
    SDL_Rect pos;

    if(font == NULL || string == NULL)
    {
        return 0;
    }
        
    font->color=color;

    p=string;

    while(*p != 0)
    {
        pos.x=xoffset;
        pos.y=y;
        pos.w=0;
        pos.h=0;
        xoffset += SDL_FontDrawChar(dest,font,*p,&pos,NULL);
        p++;
    }
    return 1;
}

int SDL_FontDrawStringLimited(SDL_Surface *dest,SDL_Font *font,
                              char *string,unsigned int color,SDL_Rect *rect,SDL_Rect *clip)
{
    int xoffset=0;
    char *p;
    SDL_Rect pos;

    if(font == NULL || string == NULL)
    {
        return 0;
    }

    font->color=color;        

    p=string;

    while(*p != 0)
    {
        pos.x = xoffset+rect->x;
        pos.y = rect->y;
        pos.w = rect->w;
        pos.h = rect->h;
        xoffset+=SDL_FontDrawChar(dest,font,*p,&pos,clip);
        p++;
    }
    return 1;
}

int SDL_FontDrawChar(SDL_Surface *dest,SDL_Font *font,
                     char character,SDL_Rect *pos,SDL_Rect *clip)
{
    int width;
    T_Font_DrawChar DrawChar;

    DrawChar = FontTable[font->type]->drawchar;
    width=DrawChar(dest,font,character,pos,clip);
    return width;
}


int SDL_FontGetCharWidth(SDL_Font* font,char character)
{
    int width;
    if(font->type ==FNT_FONT)
    {
        FNT_Font *fnt=(FNT_Font*)font->fontdata;
        if(fnt)
        {
            character -= (char)fnt->firstchar;
            return fnt->chartable[character*4+1]<<8|fnt->chartable[character*4];
        }
        else
        {
            return 9;
        }
    }
    else
    {
        BDF_Font *fnt=(BDF_Font*)font->fontdata;
        width=fnt->bdffont[(int)character]->bbx_x;
        if(width == 0)
           return 9;
        else
            return width + 1;
    }

}



int SDL_FontGetStringWidth(SDL_Font* font,char *string)
{
    int length,width=0;
    int i;

    if(font == NULL || string == NULL)
        return 0;

    length=strlen(string);
    for(i=0;i<length;i++)
        width+=SDL_FontGetCharWidth(font,string[i]);

    return width;

}

int SDL_FontGetHeight(SDL_Font *font)
{
    if(font != NULL)
        return font->Height;
    else
        return 0;
}

SDL_Font *SDL_FontGet(char *fontid)
{
    SDL_FontList *font;

    font=LoadedFonts;

    while(font && fontid)
    {
        if(!strcmp(fontid,font->FontId))
        {
            if(font->Font==NULL)
                font->Font=SDL_FontInit(font->Filename);
             return font->Font;
        }
        font=font->Next;
    }
    return NULL;
}



