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
#include "SDL_Edit.h"

typedef struct SDL_TableRow
{
    int index;
    struct SDL_TableRow *next;
    struct SDL_TableRow *prev;

}SDL_TableRow;

typedef struct SDL_Table
{
    SDL_Rect            rect;
    SDL_TableRow        *selected;
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

    int Selectable;

    //helper variables
    int TablePreviousHighlightedRow;
    int TableSelectionChanged;
    int TableInitialDraw;

    //event handler functions
    void (*Clicked)(void*);
    void *ClickedData;
    void (*OnReturn) ();

    //functions to retreive data
    char *(*Table_GetString)  (long row,int column,char* dest);

    SDL_Surface *Background;

    SDL_Edit *edit;

    char *editcaption;
    void* Scrollbar;
    int   ScrollbarWidth;

    struct SDL_Table    *next;
}SDL_Table;


void* SDL_TableCreate(SDL_Rect* rect);
void  SDL_TableDraw (void *table,SDL_Surface *dest);
int  SDL_TableProperties(void *table,int feature,va_list list);

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


void  SDL_TableEventHandler(void *table,SDL_Event *event);

