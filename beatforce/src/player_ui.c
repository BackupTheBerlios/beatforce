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

#include <memory.h> 

#include "config.h"

#include "osa.h"
#include "player.h"
#include "mixer.h"
#include "playlist.h"
#include "player_ui.h"
#include "audio_output.h"
#include "theme.h"
#include "wndmgr.h"

#include "SDL_Font.h"
#include "SDL_Slider.h"
#include "SDL_Widget.h"
#include "SDL_ProgressBar.h"

#define MODULE_ID PLAYER_UI
#include "debug.h"

PlayerDisplay UI_Players[2];

/* local prototypes */
static void PLAYERUI_SetSpeed(void *data);   /* Speed slider callback */
static void PLAYERUI_PlayButton(void *data); /* Play  button callback */

void UI_PlayerUpdateTimeLabel(void *data);
void UI_ProgressBarClicked(void *data);

/* Prototypes for redraw functions */
static void PLAYERUI_UpdateArtist(int player);  /* Redraw of the artist label */
static void PLAYERUI_UpdateTime(int player);
static void PLAYERUI_UpdateTitle(int player);
static void PLAYERUI_UpdateFileInfo(int player);
static void PLAYERUI_UpdateVolume(int player);
static void PLAYERUI_UpdateState(int player);

/* Callback for edit title */
static void PLAYERUI_EditTitleReturn(void *data);

 /* Exported functions */
