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
#ifndef __THEME_H__
#define __THEME_H__

#include <SDL/SDL.h>
#include <SDL_Font.h>

int THEME_Init();
SDL_Font *THEME_Font(char *fontid);

enum
{
    PLAY=1,
    PAUSE,
    RESET_FADER
}eButtonAction;

enum
{
    PITCH=1,
    SPEED,
    
    MAIN_VOLUME,
    FADER
}eSliderAction;


enum
{
    TIME_ELAPSED = 1,
    TIME_REMAINING,
    SONG_TITLE,
    SONG_ARTIST,
    SAMPLERATE,
    BITRATE
}eTextDisplay;

enum
{
    VOLUME_LEFT = 1,
    VOLUME_RIGHT,
    MAIN_INDICATION
}eVolumeBarDisplay;

typedef struct ThemeFont
{
    char *id;
    char *filename;
    SDL_Font *font;
    struct ThemeFont *next;
}ThemeFont;

typedef struct ThemeText
{
    SDL_Rect Rect;
    int display;
    char *font;
    unsigned int fgcolor;
    struct ThemeText *next;
}ThemeText;

typedef struct ThemeButton
{
    SDL_Rect Rect;
    int action;
    char *normal;         /* filename */
    char *highlighted;    /* filename */
    char *pressed;        /* filename */

    struct ThemeButton *next;
}ThemeButton;

typedef struct ThemeImage
{
    SDL_Rect Rect;
    char *filename;        /* filename */
    struct ThemeImage *next;
    
}ThemeImage;

typedef struct
{
    int FullScreen;
    int Width;
    int Height;
    int NoFrame;
    int BPP;
}ThemeScreen;

typedef struct ThemeColumn
{
    int width;
    struct ThemeColumn *next;
}ThemeColumn;

typedef struct
{
    SDL_Rect Rect;
    int Rows;
    int Columns;
    struct ThemeColumn *Column;
}ThemeTable;

typedef struct ThemeVolumeBar
{
    SDL_Rect Rect;
    int display;
    struct ThemeVolumeBar *next;
}ThemeVolumeBar;

typedef struct ThemeProgressBar
{
    SDL_Rect Rect;
}ThemeProgressBar;

typedef struct ThemeSlider
{
    SDL_Rect Rect;
    int action;
    char *button; /* filename of the button */
    struct ThemeSlider *next;
}ThemeSlider;

typedef struct
{
    ThemeText        *Text;
    ThemeImage       *Image;
    ThemeButton      *Button;
    ThemeVolumeBar   *VolumeBar;
    ThemeProgressBar *ProgressBar;
    ThemeSlider      *Slider;
}ThemePlayer;

typedef struct 
{
    ThemeTable *Table;
}ThemeSongdb;

typedef struct 
{
    ThemeTable *Table;
}ThemePlaylist;

typedef struct ThemeClock
{
    SDL_Rect Rect;
    char *font;
    unsigned int fgcolor;
    unsigned int bgcolor;
}ThemeClock;

typedef struct ThemeMixer
{
    ThemeImage       *Image;
    ThemeVolumeBar   *VolumeBar;
    ThemeSlider      *Slider;
    ThemeButton      *Button;    
}ThemeMixer;

typedef struct 
{
    ThemeImage *Image;
    ThemeClock *Clock;    
    ThemePlayer *Player[2];
    ThemeSongdb   *Songdb;
    ThemePlaylist *Playlist;
    ThemeMixer    *Mixer;
}ThemeMainWindow;

typedef struct ThemeSearchWindow
{
    ThemeImage *Image;
}ThemeSearchWindow;

typedef struct ThemeFileWindow
{
    ThemeImage *Image;
}ThemeFileWindow;

typedef struct
{
    ThemeScreen       *Screen;
    ThemeFont         *Font;
    ThemeMainWindow   *MainWindow;
    ThemeSearchWindow *SearchWindow;
    ThemeFileWindow   *FileWindow;
}
ThemeConfig;

ThemeConfig *THEME_GetActive();

#endif /* __THEME_H__ */