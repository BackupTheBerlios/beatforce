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
#include <string.h>

#include <SDL/SDL.h>

#include "SDL_Widget.h"
#include "SDL_Table.h"

#include "SDL_Slider.h"  /* used for the scrollbar */
#include "SDL_WidTool.h" /* used for background backup */ 

static void SDL_TableAttachScrollbar(SDL_Table *table);
static void SDL_TableDrawRow(SDL_Surface *dest,SDL_Table *table,int row);
static void SDL_TableAddSelected(SDL_Table *table);
static int SDL_TableIsRowSelected(SDL_Table *Table,int row);
static void Table_EditReturnKeyPressed(void *data);

const struct S_Widget_FunctionList SDL_Table_FunctionList =
{
    SDL_TableCreate,
    SDL_TableDraw,
    SDL_TableProperties,
    SDL_TableEventHandler,
    NULL
};

SDL_Widget* SDL_TableCreate(SDL_Rect* rect)
{
    SDL_Table  *Table;
    SDL_Widget *Widget;

    Table =(SDL_Table*)malloc(sizeof(SDL_Table));
    Widget            = (SDL_Widget*)Table;
    Widget->Type      = SDL_TABLE;
    Widget->Rect.x    = rect->x;
    Widget->Rect.y    = rect->y;
    Widget->Rect.w    = rect->w;
    Widget->Rect.h    = rect->h;
    Widget->Focusable = 1;

    Table->fgcolor           = BLACK;
    Table->bgcolor           = 0x00000f;
    Table->next              = NULL;
    Table->RowHeight         = 15;
    Table->ColumnWidths      = NULL;
    
    Table->VisibleRows       = 0;
    Table->VisibleColumns    = 0;
    Table->Rows              = 0;

    Table->font              = NULL;
    /* backup of the background */
    Table->Background        = NULL;

    //data retreival function
    Table->Table_GetString   = NULL;

    // event function
    Table->Clicked           = NULL;
    Table->ClickedData       = NULL;

    Table->CurrentRow        = -1;
    Table->CurrentColumn     = -1;

    /* Handle to the scollbar widget */
    Table->Scrollbar         = NULL;
    Table->ScrollbarWidth    = 0;

    Table->TablePreviousHighlightedRow = -1;
    Table->TableSelectionChanged       = 1;
    Table->TableInitialDraw            = 1; 

    Table->edit              = NULL;
    Table->ScrollbarImage    = NULL;
    Table->bgcolor           = TRANSPARANT;
   
    Table->Selected          = NULL;
    Table->SelectedCount     = 0;
    Table->SelectMode        = TABLE_MODE_NONE;
    Table->FirstVisibleRow   = 0;
    Table->HighlightedRow    = -1;
    Table->editcaption       = NULL;
    Table->ActiveEntry       = -1;
    return (SDL_Widget*)Table;
}

void SDL_TableDraw(SDL_Widget *widget,SDL_Surface *dest)
{
    SDL_Table *Table=(SDL_Table*)widget;
    int row;

    /*
     * Before the first draw we make a backup of the drawarea
     */

    if(Table->Background == NULL)
    {
        Table->Background=SDL_WidgetGetBackground(dest,&widget->Rect);
    }

    /* 
     * Check if a scrollbar is needed and if it is changed 
     */
    if(Table->Rows > Table->VisibleRows)
    {
        int width,SliderState;
        double row;
        SDL_TableAttachScrollbar(Table);


        SDL_WidgetPropertiesOf(Table->Scrollbar,GET_CUR_VALUE,&row);
        SDL_WidgetPropertiesOf(Table->Scrollbar,GET_WIDTH,&width);
        SDL_WidgetPropertiesOf(Table->Scrollbar,GET_STATE,&SliderState);

        Table->ScrollbarWidth = width;
        
        if(SliderState == SLIDER_DRAG)
        {
        }
        else
        {        
            if((int)row != Table->FirstVisibleRow)
            {
                Table->FirstVisibleRow  = (int)row;
                Table->TableInitialDraw = 1 ;
            }
        }
        
    }

   
    /*
     * Complete redraw of the entire table (== slow) 
     */
//    if(Table->TableInitialDraw || SDL_WidgetNeedsRedraw())
    {
        if(Table->bgcolor ==  TRANSPARANT)
        {
            if(SDL_BlitSurface(Table->Background,NULL,dest,&widget->Rect)<0)
                ;
        }
        else
        {
            SDL_Rect r;
//            SDL_FillRect(dest,&widget->Rect,BLACK);
            
            r.x = widget->Rect.x+3;
            r.y = widget->Rect.y+3;
            r.w = widget->Rect.w-6;
            r.h = widget->Rect.h-6;
            //            SDL_FillRect(dest,&r,Table->bgcolor);
        }

        for(row=0;row<Table->VisibleRows;row++)
        {
            SDL_TableDrawRow(dest,Table,row);
        }
        
        Table->TableInitialDraw = 0;
    }
    if(Table->edit)
    {
        SDL_WidgetPropertiesOf((SDL_Widget*)Table->edit,FORCE_REDRAW,1);
    }
}