void PLAYERUI_CreateWindow(int nr,ThemePlayer *pt)
{
    ThemeImage     *Image     = NULL;
    ThemeButton    *Button    = NULL;
    ThemeText      *Text      = NULL;
    ThemeVolumeBar *VolumeBar = NULL;
    ThemeEdit      *Edit      = NULL;
    
    if(pt == NULL)
        return;
    
    Image     = pt->Image;
    Button    = pt->Button;
    Text      = pt->Text;
    VolumeBar = pt->VolumeBar;
    Edit      = pt->Edit;

    UI_Players[nr].PlayerNr=nr;

    /* Background window */
    while(Image)
    {
        SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_WidgetProperties(SET_NORMAL_IMAGE,Image->filename);
        Image=Image->next;
    }        

    while(Button)
    {
        switch(Button->action)
        {
        case BUTTON_PLAY:
            /* Create the play button */
            UI_Players[nr].ButtonPlay=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,   Button->normal);
            SDL_WidgetProperties(SET_HIGHLIGHT_IMAGE,Button->highlighted);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,  Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,PLAYERUI_PlayButton,&UI_Players[nr]);
            break;
        case BUTTON_PAUSE:
            /* Create the pause button */
            UI_Players[nr].ButtonPause=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            SDL_WidgetProperties(SET_NORMAL_IMAGE,   Button->normal);
            SDL_WidgetProperties(SET_HIGHLIGHT_IMAGE,Button->highlighted);
            SDL_WidgetProperties(SET_PRESSED_IMAGE,  Button->pressed);
            SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,PLAYERUI_PlayButton,&UI_Players[nr]);
            SDL_WidgetProperties(SET_VISIBLE,0);
            break;
        }
        Button=Button->next;
    }
   
    while(Text)
    {
        switch(Text->display)
        {
        case TEXT_TIME_ELAPSED:
            /* Time elapsed */
            UI_Players[nr].TimeElapsed=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetProperties(SET_FONT,THEME_Font(Text->font));
            SDL_WidgetProperties(SET_FG_COLOR,Text->fgcolor);
            break;

        case TEXT_TIME_REMAINING:
            /* Time remaining */
            UI_Players[nr].TimeRemaining=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetProperties(SET_FONT,THEME_Font(Text->font));
            SDL_WidgetProperties(SET_FG_COLOR,Text->fgcolor);
            break;

        case TEXT_SONG_ARTIST:
            /* Create the artist label */
            UI_Players[nr].Artist=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetProperties(SET_FONT,THEME_Font(Text->font));
            SDL_WidgetProperties(SET_FG_COLOR,Text->fgcolor);
            break;

        case TEXT_SONG_TITLE:
            /* Create the title label */
            UI_Players[nr].Title=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetProperties(SET_FONT,THEME_Font(Text->font));
            SDL_WidgetProperties(SET_FG_COLOR,Text->fgcolor);
            break;

        case TEXT_PLAYER_STATE:
            /* Draw the state of the player  as a string */
            UI_Players[nr].State=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetProperties(SET_FONT,THEME_Font(Text->font));
            SDL_WidgetProperties(SET_FG_COLOR,Text->fgcolor);
            break;

        case TEXT_SAMPLERATE:
            /* Create the samplerate label */
            UI_Players[nr].Samplerate=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetProperties(SET_FONT,THEME_Font(Text->font));
            SDL_WidgetProperties(SET_FG_COLOR,Text->fgcolor);
            break;
        case TEXT_BITRATE:
            /* Create the bitrate label */
            UI_Players[nr].Bitrate=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetProperties(SET_FONT,THEME_Font(Text->font));
            SDL_WidgetProperties(SET_FG_COLOR,Text->fgcolor);
            break;

        }
        Text=Text->next;
    }
   
    while(VolumeBar)
    {
        switch(VolumeBar->display)
        {
        case VOLUMEBAR_VOLUME_LEFT:
            /* Create the volume bar widget */
            UI_Players[nr].VolumeLeft=SDL_WidgetCreateR(SDL_VOLUMEBAR,VolumeBar->Rect);
            SDL_WidgetProperties(SET_MAX_VALUE,127);
            SDL_WidgetProperties(SET_MIN_VALUE,0);
            break;
        case VOLUMEBAR_VOLUME_RIGHT:
            UI_Players[nr].VolumeRight=SDL_WidgetCreateR(SDL_VOLUMEBAR,VolumeBar->Rect);
            SDL_WidgetProperties(SET_MAX_VALUE,127);
            SDL_WidgetProperties(SET_MIN_VALUE,0);
            break;
        }
        VolumeBar=VolumeBar->next;
    }
    
    if(pt->ProgressBar)
    {
        /* Create the progressbar */
        UI_Players[nr].SongProgress=SDL_WidgetCreateR(SDL_PROGRESSBAR,pt->ProgressBar->Rect);
        SDL_WidgetProperties(SET_CALLBACK,SDL_CLICKED,UI_ProgressBarClicked,&UI_Players[nr]);
    }

    if(pt->Slider)
    {
        /* Create the pitch slider */
        UI_Players[nr].Pitch=SDL_WidgetCreateR(SDL_SLIDER,pt->Slider->Rect);
        SDL_WidgetProperties(SET_BUTTON_IMAGE,pt->Slider->button);
        SDL_WidgetProperties(SET_MAX_VALUE,2);
        SDL_WidgetProperties(SET_MIN_VALUE,0);
        SDL_WidgetProperties(SET_CUR_VALUE,1.0);    
        SDL_WidgetProperties(SET_NORMAL_STEP_SIZE,0.1);
        SDL_WidgetProperties(SET_CALLBACK,SDL_CHANGED,PLAYERUI_SetSpeed,&UI_Players[nr]);
    }
    
    if(pt->Edit)
    {
        UI_Players[nr].EditTitle=SDL_WidgetCreateR(SDL_EDIT,pt->Edit->Rect);
        SDL_WidgetProperties(SET_FONT,THEME_Font("normal"));
        SDL_WidgetProperties(SET_CALLBACK,SDL_KEYDOWN_RETURN,PLAYERUI_EditTitleReturn,&UI_Players[nr]);
    }
}

