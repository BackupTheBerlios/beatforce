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
void FONT_BDF_Read(char *filename,SDL_Font *font);
void FONT_BDF_DrawString(SDL_Surface *screen,SDL_Font *font,char *string,int x, int y);
void FONT_BDF_DrawChar(SDL_Surface *screen,SDL_Font *font,char character,int x, int y);

//local protypes
void readbdf(FILE *fp,SDL_Font *f);
void readline(FILE *fp,char *buffer);
void drawbdfchar(SDL_Surface *screen,struct BDF_Char *ch,int x, int y,Uint32 color);

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

void FONT_BDF_Read(char *filename,SDL_Font *font)
{
    FILE *fp;

    fp=fopen(filename,"r");
    if(fp == NULL)
    {
        fprintf(stderr,"SDL_Font: Font file %s not found\n",filename);
        return;
    }
    readbdf(fp,font);
    font->type=BDF_FONT;
//    fclose(fp);
}

void FONT_BDF_DrawString(SDL_Surface *screen,SDL_Font *font,char *string,int x, int y)
{

}

void FONT_BDF_DrawChar(SDL_Surface *screen,SDL_Font *font,char character,int x, int y)
{

    BDF_Font *fnt=(BDF_Font*)font->fontdata;
    drawbdfchar(screen,fnt->bdffont[(int)character],x,y,font->color);
}

void readbdf(FILE *fp,SDL_Font *f)
{
    char buffer[255];

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

            if(value > 255)
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
            current->data=malloc(12*2*sizeof(char));
            memset(current->data,0,24);
            bits=current->data;

            for(i=0;i<current->bbx_y;i++)
            {
                readline(fp,buffer);
                *bits++=XVAL(buffer[0])*16+XVAL(buffer[1]);
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


/* Draws a char on the surface. */
void drawbdfchar(SDL_Surface *screen,struct BDF_Char *ch,int x, int y,Uint32 color)
{
    int           xx;
    unsigned char *bits, *endfont, *endline;
    
    y+=12;
    /* Calculate the position of the first pixel. */
    x += ch->bbx_xo;
    y -= (ch->bbx_yo + ch->bbx_y);
    bits = ch->data;
 
    /* Put them! */
    for (endfont = bits + /*chr->wbytes*/ 1 * ch->bbx_y; bits < endfont; y++)
    {
        for (endline = bits + /*chr->wbytes*/ 1, xx = x; bits < endline; xx += 8, bits++) 
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
}


#if 0
void drawbdfchar(SDL_Surface *screen,struct BDF_Char *ch,int x, int y,Uint32 color)
{
        int i;
        int bit=0;
        unsigned char *dat=ch->data;

        int xo = ch->bbx_xo;
        int yo = ch->bbx_yo;


        xo+=x;
        yo+=y;

        if(ch->encoding == 53 || ch->encoding == 65)
        {
            printf("x %d y %d\n",x,y);
        }

        dat+=ch->bbx_y;
        for(i=0;i<ch->bbx_y;i++)
        {
            if(dat[0]&0x80)
                DrawPixel(screen,xo+0, yo+i, color);
            if(dat[0]&0x40)
                DrawPixel(screen,xo+1, yo+i, color);
            if(dat[0]&0x20)
                DrawPixel(screen,xo+2, yo+i, color);
            if(dat[0]&0x10)
                DrawPixel(screen,xo+3, yo+i, color);
            if(dat[0]&0x08)
                DrawPixel(screen,xo+4, yo+i, color);
            if(dat[0]&0x04)
                DrawPixel(screen,xo+5, yo+i, color);
            if(dat[0]&0x02)
                DrawPixel(screen,xo+6, yo+i, color);
            if(dat[0]&0x01)
                DrawPixel(screen,xo+7, yo+i, color);
                
            dat--;
        }

}
#endif
