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

#include <SDL/SDL.h>

#include "SDL_Widget.h"
#include "SDL_Font.h"

typedef struct SDL_TableRow
{
    char **data;
    struct SDL_TableRow *next;
    struct SDL_TableRow *prev;

}SDL_TableRow;

typedef struct SDL_Table
{
    SDL_Rect            rect;
    SDL_TableRow        *rowdata;
    SDL_Font            *font;
    char                *caption;    
    
    //colors
    Uint32   bgcolor;
    Uint32   fgcolor;
    Uint32   sel_bg_color;
    Uint32   sel_fg_color;

    //number of ..
    int Rows;
    int Columns;

    int RowHeight;
    int *ColumnWidths;

    int VisibleRows;
    int VisibleColumns;
    int FirstVisibleRow;
    int HighlightedRow; // Where the mouse is on
    int ActiveEntry;

    int CurrentRow;
    int CurrentColumn;

    //helper variables
    int TablePreviousHighlightedRow;
    int TableSelectionChanged;
    int TableInitialDraw;

    //event handler functions
    void (*Clicked)(void*);

    //functions to retreive data
    char *(*Table_GetString)  (long,int,char*);
//    int (*Table_NeedRedraw)   ();

//    SDL_Rect            ScrollbarRect;
//    SDL_Scrollbar       *Scrollbar;

    SDL_Surface *Background;

    void* Scrollbar;
    int   ScrollbarWidth;

    struct SDL_Table    *next;
}SDL_Table;


void* SDL_TableCreate(SDL_Rect* rect);
void  SDL_TableDraw (void *table,SDL_Surface *dest);
void  SDL_TableProperties(void *table,int feature,va_list list);
void  SDL_TableEventHandler(void *table,SDL_Event *event);
void  SDL_TableSetCallback(void* table,void *function,E_Widget_Event event);
