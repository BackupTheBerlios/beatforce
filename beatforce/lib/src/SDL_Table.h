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

#ifndef __SDL_TABLE_H__
#define __SDL_TABLE_H__

#include <SDL/SDL.h>

#include "SDL_Widget.h"
#include "SDL_Font.h"
#include "SDL_Edit.h"


typedef enum
{
    TABLE_MODE_NONE,       /* No selection possible             */
    TABLE_MODE_BROWSE,     /* Only one can be selected          */
    TABLE_MODE_MULTIPLE    /* Multiple entries can be selected  */
}E_TableMode;


typedef struct SDL_TableCell
{
    char *String;
}SDL_TableCell;

typedef struct SDL_TableRow
{
    SDL_TableCell *Cell;
    struct SDL_TableRow *Next;
}SDL_TableRow;

typedef struct SDL_TableColumn
{
    int Width;
    char *Title;
    SDL_Widget *Button;
}SDL_TableColumn;

typedef struct SDL_Table
{
    SDL_Widget          Widget;
    SDL_Font            *font;
    char                *caption;    
    int                 *Selected;
    int                 SelectedCount;
    
    //colors
    Uint32   bgcolor;
    Uint32   fgcolor;
    Uint32   sel_bg_color;
    Uint32   sel_fg_color;

    //number of ..
    int Rows;
    int Columns;


    SDL_TableColumn   *Column;
    SDL_TableRow      *RowData;

    int RowHeight;

    int VisibleRows;
    int FirstVisibleRow;
    int HighlightedRow; // Where the mouse is on
    int ActiveEntry;

    int CurrentRow;
    int CurrentColumn;

    E_TableMode SelectMode;

    //helper variables
    int TablePreviousHighlightedRow;
    int TableSelectionChanged;

    //event handler functions
    void (*Clicked)(void* data,SDL_Event *event);
    void *ClickedData;
    void (*OnReturn) ();

    SDL_Widget *edit;

    char *editcaption;

    int ButtonHeight;

    /* Scrollbar */
    void* Scrollbar;
    int   ScrollbarWidth;
    SDL_Surface *ScrollbarImage;

    struct SDL_Table    *next;
}SDL_Table;


SDL_Widget* SDL_TableCreate(SDL_Rect* rect);
void        SDL_TableDraw (SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area);
int         SDL_TableProperties(SDL_Widget *widget,int feature,va_list list);

/*
  Currently implemented properties:

  required:
    SET_FONT:            Set the font
    SET_VISIBLE_ROWS:    Set the number of visible rows.
                         If the number of rows is larger than the display area
                         the number of visible rows is adjusted.
    SET_VISIBLE_COLUMNS: Set the number of visible columns
    
    ROWS:                The number of rows which are in the data set.
                         If larger than the number of visible rows a scrollbar
                         will be attached.
    
    SET_DATA_RETREIVAL_FUNCTION:
                         Set a function which is called when the table draw function
                         needs data for the current row and column.
                         PROTOTYPE: char GetString(long row,int column,char* dest);
                         dest is allocated by the table and this is where the data
                         has to be copied into.

  optional:
    SET_FG_COLOR:        Sets the color of the font
    SET_BG_COLOR         Sets the background color of the entire table 
                         (if set to transparant the background is used)
    COLUMN_WIDTH:        Sets the width in pixels for the column.
                         Requires two parametets: column number and pixel width
    SET_CALLBACK:        Curently a callback can be set to the SDL_CLICKED event.

*/


int  SDL_TableEventHandler(SDL_Widget *widget,SDL_Event *event);


/* Modification functions */
void SDL_TableAddRow(SDL_Widget *Table,char *Titles[]);

#endif /* __SDL_TABLE_H__ */
