/*
  Beatforce/SDLTk

  one line to give the program's name and an idea of what it does.
  Copyright (C) 2003-2004 John Beuving (john.beuving@beatforce.org)

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

#ifndef __SDL_WIDGET_H
#define __SDL_WIDGET_H

#include <stdarg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#define TRANSPARANT 0xff00ff
#define WHITE       0xffffff
#define BLACK       0x000000

// event handler callback description
// SDL_Table: prototype

typedef enum E_Widget_Properties
{
    SET_NORMAL_IMAGE,    // general (char*)
    SET_HIGHLIGHT_IMAGE, // char *
    SET_PRESSED_IMAGE,   // char *
    SET_DISABLED_IMAGE,  // char *
    SET_FONT,            // SDL_Font*
    SET_BG_COLOR,        // Uint32 eg 0xff0011
    SET_FG_COLOR,        // Uint32 eg 0xff0011
    SET_CAPTION,         // char * , sets the caption of the highlighted widget/subtab
    
  

   // GET_CAPTION,         // char *, read the caption
    GET_WIDTH,           // int *
    GET_HEIGHT,          // int *
    GET_STATE,           // int *
    

    //COLUMN_WIDTH,
    SET_VISIBLE_COLUMNS, // int 

    GET_SELECTED,         // void *
    CLEAR_SELECTED,
    SET_SELECTION_MODE,  // to enable selection

  
    SET_MAX_VALUE,      
    SET_MIN_VALUE,
//    SET_CUR_VALUE,
    SET_LINE_IMAGE,
    SET_BUTTON_IMAGE,
    SET_NORMAL_STEP_SIZE,
    
   
    SET_STATE_EDIT,

    SET_HIGHLIGHTED,
    SET_IMAGE,
    
}E_Widget_Properties;




/*
 * Widgets implemented
 */

typedef enum E_Widget_Type
{
    SDL_BUTTON,             /* 0  */
    SDL_TAB,                /* 1  */
    SDL_TABLE,              /* 2  */
    SDL_SLIDER,             /* 3  */
    SDL_LABEL,              /* 4  */
    SDL_EDIT,               /* 5  */
    SDL_PANEL,              /* 6  */
    SDL_VOLUMEBAR,          /* 7  */
    SDL_PROGRESSBAR,        /* 8  */
    SDL_TREE,               /* 9  */
    SDL_SCROLLBAR,          /* 10 */
    SDL_MENU,               /* 11 */
    SDL_TOGGLEBUTTON,       /* 12 */
    SDL_TOOLTIP             /* 13 */
}E_Widget_Type;

typedef struct SDL_Widget
{
    SDL_Rect      Rect;
    E_Widget_Type Type;
    int           Focusable;
    int           Visible;
    int           Inside;
}SDL_Widget;

/**
 *  Converter function pointer types
 */
typedef SDL_Widget*       (*T_Widget_Create)       (SDL_Rect*);
typedef void              (*T_Widget_Draw)         (SDL_Widget*,SDL_Surface *,SDL_Rect*);
typedef int               (*T_Widget_Properties)   (SDL_Widget*,int,va_list ap);
typedef int               (*T_Widget_EventHandler) (SDL_Widget*,SDL_Event*);
typedef void              (*T_Widget_Close)        (SDL_Widget*);

/**
 *  Structure type for converter functions
 */
struct S_Widget_FunctionList
{
  
    T_Widget_Create         create;          /* Creation of the widget                     */
    T_Widget_Draw           draw;            /* Draw function of the widget                */
    T_Widget_Properties     properties;      /* Change properties of the widget            */
    T_Widget_EventHandler   eventhandler;    /* Handles basic SDL events of a widget       */
    T_Widget_Close          close;            /* Handles the cleanup of alloced memory      */
};

extern const struct S_Widget_FunctionList SDL_Button_FunctionList;
extern const struct S_Widget_FunctionList SDL_Tab_FunctionList;
extern const struct S_Widget_FunctionList SDL_Table_FunctionList;
extern const struct S_Widget_FunctionList SDL_Slider_FunctionList;
extern const struct S_Widget_FunctionList SDL_Label_FunctionList;
extern const struct S_Widget_FunctionList SDL_Edit_FunctionList;
extern const struct S_Widget_FunctionList SDL_Panel_FunctionList;
extern const struct S_Widget_FunctionList SDL_VolumeBar_FunctionList;
extern const struct S_Widget_FunctionList SDL_ProgressBar_FunctionList;
extern const struct S_Widget_FunctionList SDL_Tree_FunctionList;
extern const struct S_Widget_FunctionList SDL_Scrollbar_FunctionList;
extern const struct S_Widget_FunctionList SDL_Menu_FunctionList;
extern const struct S_Widget_FunctionList SDL_ToggleButton_FunctionList;
extern const struct S_Widget_FunctionList SDL_Tooltip_FunctionList;
/**
 *  Lookup table for converter functions
 *
 *  @see convTable
 *
 *  @note
 *  Modification depends on order of E_Conv_InputType
 */
static const struct S_Widget_FunctionList * const WidgetTable[] =
{
    &SDL_Button_FunctionList,      //SDL_BUTTON
    &SDL_Tab_FunctionList,         //SDL_TAB
    &SDL_Table_FunctionList,       //SDL_TABLE
    &SDL_Slider_FunctionList,      //SDL_SLIDER  
    &SDL_Label_FunctionList,       //SDL_LABEL
    &SDL_Edit_FunctionList,        //SDL_EDIT
    &SDL_Panel_FunctionList,       //SDL_PANEL
    &SDL_VolumeBar_FunctionList,   //SDL_VOLUMEBAR
    &SDL_ProgressBar_FunctionList, //SDL_PROGRESSBAR
    &SDL_Tree_FunctionList,        //SDL_TREE
    &SDL_Scrollbar_FunctionList,   //SDL_SCROLLBAR
    &SDL_Menu_FunctionList,        //SDL_MENU
    &SDL_ToggleButton_FunctionList,//SDL_TOGGLEBUTTON
    &SDL_Tooltip_FunctionList      //SDL_TOOLTIP
};




int SDL_WidgetInit();
void SDL_WidgetMove(SDL_Widget *widget,int x,int y);

SDL_Surface *SDL_WidgetGetActiveSurface();
int SDL_WidgetForceRedraw(SDL_Surface *surface);

SDL_Widget* SDL_WidgetCreate(E_Widget_Type widget,int x,int y, int w, int h);
SDL_Widget* SDL_WidgetCreateR(E_Widget_Type widget,SDL_Rect dest);
int SDL_WidgetMain();

int   SDL_WidgetPropertiesOf(SDL_Widget* widget, int feature,...);

int   SDL_DrawAllWidgets(SDL_Surface *screen);
//int   SDL_WidgetEventCallback(void *function,E_Widget_Event event);
int SDL_WidgetEvent(SDL_Widget *widget,SDL_Event *event);

int   SDL_WidgetHasFocus(SDL_Widget *widget);
int   SDL_WidgetLoseFocus();
int SDL_WidgetSetFocus(SDL_Widget *widget);

int SDL_WidgetNeedsRedraw();
int SDL_WidgetClose(SDL_Widget *widget);


void SDL_WidgetHide(SDL_Widget *widget);
void SDL_WidgetShow(SDL_Widget *widget);
void SDL_WidgetRedrawEvent(SDL_Widget *widget);
void SDL_WidgetResize(SDL_Widget *widget,int w,int h);
#endif //__SDL_WIDGET_H


