/*
  Beatforce/SDLTk

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003-2004 John Beuving (john.beuving@wanadoo.nl)

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

#include "SDL_Scrollbar.h"  /* used for the scrollbar */
#include "SDL_WidTool.h" /* used for background backup */ 
#include "SDL_Signal.h"

static void SDL_TableAttachScrollbar(SDL_Table *table);
static void SDL_TableDrawRow(SDL_Surface *dest,SDL_Table *Table,SDL_TableRow *Row,int row);
static void SDL_TableAddSelected(SDL_Table *table);
static int SDL_TableIsRowSelected(SDL_Table *Table,int row);
static void Table_EditReturnKeyPressed(void *data);
void ScrollBarChanged(SDL_Widget *widget);
void SDL_TableHideCB(SDL_Widget *widget); 
void SDL_TableShowCB(SDL_Widget *widget); 
static void SDL_TableEditActivateCB(SDL_Widget *widget);

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
    memset(Table,0,sizeof(SDL_Table));


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

    Table->Column            = NULL;
    Table->RowData           = NULL;
    
    Table->VisibleRows       = 0;

    Table->Columns           = 0;
    Table->Rows              = 0;

    Table->font              = &DefaultFont;

    Table->CurrentRow        = -1;
    Table->CurrentColumn     = -1;

    /* Handle to the scollbar widget */
    Table->Scrollbar         = NULL;
    Table->ScrollbarWidth    = 0;

    Table->TablePreviousHighlightedRow = -1;
    Table->TableSelectionChanged       = 1;

    Table->Edit              = NULL;
    Table->EditCell          = NULL;
    Table->ScrollbarImage    = NULL;
    Table->bgcolor           = TRANSPARANT;
    Table->ButtonHeight      = 0;

    Table->Selected          = NULL;
    Table->SelectedCount     = 0;
    Table->SelectMode        = TABLE_MODE_NONE;
    Table->FirstVisibleRow   = 0;
    Table->HighlightedRow    = -1;
    Table->editcaption       = NULL;
    Table->ActiveEntry       = -1;

    SDL_SignalConnect(Widget,"hide",SDL_TableHideCB,Widget);
    SDL_SignalConnect(Widget,"show",SDL_TableShowCB,Widget);
    return (SDL_Widget*)Table;
}

void SDL_TableDraw(SDL_Widget *widget,SDL_Surface *dest,SDL_Rect *Area)
{
    SDL_Table *Table=(SDL_Table*)widget;
    int row;

    if(Table->Scrollbar)
        Table->FirstVisibleRow = SDL_ScrollbarGetCurrentValue(Table->Scrollbar);
    /*
     * Complete redraw of the entire table
     */
    {
        if(Table->bgcolor !=  TRANSPARANT)
        {
            SDL_Rect r;
            
            r.x = widget->Rect.x;
            r.y = widget->Rect.y;
            r.w = widget->Rect.w;
            r.h = widget->Rect.h;
            SDL_FillRect(dest,&r,Table->bgcolor);
        }

        if(Table->RowData)
        {
            SDL_TableRow *tmp;
            int fv=Table->FirstVisibleRow;
            int i;
            tmp=Table->RowData;


            row=0;
            while(fv)
            {
                tmp=tmp->Next;
                fv--;
            }

            for(i=0;i<Table->VisibleRows;i++)
            {
                SDL_TableDrawRow(dest,Table,tmp,row);
                tmp=tmp->Next;
                row++;
                if(tmp == NULL)
                    break;
            }
        }
    }
}


int SDL_TableProperties(SDL_Widget *widget,int feature,va_list list)
{
    SDL_Table *Table=(SDL_Table*)widget;
    int i;
    double row;

    switch(feature)
    {

    case  SET_VISIBLE_COLUMNS:
        Table->Columns = va_arg(list,int);
        if(Table->Columns)
        {
            Table->Column = calloc(sizeof(SDL_TableColumn)*Table->Columns,
                                       Table->Columns);
            for(i=0;i<Table->Columns;i++)
            {
                Table->Column[i].Width  = widget->Rect.w/Table->Columns;
               
            }
        }
        break;

    case SET_FG_COLOR:
        Table->fgcolor=va_arg(list,Uint32);
        break;

    case SET_BG_COLOR:
        Table->bgcolor=va_arg(list,Uint32);
        break;

    case SET_FONT:
        Table->font=va_arg(list,SDL_Font*);
        Table->RowHeight = SDL_FontGetHeight(Table->font)+2;
        Table->VisibleRows = (widget->Rect.h - Table->ButtonHeight) / Table->RowHeight;
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
        if(Table->SelectMode == TABLE_MODE_SINGLE)
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
    }
    return 1;
}

