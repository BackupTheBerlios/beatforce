/*
  Beatforce/SDLTk

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003 John Beuving (john.beuving@beatforce.org)

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

#ifndef __SDL_FONT_H__
#define __SDL_FONT_H__

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
   
    int Height;
    unsigned int color;
}SDL_Font;

typedef struct SDL_FontList
{
    struct SDL_Font     *Font;
    char *FontId;
    char *Filename;
    struct SDL_FontList *Next;
}SDL_FontList;


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


SDL_Font *SDL_FontGet(char *fontid);

int SDL_FontDrawString(SDL_Surface *,SDL_Font *font,
                       char *string,unsigned int color,int x, int y);

int SDL_FontDrawStringLimited(SDL_Surface *dest,SDL_Font *font,
                              char *string,unsigned int color,SDL_Rect *rect,SDL_Rect *clip);

void SDL_FontDrawStringRect(SDL_Surface *dest,SDL_Font *font,
                            char *string,unsigned int color,SDL_Rect *rect);

int SDL_FontGetStringWidth(SDL_Font* font,char* string);
int SDL_FontGetHeight(SDL_Font *font);

void SDL_FontLoad(char *fontid,char *filename);



extern SDL_Font DefaultFont;

#endif /* __SDL_FONT_H__ */
