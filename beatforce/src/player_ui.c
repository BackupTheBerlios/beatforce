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

#include "osa.h"
#include "player.h"
#include "mixer.h"
#include "playlist.h"
#include "player_ui.h"
#include "audio_output.h"

#include "SDL_Font.h"
#include "SDL_Slider.h"
#include "SDL_Widget.h"
#include "SDL_ProgressBar.h"

extern SDL_Font *LargeBoldFont;
extern SDL_Font *SmallFont;
extern SDL_Font *DigitsFont;

PlayerDisplay UI_Players[2];

/* local prototypes */
void playerui_SetSpeed(void *data);
void playerui_PlayButton(void *data);
void UI_PlayerUpdateTimeLabel(void *data);
void UI_ProgressBarClicked(void *data);

/* Prototypes for redraw functions */
void playerui_UpdateArtist(int player);
void playerui_UpdateTime(int player);
void playerui_UpdateTitle(int player);
void playerui_UpdateFileInfo(int player);
void playerui_UpdateVolume(int player);


 /* Exported functions */
void PLAYERUI_CreateWindow(int nr, int x)
{
    UI_Players[nr].PlayerNr=nr;

    /* Create the beackground panel */
//    SDL_WidgetCreate(SDL_PANEL,x,29,310,190);
//    SDL_WidgetProperties(SET_BG_COLOR,0x0da0c0);

    /* Background window */
    SDL_WidgetCreate(SDL_PANEL,x+4,55,100,100);
    SDL_WidgetProperties(SET_NORMAL_IMAGE,   THEME_DIR"/beatforce/playerbg.bmp");

    /* Create the play/pause button */
    SDL_WidgetCreate(SDL_BUTTON,x+157,176,80,20);
    SDL_WidgetProperties(SET_NORMAL_IMAGE,   THEME_DIR"/beatforce/playonly_green.bmp");
    SDL_WidgetProperties(SET_HIGHLIGHT_IMAGE,THEME_DIR"/beatforce/playonly_green_highlight.bmp");
    SDL_WidgetProperties(SET_PRESSED_IMAGE,  THEME_DIR"/beatforce/playonly_green_pressed.bmp");
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,playerui_PlayButton,&UI_Players[nr]);

    /* Time elapsed */
    UI_Players[nr].TimeElapsed=SDL_WidgetCreate(SDL_LABEL,x+9,90,105,22);
    SDL_WidgetProperties(SET_FONT,DigitsFont);
    SDL_WidgetProperties(SET_FG_COLOR,0xf0f0f0);

    /* Time remaining */
    UI_Players[nr].TimeRemaining=SDL_WidgetCreate(SDL_LABEL,x+165,90,105,22);
    SDL_WidgetProperties(SET_FONT,DigitsFont);
    SDL_WidgetProperties(SET_FG_COLOR,0xf0f0f0);

    /* Create the artist label */
    UI_Players[nr].Artist=SDL_WidgetCreate(SDL_LABEL,x+9,120,244,14);
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_FG_COLOR,0xfffff7);

    /* Create the title label */
    UI_Players[nr].Title=SDL_WidgetCreate(SDL_LABEL,x+9,136,244,14);
    SDL_WidgetProperties(SET_FONT,LargeBoldFont);
    SDL_WidgetProperties(SET_FG_COLOR,0xfffff7);

    /* Create the samplerate label */
    UI_Players[nr].Samplerate=SDL_WidgetCreate(SDL_LABEL,x+99,85,68,14);
    SDL_WidgetProperties(SET_FONT,SmallFont);
    SDL_WidgetProperties(SET_FG_COLOR,0xfffff7);

    /* Create the bitrate label */
    UI_Players[nr].Bitrate=SDL_WidgetCreate(SDL_LABEL,x+100,94,64,14);
    SDL_WidgetProperties(SET_FONT,SmallFont);
    SDL_WidgetProperties(SET_FG_COLOR,0xfffff7);

    /* Create the volume bar widget */
    UI_Players[nr].VolumeLeft=SDL_WidgetCreate(SDL_VOLUMEBAR,x+268,68,6,95);
    SDL_WidgetProperties(SET_MAX_VALUE,127);
    SDL_WidgetProperties(SET_MIN_VALUE,0);

    UI_Players[nr].VolumeRight=SDL_WidgetCreate(SDL_VOLUMEBAR,x+278,68,6,95);
    SDL_WidgetProperties(SET_MAX_VALUE,127);
    SDL_WidgetProperties(SET_MIN_VALUE,0);

    /* Create the progressbar */
    UI_Players[nr].SongProgress=SDL_WidgetCreate(SDL_PROGRESSBAR,x+6,34,250,13);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,UI_ProgressBarClicked,&UI_Players[nr]);

    /* Create the bslider background */
    SDL_WidgetCreate(SDL_PANEL,x+304,46,45,100);
    SDL_WidgetProperties(SET_NORMAL_IMAGE,   THEME_DIR"/beatforce/horline.bmp");

    /* Create the pitch slider */
    UI_Players[nr].Pitch=SDL_WidgetCreate(SDL_SLIDER,x+310,60,45,100);
    SDL_WidgetProperties(SET_BUTTON_IMAGE,THEME_DIR"/beatforce/slibut.bmp");
    SDL_WidgetProperties(SET_MAX_VALUE,2);
    SDL_WidgetProperties(SET_MIN_VALUE,0);
    SDL_WidgetProperties(SET_CUR_VALUE,1.0);    
    SDL_WidgetProperties(SET_NORMAL_STEP_SIZE,0.1);
    SDL_WidgetProperties(SET_CALLBACK,SDL_CHANGED,playerui_SetSpeed,&UI_Players[nr]);

}

