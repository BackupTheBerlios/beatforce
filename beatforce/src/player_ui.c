/*
  Beatforce/ Player user interface

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

#include "config.h"

#include "player.h"
#include "mixer.h"
#include "playlist.h"
#include "player_ui.h"
#include "audio_output.h"

#include "SDL_Font.h"
#include "SDL_Widget.h"
#include "SDL_ProgressBar.h"

extern SDL_Font *LargeBoldFont;
extern SDL_Font *DigitsFont;

PlayerDisplay UI_Players[2];

 // local prototypes
void UI_PlayerUpdateTimeLabel(void *data);
void UI_ProgressBarClicked(void *data);



 /* Exported functions */
void PLAYERUI_CreateWindow(int nr, int x)
{
    UI_Players[nr].PlayerNr=nr;

    /* Create the beackground panel */
//    SDL_WidgetCreate(SDL_PANEL,x,29,310,190);
//    SDL_WidgetProperties(SET_BG_COLOR,0x0da0c0);

    /* Create the play/pause button */
    SDL_WidgetCreate(SDL_BUTTON,x+157,176,80,20);
    SDL_WidgetProperties(SET_NORMAL_IMAGE,   THEME_DIR"/beatforce/playonly_green.bmp");
    SDL_WidgetProperties(SET_HIGHLIGHT_IMAGE,THEME_DIR"/beatforce/playonly_green_highlight.bmp");
    SDL_WidgetProperties(SET_PRESSED_IMAGE,  THEME_DIR"/beatforce/playonly_green_pressed.bmp");
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,UI_PlayerPlayButtonPressed,&UI_Players[nr]);

    /* Time elapsed */
    UI_Players[nr].TimeElapsed=SDL_WidgetCreate(SDL_LABEL,x+9,90,105,22);
    SDL_WidgetProperties(BEFORE_DRAW_FUNCTION,UI_PlayerUpdateTimeLabel,&UI_Players[nr]);
    SDL_WidgetProperties(SET_FONT,DigitsFont);
    SDL_WidgetProperties(SET_FG_COLOR,0xf0f0f0);

    /* Time remaining */
    UI_Players[nr].TimeRemaining=SDL_WidgetCreate(SDL_LABEL,x+162,90,105,22);
    SDL_WidgetProperties(SET_FONT,DigitsFont);
    SDL_WidgetProperties(SET_FG_COLOR,0xf0f0f0);

    /* Create the artist/title label */
    UI_Players[nr].songlabel=SDL_WidgetCreate(SDL_LABEL,x+9,120,244,14);
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_FG_COLOR,0xfffff7);

    /* Create the volume bar widget */
    UI_Players[nr].volumebarleft=SDL_WidgetCreate(SDL_VOLUMEBAR,x+268,68,6,95);
    SDL_WidgetProperties(SET_MAX_VALUE,127);
    SDL_WidgetProperties(SET_MIN_VALUE,0);

    UI_Players[nr].volumebarright=SDL_WidgetCreate(SDL_VOLUMEBAR,x+278,68,6,95);
    SDL_WidgetProperties(SET_MAX_VALUE,127);
    SDL_WidgetProperties(SET_MIN_VALUE,0);

    /* Create the progressbar */
    UI_Players[nr].songprogress=SDL_WidgetCreate(SDL_PROGRESSBAR,x+6,34,250,13);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,UI_ProgressBarClicked,&UI_Players[nr]);


}

void UI_PlayerSetArtistTitle(int player)
{
    char label[255];

    memset(label,0,255);

    /* Get and set the artist information */
    player_get_songfield(player,label);

    if(UI_Players[player].songlabel)
        SDL_WidgetPropertiesOf(UI_Players[player].songlabel,SET_CAPTION,label);
}

/* 
 *
 * Internal functions (called by the functions above)
 * 
 *
 */

void UI_PlayerUpdateTimeLabel(void *data)
{
    char string[255];
    long time=0;
    long timeleft=0;
    long totaltime=0;
//    double progress;
    double vol;
    int sec;
    int msec;
    int min;
 
    PlayerDisplay *john;
    
    john=data;

    UI_PlayerSetArtistTitle(john->PlayerNr);
    time     = PLAYER_GetTimePlayed(john->PlayerNr);
    timeleft = PLAYER_GetTimeLeft(john->PlayerNr);
    totaltime= PLAYER_GetTimeTotal(john->PlayerNr);

    if(totaltime)
    {
        msec = (time % 60000) % 1000;
        sec  = (time % 60000) / 1000;
        min  =  time / 60000;
        sprintf(string,"%02d:%02d.%02d",min,sec,msec/10);
    }
    else
    {
        sprintf(string,"--:--.--");
    }
    
    if(timeleft / 1000 == 5)
    {
        struct SongDBEntry * e;
        if(!MIXER_FadeInProgress())
        {
            long id;
            player_get_song(john->PlayerNr,&id);
            id++;
            e=SONGDB_GetEntry(id);
            playlist_set_entry(!john->PlayerNr,e);
            player_set_song(!john->PlayerNr,0);  // when set_entry is excecuted we only have 1 item thus 0
            MIXER_DoFade(1,0);
            totaltime = 0;
            timeleft  = 0;
            time      = 0;
            UI_PlayerSetArtistTitle(john->PlayerNr);
            UI_PlayerSetArtistTitle(!john->PlayerNr);
            
        }
    }

    SDL_WidgetPropertiesOf(john->TimeElapsed,SET_CAPTION,string);
    if(totaltime)
    {
        msec = ((timeleft % 60000) % 1000)/10;
        sec  = (timeleft % 60000) / 1000;
        min  =  timeleft / 60000;
        sprintf(string,"%02d:%02d.%02d",min,sec,msec);
    }
    else
    {
        sprintf(string,"--:--.--");
    }
    SDL_WidgetPropertiesOf(john->TimeRemaining,SET_CAPTION,string);

    if(john->songprogress)
    {
        SDL_WidgetPropertiesOf(john->songprogress,SET_MAX_VALUE,totaltime/10);
        SDL_WidgetPropertiesOf(john->songprogress,SET_CUR_VALUE,(double)(time/10));
        
    }
    {
        int left,right;
        output_get_volume_level(john->PlayerNr,&left,&right);
        vol=(double)left;
        SDL_WidgetPropertiesOf(john->volumebarleft,SET_CUR_VALUE,vol);
        vol=(double)right;
        SDL_WidgetPropertiesOf(john->volumebarright,SET_CUR_VALUE,vol);
    }
    
}


void UI_PlayerPlayButtonPressed(void *data)
{
    PlayerDisplay *john;

    john=data;

    
    if(PLAYER_IsPlaying(john->PlayerNr))
        PLAYER_Pause (john->PlayerNr);
    else
        PLAYER_Play  (john->PlayerNr);
    
}

/* Event handler for the progress bar SDL_CLICKED*/
void UI_ProgressBarClicked(void *playerdata)
{
    PlayerDisplay *Player= (PlayerDisplay*)playerdata;

    int time=((SDL_ProgressBar*)UI_Players[Player->PlayerNr].songprogress)->CurrentValue;

    PLAYER_SetTimePlayed(Player->PlayerNr,time/100);
}

