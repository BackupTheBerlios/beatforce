/*
  Beatforce/SDLTk

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





SDL_Font * SDL_FontInit(char *filename)
{
    SDL_Font *font=NULL;
    int i;        
    T_Font_IsFormat isformat;
    T_Font_Read     readfont;
    
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

void SDL_FontDrawStringRect(SDL_Surface *dest,SDL_Font *font,
                            char *string,SDL_Rect *rect)
{
    int i;
    int size;
    int xoffset=0;
    int height;
    SDL_Rect pos;
    
    if(font == NULL || string == NULL)
        return;

    height=SDL_FontGetHeight(font);
    size=strlen(string);
    
    for(i=0;i<size;i++)
    {
        if(SDL_FontGetCharWidth(font,string[i]) + xoffset > rect->w)
            return;

        pos.x=xoffset+rect->x;
        pos.y=rect->y + ((rect->h - height)/2);
        pos.w=rect->w;
        pos.h=rect->h;
        xoffset+=SDL_FontDrawChar(dest,font,string[i],&pos,NULL);
    }
}

int SDL_FontDrawString(SDL_Surface *dest,SDL_Font *font,
                       char *string,int x, int y)
{
    int i;
    int size;
    int xoffset=x;
    SDL_Rect pos;

    if(font == NULL )//|| font->font == NULL)
    {
        return 0;
    }
    if(string == NULL)
    {
        return 0;
    }
        

    size=strlen(string);

    for(i=0;i<size;i++)
    {
        pos.x=xoffset;
        pos.y=y;
        pos.w=0;
        pos.h=0;
        xoffset+=SDL_FontDrawChar(dest,font,string[i],&pos,NULL);
    }
    return 1;
}

int SDL_FontDrawStringLimited(SDL_Surface *dest,SDL_Font *font,
                              char *string,SDL_Rect *rect,SDL_Rect *clip)
{
    int i;
    int size;
    int pixelsize;
    int xoffset=0;
    SDL_Rect pos;

    if(font == NULL )
    {
        return 0;
    }
    if(string == NULL)
    {
        return 0;
    }
        
    size=strlen(string);
    pixelsize=SDL_FontGetStringWidth(font,string);

    for(i=0;i<size;i++)
    {
        pos.x = xoffset+rect->x;
        pos.y = rect->y;
        pos.w = rect->w;
        pos.h = rect->h;
        xoffset+=SDL_FontDrawChar(dest,font,string[i],&pos,clip);
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


int SDL_FontSetColor(SDL_Font *font,unsigned int color)
{
    if(font == NULL )//|| font->font == NULL)
        return 0;
    font->color=color;
    return 1;
}                         


int SDL_FontGetHeight(SDL_Font *font)
{
    return font->Height;

}




