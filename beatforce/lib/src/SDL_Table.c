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

void* SDL_TableCreate(SDL_Rect* rect)
{
    SDL_Table *newtable;

    newtable=(SDL_Table*)malloc(sizeof(SDL_Table));
    newtable->fgcolor           = BLACK;//0xffffef;
    newtable->bgcolor           = 0x00000f;
    newtable->rect.x            = rect->x;
    newtable->rect.y            = rect->y;
    newtable->rect.w            = rect->w;
    newtable->rect.h            = rect->h;
    newtable->next              = NULL;
    newtable->RowHeight         = 15;
    newtable->ColumnWidths      = NULL;
    
    newtable->VisibleRows       = 0;
    newtable->VisibleColumns    = 0;
    newtable->Rows              = 0;

    newtable->font              = NULL;
    // backup of the background
    newtable->Background        = NULL;

    //data retreival function
    newtable->Table_GetString   = NULL;

   // event function
    newtable->Clicked           = NULL;
    newtable->ClickedData       = NULL;

    newtable->CurrentRow        = -1;
    newtable->CurrentColumn     = -1;

    /* Handle to the scollbar widget */
    newtable->Scrollbar         = NULL;
    newtable->ScrollbarWidth    = 0;

    newtable->TablePreviousHighlightedRow = -1;
    newtable->TableSelectionChanged       = 1;
    newtable->TableInitialDraw            = 1; 

    newtable->edit              = NULL;
    newtable->ScrollbarImage    = NULL;
    newtable->bgcolor           = TRANSPARANT;
   
    newtable->Selected          = NULL;
    newtable->SelectedCount     = 0;
    newtable->Selectable        = 0;
    newtable->FirstVisibleRow   = 0;
    newtable->HighlightedRow    = -1;
    newtable->editcaption       = NULL;
    newtable->ActiveEntry       = -1;
    return newtable;
}

void SDL_TableDraw(void *table,SDL_Surface *dest)
{
    SDL_Table *Table=(SDL_Table*)table;
    int row;

    /*
     * Before the first draw we make a backup of the drawarea
     */

    if(Table->Background == NULL)
    {
        Table->Background=SDL_WidgetGetBackground(dest,&Table->rect);
    }

    /* 
     * Check if a scrollbar is needed and if it is changed 
     */
    if(Table->Rows > Table->VisibleRows)
    {
        int width,state;
        double row;
        SDL_TableAttachScrollbar(Table);


        SDL_WidgetPropertiesOf(Table->Scrollbar,GET_CUR_VALUE,&row);
        SDL_WidgetPropertiesOf(Table->Scrollbar,GET_WIDTH,&width);
        SDL_WidgetPropertiesOf(Table->Scrollbar,GET_STATE,&state);

        Table->ScrollbarWidth = width;
        
        if(state == SLIDER_DRAG)
            return;

        if((int)row != Table->FirstVisibleRow)
        {
            Table->FirstVisibleRow  = (int)row;
            Table->TableInitialDraw = 1 ;
        }
    }

   
    /*
     * Complete redraw of the entire table (== slow) 
     */
//    if(Table->TableInitialDraw || SDL_WidgetNeedsRedraw())
    {
        if(Table->bgcolor ==  TRANSPARANT)
        {
            if(SDL_BlitSurface(Table->Background,NULL,dest,&Table->rect)<0)
                fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
        }
        else
        {
            SDL_FillRect(dest,&Table->rect,Table->bgcolor);
        }

        for(row=0;row<Table->VisibleRows;row++)
        {
            SDL_TableDrawRow(dest,Table,row);
        }
        
        Table->TableInitialDraw = 0;
    }
    if(Table->edit)
    {
        SDL_WidgetPropertiesOf(Table->edit,FORCE_REDRAW,1);
    }
}