void PLAYERUI_Redraw()
{
    playerui_UpdateArtist(0);
    playerui_UpdateArtist(1);

    playerui_UpdateTitle(0);
    playerui_UpdateTitle(1);

    playerui_UpdateTime(0);
    playerui_UpdateTime(1);

    playerui_UpdateFileInfo(0);
    playerui_UpdateFileInfo(1);

    playerui_UpdateVolume(0);
    playerui_UpdateVolume(1);
}

/* 
 *
 * Internal functions (called by the functions above)
 * 
 *
 */
void playerui_UpdateArtist(int player)
{
    char artist[255];
    memset(artist,0,255);
    /* Get and set the artist information */
    if(!PLAYER_GetArtist(player,artist))
    {
        char *filename;
        PLAYER_GetFilename(player,artist);
        filename=OSA_SearchFilename(artist);
        if(filename)
            sprintf(artist,"%s",filename);
        
    }
    if(UI_Players[player].Artist)
        SDL_WidgetPropertiesOf(UI_Players[player].Artist,SET_CAPTION,artist);
}


void playerui_UpdateTitle(int player)
{
    char title[255];

    memset(title,0,255);
    PLAYER_GetTitle(player,title);

    if(UI_Players[player].Title)
        SDL_WidgetPropertiesOf(UI_Players[player].Title,SET_CAPTION,title);

}

void playerui_UpdateTime(int player)
{
    char string[255];
    long time,timeleft,totaltime;
    int min,sec,msec;

    timeleft = PLAYER_GetTimeLeft(player);
    time     = PLAYER_GetTimePlayed(player);
    totaltime= PLAYER_GetTimeTotal(player);

    /* Time elapsed */
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
    SDL_WidgetPropertiesOf(UI_Players[player].TimeElapsed,SET_CAPTION,string);


    /* Time remaining */
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
    SDL_WidgetPropertiesOf(UI_Players[player].TimeRemaining,SET_CAPTION,string);


    /* Check for the seconds left */
    if(timeleft / 1000 == 5)
    {
        struct SongDBEntry * e;
        if(!MIXER_FadeInProgress())
        {
            long id;
            player_get_song(player,&id);
            id++;
            e=SONGDB_GetEntry(id);
            PLAYLIST_SetEntry(!player,e);
            player_set_song(!player,0);  // when set_entry is excecuted we only have 1 item thus 0
            MIXER_DoFade(1,0);
            totaltime = 0;
            timeleft  = 0;
            time      = 0;
            
        }
    }

    if(UI_Players[player].SongProgress)
    {
        SDL_WidgetPropertiesOf(UI_Players[player].SongProgress,SET_MAX_VALUE,totaltime/10);
        SDL_WidgetPropertiesOf(UI_Players[player].SongProgress,SET_CUR_VALUE,(double)(time/10));
        
    }
}

void playerui_UpdateVolume(int player)
{
    int left=0,right=0;
    double vol;
    
    output_get_volume_level(player,&left,&right);
    vol=(double)left;
    SDL_WidgetPropertiesOf(UI_Players[player].VolumeLeft,SET_CUR_VALUE,vol);
    vol=(double)right;
    SDL_WidgetPropertiesOf(UI_Players[player].VolumeRight,SET_CUR_VALUE,vol);
}

void playerui_UpdateFileInfo(int player)
{
    char label[255];
    sprintf(label,"%d Smpls",PLAYER_GetSamplerate(player));
    SDL_WidgetPropertiesOf(UI_Players[player].Samplerate,SET_CAPTION,label);
    sprintf(label,"%d KBit",PLAYER_GetBitrate(player)/1000);
    SDL_WidgetPropertiesOf(UI_Players[player].Bitrate,SET_CAPTION,label);
}
        
  




void playerui_PlayButton(void *data)
{
    PlayerDisplay *current=(PlayerDisplay*)data;
    
    if(PLAYER_IsPlaying(current->PlayerNr))
        PLAYER_Pause (current->PlayerNr);
    else
        PLAYER_Play  (current->PlayerNr);
    
}

/* Event handler for the progress bar SDL_CLICKED*/
void UI_ProgressBarClicked(void *playerdata)
{
    PlayerDisplay *Player= (PlayerDisplay*)playerdata;

    int time=((SDL_ProgressBar*)UI_Players[Player->PlayerNr].SongProgress)->CurrentValue;

    PLAYER_SetTimePlayed(Player->PlayerNr,time/100);
}


/* Callback function for pitch slider */
void playerui_SetSpeed(void *data)
{
    double curval;
    PlayerDisplay *active=(PlayerDisplay*)data;
   
    SDL_WidgetPropertiesOf(active->Pitch,GET_CUR_VALUE,&curval);
    curval=2.0-curval;
    AUDIOOUTPUT_SetSpeed(active->PlayerNr,curval);
}
