/*
  Beatforce/ Player user interface

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

#include "SDL_Signal.h"
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

    UI_Players[nr].PlayerNr = nr;
    UI_Players[nr].Images   = NULL;

    /* Background window */
    while(Image)
    {
        t=SDL_WidgetCreateR(SDL_PANEL,Image->Rect);
        SDL_PanelSetImage(t,IMG_Load(Image->filename));
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
            SDL_TooltipCreate(UI_Players[nr].Normal.ButtonPlay,"Plays the loaded song");

            SDL_SignalConnect(UI_Players[nr].Normal.ButtonPlay,"clicked",PLAYERUI_PlayButton,
                              &UI_Players[nr]);
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

            SDL_TooltipCreate(UI_Players[nr].Normal.ButtonPause,"Pauses the playing song");
            SDL_SignalConnect(UI_Players[nr].Normal.ButtonPause,"clicked",PLAYERUI_PlayButton,
                              &UI_Players[nr]);

            
            SDL_WidgetHide(UI_Players[nr].Normal.ButtonPause);
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
#if 0
            SDL_SignalConnect(UI_Players[nr].Normal.ButtonInfo,"clicked",PLAYERUI_HidePlayer,&UI_Players[nr]);
#endif
            
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
        SDL_SignalConnect(UI_Players[nr].Normal.SongProgress,"clicked",
                          UI_ProgressBarClicked,&UI_Players[nr]);
    }

    if(pt->Slider)
    {
        /* Create the pitch slider */
        UI_Players[nr].Normal.Pitch=SDL_WidgetCreateR(SDL_SLIDER,pt->Slider->Rect);
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_BUTTON_IMAGE,IMG_Load(pt->Slider->button));
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_MAX_VALUE,200);
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_MIN_VALUE,0);
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_CUR_VALUE,100);    
        SDL_WidgetPropertiesOf(UI_Players[nr].Normal.Pitch,SET_NORMAL_STEP_SIZE,1);
        SDL_SignalConnect(UI_Players[nr].Normal.Pitch,"value-changed",PLAYERUI_SetSpeed,
                          &UI_Players[nr]);
    }

    UI_Players[nr].State = PLAYERUI_STATE_NORMAL;
}

void PLAYERUI_Redraw()
{
    int i;
    int Players=0;
    if(PLAYER_GetData(0))
        Players++;
    if(PLAYER_GetData(1))
        Players++;

    for(i=0;i<Players;i++)
    {
        PLAYERUI_UpdateArtist(i);
        PLAYERUI_UpdateTitle(i);
        PLAYERUI_UpdateTime(i);
        PLAYERUI_UpdateFileInfo(i);
        PLAYERUI_UpdateVolume(i);
        PLAYERUI_UpdateState(i);
    }
}

/* 
 *
 * Internal functions (called by the functions above)
 * 
 *
 */
static void PLAYERUI_UpdateArtist(int player)
{
    /* Get and set the artist information */
    if(!PLAYER_GetArtist(player,UI_Players[player].Normal.sArtist))
    {
        char *filename;
        PLAYER_GetFilename(player,UI_Players[player].Normal.sArtist);

        filename=OSA_SearchFilename(UI_Players[player].Normal.sArtist);
        if(filename)
            sprintf(UI_Players[player].Normal.sArtist,"%s",filename);
        
    }

    if(UI_Players[player].Normal.Artist)
        SDL_LabelSetText(UI_Players[player].Normal.Artist,UI_Players[player].Normal.sArtist);
}


static void PLAYERUI_UpdateTitle(int player)
{
    static char title[255];

    memset(title,0,255);
    PLAYER_GetTitle(player,title);

    if(UI_Players[player].Normal.Title)
        SDL_LabelSetText(UI_Players[player].Normal.Title,title);

}

