/*
  Beatforce/ Startup of beatforce

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
#include <string.h>
#include <malloc.h>
#include <ctype.h> //for isdigit

#include "SDL_Font.h"
#include "SDL_FontBDF.h"

//exported API calls
int  FONT_BDF_IsBDF(char *filename);
int FONT_BDF_Read(char *filename,SDL_Font *font);
void FONT_BDF_DrawString(SDL_Surface *screen,SDL_Font *font,char *string,int x, int y);
int FONT_BDF_DrawChar(SDL_Surface *dest,SDL_Font *font,
                     char character,SDL_Rect *pos,SDL_Rect* clip);



//local protypes
void readbdf(FILE *fp,SDL_Font *f);
void readline(FILE *fp,char *buffer);
int  drawbdfchar(SDL_Surface *screen,struct BDF_Char *ch,int x, int y,Uint32 color);

//extern prototypes
extern void DrawPixel(SDL_Surface *screen, int x, int y,unsigned int color2);

#define XVAL(x) (isdigit((x)) ? (x) - '0' : toupper((x)) - 'A' + 10)

const struct S_Font_FunctionList FONT_BDF_FunctionList =
{
    FONT_BDF_IsBDF,
    FONT_BDF_Read,
    FONT_BDF_DrawString,
    FONT_BDF_DrawChar
};


int FONT_BDF_IsBDF(char *filename)
{
    char *ext;
    ext=strrchr(filename,'.');

    if(!strcmp(".bdf",ext) || !strcmp(".BDF",ext))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int FONT_BDF_Read(char *filename,SDL_Font *font)
{
    FILE *fp;

    fp=fopen(filename,"r");
    if(fp == NULL)
    {
        fprintf(stderr,"SDL_FontBDF: Font file %s not found\n",filename);
        return 0;
    }
    readbdf(fp,font);
    font->type=BDF_FONT;
    fclose(fp);
    return 1;
}

void FONT_BDF_DrawString(SDL_Surface *screen,SDL_Font *font,char *string,int x, int y)
{

}

int FONT_BDF_DrawChar(SDL_Surface *dest,SDL_Font *font,
                      char character,SDL_Rect* pos,SDL_Rect* clip)
{

    BDF_Font *fnt=(BDF_Font*)font->fontdata;
    return drawbdfchar(dest,fnt->bdffont[(int)character],pos->x,pos->y,font->color);
}

void readbdf(FILE *fp,SDL_Font *f)
{
    char buffer[500];
    register char  *aux;

    BDF_Font *font;
    BDF_Char *current;
    
    current=NULL;
    font=malloc(sizeof(BDF_Font));
    memset(font->bdffont,0,255*sizeof(BDF_Char*));
    


    while(!feof(fp))
    {
        readline(fp,buffer);
        if(!strncmp(buffer,"STARTCHAR",9))
        {
            
        }
        else if(!strncmp(buffer,"ENCODING",8))
        {
            char *p;
            int value;

            p=buffer+8;
            while(*p == ' ')
                p++;

            sscanf(p,"%d",&value);

            if(value > 254)
                break;
            current=malloc(sizeof(BDF_Char));
            current->encoding=value;
        }
        else if(!strncmp(buffer,"BBX",3))
        {
            char *p;
            p=buffer+3;
            while(*p == ' ')
                p++;

            sscanf(p,"%d %d %d %d",&current->bbx_x, &current->bbx_y,
                                   &current->bbx_xo,&current->bbx_yo);
        }
        else if(!strncmp(buffer,"BITMAP",6))
        {

            int i;
            unsigned char *bits;
            /* wbytes is the width of the char in bytes. */
            current->wbytes = (current->bbx_x + 7) / 8;
            current->data=malloc(current->wbytes * current->bbx_y);
            memset(current->data,0,current->wbytes * current->bbx_y);
            bits=current->data;
					
            /* Read all pixels from file. */
			for (i = current->bbx_y, bits = current->data; i != 0; i--) 
            {
			    readline(fp,buffer);
                aux = buffer;
				while (aux[0] != '\0' && aux[1] != '\0') 
                {
				    *bits++ = XVAL(aux[0]) * 16 + XVAL(aux[1]);
					aux += 2;
                }
            }


        }
        else if(!strncmp(buffer,"ENDCHAR",7))
        {
            font->bdffont[current->encoding]=current;
        }
        else
        {
         
        }
    }
    f->fontdata=font;
}

void readline(FILE *fp,char *buffer)
{
    char line[255];
    
    int teller=0;

    memset(line,0,255);
    do
    {
        fread(buffer+teller,1,1,fp);
        teller++;
    }
    while(buffer[teller-1] != '\n' && !feof(fp));
        buffer[teller-1]=0;
    
}


/* Draws a char on the surface.     */
/* and return thw width of the font */
int drawbdfchar(SDL_Surface *screen,struct BDF_Char *ch,int x, int y,Uint32 color)
{
    int           xx;
    unsigned char *bits, *endfont, *endline;
    
    if(ch == NULL)
        return 0;

    y+=15;
    /* Calculate the position of the first pixel. */
    x += ch->bbx_xo;
    y -= (ch->bbx_yo + ch->bbx_y);
    bits = ch->data;
 
    /* Put them! */
    for (endfont = bits + ch->wbytes * ch->bbx_y; bits < endfont; y++)
    {
        for (endline = bits + ch->wbytes, xx = x; bits < endline; xx += 8, bits++) 
        {
            if ((*bits) & 0x80) 
                DrawPixel(screen, xx    , y, color);
            if ((*bits) & 0x40) 
                DrawPixel(screen, xx + 1, y, color);
            if ((*bits) & 0x20) 
                DrawPixel(screen, xx + 2, y, color);
            if ((*bits) & 0x10) 
                DrawPixel(screen, xx + 3, y, color);
            if ((*bits) & 0x08) 
                DrawPixel(screen, xx + 4, y, color);
            if ((*bits) & 0x04) 
                DrawPixel(screen, xx + 5, y, color);
            if ((*bits) & 0x02) 
                DrawPixel(screen, xx + 6, y, color);
            if ((*bits) & 0x01) 
                DrawPixel(screen, xx + 7, y, color);
        }
    }
    return (ch->bbx_xo  + ch->bbx_x) + 1;
}



