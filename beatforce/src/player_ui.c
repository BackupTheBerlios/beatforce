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
#include <stdlib.h>

#include "config.h"

#include "osa.h"
#include "player.h"
#include "mixer.h"
#include "playlist.h"
#include "player_ui.h"
#include "audio_output.h"
#include "theme.h"
#include "wndmgr.h"
#include "songdb_ui.h"

#include "SDLTk.h"

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

#if 0
/* Callback for edit title */
static void PLAYERUI_EditTitleReturn(void *data);
#endif


/* Exported functions */
void PLAYERUI_CreateWindow(int nr,ThemePlayer *pt)
{
    ThemeImage     *Image     = NULL;
    ThemeButton    *Button    = NULL;
    ThemeText      *Text      = NULL;
    ThemeVolumeBar *VolumeBar = NULL;
    ThemeEdit      *Edit      = NULL;
    void *t;

    if(pt == NULL)
        return;

    /* Initialize the player functionality */
    PLAYER_Init(nr);

    Image     = pt->Image;
    Button    = pt->Button;
    Text      = pt->Text;
    VolumeBar = pt->VolumeBar;
    Edit      = pt->Edit;

    UI_Players[nr].PlayerNr=nr;
    UI_Players[nr].Images=NULL;

    /* Background window */
    while(Image)
    {
        t=SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_WidgetPropertiesOf(t,SET_IMAGE,IMG_Load(Image->filename));
        UI_Players[nr].Images=LLIST_Append(UI_Players[nr].Images,t);
        Image=Image->next;
        
    }        
    while(Button)
    {
        switch(Button->action)
        {
        case BUTTON_PLAY:
            /* Create the play button */
            UI_Players[nr].Normal.ButtonPlay=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);

            if(Button->normal)
                SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonPlay,
                                     SET_NORMAL_IMAGE,   IMG_Load(Button->normal));
            if(Button->highlighted)
                SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonPlay,
                                     SET_HIGHLIGHT_IMAGE,IMG_Load(Button->highlighted));
            if(Button->pressed)
                SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonPlay,
                                     SET_PRESSED_IMAGE,  IMG_Load(Button->pressed));

            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonPlay,
                                 SET_CALLBACK,SDL_CLICKED,PLAYERUI_PlayButton,&UI_Players[nr]);
            break;
        case BUTTON_PAUSE:
            /* Create the pause button */
            UI_Players[nr].Normal.ButtonPause=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);

            if(Button->normal)
                SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonPause,
                                     SET_NORMAL_IMAGE,   IMG_Load(Button->normal));
            if(Button->highlighted)
                SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonPause,
                                     SET_HIGHLIGHT_IMAGE,IMG_Load(Button->highlighted));
            if(Button->pressed)
                SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonPause,
                                     SET_PRESSED_IMAGE,  IMG_Load(Button->pressed));

            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonPause,
                                   SET_CALLBACK,SDL_CLICKED,PLAYERUI_PlayButton,&UI_Players[nr]);

            break;
        case BUTTON_INFO:
            /* Create the info button */
            UI_Players[nr].Normal.ButtonInfo=SDL_WidgetCreateR(SDL_BUTTON,Button->Rect);
            if(Button->normal)
                SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonInfo,
                                     SET_NORMAL_IMAGE,   IMG_Load(Button->normal));
            if(Button->highlighted)
                SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonInfo,
                                     SET_HIGHLIGHT_IMAGE,IMG_Load(Button->highlighted));
            if(Button->pressed)
                SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonInfo,
                                     SET_PRESSED_IMAGE,  IMG_Load(Button->pressed));