int SDL_TableEventHandler(SDL_Widget *widget,SDL_Event *event)
{
    SDL_Table *Table=(SDL_Table*)widget;
    int y;
    int i;

    Table->TablePreviousHighlightedRow = Table->HighlightedRow;
    
    switch(event->type)
    {
    case SDL_MOUSEMOTION:
        if(SDL_WidgetIsInside(widget,event->motion.x,event->motion.y))
        {
            if(Table->Scrollbar && (event->motion.x > (widget->Rect.x + widget->Rect.w - Table->ScrollbarWidth)))
            {
                /* Event are for the scrollbar */
                break;
            }
            else if(event->motion.y > (widget->Rect.y + Table->ButtonHeight))
            {
                y=widget->Rect.y + Table->ButtonHeight;
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
            else
            {
                if(Table->TablePreviousHighlightedRow != -1)
                    Table->TableSelectionChanged=1;
            
                Table->HighlightedRow=-1;
            }
            
        }
        else
        {
            if(Table->TablePreviousHighlightedRow != -1)
                Table->TableSelectionChanged=1;
            
            Table->HighlightedRow=-1;
        }
        if(Table->TableSelectionChanged)
        {
            SDL_Rect r;
            if(Table->Scrollbar)
            {
                r.x = widget->Rect.x;
                r.y = widget->Rect.y;
                r.w = widget->Rect.w - Table->ScrollbarWidth;
                r.h = widget->Rect.h;
                SDL_WidgetDraw(widget,&r);
            }
            else
                SDL_WidgetDraw(widget,&widget->Rect);
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
                else if(event->motion.y > (widget->Rect.y + Table->ButtonHeight))
                {
                    Table->CurrentRow = Table->FirstVisibleRow + 
                        (event->motion.y - (widget->Rect.y+Table->ButtonHeight)) / Table->RowHeight;

                    if(Table->SelectMode)
                    {
                        /* Add to selected */
                        SDL_TableAddSelected(Table);
                    }
                    SDL_SignalEmit(widget,"select-row");
                }
            }
            
            if(event->button.button == 4) //mousehweel down
            {
                int row;
                if(Table->Scrollbar)
                {
                    row=SDL_ScrollbarGetCurrentValue(Table->Scrollbar);
                    row--;
                    SDL_ScrollbarSetCurrentValue(Table->Scrollbar,row);
                    SDL_WidgetDraw(widget,&widget->Rect);
                }

            }
            if(event->button.button == 5)
            {
                int row;
                if(Table->Scrollbar)
                {
                    row=SDL_ScrollbarGetCurrentValue(Table->Scrollbar);
                    row++;
                    SDL_ScrollbarSetCurrentValue(Table->Scrollbar,row);
                    SDL_WidgetDraw(widget,&widget->Rect);
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
                        if(Table->Scrollbar != NULL)
                        {
                            Table->FirstVisibleRow++;
                            SDL_ScrollbarSetCurrentValue(Table->Scrollbar,Table->FirstVisibleRow);
                        }
                    }
                    else
                    {
                        Table->HighlightedRow++;
                    }
                }
                SDL_WidgetDraw(widget,&widget->Rect);
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
                        if(Table->Scrollbar != NULL)
                        {
                            Table->FirstVisibleRow--;
                            SDL_ScrollbarSetCurrentValue(Table->Scrollbar,Table->FirstVisibleRow);
                        }
                    }
                }
                SDL_WidgetDraw(widget,&widget->Rect);
            }
            break;
        case SDLK_HOME:
            if(SDL_WidgetHasFocus(widget))
            {
                Table->HighlightedRow  = 0;
                Table->FirstVisibleRow = 0;
                SDL_ScrollbarSetCurrentValue(Table->Scrollbar,Table->FirstVisibleRow);
                SDL_WidgetDraw(widget,&widget->Rect);
            }
            break;
        case SDLK_END:
            if(SDL_WidgetHasFocus(widget))
            {
                Table->HighlightedRow  = Table->VisibleRows - 1;
                Table->FirstVisibleRow = Table->Rows - Table->VisibleRows - 1;
                SDL_ScrollbarSetCurrentValue(Table->Scrollbar,Table->FirstVisibleRow);
                SDL_WidgetDraw(widget,&widget->Rect);
            }
            break;
        case SDLK_PAGEUP:
            if(Table->Scrollbar)
            {
                int row;
                row=SDL_ScrollbarGetCurrentValue(Table->Scrollbar);
                row-=10;
                SDL_ScrollbarSetCurrentValue(Table->Scrollbar,row);
                SDL_WidgetDraw(widget,&widget->Rect);
            }
            break;
        case SDLK_PAGEDOWN:
            if(Table->Scrollbar)
            {
                int row;
                row=SDL_ScrollbarGetCurrentValue(Table->Scrollbar);
                row+=10;
                SDL_ScrollbarSetCurrentValue(Table->Scrollbar,row);
                SDL_WidgetDraw(widget,&widget->Rect);
            }
            break;
        case SDLK_RETURN:
            SDL_SignalEmit(widget,"activate");
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
            Table->ScrollbarWidth = 10;

            SliderRect.x     = Widget->Rect.x + Widget->Rect.w - Table->ScrollbarWidth;
            SliderRect.y     = Widget->Rect.y;
            SliderRect.h     = Widget->Rect.h;
            SliderRect.w     = Table->ScrollbarWidth;
            
            
            Table->Scrollbar = SDL_WidgetCreateR(SDL_SCROLLBAR,SliderRect);

            SDL_ScrollbarSetCallback(Table->Scrollbar,ScrollBarChanged,Widget);
            
            SDL_WidgetHide(Table->Scrollbar);
        }
        else
        {
            SDL_ScrollbarSetMaxValue(Table->Scrollbar,Table->Rows - Table->VisibleRows);
        }
    }
}