int SDL_TableProperties(void *table,int feature,va_list list)
{
    SDL_Table *Table=(SDL_Table*)table;
    int column,width;
    int totalwidth,i;
    double row;

    switch(feature)
    {
    case  SET_VISIBLE_ROWS:
        Table->VisibleRows    = va_arg(list,int);
        if(Table->rect.h / Table->RowHeight < Table->VisibleRows)
        {
            Table->VisibleRows = Table->rect.h / Table->RowHeight;
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
                Table->ColumnWidths[i]=Table->rect.w/Table->VisibleColumns;
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
        if(totalwidth + width <= Table->rect.w)
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

        if(Table->Rows > Table->VisibleRows && Table->Scrollbar)
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
//                SDL_WidgetClose(Table->Scrollbar);
            }
        }
        break;      
    }
    case SET_FONT:
        Table->font=va_arg(list,SDL_Font*);
        Table->RowHeight=SDL_FontGetHeight(Table->font)+2;
        Table->VisibleRows = Table->rect.h / Table->RowHeight;
        break;
    case SET_DATA_RETREIVAL_FUNCTION:
        Table->Table_GetString=va_arg(list,void*);
        break;
    case SET_CALLBACK:
        {
            int event=va_arg(list,int);
            if(event== SDL_CLICKED)
            {
                Table->Clicked=va_arg(list,void*);
                Table->ClickedData=va_arg(list,void*);
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
            row=(double)Table->ActiveEntry;
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
    case SET_SELECTABLE:
        Table->Selectable = va_arg(list,int);
        break;
    case SET_IMAGE:
        Table->ScrollbarImage = strdup(va_arg(list,char*));
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
                        Dims.x = Table->rect.x;
                        Dims.y = Table->rect.y+Table->RowHeight * row;
                        Dims.h = Table->RowHeight;

                        Table->edit=SDL_WidgetCreateR(SDL_EDIT,Dims);
                        SDL_WidgetProperties(SET_FONT,Table->font);
                        SDL_WidgetProperties(SET_ALWAYS_FOCUS,1);
                        SDL_WidgetProperties(SET_BG_COLOR,Table->bgcolor);
                        SDL_WidgetProperties(SET_FG_COLOR,BLACK);
                        SDL_WidgetProperties(SET_CAPTION,label);
                        SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN , Table_EditReturnKeyPressed , Table);
                    }
                 }
            }
        }
        break;
    }
    return 1;
}

void SDL_TableEventHandler(void *table,SDL_Event *event)
{
    SDL_Table *Table=(SDL_Table*)table;
    char string[255];
    int y;
    int i;

    Table->TablePreviousHighlightedRow = Table->HighlightedRow;
    
    switch(event->type)
    {
    case SDL_MOUSEMOTION:
        if(SDL_WidgetIsInside(&Table->rect,event->motion.x,event->motion.y))
        {
#if 0
            if(event->motion.x > (Table->rect.x + Table->rect.w - Table->ScrollbarWidth))
            {
                /* Event are for the scrollbar */
                break;
            }
            else
#endif
            {
                y=Table->rect.y;
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
        if(SDL_WidgetIsInside(&Table->rect,event->motion.x,event->motion.y))
        {
            if(event->button.button == 1 || event->button.button == 3)
            {
                if(event->motion.x > (Table->rect.x + Table->rect.w - Table->ScrollbarWidth))
                {
                    break;
                }
                else
                {
                    Table->CurrentRow = Table->FirstVisibleRow + 
                        (event->motion.y - Table->rect.y) / Table->RowHeight;

                    if(Table->Table_GetString) 
                    {
                        memset(string,0,255);
                        Table->Table_GetString(Table->CurrentRow,0,(char*)&string);
                     
                        if(strlen(string) && Table->Selectable)
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
    default:
        break;
    }
}

static void SDL_TableAttachScrollbar(SDL_Table *Table)
{
    SDL_Rect SliderRect;

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
            SliderRect.x     = Table->rect.x + Table->rect.w - 45;
            SliderRect.y     = Table->rect.y;
            SliderRect.h     = Table->rect.h;
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
    SDL_Rect RowDims;
    SDL_Rect la;
    int column;
    char string[255];
    
    if(row > Table->VisibleRows)
        return;

    memset(string,0,255);
    RowDims.w = Table->rect.w;// - Table->ScrollbarWidth;
    RowDims.x = Table->rect.x;
    RowDims.y = Table->rect.y+Table->RowHeight * row;
    RowDims.h = Table->RowHeight;

    if(row == Table->HighlightedRow)
    {
        SDL_FillRect(dest,&RowDims,SDL_MapRGB(dest->format,255,55,0));
        SDL_FontSetColor(Table->font,Table->fgcolor);
    }
    else if(Table->Selectable && SDL_TableIsRowSelected(Table,row))
    {
        SDL_FillRect(dest,&RowDims,0x00ff00);
    }
    else
    {
        if(Table->bgcolor == TRANSPARANT)
        {
            la.x = 0;
            la.y = RowDims.y;
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
                table->Selected[i]=-1;
                t(table);
                return;
            }
        }

        if(table->SelectedCount < table->Selectable || table->Selectable < 0)
        {
            table->SelectedCount++;
            table->Selected=realloc(table->Selected,table->SelectedCount*sizeof(int*));
            table->Selected[table->SelectedCount-1]=table->CurrentRow;
        }
        else if (table->Selectable == 1)
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

    SDL_WidgetPropertiesOf(Table->edit,GET_CAPTION,&string);
    SDL_WidgetClose(Table->edit);
    Table->edit=NULL;

    if(Table->editcaption)
        free(Table->editcaption);
    Table->editcaption = strdup(string);

    if(Table->OnReturn)
        Table->OnReturn();

    
}