//            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.ButtonInfo,
//                                   SET_CALLBACK,SDL_CLICKED,PLAYERUI_HidePlayer,&UI_Players[nr]);
            
        }
        Button=Button->next;
    }
   
    while(Text)
    {
        switch(Text->display)
        {
        case TEXT_TIME_ELAPSED:
            /* Time elapsed */
            UI_Players[nr].Normal.TimeElapsed=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.TimeElapsed,
                                   SET_FONT,THEME_Font(Text->font));
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.TimeElapsed,
                                   SET_FG_COLOR,Text->fgcolor);
            break;

        case TEXT_TIME_REMAINING:
            /* Time remaining */
            UI_Players[nr].Normal.TimeRemaining=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.TimeRemaining,
                                   SET_FONT,THEME_Font(Text->font));
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.TimeRemaining,
                                   SET_FG_COLOR,Text->fgcolor);
            break;

        case TEXT_SONG_ARTIST:
            /* Create the artist label */
            UI_Players[nr].Normal.Artist=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Artist,
                                   SET_FONT,THEME_Font(Text->font));
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Artist,
                                   SET_FG_COLOR,Text->fgcolor);
            break;

        case TEXT_SONG_TITLE:
            /* Create the title label */
            UI_Players[nr].Normal.Title=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Title,
                                   SET_FONT,THEME_Font(Text->font));
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Title,
                                   SET_FG_COLOR,Text->fgcolor);
            break;

        case TEXT_PLAYER_STATE:
            /* Draw the state of the player  as a string */
            UI_Players[nr].Normal.PlayerState=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.PlayerState,
                                   SET_FONT,THEME_Font(Text->font));
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.PlayerState,
                                   SET_FG_COLOR,Text->fgcolor);
            break;

        case TEXT_SAMPLERATE:
            /* Create the samplerate label */
            UI_Players[nr].Normal.Samplerate=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Samplerate,
                                   SET_FONT,THEME_Font(Text->font));
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Samplerate,
                                   SET_FG_COLOR,Text->fgcolor);
            break;
        case TEXT_BITRATE:
            /* Create the bitrate label */
            UI_Players[nr].Normal.Bitrate=SDL_WidgetCreateR(SDL_LABEL,Text->Rect);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Bitrate,
                                   SET_FONT,THEME_Font(Text->font));
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Bitrate,
                                   SET_FG_COLOR,Text->fgcolor);
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
            UI_Players[nr].Normal.VolumeLeft=SDL_WidgetCreateR(SDL_VOLUMEBAR,VolumeBar->Rect);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.VolumeLeft,
                                   SET_MAX_VALUE,127);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.VolumeLeft,
                                   SET_MIN_VALUE,0);
            break;
        case VOLUMEBAR_VOLUME_RIGHT:
            UI_Players[nr].Normal.VolumeRight=SDL_WidgetCreateR(SDL_VOLUMEBAR,VolumeBar->Rect);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.VolumeRight,
                                   SET_MAX_VALUE,127);
            SDL_WidgetPropertiesOf(UI_Players[nr].Normal.VolumeRight,
                                   SET_MIN_VALUE,0);
            break;
        }
        VolumeBar=VolumeBar->next;
    }
    
    if(pt->ProgressBar)
    {
        /* Create the progressbar */
        UI_Players[nr].Normal.SongProgress=SDL_WidgetCreateR(SDL_PROGRESSBAR,pt->ProgressBar->Rect);
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.SongProgress,
                               SET_CALLBACK,SDL_CLICKED,UI_ProgressBarClicked,&UI_Players[nr]);
    }

    if(pt->Slider)
    {
        /* Create the pitch slider */
        UI_Players[nr].Normal.Pitch=SDL_WidgetCreateR(SDL_SLIDER,pt->Slider->Rect);
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_BUTTON_IMAGE,IMG_Load(pt->Slider->button));
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_MAX_VALUE,2);
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_MIN_VALUE,0);
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_CUR_VALUE,1.0);    
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_NORMAL_STEP_SIZE,0.1);
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_CALLBACK,SDL_CHANGED,PLAYERUI_SetSpeed,&UI_Players[nr]);
    }

    UI_Players[nr].State = PLAYERUI_STATE_NORMAL;
}

void PLAYERUI_Redraw()
{
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

    if(UI_Players[player].Normal.Artist)
        SDL_WidgetPropertiesOf(UI_Players[player].Normal.Artist,SET_CAPTION,artist);
}