void PLAYERUI_Redraw()
{
    /* Automaticly load a song when no player is playing */
    if(PLAYER_IsPlaying(0) == 0 &&
       PLAYER_IsPlaying(1) == 0 &&
       PLAYLIST_GetSong(0,0))
    {
        player_set_song(0,0);
        PLAYER_Play(0);
    }


    PLAYERUI_UpdateArtist(0);
    PLAYERUI_UpdateArtist(1);

    PLAYERUI_UpdateTitle(0);
    PLAYERUI_UpdateTitle(1);

    PLAYERUI_UpdateTime(0);
    PLAYERUI_UpdateTime(1);

    PLAYERUI_UpdateFileInfo(0);
    PLAYERUI_UpdateFileInfo(1);

    PLAYERUI_UpdateVolume(0);
    PLAYERUI_UpdateVolume(1);

    PLAYERUI_UpdateState(0);
    PLAYERUI_UpdateState(1);

    if(PLAYER_IsPlaying(0))
    {
        SDL_WidgetPropertiesOf(UI_Players[0].ButtonPlay,SET_VISIBLE,0);
        SDL_WidgetPropertiesOf(UI_Players[0].ButtonPause,SET_VISIBLE,1);
    }    
    else
    {
        SDL_WidgetPropertiesOf(UI_Players[0].ButtonPlay,SET_VISIBLE,1);
        SDL_WidgetPropertiesOf(UI_Players[0].ButtonPause,SET_VISIBLE,0);
    }

    if(UI_Players[1].ButtonPause)
    {
        if(PLAYER_IsPlaying(1))
        {
            SDL_WidgetPropertiesOf(UI_Players[1].ButtonPlay,SET_VISIBLE,0);
            SDL_WidgetPropertiesOf(UI_Players[1].ButtonPause,SET_VISIBLE,1);
        }
        else
        {
            SDL_WidgetPropertiesOf(UI_Players[1].ButtonPlay,SET_VISIBLE,1);
            SDL_WidgetPropertiesOf(UI_Players[1].ButtonPause,SET_VISIBLE,0);
        }
    }

    if(SDL_WidgetHasFocus(UI_Players[0].EditTitle) ||
       SDL_WidgetHasFocus(UI_Players[0].EditTitle))
        WNDMGR_DisableEventhandler();
    else
        WNDMGR_EnableEventhandler();
}

/* 
 *
 * Internal functions (called by the functions above)
 * 
 *
 */
static void PLAYERUI_UpdateArtist(int player)
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


static void PLAYERUI_UpdateTitle(int player)
{
    char title[255];

    memset(title,0,255);
    PLAYER_GetTitle(player,title);

    if(UI_Players[player].Title)
        SDL_WidgetPropertiesOf(UI_Players[player].Title,SET_CAPTION,title);

}

static void PLAYERUI_UpdateTime(int player)
{
    char string[255];
    long time,timeleft,totaltime;
    int min,sec,msec;
    int state;

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

    SDL_WidgetPropertiesOf(UI_Players[player].SongProgress,GET_STATE,&state);
    if(state == PROGRESSBAR_DRAG)
    {
        double curvalue;
        int curval;
        SDL_WidgetPropertiesOf(UI_Players[player].SongProgress,GET_CUR_VALUE,&curvalue);
        curval=(int)curvalue*10;
        msec = ((curval % 60000) % 1000)/10;
        sec  = (curval % 60000) / 1000;
        min  =  curval / 60000;
        sprintf(string,"%02d:%02d.%02d",min,sec,msec);
        SDL_WidgetPropertiesOf(UI_Players[player].TimeElapsed,SET_CAPTION,string);
    }
    else
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
    
    if(state == PROGRESSBAR_DRAG)
    {
        double curvalue;
        int curval;
        SDL_WidgetPropertiesOf(UI_Players[player].SongProgress,GET_CUR_VALUE,&curvalue);
        curval=(int)curvalue*10;
        curval=totaltime-curval;
        msec = ((curval % 60000) % 1000)/10;
        sec  = (curval % 60000) / 1000;
        min  =  curval / 60000;
        sprintf(string,"%02d:%02d.%02d",min,sec,msec);
        SDL_WidgetPropertiesOf(UI_Players[player].TimeRemaining,SET_CAPTION,string);
    }
    else
        SDL_WidgetPropertiesOf(UI_Players[player].TimeRemaining,SET_CAPTION,string);


    /* Check for the seconds left */
    if(timeleft / 1000 == 5)
    {
        struct SongDBEntry * e;
        if(!MIXER_FadeInProgress())
        {
            if(PLAYLIST_GetSong(player,0))
            {
                player_set_song(!player,0);
                MIXER_DoFade(1,0);
                totaltime = 0;
                timeleft  = 0;
                time      = 0;
            }
            else
            {
                long id;
                PLAYER_GetPlayingID(player,&id);
                id++;
                e=SONGDB_GetEntryID(id);
                PLAYLIST_SetEntry(!player,e);
                player_set_song(!player,0);  /* when set_entry is excecuted we only have 1 item thus 0 */
                MIXER_DoFade(1,0);
                totaltime = 0;
                timeleft  = 0;
                time      = 0;
            }
            
        }
    }
    
    if(UI_Players[player].SongProgress)
    {
        if(state == PROGRESSBAR_NORMAL)
        {
            SDL_WidgetPropertiesOf(UI_Players[player].SongProgress,SET_MAX_VALUE,totaltime/10);
            SDL_WidgetPropertiesOf(UI_Players[player].SongProgress,SET_CUR_VALUE,(double)(time/10));
        }
    }
}