static void SDL_TableDrawRow(SDL_Surface *dest,SDL_Table *Table,SDL_TableRow *Row, int row)
{
    SDL_Widget *Widget=(SDL_Widget*)Table;
    SDL_Rect RowDims;
    SDL_Rect la;
    int column;
    char string[255];
    

    memset(string,0,255);
    RowDims.w = Widget->Rect.w - Table->ScrollbarWidth;
    RowDims.x = Widget->Rect.x;
    RowDims.y = Table->ButtonHeight + Widget->Rect.y + Table->RowHeight * row; 
    RowDims.h = Table->RowHeight;

    if(RowDims.y > Widget->Rect.y + Widget->Rect.h)
    {
        printf("SDL_TableDrawRow cannot draw outside table 1 %d\n",row);
        return;
    }

    if(RowDims.y + RowDims.h > Widget->Rect.y + Widget->Rect.h)
    {
        printf("SDL_TableDrawRow cannot draw outside table %d\n",row);
        return;
    }



    if(row == Table->HighlightedRow)
    {
        SDL_FillRect(dest,&RowDims,SDL_MapRGB(dest->format,255,55,0));
        SDL_FontSetColor(Table->font,Row->fgcolor);
    }
    else if(Table->SelectMode && SDL_TableIsRowSelected(Table,row))
    {
        SDL_FillRect(dest,&RowDims,0x00ff00);
    }
    else
    {
        if(Table->bgcolor != TRANSPARANT)
            SDL_FillRect(dest,&RowDims,Table->bgcolor);

        SDL_FontSetColor(Table->font,Row->fgcolor);
    }

    if(Table->ActiveEntry == row + Table->FirstVisibleRow)
    {
        SDL_FontSetColor(Table->font,0x0000ff);
    }

    if (Row)
    {
        for(column=0; column<Table->Columns; column++)
        {
            sprintf(string,Row->Cell[column].String);
           
            la.x = RowDims.x + 1;
            la.y = RowDims.y + 1;
            la.w = Table->Column[column].Width - 2;
            la.h = RowDims.h - 2;

            //SDL_FillRect(dest,&la,SDL_MapRGB(dest->format,155,155,155));

            if(strlen(string))
            {
                SDL_FontDrawStringRect(dest,Table->font,string,&la);
            }
            RowDims.x +=Table->Column[column].Width;
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
                if(table->SelectMode != TABLE_MODE_SINGLE)
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
        else if (table->SelectMode == TABLE_MODE_SINGLE)
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

void SDL_TableAddRow(SDL_Widget *Widget,char *Titles[])
{
    SDL_Table *Table=(SDL_Table*)Widget;

    if(Table == NULL || Table->Columns == 0)
        return;

    if(Table->RowData == NULL)
    {
        int i;
        Table->RowData=malloc(sizeof(SDL_TableRow));
        Table->RowData->Cell = calloc(sizeof(SDL_TableCell)*Table->Columns,Table->Columns);
        Table->RowData->fgcolor = Table->fgcolor;
        Table->RowData->Next = NULL;
        for(i=0;i<Table->Columns;i++)
            Table->RowData->Cell[i].String = strdup(Titles[i]);
        Table->Rows++;
    }
    else
    {
        SDL_TableRow *tmp;
        int i;

        tmp=Table->RowData;
        while(tmp->Next)
            tmp=tmp->Next;

        tmp->Next=malloc(sizeof(SDL_TableRow));
        tmp->Next->Cell = calloc(sizeof(SDL_TableCell)*Table->Columns,Table->Columns);
        tmp->Next->fgcolor = Table->fgcolor;
        tmp->Next->Next = NULL;
        for(i=0;i<Table->Columns;i++)
        {
            tmp->Next->Cell[i].String=strdup(Titles[i]);
        }
        Table->Rows++;
    }

    /* 
     * Check if a scrollbar is needed and if it is changed 
     */
    if(Table->Rows > Table->VisibleRows)
    {
        int row;
        SDL_TableAttachScrollbar(Table);
        row = SDL_ScrollbarGetCurrentValue(Table->Scrollbar);
        if(row != Table->FirstVisibleRow)
        {
            Table->FirstVisibleRow = row;
        }
        
    }

   

}

void SDL_TableDeleteRow(SDL_Widget *Widget,int row)
{
    SDL_Table *Table=(SDL_Table*)Widget;
    SDL_TableRow *tmp,*tmp2;
    int i;

    if(Table->RowData == NULL)
        return;

    tmp=Table->RowData;
    if(row == 0)
    {
        Table->RowData=Table->RowData->Next;
        for(i=0;i<Table->Columns;i++)
        {
            free(tmp->Cell[i].String);
        }
        free(tmp->Cell);
        free(tmp);
    }
    else
    {
        while(row--  && tmp->Next)
        {
            tmp2=tmp;
            tmp=tmp->Next;
        }
        tmp2->Next=tmp->Next;
        for(i=0;i<Table->Columns;i++)
        {
            free(tmp->Next->Cell[i].String);
        }
        free(tmp->Cell);
        free(tmp);
        
    }
    Table->Rows--;

}

void SDL_TableDeleteAllRows(SDL_Widget *widget)
{
    SDL_Table *Table=(SDL_Table*)widget;
    SDL_TableRow *tmp,*tmp2;
    int i;

    if(Table->RowData == NULL)
        return;

    tmp=Table->RowData;

    while(tmp)
    {
        tmp2=tmp->Next;
        for(i=0;i<Table->Columns;i++)
        {
            free(tmp->Cell[i].String);
        }
        free(tmp->Cell);
        free(tmp);
        tmp=tmp2;
    }
    Table->RowData=NULL;
    Table->Rows=0;
}

int SDL_TableSetColumnTitle(SDL_Widget *widget,int column, char *title)
{
    SDL_Table *Table=(SDL_Table*)widget;
    int x=0;

    if(column < 0 || column > Table->Columns)
        return 0;

            
    if(column > 0)
    {
        int i;
        for(i=0;i<column;i++)
            x+=Table->Column[i].Width;

    }
    Table->ButtonHeight=18;
    Table->VisibleRows = (widget->Rect.h - Table->ButtonHeight) / Table->RowHeight;
    if(Table->Column[column].Button == NULL)
        Table->Column[column].Button = SDL_WidgetCreate(SDL_BUTTON,widget->Rect.x + x,
                                                   widget->Rect.y,
                                                   Table->Column[column].Width,
                                                   Table->ButtonHeight);

    SDL_ButtonSetLabel(Table->Column[column].Button,title);

    return 1;

}

void ScrollBarChanged(SDL_Widget *widget)
{
    SDL_WidgetRedrawEvent(widget);
}

void SDL_TableHideCB(SDL_Widget *widget)
{
    SDL_Table *Table=(SDL_Table*)widget;
    if(Table->Scrollbar)
        SDL_WidgetHide(Table->Scrollbar);

}


void SDL_TableShowCB(SDL_Widget *widget)
{
    SDL_Table *Table=(SDL_Table*)widget;
    if(Table->Scrollbar)
        SDL_WidgetShow(Table->Scrollbar);

}

void SDL_TableSetEditable(SDL_Widget *widget,int value)
{
    SDL_Table *Table=(SDL_Table*)widget;
    Table->Editable = value;
}

static void SDL_TableEditActivateCB(SDL_Widget *widget)
{
    SDL_Table *Table=(SDL_Table*)widget;

    if(Table->Edit)
    {

        Table->EditCell->String=strdup(SDL_EditGetText(Table->Edit));

        SDL_WidgetClose(Table->Edit);
        Table->Edit     = NULL;
        Table->EditCell = NULL;
        SDL_SignalEmit(widget,"edited");
        SDL_WidgetRedrawEvent(widget);
    }
}

SDL_TableCell *SDL_TableGetCell(SDL_Widget *widget,int row,int column)
{
    SDL_Table *Table=(SDL_Table*)widget;
    SDL_TableRow *tmp;
    
    tmp=Table->RowData;
    while(row--)
        tmp=tmp->Next;

    return &tmp->Cell[column];

}

void SDL_TableSetCursor(SDL_Widget *widget,int row,int column)
{
    SDL_Table *Table=(SDL_Table*)widget;
    SDL_Rect RowDims;
    int i;

    if(Table != NULL && Table->Editable)
    {
        if(Table->Edit == NULL)
        {
            RowDims.w = Table->Column[column].Width;
            RowDims.x = widget->Rect.x;
            for(i=0;i<column;i++)
                RowDims.x += Table->Column[column].Width;
            RowDims.y = widget->Rect.y + Table->RowHeight * row + Table->ButtonHeight;
            RowDims.h = Table->RowHeight;

            SDL_TableEnsureRowVisible(widget,row);        
    
            Table->EditCell = SDL_TableGetCell(widget,row,column);

            Table->Edit=SDL_WidgetCreate(SDL_EDIT,RowDims.x,RowDims.y,RowDims.w,RowDims.h);
            SDL_WidgetPropertiesOf(Table->Edit,SET_FONT,Table->font);
            
            SDL_EditSetText(Table->Edit,Table->EditCell[column].String);
            SDL_WidgetSetFocus(Table->Edit);
            SDL_SignalConnect(Table->Edit,"activate",SDL_TableEditActivateCB,Table);
            SDL_WidgetRedrawEvent(widget);
        }
        
    }
}

void SDL_TableSetText(SDL_Widget *widget,int row,int column,char *text)
{
    SDL_TableCell *Cell;
    
    if(widget == NULL)
        return;

    Cell=SDL_TableGetCell(widget,row,column);
    
    if(Cell->String != NULL)
        free(Cell->String);
    Cell->String = strdup(text);

}

void SDL_TableSetStyle(SDL_Widget *widget,int row,Uint32 color)
{
    SDL_Table *Table=(SDL_Table*)widget;
    SDL_TableRow *tmp;
    
    tmp=Table->RowData;
    while(row--)
        tmp=tmp->Next;

    tmp->fgcolor=color;
}

int SDL_TableIsVisible(SDL_Widget *widget,int row)
{
    SDL_Table *Table=(SDL_Table*)widget;

    if(row > Table->FirstVisibleRow &&
       row < (Table->FirstVisibleRow + Table->VisibleRows))
        return 1;
    return 0;
}

void SDL_TableEnsureRowVisible(SDL_Widget *widget,int row)
{
    SDL_Table *Table=(SDL_Table*)widget;

    if(SDL_TableIsVisible(widget,row) == 0)
    {
        if(Table->Scrollbar)
        {
            Table->FirstVisibleRow = row - Table->VisibleRows/2;
            SDL_ScrollbarSetCurrentValue(Table->Scrollbar,Table->FirstVisibleRow);
            SDL_WidgetRedrawEvent(widget);
        }
    }

}

void SDL_TableSetColumnWidth(SDL_Widget *widget,int column,int width)
{
    int totalwidth,i;
    SDL_Table *Table=(SDL_Table*)widget;

    if(Table->Columns <= 0)
        return;
        
    if(column > Table->Columns)
        return;
        
    totalwidth=0;
    for(i=0;i<Table->Columns;i++)
    {
        totalwidth += Table->Column[i].Width;
    }
    totalwidth -= Table->Column[column].Width;
    if(totalwidth + width <= widget->Rect.w)
    {
        Table->Column[column].Width=width;
        if(Table->Column[column].Button)
        {
            Table->Column[column].Button->Rect.w = width;
            if(column > 0)
            {
                int x=Table->Widget.Rect.x;
                for(i=0;i<column;i++)
                    x+=Table->Column[i].Width;
                    
                Table->Column[column].Button->Rect.x = x;

                for(i=column+1;i<Table->Columns;i++)
                {
                    if(Table->Column[i].Button)
                    {
                        Table->Column[i].Button->Rect.x = 
                            Table->Column[i-1].Button->Rect.x + Table->Column[i-1].Button->Rect.w;
                    }
                }
            }
        }
    }
    
}