static void PLAYERUI_UpdateTitle(int player)
{
    char title[255];

    memset(title,0,255);
    PLAYER_GetTitle(player,title);

    if(UI_Players[player].Normal.Title)
        SDL_WidgetPropertiesOf(UI_Players[player].Normal.Title,SET_CAPTION,title);

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


    SDL_WidgetPropertiesOf(UI_Players[player].Normal.SongProgress,GET_STATE,&state);
    if(state == PROGRESSBAR_DRAG)
    {
        double curvalue;
        int curval;
        SDL_WidgetPropertiesOf(UI_Players[player].Normal.SongProgress,GET_CUR_VALUE,&curvalue);
        curval=(int)curvalue*10;
        msec = ((curval % 60000) % 1000)/10;
        sec  = (curval % 60000) / 1000;
        min  =  curval / 60000;
        sprintf(string,"%02d:%02d.%02d",min,sec,msec);
        SDL_WidgetPropertiesOf(UI_Players[player].Normal.TimeElapsed,SET_CAPTION,string);
    }
    else
        SDL_WidgetPropertiesOf(UI_Players[player].Normal.TimeElapsed,SET_CAPTION,string);


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
        SDL_WidgetPropertiesOf(UI_Players[player].Normal.SongProgress,GET_CUR_VALUE,&curvalue);
        curval=(int)curvalue*10;
        curval=totaltime-curval;
        msec = ((curval % 60000) % 1000)/10;
        sec  = (curval % 60000) / 1000;
        min  =  curval / 60000;
        sprintf(string,"%02d:%02d.%02d",min,sec,msec);
        SDL_WidgetPropertiesOf(UI_Players[player].Normal.TimeRemaining,SET_CAPTION,string);
    }
    else
        SDL_WidgetPropertiesOf(UI_Players[player].Normal.TimeRemaining,SET_CAPTION,string);


    /* Check for the seconds left */
    if(timeleft / 1000 == 5)
    {
        struct SongDBEntry * e;
        if(!MIXER_FadeInProgress())
        {
            if(PLAYLIST_GetSong(player,0))
            {
                PLAYER_SetSong(!player,0);
                MIXER_DoFade(1,1);
                SONGDBUI_Play(!player);
                totaltime = 0;
                timeleft  = 0;
                time      = 0;
            }
            else
            {

                long id;
#if 0
                PLAYER_GetPlayingID(player,&id);
                id++;
#endif
                id=rand()%SONGDB_GetNoOfEntries();
                e=SONGDB_GetEntryID(id);
                PLAYLIST_AddEntry(!player,e);
                PLAYER_SetSong(!player,0);  /* when set_entry is excecuted we only have 1 item thus 0 */
                MIXER_DoFade(1,0);
                SONGDBUI_Play(!player);
            }
            
        }
    }
    
    if(UI_Players[player].Normal.SongProgress)
    {
        if(state == PROGRESSBAR_NORMAL)
        {
            SDL_WidgetPropertiesOf(UI_Players[player].Normal.SongProgress,SET_CUR_VALUE,(double)(time/10));
            SDL_WidgetPropertiesOf(UI_Players[player].Normal.SongProgress,SET_MAX_VALUE,totaltime/10);
        }
    }
}

static void PLAYERUI_UpdateVolume(int player)
{
    int left=0,right=0;
    double vol;
    
    AUDIOOUTPUT_GetVolumeLevel(player,&left,&right);
    vol=(double)left;
    SDL_WidgetPropertiesOf(UI_Players[player].Normal.VolumeLeft,SET_CUR_VALUE,vol);
    vol=(double)right;
    SDL_WidgetPropertiesOf(UI_Players[player].Normal.VolumeRight,SET_CUR_VALUE,vol);
}

static void PLAYERUI_UpdateFileInfo(int player)
{
    char label[255];
    sprintf(label,"%d Smpls",PLAYER_GetSamplerate(player));
    SDL_WidgetPropertiesOf(UI_Players[player].Normal.Samplerate,SET_CAPTION,label);
    sprintf(label,"%d KBit",PLAYER_GetBitrate(player)/1000);
    SDL_WidgetPropertiesOf(UI_Players[player].Normal.Bitrate,SET_CAPTION,label);
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
    SDL_WidgetPropertiesOf(UI_Players[player].Normal.PlayerState,SET_CAPTION,label);
}

static void PLAYERUI_PlayButton(void *data)
{
    char title[255];
    PlayerDisplay *current=(PlayerDisplay*)data;

    memset(title,0,255);
    PLAYER_GetTitle(current->PlayerNr,title);
    
    SDL_WidgetPropertiesOf(UI_Players[current->PlayerNr].Info.EditTitle,SET_CAPTION,title);

    TRACE("PLAYERUI_PlayButton %d",current->PlayerNr);    

    if(PLAYER_IsPlaying(current->PlayerNr))
    {
        SDL_WidgetPropertiesOf(UI_Players[0].Normal.ButtonPlay,SET_VISIBLE,1);
        SDL_WidgetPropertiesOf(UI_Players[0].Normal.ButtonPause,SET_VISIBLE,0);
        PLAYER_Pause (current->PlayerNr);
    }
    else
    {
        PLAYER_Play  (current->PlayerNr);
        SONGDBUI_Play(current->PlayerNr);
    }
}

/* Event handler for the progress bar SDL_CLICKED*/
void UI_ProgressBarClicked(void *playerdata)
{
    PlayerDisplay *Player= (PlayerDisplay*)playerdata;
    SDL_Widget *ProgressBar=(SDL_Widget*)UI_Players[Player->PlayerNr].Normal.SongProgress;

    int time=((SDL_ProgressBar*)ProgressBar)->CurrentValue;

    PLAYER_SetTimePlayed(Player->PlayerNr,time/100);
}


/* Callback function for pitch slider */
static void PLAYERUI_SetSpeed(void *data)
{
    double curval;
    PlayerDisplay *active=(PlayerDisplay*)data;
   
    SDL_WidgetPropertiesOf(active->Normal.Pitch,GET_CUR_VALUE,&curval);
    curval=2.0-curval;
    PLAYER_SetSpeed(active->PlayerNr,curval);
}
