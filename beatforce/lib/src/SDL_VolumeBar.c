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
#include <stdarg.h>
#include <malloc.h>

#include "SDL_Widget.h"
#include "SDL_VolumeBar.h"


/* Prototypes for Internal functions */
void VolumeBar_DrawStripe(SDL_Surface *dest,SDL_VolumeBar *VolumeBar,int line,Uint32 color);

/* Exported data */
const struct S_Widget_FunctionList SDL_VolumeBar_FunctionList =
{
    SDL_VolumeBarCreate,
    SDL_VolumeBarDraw,
    SDL_VolumeBarProperties,
    SDL_VolumeBarEventHandler,
    NULL,
};


void* SDL_VolumeBarCreate(SDL_Rect* rect)
{
    SDL_VolumeBar *volumebar;

    volumebar=(SDL_VolumeBar*)malloc(sizeof(SDL_VolumeBar));

    volumebar->rect.x  = rect->x;
    volumebar->rect.y  = rect->y;
    volumebar->rect.w  = rect->w;
    volumebar->rect.h  = rect->h;

    volumebar->color1  = 0x790000;
    volumebar->color2  = 0xfa0000;
    volumebar->color3  = 0x007c00;
    volumebar->color4  = 0x00fe00;

    /* Default range settings */
    volumebar->MaxValue      = 100;
    volumebar->MinValue      = 0;
    volumebar->CriticalValue = 70;
    volumebar->CurrentValue  = 0;

    /* Used for drawing */
    volumebar->CriticalLine  = (volumebar->CriticalValue * volumebar->rect.h) / volumebar->MaxValue;
    volumebar->CurrentLine   = 0;
    
    volumebar->Redraw  = 1;

    return volumebar;
}

void SDL_VolumeBarDraw(void *volumebar,SDL_Surface *dest)
{
    SDL_VolumeBar *VolumeBar=(SDL_VolumeBar*)volumebar;
    int line;
    int highlight=0;

    if(VolumeBar->Redraw)
    {
        SDL_FillRect(dest,&VolumeBar->rect,0x000000);

        for( line=0; line < VolumeBar->rect.h ; line++)
        {
            highlight= (line < VolumeBar->CurrentLine);

            if(line % 2  != 0)
            {
                if(line > VolumeBar->CriticalLine)
                {
                    if(highlight)
                        VolumeBar_DrawStripe(dest,VolumeBar,line,VolumeBar->color2);
                    else
                        VolumeBar_DrawStripe(dest,VolumeBar,line,VolumeBar->color1);
                    
                }
                else
                {
                    if(highlight)
                        VolumeBar_DrawStripe(dest,VolumeBar,line,VolumeBar->color4);
                    else
                        VolumeBar_DrawStripe(dest,VolumeBar,line,VolumeBar->color3);
                }
            }
        }
        SDL_UpdateRect(dest,
                       VolumeBar->rect.x,
                       VolumeBar->rect.y,
                       VolumeBar->rect.w,
                       VolumeBar->rect.h);

        VolumeBar->Redraw = 0;
    }
}

void  SDL_VolumeBarProperties(void *volumebar,int feature,va_list list)
{
    SDL_VolumeBar *VolumeBar=(SDL_VolumeBar*)volumebar;
    double val;

    switch(feature)
    {
    case SET_MAX_VALUE:
        break;
    case SET_MIN_VALUE:
        break;
    case SET_CUR_VALUE:
        val=va_arg(list,double);
        VolumeBar->CurrentValue = val; 
        VolumeBar->CurrentLine  = (val * VolumeBar->rect.h) / (VolumeBar->MaxValue - VolumeBar->MinValue); 
        VolumeBar->Redraw       = 1;
        break;
    }
}

void SDL_VolumeBarEventHandler(void *volumebar,SDL_Event *event)
{


}


/* Internal Helper functions */


void VolumeBar_DrawStripe(SDL_Surface *dest,SDL_VolumeBar *VolumeBar,int line,Uint32 color)
{
    SDL_Rect rect;

    rect.x = VolumeBar->rect.x;
    rect.y = VolumeBar->rect.y + VolumeBar->rect.h - line;
    rect.w = VolumeBar->rect.w;
    rect.h = 1;
    
    SDL_FillRect(dest,&rect,color);

}

