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

#ifndef __SDL_WIDGET_H
#define __SDL_WIDGET_H

#include <stdarg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#define TRANSPARANT 0xfefefe
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
    SET_CALLBACK,        // sets a callback function for an event
    
    
//    BEFORE_DRAW_FUNCTION,   // This function is executed before the draw to set additional params
    SET_DATA_RETREIVAL_FUNCTION,      // Function for retreiving data

    GET_CAPTION,         // char *, read the caption
    GET_WIDTH,           // int *
    GET_HEIGHT,          // int *
    GET_STATE,           // int *
    

    SET_VISIBLE_ROWS,    // int table specific
    SET_VISIBLE_COLUMNS, // int 
    ROWS,                // int
    COLUMN_WIDTH,        // int
    GET_SELECTED,         // void *
    CLEAR_SELECTED,
    SET_SELECTABLE,  // to enable selection

    SET_PERCENTAGE,       // int, slider specific
    SET_MAX_VALUE,      
    SET_MIN_VALUE,
    SET_CUR_VALUE,
    SET_LINE_IMAGE,
    SET_BUTTON_IMAGE,
    SET_NORMAL_STEP_SIZE,

    SET_ALWAYS_FOCUS,
    
    FORCE_REDRAW,
    GET_CUR_VALUE,       // float *

    TAB_ADD,             ////cchar * with caption  tab specific can only be started with loaded font
    TAB_REMOVE,
    
    SET_STATE_EDIT
    
}E_Widget_Properties;


/*
 * Widget specific events, additional to SDL events 
 */
typedef enum E_Widget_Event
{
    SDL_CLICKED,
    SDL_KEYDOWN_RETURN,  /* SDL events: SDL_KEYDOWN with keysym SDLK_RETURN */
    SDL_KEYDOWN_ANY,
    SDL_CHANGED
}E_Widget_Event;

/*
 * Widgets implemented
 */

typedef enum E_Widget_Type
{
    SDL_BUTTON,
    SDL_TAB,
    SDL_TABLE,
    SDL_SLIDER,
    SDL_LABEL,
    SDL_EDIT,
    SDL_PANEL,
    SDL_VOLUMEBAR,
    SDL_PROGRESSBAR,
}E_Widget_Type;

/**
 *  Converter function pointer types
 */
typedef void*             (*T_Widget_Create)       (SDL_Rect*);
typedef void              (*T_Widget_Draw)         (void*,SDL_Surface *);
typedef int               (*T_Widget_Properties)   (void*,int,va_list ap);
typedef void              (*T_Widget_EventHandler) (void*,SDL_Event*);
typedef void              (*T_Widget_SetCallback)  (void*,void *function,E_Widget_Event event);
typedef void              (*T_Widget_Close)        (void*);

/**
 *  Structure type for converter functions
 */
struct S_Widget_FunctionList
{
  
    T_Widget_Create         create;          /* Creation of the widget                     */
    T_Widget_Draw           draw;            /* Draw function of the widget                */
    T_Widget_Properties     properties;      /* Change properties of the widget            */
    T_Widget_EventHandler   eventhandler;    /* Handles basic SDL events of a widget       */
    T_Widget_SetCallback    setcallback;     /* Sets a callback for widget specific events */
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
    &SDL_ProgressBar_FunctionList  //SDL_PROGRESSBAR
};

int   SDL_WidgetUseSurface(SDL_Surface *surface);
SDL_Surface *SDL_WidgetGetActiveSurface();


void* SDL_WidgetCreate(E_Widget_Type widget,int x,int y, int w, int h);
void* SDL_WidgetCreateR(E_Widget_Type widget,SDL_Rect dest);

int   SDL_WidgetProperties(int feature,...);
int   SDL_WidgetPropertiesOf(void* widget, int feature,...);

int   SDL_DrawAllWidgets(SDL_Surface *screen);
int   SDL_WidgetEventCallback(void *function,E_Widget_Event event);
void  SDL_WidgetEvent(SDL_Event *event);
int   SDL_WidgetIsInside(SDL_Rect *rect,int x, int y);
int   SDL_WidgetHasFocus(void *widget);

int SDL_WidgetNeedsRedraw();
int SDL_WidgetClose(void *widget);

int SDL_WidgetLOCK();
int SDL_WidgetUNLOCK();
#endif //__SDL_WIDGET_H


