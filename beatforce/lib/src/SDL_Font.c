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
                     char character,SDL_Rect pos);
void DrawPixel(SDL_Surface *screen, int x, int y,unsigned int color2);
int SDL_FontGetCharWidth(SDL_Font* font,char character);

extern SDL_Surface *screen;





SDL_Font * SDL_FontInit(char *filename)
{
    SDL_Font *font;
    int i;        
    T_Font_IsFormat isformat;
    T_Font_Read     readfont;
    
    font=malloc(sizeof(SDL_Font));
    memset(font,0,sizeof(SDL_Font));
    
    for(i=0;i<LAST_FONT;i++)
    {
        isformat = FontTable[i]->isformat;
        if(isformat(filename))
            break;
    }

    readfont = FontTable[i]->read;
    readfont(filename,font);
      
    if(font)
        font->color=0xfefefe;
 
    return font;
}

void SDL_FontDrawStringRect(SDL_Surface *dest,SDL_Font *font,
                            char *string,SDL_Rect *rect)
{
    int i;
    int size;
    int xoffset=0;
    SDL_Rect pos;
    
    if(font == NULL )
    {
        return;
    }
    size=strlen(string);
    
    for(i=0;i<size;i++)
    {
        if(SDL_FontGetCharWidth(font,string[i]) + xoffset > rect->w)
            return;
        pos.x=xoffset+rect->x;
        pos.y=rect->y;
        pos.w=rect->w;
        pos.h=rect->h;
        xoffset+=SDL_FontDrawChar(dest,font,string[i],pos);
    }
}

void SDL_FontDrawString(SDL_Surface *dest,SDL_Font *font,
                        char *string,int x, int y)
{
    int i;
    int size;
    int xoffset=x;
    SDL_Rect pos;

    if(font == NULL )//|| font->font == NULL)
    {
        printf("No proper font\n");
        return;
    }
    if(string == NULL)
    {
        printf("Invalid string\n");
        return;
    }
        

    size=strlen(string);

    for(i=0;i<size;i++)
    {
        pos.x=xoffset;
        pos.y=y;
        pos.w=0;
        pos.h=0;
        xoffset+=SDL_FontDrawChar(dest,font,string[i],pos);
    }
}

int SDL_FontDrawChar(SDL_Surface *dest,SDL_Font *font,
                     char character,SDL_Rect pos)
{
    int width;
    T_Font_DrawChar DrawChar;

    width = SDL_FontGetCharWidth(font,character);
    
    DrawChar = FontTable[font->type]->drawchar;
    DrawChar(dest,font,character,pos.x,pos.y);
    return width;
}


int SDL_FontGetCharWidth(SDL_Font* font,char character)
{
    int width;

    if(font == NULL || font->fontdata == NULL)
        return 0;

    if(font->type ==FNT_FONT)
    {
        FNT_Font *fnt=(FNT_Font*)font->fontdata;
        character -= (char)fnt->firstchar;
        return fnt->chartable[character*4+1]<<8|fnt->chartable[character*4];
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

void DrawPixel(SDL_Surface *screen, int x, int y,unsigned int color2)
{

    Uint32 color=SDL_MapRGB(screen->format,(color2&0xff0000)>>16,(color2&0x00ff00)>>8,(color2&0x0000ff));
//    Uint32 color=SDL_MapRGB(screen->format,253,253,253);
    if(y > screen->h)
        return;
    if(x > screen->w)
        return;

//    if ( SDL_MUSTLOCK(screen) ) {
//        if ( SDL_LockSurface(screen) < 0 ) {
//            return;
//        }
//    }
    switch (screen->format->BytesPerPixel) {
    case 1: { /* Assuming 8-bpp */
        Uint8 *bufp;

        bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
        *bufp = color;
    }
    break;

    case 2: { /* Probably 15-bpp or 16-bpp */
        Uint16 *bufp;

        bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
        *bufp = color;
    }
    break;

    case 3: { /* Slow 24-bpp mode, usually not used */
        Uint8 *bufp;

        bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
        if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
            bufp[0] = color;
            bufp[1] = color >> 8;
            bufp[2] = color >> 16;
        } else {
            bufp[2] = color;
            bufp[1] = color >> 8;
            bufp[0] = color >> 16;
        }
    }
    break;

    case 4: { /* Probably 32-bpp */
        Uint32 *bufp;

        bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
        *bufp = color;
    }
    break;
    }
//    if ( SDL_MUSTLOCK(screen) ) {
//        SDL_UnlockSurface(screen);
//    }
//    SDL_UpdateRect(screen, x, y, 1, 1);
}





