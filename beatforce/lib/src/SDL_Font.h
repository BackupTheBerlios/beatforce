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

#ifndef SDL_FONT_H
#define SDL_FONT_H

#include <SDL/SDL.h>

typedef enum FontType
{
    FNT_FONT,
    BDF_FONT,
    LAST_FONT
}FontType;


typedef struct SDL_Font
{
    FontType type;
    void *fontdata;
    
    int height;
    unsigned int color;
}SDL_Font;


typedef int               (*T_Font_IsFormat)       (char*);
typedef int               (*T_Font_Read)           (char*,SDL_Font*);
typedef void              (*T_Font_DrawString)     (SDL_Surface *,SDL_Font *,char *,int,int);
typedef int               (*T_Font_DrawChar)       (SDL_Surface *,SDL_Font *,char,SDL_Rect*,SDL_Rect*);


struct S_Font_FunctionList
{
  
    T_Font_IsFormat         isformat;
    T_Font_Read             read;
    T_Font_DrawString       drawstring;
    T_Font_DrawChar         drawchar;
};

extern const struct S_Font_FunctionList FONT_FNT_FunctionList;
extern const struct S_Font_FunctionList FONT_BDF_FunctionList;

static const struct S_Font_FunctionList * const FontTable[] =
{
    &FONT_FNT_FunctionList,      //FONT_FNT
    &FONT_BDF_FunctionList       //FONT_BDF
};


SDL_Font* SDL_FontInit(char *filename);
int SDL_FontDrawString(SDL_Surface *,SDL_Font *font,
                       char *string,int x, int y);

int SDL_FontDrawStringLimited(SDL_Surface *dest,SDL_Font *font,
                              char *string,SDL_Rect *rect,SDL_Rect *clip);

void SDL_FontDrawStringRect(SDL_Surface *dest,SDL_Font *font,
                            char *string,SDL_Rect *rect);

int SDL_FontGetStringWidth(SDL_Font* font,char* string);
int SDL_FontSetColor(SDL_Font *font,unsigned int color);

#endif// SDL_FONT_H