static void PLAYERUI_UpdateTime(int player)
{
    static char sRemainingTime[255];
    static char sTotalTime[255];

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
        sprintf(sTotalTime,"%02d:%02d.%02d",min,sec,msec/10);
    }
    else
    {
        sprintf(sTotalTime,"--:--.--");
    }


    SDL_WidgetPropertiesOf(UI_Players[player].Normal.SongProgress,GET_STATE,&state);
    if(state == PROGRESSBAR_DRAG)
    {
        double curvalue;
        int curval;
        curvalue=SDL_ProgressBarGetCurrentValue(UI_Players[player].Normal.SongProgress);
        curval=(int)curvalue*10;
        msec = ((curval % 60000) % 1000)/10;
        sec  = (curval % 60000) / 1000;
        min  =  curval / 60000;
        sprintf(sTotalTime,"%02d:%02d.%02d",min,sec,msec);
    }
    SDL_LabelSetText(UI_Players[player].Normal.TimeElapsed,sTotalTime);

    /* Time remaining */
    if(totaltime)
    {
        msec = ((timeleft % 60000) % 1000)/10;
        sec  = (timeleft % 60000) / 1000;
        min  =  timeleft / 60000;
        sprintf(sRemainingTime,"%02d:%02d.%02d",min,sec,msec);
    }
    else
    {
        sprintf(sRemainingTime,"--:--.--");
    }
    
    if(state == PROGRESSBAR_DRAG)
    {
        double curvalue;
        int curval;
        curvalue=SDL_ProgressBarGetCurrentValue(UI_Players[player].Normal.SongProgress);
        curval=(int)curvalue*10;
        curval=totaltime-curval;
        msec = ((curval % 60000) % 1000)/10;
        sec  = (curval % 60000) / 1000;
        min  =  curval / 60000;
        sprintf(sRemainingTime,"%02d:%02d.%02d",min,sec,msec);
    }

    SDL_LabelSetText(UI_Players[player].Normal.TimeRemaining,sRemainingTime);



    /* Check for the seconds left */
    if(PLAYER_GetData(1))
    {
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
                    if(!PLAYER_SetSong(!player,0))  /* when set_entry is excecuted we only have 1 item thus 0 */
                        ERROR("Could not set song");
                    MIXER_DoFade(1,0);
                    SONGDBUI_Play(!player);
                }
                
            }
            else
            {
                ERROR("Fade in progress");
            }
        }
    }
    else /* If we have only 1 player */
    {
        struct SongDBEntry * e;
        long id;

        if(PLAYER_GetState(player) == PLAYER_PAUSE_EOF)
        {
            id=rand()%SONGDB_GetNoOfEntries();
            e=SONGDB_GetEntryID(id);
            PLAYLIST_AddEntry(player,e);
            PLAYER_SetSong(player,0);  /* when set_entry is excecuted we only have 1 item thus 0 */
            SONGDBUI_Play(player);
            PLAYER_Play(player);
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
    static char Samples[255];
    static char KBit[255];

    sprintf(Samples,"%d Smpls",PLAYER_GetSamplerate(player));
    SDL_LabelSetText(UI_Players[player].Normal.Samplerate,Samples);
    sprintf(KBit,"%d KBit",PLAYER_GetBitrate(player)/1000);
    SDL_LabelSetText(UI_Players[player].Normal.Bitrate,KBit);
}

static void PLAYERUI_UpdateState(int player)
{
    static char label[255];
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
    SDL_LabelSetText(UI_Players[player].Normal.PlayerState,label);

    if(PLAYER_IsPlaying(player))
    {
        SDL_WidgetHide(UI_Players[player].Normal.ButtonPlay);
        SDL_WidgetShow(UI_Players[player].Normal.ButtonPause);
    }
    else
    {
        SDL_WidgetShow(UI_Players[player].Normal.ButtonPlay);
        SDL_WidgetHide(UI_Players[player].Normal.ButtonPause);
    }


}

static void PLAYERUI_PlayButton(void *data)
{
    char title[255];
    PlayerDisplay *current=(PlayerDisplay*)data;

    memset(title,0,255);
    PLAYER_GetTitle(current->PlayerNr,title);

    if(UI_Players[current->PlayerNr].Info.EditTitle != NULL)
        SDL_LabelSetText(UI_Players[current->PlayerNr].Info.EditTitle,title);

    TRACE("PLAYERUI_PlayButton %d",current->PlayerNr);    

    if(PLAYER_IsPlaying(current->PlayerNr))
    {
        PLAYER_Pause (current->PlayerNr);
    }
    else
    {
        if(PLAYER_Play  (current->PlayerNr))
        {
            /* Only update the user interface when a song actually starts to play */
            SONGDBUI_Play(current->PlayerNr);
        }
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
    int curval;
    PlayerDisplay *active=(PlayerDisplay*)data;
   
    curval=SDL_SliderGetCurrentValue(active->Normal.Pitch);   
    curval=200 - curval;

    PLAYER_SetSpeed(active->PlayerNr,curval);
}