int SDL_TableProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Table *Table=(SDL_Table*)widget;
    int column,width;
    int totalwidth,i;
    double row;

    switch(feature)
    {
    case  SET_VISIBLE_ROWS:
        Table->VisibleRows    = va_arg(list,int);
        if(widget->Rect.h / Table->RowHeight < Table->VisibleRows)
        {
            Table->VisibleRows = widget->Rect.h / Table->RowHeight;
        }
        break;
    case  SET_VISIBLE_COLUMNS:
        Table->VisibleColumns = va_arg(list,int);
        if(Table->VisibleColumns)
        {
            Table->ColumnWidths = (int*)calloc(sizeof(int)*Table->VisibleColumns,
                                               Table->VisibleColumns);
            for(i=0;i<Table->VisibleColumns;i++)
            {
                Table->ColumnWidths[i]=widget->Rect.w/Table->VisibleColumns;
            }
        }
        break;

    case SET_FG_COLOR:
        Table->fgcolor=va_arg(list,Uint32);
        break;

    case SET_BG_COLOR:
        Table->bgcolor=va_arg(list,Uint32);
        break;

    case COLUMN_WIDTH:
        if(Table->VisibleColumns <= 0)
            break;
        column = va_arg(list,int);
        column--; /* our column counting starts at zero */
        if(column > Table->VisibleColumns)
            break;
        width = va_arg(list,int);
        totalwidth=0;
        for(i=0;i<Table->VisibleColumns;i++)
        {
            totalwidth+=Table->ColumnWidths[i];
        }
        totalwidth-=Table->ColumnWidths[column];
        if(totalwidth + width <= widget->Rect.w)
        {
            Table->ColumnWidths[column]=width;
        }
        break;
        
    case ROWS:
    {
        long newrows=va_arg(list,int);
        if(newrows == Table->Rows)
            return 0;
        else
            Table->Rows=newrows;

        if((Table->Rows > Table->VisibleRows) && Table->Scrollbar)
        {
            SDL_WidgetPropertiesOf(Table->Scrollbar,SET_CUR_VALUE,0.0);
            SDL_WidgetPropertiesOf(Table->Scrollbar,SET_MAX_VALUE,(int)(Table->Rows - Table->VisibleRows));
        }
        else
        {
            if(Table->Scrollbar)
            {
                SDL_WidgetPropertiesOf(Table->Scrollbar,SET_MIN_VALUE,0);
                SDL_WidgetPropertiesOf(Table->Scrollbar,SET_CUR_VALUE,0);
                SDL_WidgetPropertiesOf(Table->Scrollbar,SET_MAX_VALUE,Table->Rows);
                Table->FirstVisibleRow = 0;
                SDL_WidgetClose(Table->Scrollbar);
                Table->Scrollbar=NULL;
            }
        }
        break;      
    }
    case SET_FONT:
        Table->font=va_arg(list,SDL_Font*);
        Table->RowHeight=SDL_FontGetHeight(Table->font)+2;
        Table->VisibleRows = widget->Rect.h / Table->RowHeight;
        break;
    case SET_DATA_RETREIVAL_FUNCTION:
        Table->Table_GetString=va_arg(list,void*);
        break;
    case SET_CALLBACK:
    {
        int event=va_arg(list,int);
        if(event == SDL_CLICKED)
        {
            Table->Clicked     = va_arg(list,void*);
            Table->ClickedData = va_arg(list,void*);
        }
        else if(event == SDL_KEYDOWN_RETURN)
        {
            //for the edit/rename widget
            Table->OnReturn = va_arg(list,void*);
        }
    }
    break;
    case GET_CAPTION:
        sprintf(va_arg(list,char*),"%s",Table->editcaption);
        break;
    case GET_SELECTED:
    {
        int *c;
        int **d=va_arg(list,int**);
        if(d)
            *d=Table->Selected;

        c=va_arg(list,int*);
        if(c)
            *c=Table->SelectedCount;
            
    }
    break;
    case SET_HIGHLIGHTED:
        Table->ActiveEntry=va_arg(list,int);
        if(Table->Scrollbar)
        {
            row=(double)Table->ActiveEntry - (Table->VisibleRows/2);
            SDL_WidgetPropertiesOf(Table->Scrollbar,SET_CUR_VALUE,row);
        }
        break;
    case CLEAR_SELECTED:
    {
        if(Table->Selected)
        {
            free(Table->Selected);
            Table->Selected=NULL;
            Table->SelectedCount=0;
        }
    }
    break;
    case SET_SELECTION_MODE:
        Table->SelectMode = va_arg(list,int);
        
        /* At least one item has to be selected */
        if(Table->SelectMode == TABLE_MODE_BROWSE)
        {
            if(Table->Selected == NULL)
            {
                Table->CurrentRow=0;
                /* Add to selected */
                SDL_TableAddSelected(Table);
            }
        }
        break;
    case SET_IMAGE:
        Table->ScrollbarImage = va_arg(list,SDL_Surface*);
        break;
    case SET_STATE_EDIT:
    {
        if(Table->edit == NULL)
        {
            int row    = va_arg(list,int);
            int column = va_arg(list,int);
            char label[255];
            SDL_Rect Dims;
                
            /* Internal count starts at 0 */
            column--;
            if(Table->Table_GetString) 
            {
                Table->Table_GetString(row,column,(char*)&label);
                 
                if(strlen(label))
                {
                    Dims.w = Table->ColumnWidths[column];
                    Dims.x = widget->Rect.x;
                    Dims.y = widget->Rect.y+Table->RowHeight * row;
                    Dims.h = Table->RowHeight;

                    Table->edit=(SDL_Edit*)SDL_WidgetCreateR(SDL_EDIT,Dims);
                    SDL_WidgetProperties(SET_FONT,Table->font);
                    SDL_WidgetProperties(SET_BG_COLOR,Table->bgcolor);
                    SDL_WidgetProperties(SET_FG_COLOR,BLACK);
                    SDL_WidgetProperties(SET_CAPTION,label);
                    SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN,Table_EditReturnKeyPressed,Table);
                }
            }
        }
    }
    break;
    }
    return 1;
}