static void PLAYERUI_UpdateVolume(int player)
{
    int left=0,right=0;
    double vol;
    
    AUDIOOUTPUT_GetVolumeLevel(player,&left,&right);
    vol=(double)left;
    SDL_WidgetPropertiesOf(UI_Players[player].VolumeLeft,SET_CUR_VALUE,vol);
    vol=(double)right;
    SDL_WidgetPropertiesOf(UI_Players[player].VolumeRight,SET_CUR_VALUE,vol);
}

static void PLAYERUI_UpdateFileInfo(int player)
{
    char label[255];
    sprintf(label,"%d Smpls",PLAYER_GetSamplerate(player));
    SDL_WidgetPropertiesOf(UI_Players[player].Samplerate,SET_CAPTION,label);
    sprintf(label,"%d KBit",PLAYER_GetBitrate(player)/1000);
    SDL_WidgetPropertiesOf(UI_Players[player].Bitrate,SET_CAPTION,label);
}


static void PLAYERUI_EditTitleReturn(void *data)
{
    char caption[255];
    PlayerDisplay *cur=(PlayerDisplay*)data;

    SDL_WidgetPropertiesOf(cur->EditTitle,GET_CAPTION,&caption);
    PLAYER_SetTitle(cur->PlayerNr,caption);
}        
  
static void PLAYERUI_UpdateState(int player)
{
    char label[255];
    int state;

    state=PLAYER_GetState(player);
    switch(state)
    {
    case PLAYER_PLAY:
        sprintf(label,"PLAY");
        break;
    case PLAYER_IDLE:
        sprintf(label,"IDLE");
        break;
    case PLAYER_PAUSE:
        sprintf(label,"PAUSE");
        break;
    case PLAYER_PAUSE_EOF:
        sprintf(label,"PAUSE (END OF FILE)");
        break;
    default:
        sprintf(label,"INTERNAL ERROR");
        break;
    }
    SDL_WidgetPropertiesOf(UI_Players[player].State,SET_CAPTION,label);
}

static void PLAYERUI_PlayButton(void *data)
{
    char title[255];
    PlayerDisplay *current=(PlayerDisplay*)data;

    memset(title,0,255);
    PLAYER_GetTitle(current->PlayerNr,title);
    
    SDL_WidgetPropertiesOf(UI_Players[current->PlayerNr].EditTitle,SET_CAPTION,title);

    TRACE("playerui_PlayButton %d",current->PlayerNr);    

    if(PLAYER_IsPlaying(current->PlayerNr))
    {
        SDL_WidgetPropertiesOf(UI_Players[0].ButtonPlay,SET_VISIBLE,1);
        SDL_WidgetPropertiesOf(UI_Players[0].ButtonPause,SET_VISIBLE,0);
        printf("Player is playing %d\n",current->PlayerNr);
        PLAYER_Pause (current->PlayerNr);
    }
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
static void PLAYERUI_SetSpeed(void *data)
{
    double curval;
    PlayerDisplay *active=(PlayerDisplay*)data;
   
    SDL_WidgetPropertiesOf(active->Pitch,GET_CUR_VALUE,&curval);
    curval=2.0-curval;
    PLAYER_SetSpeed(active->PlayerNr,curval);
}
