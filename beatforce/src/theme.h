/*
  Beatforce/ Startup of beatforce

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
#ifndef __THEME_H__
#define __THEME_H__

int THEME_Init();


enum
{
    /*Main window */
    BUTTON_CONFIG_WINDOW,
    /*player*/
    BUTTON_PLAY=1, 
    BUTTON_PAUSE,
    BUTTON_INFO,
    /*mixer*/
    BUTTON_RESET_FADER,
    /*songdb*/
    BUTTON_EDIT_GROUP,

    /*File window */
    BUTTON_RENAME,
    BUTTON_CREATE,
    BUTTON_REMOVE,
    BUTTON_ADDALL,
    BUTTON_ADDSELECTED,
    BUTTON_DELETESELECTED
}eButtonAction;

enum
{
    /* Player */
    SLIDER_PITCH=1,
    SLIDER_SPEED,
    
    /* Mixer */
    SLIDER_MAIN_VOLUME,
    SLIDER_MONITOR_VOLUME,
    SLIDER_FADER,
    SLIDER_VOLUME_LEFT_PLAYER1,
    SLIDER_VOLUME_RIGHT_PLAYER1,
    SLIDER_VOLUME_LEFT_PLAYER2,
    SLIDER_VOLUME_RIGHT_PLAYER2,
    SLIDER_EQUALIZER_10HZ_PLAYER1,
    SLIDER_EQUALIZER_10HZ_PLAYER2

}eSliderAction;


enum
{
    TEXT_TIME_ELAPSED = 1,
    TEXT_TIME_REMAINING,
    TEXT_SONG_TITLE,
    TEXT_SONG_ARTIST,
    TEXT_SAMPLERATE,
    TEXT_PLAYER_STATE,
    TEXT_BITRATE,
    TEXT_BPM
}eTextDisplay;

enum
{
    VOLUMEBAR_VOLUME_LEFT = 1,
    VOLUMEBAR_VOLUME_RIGHT,
    VOLUMEBAR_MAIN_INDICATION
}eVolumeBarDisplay;

enum
{
    
    LABEL_EMPTY
}eLabelDisplay;

enum
{
    CONTENTS_SUBGROUPS = 1,
    CONTENTS_FILESINDIRECTORY,
    CONTENTS_FILESINSUBGROUP,
    CONTENTS_DIRECTORIES
}eTableContents;

typedef struct ThemeFont
{
    char *id;
    char *filename;
    struct ThemeFont *Next;
}ThemeFont;

typedef struct ThemeText
{
    short x,y,w,h;
    int display;
    char *font;
    unsigned int fgcolor;
    struct ThemeText *next;
}ThemeText;

typedef struct ThemeButton
{
    short x,y,w,h;
    int action;
    char *normal;         /* filename */
    char *highlighted;    /* filename */
    char *pressed;        /* filename */

    struct ThemeButton *next;
}ThemeButton;

typedef struct ThemeImage
{
    short x,y,w,h;
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

typedef struct ThemeEdit
{
    short x,y,w,h;
    int display;
    struct ThemeEdit *next;
}ThemeEdit;

typedef struct ThemeTable
{
    short x,y,w,h;
    int Rows;
    int Columns;
    int ContentType;
    char *ScrollbarButton; /* Filename of the scrollbarbutton */
    char *ScrollbarLine;   /* Filename of the scrollbarline   */
    
    struct ThemeColumn *Column;
    struct ThemeTable *next;
}ThemeTable;

typedef struct ThemeTree
{
    short x,y,w,h;
    char *Font;
    unsigned int fgcolor;
    unsigned int bgcolor;
}ThemeTree;

typedef struct ThemeVolumeBar
{
    short x,y,w,h;
    int display;
    struct ThemeVolumeBar *next;
}ThemeVolumeBar;

typedef struct ThemeProgressBar
{
    short x,y,w,h;
}ThemeProgressBar;

typedef struct ThemeSlider
{
    short x,y,w,h;
    int action;
    char *button; /* filename of the button */
    struct ThemeSlider *next;
}ThemeSlider;

typedef struct ThemePlayer
{
    ThemeEdit        *Edit;
    ThemeText        *Text;
    ThemeImage       *Image;
    ThemeButton      *Button;
    ThemeVolumeBar   *VolumeBar;
    ThemeProgressBar *ProgressBar;
    ThemeSlider      *Slider;
    
    struct ThemePlayer *Next;
}ThemePlayer;

typedef struct ThemeSongdb
{
    ThemeButton *Button;
    ThemeImage  *Image;
    ThemeTable  *Table;
}ThemeSongdb;

typedef struct ThemePlaylist
{
    ThemeImage    *Image;
    ThemeTable    *Table;
    
    struct ThemePlaylist *Next;
}ThemePlaylist;

typedef struct ThemeClock
{
    short x,y,w,h;
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
    ThemeImage    *Image;
    ThemeClock    *Clock;    
    ThemePlayer   *Player;
    ThemeSongdb   *Songdb;
    ThemePlaylist *Playlist;
    ThemeMixer    *Mixer;
}ThemeMainWindow;

typedef struct ThemeConfigWindow
{
    ThemeImage *Image;
    ThemeClock *Clock;    
}ThemeConfigWindow;

typedef struct ThemeSearchWindow
{
    ThemeImage *Image;
}ThemeSearchWindow;

typedef struct ThemeFileWindow
{
    short x,y,w,h;
    ThemeClock  *Clock;    
    ThemeTable  *Table;
    ThemeImage  *Image;
    ThemeText   *Text;
    ThemeButton *Button;
    ThemeTree   *Tree;
}ThemeFileWindow;

typedef struct
{
    ThemeScreen       *Screen;
    ThemeFont         *Font;
    ThemeMainWindow   *MainWindow;
    ThemeSearchWindow *SearchWindow;
    ThemeFileWindow   *FileWindow;
    ThemeConfigWindow *ConfigWindow;
}
ThemeConfig;

ThemeConfig *THEME_GetActive();

#endif /* __THEME_H__ */