int SDL_TableEventHandler(SDL_Widget *widget,SDL_Event *event)
{
    SDL_Table *Table=(SDL_Table*)widget;
    char string[255];
    int y;
    int i;

    Table->TablePreviousHighlightedRow = Table->HighlightedRow;
    
    switch(event->type)
    {
    case SDL_MOUSEMOTION:
        if(SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
        {
            if(event->motion.x > (widget->Rect.x + widget->Rect.w - Table->ScrollbarWidth))
            {
                /* Event are for the scrollbar */
                break;
            }
            else
            {
                y=widget->Rect.y;
                for(i=0;i<Table->VisibleRows;i++)
                {
                    if(event->motion.y > y && event->motion.y < y+Table->RowHeight)
                    {
                        if(Table->TablePreviousHighlightedRow!=i)
                            Table->TableSelectionChanged=1;
                            
                        Table->HighlightedRow=i;
                        break;
                    }
                    y+=Table->RowHeight;
                }
            }
            
        }
        else
        {
            if(Table->TablePreviousHighlightedRow != -1)
                Table->TableSelectionChanged=1;
            
            Table->HighlightedRow=-1;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if(SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
        {
            if(event->button.button == 1 || event->button.button == 3)
            {
                if(event->motion.x > (widget->Rect.x + widget->Rect.w - Table->ScrollbarWidth))
                {
                    /* Event is for the scrollbar */
                    break;
                }
                else
                {
                    Table->CurrentRow = Table->FirstVisibleRow + 
                        (event->motion.y - widget->Rect.y) / Table->RowHeight;

                    if(Table->Table_GetString) 
                    {
                        memset(string,0,255);
                        Table->Table_GetString(Table->CurrentRow,0,(char*)&string);
                     
                        if(strlen(string) && Table->SelectMode)
                        {
                            /* Add to selected */
                            SDL_TableAddSelected(Table);
                        }
                    }

                    if(Table->Clicked)
                        Table->Clicked(Table,event);
                }
            }
            
            if(event->button.button == 4) //mousehweel down
            {
                double row;
                if(Table->Scrollbar)
                {
                    SDL_WidgetPropertiesOf(Table->Scrollbar,GET_CUR_VALUE,&row);
                    row-=5;
                    SDL_WidgetPropertiesOf(Table->Scrollbar,SET_CUR_VALUE,row);
                }

            }
            if(event->button.button == 5)
            {
                double row;
                if(Table->Scrollbar)
                {
                    SDL_WidgetPropertiesOf(Table->Scrollbar,GET_CUR_VALUE,&row);
                    row+=5.0;
                    SDL_WidgetPropertiesOf(Table->Scrollbar,SET_CUR_VALUE,row);
                }
            }
        }
        break;
    case SDL_KEYDOWN:
        switch( event->key.keysym.sym ) 
        {
        case SDLK_DOWN:
            if(SDL_WidgetHasFocus(widget))
            {
                if(Table->HighlightedRow < 0)
                    Table->HighlightedRow = 0;
                else
                {
                    if(Table->HighlightedRow >=  Table->VisibleRows -1)
                    {
                        Table->HighlightedRow = Table->VisibleRows -1;
                        Table->FirstVisibleRow++;
                        SDL_WidgetPropertiesOf(Table->Scrollbar,SET_CUR_VALUE,(double)Table->FirstVisibleRow);
                    }
                    else
                    {
                        Table->HighlightedRow++;
                    }
                }
            }
            break;
        case SDLK_UP:
            if(SDL_WidgetHasFocus(widget))
            {
                if(Table->HighlightedRow < 0)
                    Table->HighlightedRow = 0;
                else
                {
                    if(Table->HighlightedRow > 0)
                        Table->HighlightedRow--;
                    else
                    {
                        Table->FirstVisibleRow--;
                        SDL_WidgetPropertiesOf(Table->Scrollbar,SET_CUR_VALUE,(double)Table->FirstVisibleRow);
                    }
                }
            }
            break;
        case SDLK_PAGEUP:
            if(Table->Scrollbar)
            {
                double row;
                SDL_WidgetPropertiesOf(Table->Scrollbar,GET_CUR_VALUE,&row);
                row-=10.0;
                SDL_WidgetPropertiesOf(Table->Scrollbar,SET_CUR_VALUE,row);
            }
            break;
        case SDLK_PAGEDOWN:
            if(Table->Scrollbar)
            {
                double row;
                SDL_WidgetPropertiesOf(Table->Scrollbar,GET_CUR_VALUE,&row);
                row+=10.0;
                SDL_WidgetPropertiesOf(Table->Scrollbar,SET_CUR_VALUE,row);
            }
            break;
        case SDLK_RETURN:
            if(Table->OnReturn)
                Table->OnReturn(Table);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
}

static void SDL_TableAttachScrollbar(SDL_Table *Table)
{
    SDL_Widget *Widget     = (SDL_Widget*)Table;
    SDL_Rect    SliderRect;

    /* 
     * Check if it is really neccesarry 
     */
    if(Table->Rows > Table->VisibleRows)
    {
        if(Table->Scrollbar == NULL)
        {
            /* 
             * Attach the Slider widget 
             */
            SliderRect.x     = Widget->Rect.x + Widget->Rect.w - 45;
            SliderRect.y     = Widget->Rect.y;
            SliderRect.h     = Widget->Rect.h;
            SliderRect.w     = 45;
            
            Table->Scrollbar = SDL_WidgetCreateR(SDL_SLIDER,SliderRect);
            if(Table->ScrollbarImage)
                SDL_WidgetProperties(SET_BUTTON_IMAGE,Table->ScrollbarImage);
            SDL_WidgetProperties(SET_MAX_VALUE,(int)(Table->Rows - Table->VisibleRows));
            SDL_WidgetProperties(SET_MIN_VALUE,0);
            /* Use the background of the table */
            SDL_WidgetProperties(STOREBACKGROUND,0);
//            SDL_WidgetProperties(SET_CUR_VALUE,(double)(Table->Rows - Table->VisibleRows));
        }
    }
}





static void SDL_TableDrawRow(SDL_Surface *dest,SDL_Table *Table,int row)
{
    SDL_Widget *Widget=(SDL_Widget*)Table;
    SDL_Rect RowDims;
    SDL_Rect la;
    int column;
    char string[255];
    
    if(row > Table->VisibleRows)
        return;

    memset(string,0,255);
    RowDims.w = Widget->Rect.w;// - Table->ScrollbarWidth;
    RowDims.x = Widget->Rect.x;
    RowDims.y = Widget->Rect.y + Table->RowHeight * row;
    RowDims.h = Table->RowHeight;

    if(row == Table->HighlightedRow)
    {
        SDL_FillRect(dest,&RowDims,SDL_MapRGB(dest->format,255,55,0));
        SDL_FontSetColor(Table->font,Table->fgcolor);
    }
    else if(Table->SelectMode && SDL_TableIsRowSelected(Table,row))
    {
        SDL_FillRect(dest,&RowDims,0x00ff00);
    }
    else
    {
        if(Table->bgcolor == TRANSPARANT)
        {
            la.x = 0;
            la.y = RowDims.y + 45;
            la.w = RowDims.w;
            la.h = RowDims.h;

            if(SDL_BlitSurface(Table->Background,&la,dest,&RowDims)<0)
                fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
        }
        else
            SDL_FillRect(dest,&RowDims,Table->bgcolor);

        SDL_FontSetColor(Table->font,Table->fgcolor);
    }

    if(Table->ActiveEntry == row + Table->FirstVisibleRow)
    {
        SDL_FontSetColor(Table->font,0x0000ff);
    }


    for(column=0; column<Table->VisibleColumns; column++)
    {
        /* 
         *   Check if we have a data retreival function 
         */
        if(Table->Table_GetString) 
        {
            Table->Table_GetString(row + Table->FirstVisibleRow,column,(char*)&string);
           
            la.x = RowDims.x + 1;
            la.y = RowDims.y + 1;
            la.w = Table->ColumnWidths[column] - 2;
            la.h = RowDims.h - 2;

            //     SDL_FillRect(dest,&la,SDL_MapRGB(dest->format,155,155,155));

            if(strlen(string))
            {
                SDL_FontDrawStringRect(dest,Table->font,string,&la);
            }
            RowDims.x +=Table->ColumnWidths[column];
        }
        
    }
}


static void t(SDL_Table *table)
{
    int i;
    int j=0;

    for(i=0;i<table->SelectedCount;i++)
    {
        if(table->Selected[i]==-1)
        {
            for(j=i;j<table->SelectedCount;j++)
                table->Selected[j]=table->Selected[j+1];
            break;
        }
    }
    table->SelectedCount--;
    table->Selected=realloc(table->Selected,table->SelectedCount*sizeof(int));

}



static void SDL_TableAddSelected(SDL_Table *table)
{
    int i=0;

    if(table->Selected == NULL)
    {
        table->Selected = malloc(sizeof(int*));
        table->Selected[0] = table->CurrentRow;
        table->SelectedCount = 1;
    }
    else
    {
        for(i=0;i<table->SelectedCount;i++)
        {
            if(table->Selected[i] == table->CurrentRow)
            {
                /* Remove the item when it is not in browse mode 
                   in browse mode at least one has to be selected */
                if(table->SelectMode != TABLE_MODE_BROWSE)
                { 
                    table->Selected[i]=-1;
                    t(table);
                    return;
                }
            }
        }

        if(table->SelectMode == TABLE_MODE_MULTIPLE)
        {
            table->SelectedCount++;
            table->Selected=realloc(table->Selected,table->SelectedCount*sizeof(int*));
            table->Selected[table->SelectedCount-1]=table->CurrentRow;
        }
        else if (table->SelectMode == TABLE_MODE_BROWSE)
        {
            table->Selected[0]=table->CurrentRow;
        }
        
    }
}


static int SDL_TableIsRowSelected(SDL_Table *Table,int row)
{
    int i;
    for(i=0;i<Table->SelectedCount;i++)
    {
        if(Table->Selected[i] == (row + Table->FirstVisibleRow))
            return 1;
    }
    return 0;
}

static void Table_EditReturnKeyPressed(void *data)
{
    SDL_Table *Table=(SDL_Table*)data;
    char string[255];

    SDL_WidgetPropertiesOf((SDL_Widget*)Table->edit,GET_CAPTION,&string);
    SDL_WidgetClose(Table->edit);
    free(Table->edit);
    Table->edit=NULL;

    if(Table->editcaption)
        free(Table->editcaption);
    Table->editcaption = strdup(string);

    if(Table->OnReturn)
        Table->OnReturn();

    
}
