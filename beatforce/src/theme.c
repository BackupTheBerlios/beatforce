/*
   BeatForce
   theme.c  - theme manager

   Thanks to the people from XMMS (xmms.org) from which this code was taken.
	   (Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public Licensse as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 */


#include <stdio.h>
#include <memory.h>
#include <string.h>

#include <SDL/SDL.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "theme.h"
#include "config.h"
#include "osa.h"
#include "configfile.h"


#define MODULE_ID THEME
#include "debug.h"

/* Window parsers */
ThemeMainWindow *XML_ParseMainwindow(xmlDocPtr doc, xmlNodePtr cur);
ThemeSearchWindow *XML_ParseSearchwindow(xmlDocPtr doc, xmlNodePtr cur);
ThemeFileWindow *XML_ParseFilewindow(xmlDocPtr doc, xmlNodePtr cur);

/* Mainwindow group parser */
ThemePlayer *XML_ParsePlayer(xmlDocPtr doc, xmlNodePtr cur);
ThemePlaylist *XML_ParsePlaylist(xmlDocPtr doc, xmlNodePtr cur);
ThemeClock *XML_ParseClock(xmlDocPtr doc, xmlNodePtr cur);
ThemeMixer *XML_ParseMixer(xmlDocPtr doc, xmlNodePtr cur);
ThemeSongdb *XML_ParseSongdb(xmlDocPtr doc, xmlNodePtr cur);

/* Widget Parser */
ThemeImage *XML_ParseImage(ThemeImage *image,xmlDocPtr doc, xmlNodePtr cur);
ThemeFont *XML_ParseFont(xmlDocPtr doc, xmlNodePtr cur);
ThemeText *XML_ParseText(ThemeText *text,xmlDocPtr doc, xmlNodePtr cur);
ThemeButton *XML_ParseButton(ThemeButton *button,xmlDocPtr doc, xmlNodePtr cur);
ThemeTable *XML_ParseTable(ThemeTable *pp,xmlDocPtr doc, xmlNodePtr cur);

static ThemeConfig *active;

int THEME_Init()
{
    BFList *dir;
    int NoOfThemes;
    ThemeConfig *current;
//    ConfigFile *themecfg;
    char themepath[255];
    xmlNodePtr cur;
//    long size;
    xmlDocPtr doc=NULL;
//    FILE *fp;
    char *buffer;
    xmlChar *key;

#if 0
    fp=fopen("c:\\skin.xml","rb");
    fseek(fp,0,SEEK_END);
    size=ftell(fp);
    fseek(fp,0,SEEK_SET);
    buffer=malloc(sizeof(char)*(size+1));
    memset(buffer,0,size+1);
    fread(buffer,1,size,fp);
    fclose(fp);
#endif

    current=(ThemeConfig*)malloc(sizeof(ThemeConfig));
    memset(current,0,sizeof(ThemeConfig));

    dir=OSA_FindDirectories(THEME_DIR);
    if(dir == NULL)
        return 0;

    NoOfThemes=LLIST_NoOfEntries(dir);
//    if(NoOfThemes == 1)
    {
        /* use it */
        LIBXML_TEST_VERSION
        xmlKeepBlanksDefault(0);

         /*
          * build an XML tree from a the file;
          */
        doc = xmlParseFile(THEME_DIR"/beatforce/skin.xml");
        if (doc == NULL) 
            return 0;
            
        cur = xmlDocGetRootElement(doc);
        if (cur == NULL) 
        {
            fprintf(stderr,"empty document\n");
	        xmlFreeDoc(doc);
	        return 0;
        }
        
        /* CHeck the root node */
        if (xmlStrcmp(cur->name, (const xmlChar *) "Beatforce")) 
        {
            fprintf(stderr,"document of the wrong type, root node != Helping");
	        xmlFreeDoc(doc);
	        return 0;
        }

        current->Screen=malloc(sizeof(ThemeScreen));
        /* Check for aditional parameters */
	    key = xmlGetProp(cur, "fullscreen");
        if(key)
        {
            current->Screen->FullScreen=atoi(key);
            xmlFree(key);
        }
        key = xmlGetProp(cur, "noframe");
        if(key)
        {
            current->Screen->NoFrame=atoi(key);
            xmlFree(key);
        }
        key = xmlGetProp(cur, "bpp");
        if(key)
        {
            current->Screen->BPP=atoi(key);
            xmlFree(key);
        }
        key = xmlGetProp(cur, "width");
        if(key)
        {
            current->Screen->Width=atoi(key);
            xmlFree(key);
        }
        key = xmlGetProp(cur, "height");
        if(key)
        {
            current->Screen->Height=atoi(key);
            xmlFree(key);
        }

        /* Go one level deeper */
        cur = cur->xmlChildrenNode;
        while (cur != NULL) 
        {
            if((!xmlStrcmp(cur->name, (const xmlChar *)"fonts"))) 
            {
                current->Font=XML_ParseFont(doc,cur);
            }
	        if ((!xmlStrcmp(cur->name, (const xmlChar *)"mainwindow"))) 
            {
                current->MainWindow=XML_ParseMainwindow(doc,cur);
 	        }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"filewindow"))) 
            {
                current->FileWindow=XML_ParseFilewindow(doc,cur);
 	        }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"searchwindow"))) 
            {
                current->SearchWindow=XML_ParseSearchwindow(doc,cur);
            }
	        cur = cur->next;
	    }
        active=current;

    }
    
    return 1;
}

ThemeConfig *THEME_GetActive()
{
    return active;
}

int StorePropertyAsInt(xmlNodePtr cur,char *property,int *value)
{
    xmlChar *key;
    key = xmlGetProp(cur, property);
    if(key)
    {
        *value=atoi(key);
        xmlFree(key);
        return 1;
    }
    return 0;
}

int StorePropertyAsShort(xmlNodePtr cur,char *property,short *value)
{
    int retval;
    xmlChar *key;

    key = xmlGetProp(cur, property);
    if(key)
    {
        retval=atoi(key);
        *value=(short)retval;
        xmlFree(key);
        return 1;
    }
    return 0;
}

int StorePropertyAsString(xmlNodePtr cur,char *property,char **string)
{
    xmlChar *key;
    key = xmlGetProp(cur, property);
    if(key)
    {
        *string=strdup(key);
        xmlFree(key);
        return 1;
    }
    return 0;
}

void RebuildFilename(char **string)
{
    char filename[255];

    if(*string)
    {
        sprintf(filename,THEME_DIR"/beatforce/%s",*string);
        free(*string);
        *string=malloc(strlen(filename));
        sprintf(*string,"%s",filename);
    }
}

ThemeFileWindow *XML_ParseFilewindow(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeFileWindow *filewindow;

    filewindow=NULL;
    cur = cur->xmlChildrenNode;

    if(cur)
    {

        filewindow=malloc(sizeof(ThemeFileWindow));
        memset(filewindow,0,sizeof(ThemeFileWindow));

        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"image"))) 
            {
                filewindow->Image=XML_ParseImage(filewindow->Image,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"button"))) 
            {
                filewindow->Button=XML_ParseButton(filewindow->Button,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"table"))) 
            {
                filewindow->Table=XML_ParseTable(filewindow->Table,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"text"))) 
            {
                filewindow->Text=XML_ParseText(filewindow->Text,doc,cur);
            }
            cur=cur->next;
        }
    }
    return filewindow;
}

ThemeSearchWindow *XML_ParseSearchwindow(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeSearchWindow *searchwindow;

    searchwindow=NULL;
    cur = cur->xmlChildrenNode;

    if(cur)
    {

        searchwindow=malloc(sizeof(ThemeSearchWindow));
        memset(searchwindow,0,sizeof(ThemeSearchWindow));

        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"image"))) 
            {
                searchwindow->Image=XML_ParseImage(searchwindow->Image,doc,cur);
            }
            cur=cur->next;
        }
    }
    return searchwindow;
}

ThemeMainWindow *XML_ParseMainwindow(xmlDocPtr doc, xmlNodePtr cur)
{
	ThemeMainWindow *mainwindow;

    mainwindow=NULL;
    cur = cur->xmlChildrenNode;

    if(cur)
    {

        mainwindow=malloc(sizeof(ThemeMainWindow));
        memset(mainwindow,0,sizeof(ThemeMainWindow));

        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"image"))) 
            {
                mainwindow->Image=XML_ParseImage(mainwindow->Image,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"clock"))) 
            {
                mainwindow->Clock=XML_ParseClock(doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"player"))) 
            {
                int id;
                StorePropertyAsInt(cur,"id",&id);
                if(id == 0 || id ==1)
                    mainwindow->Player[id]=XML_ParsePlayer(doc,cur);

            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"songdb"))) 
            {
                mainwindow->Songdb=XML_ParseSongdb(doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"playlist"))) 
            {
                mainwindow->Playlist=XML_ParsePlaylist(doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mixer"))) 
            {
                mainwindow->Mixer=XML_ParseMixer(doc,cur);
            }
            cur=cur->next; 
        }
    }
    else
    {
        return NULL;
    }
    return mainwindow;

}

int hex2int(char c)
{
    if(c >= '0' && c <= '9')
        return c - '0';
    if(c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if(c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0;
}


unsigned int getcolor(char *string)
{
    unsigned int color=0;
    int size,i;
    int l=1;
    if(string)
    {
        string+=2;
        size=strlen(string);

        for(i=size-1;i>=0;i--)
        {
            color+=(hex2int(string[i])*l);
            l*=16;
        }
        return color;
    }
    else
    {
        return 0;
    }

}

ThemeClock *XML_ParseClock(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeClock *clock;
    char *color;
    clock=malloc(sizeof(ThemeClock));
    memset(clock,0,sizeof(ThemeClock));

    StorePropertyAsShort(cur,"x",&clock->Rect.x);
    StorePropertyAsShort(cur,"y",&clock->Rect.y);
    StorePropertyAsShort(cur,"w",&clock->Rect.w);
    StorePropertyAsShort(cur,"h",&clock->Rect.h);

    StorePropertyAsString(cur,"bgcolor",&color);
    clock->bgcolor=getcolor(color);
    free(color);
    StorePropertyAsString(cur,"fgcolor",&color);

    clock->fgcolor=getcolor(color);
    free(color);

    StorePropertyAsString(cur,"font",&clock->font);
    return clock;
}

ThemeImage *XML_ParseImage(ThemeImage *image,xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeImage *pImage;

    if(image == NULL)
    {
        pImage=malloc(sizeof(ThemeImage));
        memset(pImage,0,sizeof(ThemeImage));
                    
        StorePropertyAsString(cur,"filename",&pImage->filename);
        RebuildFilename(&pImage->filename);
        
        StorePropertyAsShort(cur,"x",&pImage->Rect.x);
        StorePropertyAsShort(cur,"y",&pImage->Rect.y);
        StorePropertyAsShort(cur,"w",&pImage->Rect.w);
        StorePropertyAsShort(cur,"h",&pImage->Rect.h);
        return pImage;
    }
    else
    {
        ThemeImage *last;
        last=image;
        while(last->next)
            last=last->next;

        last->next=malloc(sizeof(ThemeImage));
        memset(last->next,0,sizeof(ThemeImage));
        
        StorePropertyAsString(cur,"filename",&last->next->filename);
        RebuildFilename(&last->next->filename);
        StorePropertyAsShort(cur,"x",&last->next->Rect.x);
        StorePropertyAsShort(cur,"y",&last->next->Rect.y);
        StorePropertyAsShort(cur,"w",&last->next->Rect.w);
        StorePropertyAsShort(cur,"h",&last->next->Rect.h);
        return image;    
    }
    
}

ThemeLabel *XML_ParseLabel(ThemeLabel *label,xmlDocPtr doc, xmlNodePtr cur)
{
    if(label == NULL)
    {
        label=malloc(sizeof(ThemeLabel));
        memset(label,0,sizeof(ThemeLabel));
                    
        StorePropertyAsShort(cur,"x",&label->Rect.x);
        StorePropertyAsShort(cur,"y",&label->Rect.y);
        StorePropertyAsShort(cur,"w",&label->Rect.w);
        StorePropertyAsShort(cur,"h",&label->Rect.h);
        
    }
    else
    {
        ThemeLabel *last;
        last=label;
        while(last->next)
            last=last->next;

        last->next=malloc(sizeof(ThemeLabel));
        memset(last->next,0,sizeof(ThemeLabel));

        StorePropertyAsShort(cur,"x",&last->next->Rect.x);
        StorePropertyAsShort(cur,"y",&last->next->Rect.y);
        StorePropertyAsShort(cur,"w",&last->next->Rect.w);
        StorePropertyAsShort(cur,"h",&last->next->Rect.h);
        
    }
    return label;
}


ThemeButton *XML_ParseButton(ThemeButton *button,xmlDocPtr doc, xmlNodePtr cur)
{
    char *action;

    if(button == NULL)
    {
        button=malloc(sizeof(ThemeButton));
        memset(button,0,sizeof(ThemeButton));
                    
        StorePropertyAsString(cur,"normal",&button->normal);
        RebuildFilename(&button->normal);
        StorePropertyAsString(cur,"highlight",&button->highlighted);
        RebuildFilename(&button->highlighted);
        StorePropertyAsString(cur,"pressed",&button->pressed);
        RebuildFilename(&button->pressed);
        
        StorePropertyAsString(cur,"action",&action);
        if(action)
        {
            if(!strcmp(action,"PLAY"))
                button->action=BUTTON_PLAY;
            if(!strcmp(action,"PAUSE"))
                button->action=BUTTON_PAUSE;
            if(!strcmp(action,"RESET_FADER"))
                button->action=BUTTON_RESET_FADER;
            if(!strcmp(action,"CHANGE_DIR"))
                button->action=BUTTON_CHANGE_DIR;
            if(!strcmp(action,"REMOVE"))
                button->action=BUTTON_REMOVE;
            if(!strcmp(action,"ADD"))
                button->action=BUTTON_ADD;
            if(!strcmp(action,"RENAME"))
                button->action=BUTTON_RENAME;
            if(!strcmp(action,"ADDSELECTED"))
                button->action=BUTTON_ADDSELECTED;
            if(!strcmp(action,"DELETESELECTED"))
                button->action=BUTTON_DELETESELECTED;
            free(action);
        }

        StorePropertyAsShort(cur,"x",&button->Rect.x);
        StorePropertyAsShort(cur,"y",&button->Rect.y);
        StorePropertyAsShort(cur,"w",&button->Rect.w);
        StorePropertyAsShort(cur,"h",&button->Rect.h);
    }
    else
    {
        ThemeButton *last;
        last=button;
        while(last->next)
            last=last->next;

        last->next=malloc(sizeof(ThemeButton));
        memset(last->next,0,sizeof(ThemeButton));
        
        StorePropertyAsString(cur,"normal",&last->next->normal);
        RebuildFilename(&last->next->normal);
        StorePropertyAsString(cur,"highlight",&last->next->highlighted);
        RebuildFilename(&last->next->highlighted);
        StorePropertyAsString(cur,"pressed",&last->next->pressed);
        RebuildFilename(&last->next->pressed);
        
        StorePropertyAsString(cur,"action",&action);
        if(action)
        {
            if(!strcmp(action,"PLAY"))
                last->next->action=BUTTON_PLAY;
            if(!strcmp(action,"PAUSE"))
                last->next->action=BUTTON_PAUSE;
            if(!strcmp(action,"RESET_FADER"))
                last->next->action=BUTTON_RESET_FADER;
            if(!strcmp(action,"CHANGE_DIR"))
                last->next->action=BUTTON_CHANGE_DIR;
            if(!strcmp(action,"REMOVE"))
                last->next->action=BUTTON_REMOVE;
            if(!strcmp(action,"ADD"))
                last->next->action=BUTTON_ADD;
            if(!strcmp(action,"RENAME"))
                last->next->action=BUTTON_RENAME;
            if(!strcmp(action,"ADDSELECTED"))
                last->next->action=BUTTON_ADDSELECTED;
            if(!strcmp(action,"DELETESELECTED"))
                last->next->action=BUTTON_DELETESELECTED;

            free(action);
        }

        StorePropertyAsShort(cur,"x",&last->next->Rect.x);
        StorePropertyAsShort(cur,"y",&last->next->Rect.y);
        StorePropertyAsShort(cur,"w",&last->next->Rect.w);
        StorePropertyAsShort(cur,"h",&last->next->Rect.h);

    }
    return button;        
}

ThemeText *XML_ParseText(ThemeText *text,xmlDocPtr doc, xmlNodePtr cur)
{
    char *display=NULL;
    char *color  =NULL;

    if(text == NULL)
    {
        text=malloc(sizeof(ThemeText));
        memset(text,0,sizeof(ThemeText));
        
        StorePropertyAsString(cur,"font",&text->font);
        StorePropertyAsString(cur,"display",&display);
        
        if(display)
        {
            if(!strcmp(display,"TIME_ELAPSED"))
                text->display=TEXT_TIME_ELAPSED;
            if(!strcmp(display,"TIME_REMAINING"))
                text->display=TEXT_TIME_REMAINING;
            if(!strcmp(display,"SONG_TITLE"))
                text->display=TEXT_SONG_TITLE;
            if(!strcmp(display,"PLAYER_STATE"))
                text->display=TEXT_PLAYER_STATE;
            if(!strcmp(display,"SAMPLERATE"))
                text->display=TEXT_SAMPLERATE;
            if(!strcmp(display,"BITRATE"))
                text->display=TEXT_BITRATE;
            free(display);
            display=NULL;
        }
        
        StorePropertyAsString(cur,"fgcolor",&color);
        if(color)
        {
            text->fgcolor=getcolor(color);
            free(color);
            color=NULL;
        }

        StorePropertyAsShort(cur,"x",&text->Rect.x);
        StorePropertyAsShort(cur,"y",&text->Rect.y);
        StorePropertyAsShort(cur,"w",&text->Rect.w);
        StorePropertyAsShort(cur,"h",&text->Rect.h);

        
        
    }
    else
    {
        ThemeText *last;
        last=text;
        while(last->next)
            last=last->next;

        last->next=malloc(sizeof(ThemeText));
        memset(last->next,0,sizeof(ThemeText));

        StorePropertyAsString(cur,"font",&last->next->font);
        StorePropertyAsString(cur,"display",&display);
        
        if(display)
        {
            if(!strcmp(display,"TIME_ELAPSED"))
                last->next->display=TEXT_TIME_ELAPSED;
            if(!strcmp(display,"TIME_REMAINING"))
                last->next->display=TEXT_TIME_REMAINING;
            if(!strcmp(display,"SONG_ARTIST"))
                last->next->display=TEXT_SONG_ARTIST;
            if(!strcmp(display,"SONG_TITLE"))
                last->next->display=TEXT_SONG_TITLE;
            if(!strcmp(display,"PLAYER_STATE"))
                last->next->display=TEXT_PLAYER_STATE;
            if(!strcmp(display,"SAMPLERATE"))
                last->next->display=TEXT_SAMPLERATE;
            if(!strcmp(display,"BITRATE"))
                last->next->display=TEXT_BITRATE;

            free(display);
        } 

        StorePropertyAsString(cur,"fgcolor",&color);
        if(color)
        {
            last->next->fgcolor=getcolor(color);
            free(color);
            color=NULL;
        }

        StorePropertyAsShort(cur,"x",&last->next->Rect.x);
        StorePropertyAsShort(cur,"y",&last->next->Rect.y);
        StorePropertyAsShort(cur,"w",&last->next->Rect.w);
        StorePropertyAsShort(cur,"h",&last->next->Rect.h);
    }
    return text;    
}

ThemeVolumeBar *XML_ParseVolumeBar(ThemeVolumeBar *volumebar, xmlDocPtr doc, 
                                   xmlNodePtr cur)
{
    char *display=NULL;

    if(volumebar == NULL)
    {
        volumebar=malloc(sizeof(ThemeVolumeBar));
        memset(volumebar,0,sizeof(ThemeVolumeBar));
        
        StorePropertyAsString(cur,"display",&display);
        
        if(display) 
        {
            if(!strcmp(display,"VOLUME_LEFT"))
                volumebar->display=VOLUMEBAR_VOLUME_LEFT;
            if(!strcmp(display,"VOLUME_RIGHT"))
                volumebar->display=VOLUMEBAR_VOLUME_RIGHT;
            free(display);
            display=NULL;
        }
        

        StorePropertyAsShort(cur,"x",&volumebar->Rect.x);
        StorePropertyAsShort(cur,"y",&volumebar->Rect.y);
        StorePropertyAsShort(cur,"w",&volumebar->Rect.w);
        StorePropertyAsShort(cur,"h",&volumebar->Rect.h);

        
        
    }
    else
    {
        ThemeVolumeBar *last;
        last=volumebar;
        while(last->next)
            last=last->next;

        last->next=malloc(sizeof(ThemeVolumeBar));
        memset(last->next,0,sizeof(ThemeVolumeBar));

        StorePropertyAsString(cur,"display",&display);
        
        if(display)
        {
            if(!strcmp(display,"VOLUME_LEFT"))
                last->next->display=VOLUMEBAR_VOLUME_LEFT;
            if(!strcmp(display,"VOLUME_RIGHT"))
                last->next->display=VOLUMEBAR_VOLUME_RIGHT;
            free(display);
            display=NULL;
        }
        
        StorePropertyAsShort(cur,"x",&last->next->Rect.x);
        StorePropertyAsShort(cur,"y",&last->next->Rect.y);
        StorePropertyAsShort(cur,"w",&last->next->Rect.w);
        StorePropertyAsShort(cur,"h",&last->next->Rect.h);
    }
    return volumebar;    

}

ThemeProgressBar *XML_ParseProgressBar(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeProgressBar *progressbar;
    
    progressbar=malloc(sizeof(ThemeProgressBar));
    memset(progressbar,0,sizeof(ThemeProgressBar));


    StorePropertyAsShort(cur,"x",&progressbar->Rect.x);
    StorePropertyAsShort(cur,"y",&progressbar->Rect.y);
    StorePropertyAsShort(cur,"w",&progressbar->Rect.w);
    StorePropertyAsShort(cur,"h",&progressbar->Rect.h);
    
    return progressbar;
}

ThemeSlider *XML_ParseSlider(ThemeSlider *slider,xmlDocPtr doc, xmlNodePtr cur)
{
    char *action;

    if(slider == NULL)
    {
        slider=malloc(sizeof(ThemeSlider));
        memset(slider,0,sizeof(ThemeSlider));
    
        StorePropertyAsString(cur,"button",&slider->button);
        RebuildFilename(&slider->button);

        StorePropertyAsString(cur,"action",&action);
        if(action)
        {
            if(!strcmp(action,"MAIN_VOLUME"))
                slider->action=SLIDER_MAIN_VOLUME;
            if(!strcmp(action,"FADER"))
                slider->action=SLIDER_FADER;
            if(!strcmp(action,"SPEED"))
                slider->action=SLIDER_SPEED;
            if(!strcmp(action,"PITCH"))
                slider->action=SLIDER_PITCH;
            free(action);
        }

        StorePropertyAsShort(cur,"x",&slider->Rect.x);
        StorePropertyAsShort(cur,"y",&slider->Rect.y);
        StorePropertyAsShort(cur,"w",&slider->Rect.w);
        StorePropertyAsShort(cur,"h",&slider->Rect.h);
    }
    else
    {
        ThemeSlider *last;
        last=slider;

        while(last->next)
            last=last->next;

        last->next=malloc(sizeof(ThemeSlider));
        memset(last->next,0,sizeof(ThemeSlider));

        StorePropertyAsString(cur,"button",&last->next->button);
        RebuildFilename(&last->next->button);

        StorePropertyAsString(cur,"action",&action);
        if(action)
        {
            if(!strcmp(action,"MAIN_VOLUME"))
                last->next->action=SLIDER_MAIN_VOLUME;
            if(!strcmp(action,"FADER"))
                last->next->action=SLIDER_FADER;
            if(!strcmp(action,"SPEED"))
                last->next->action=SLIDER_SPEED;
            if(!strcmp(action,"PITCH"))
                last->next->action=SLIDER_PITCH;
            free(action);
        }
        StorePropertyAsShort(cur,"x",&last->next->Rect.x);
        StorePropertyAsShort(cur,"y",&last->next->Rect.y);
        StorePropertyAsShort(cur,"w",&last->next->Rect.w);
        StorePropertyAsShort(cur,"h",&last->next->Rect.h);
    }
    
    return slider;
}


ThemeMixer *XML_ParseMixer(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeMixer *mixer;
    mixer=NULL;

    cur = cur->xmlChildrenNode;

    if(cur)
    {
        mixer=malloc(sizeof(ThemeMixer));
        memset(mixer,0,sizeof(ThemeMixer));
        while(cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"image"))) 
            {
                mixer->Image=XML_ParseImage(mixer->Image,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"button")))
            {
                mixer->Button=XML_ParseButton(mixer->Button,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"volumebar")))
            {
                mixer->VolumeBar=XML_ParseVolumeBar(mixer->VolumeBar,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"slider")))
            {
                mixer->Slider=XML_ParseSlider(mixer->Slider,doc,cur);
            }
            cur=cur->next;
        }
    }

    return mixer;

}


ThemePlayer *XML_ParsePlayer(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemePlayer *player;
    player=NULL;

    cur = cur->xmlChildrenNode;

    if(cur)
    {

        player=malloc(sizeof(ThemePlayer));
        memset(player,0,sizeof(ThemePlayer));

        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"image"))) 
            {
                player->Image=XML_ParseImage(player->Image,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"button")))
            {
                player->Button=XML_ParseButton(player->Button,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"text")))
            {
                player->Text=XML_ParseText(player->Text,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"volumebar")))
            {
                player->VolumeBar=XML_ParseVolumeBar(player->VolumeBar,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"progressbar")))
            {
                player->ProgressBar=XML_ParseProgressBar(doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"slider")))
            {
                player->Slider=XML_ParseSlider(player->Slider,doc,cur);
            }
            cur=cur->next;
        }
    }
    else
    {
        return NULL;
    }
    return player;
}

ThemeTable *XML_ParseTable(ThemeTable *table,xmlDocPtr doc, xmlNodePtr cur)
{
    char *contents = NULL;

    if(table == NULL)
    {
        table=malloc(sizeof(ThemeTable));
        memset(table,0,sizeof(ThemeTable));

        StorePropertyAsShort(cur,"x",&table->Rect.x);
        StorePropertyAsShort(cur,"y",&table->Rect.y);
        StorePropertyAsShort(cur,"w",&table->Rect.w);
        StorePropertyAsShort(cur,"h",&table->Rect.h);

        StorePropertyAsInt(cur,"columns",&table->Columns);
        StorePropertyAsInt(cur,"rows"   ,&table->Rows);
   
        StorePropertyAsString(cur,"contents",&contents);
        if(contents)
        {
            if(!strcmp(contents,"SUBGROUPS"))
                table->contents=CONTENTS_SUBGROUPS;
            if(!strcmp(contents,"FILESINDIRECTORY"))
                table->contents=CONTENTS_FILESINDIRECTORY;
            if(!strcmp(contents,"FILESINSUBGROUP"))
                table->contents=CONTENTS_FILESINSUBGROUP;
            free(contents);
        }
    }
    else
    {
        ThemeTable *last;
        last=table;
        while(last->next)
            last=last->next;
    
        last->next=malloc(sizeof(ThemeTable));
        memset(last->next,0,sizeof(ThemeTable));

        StorePropertyAsShort(cur,"x",&last->next->Rect.x);
        StorePropertyAsShort(cur,"y",&last->next->Rect.y);
        StorePropertyAsShort(cur,"w",&last->next->Rect.w);
        StorePropertyAsShort(cur,"h",&last->next->Rect.h);

        StorePropertyAsInt(cur,"columns",&last->next->Columns);
        StorePropertyAsInt(cur,"rows"   ,&last->next->Rows);
   
        StorePropertyAsString(cur,"contents",&contents);
        if(contents)
        {
            if(!strcmp(contents,"SUBGROUPS"))
                last->next->contents=CONTENTS_SUBGROUPS;
            if(!strcmp(contents,"FILESINDIRECTORY"))
                last->next->contents=CONTENTS_FILESINDIRECTORY;
            if(!strcmp(contents,"FILESINSUBGROUP"))
                last->next->contents=CONTENTS_FILESINSUBGROUP;
            free(contents);
        }
    }

    

    return table;

}

ThemeSongdb *XML_ParseSongdb(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeSongdb *songdb;
    songdb=NULL;

    cur = cur->xmlChildrenNode;

    if(cur)
    {
        songdb=malloc(sizeof(ThemeSongdb));
        memset(songdb,0,sizeof(ThemeSongdb));

        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"table"))) 
            {
                songdb->Table=XML_ParseTable(songdb->Table,doc,cur);
            }
            if((!xmlStrcmp(cur->name, (const xmlChar *)"button"))) 
            {
                songdb->Button=XML_ParseButton(songdb->Button,doc,cur);
            }
            cur=cur->next;
        }
    }
    else
    {
        return NULL;
    }
    return songdb;
}

ThemePlaylist *XML_ParsePlaylist(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemePlaylist *playlist;
    playlist=NULL;

    cur = cur->xmlChildrenNode;

    if(cur)
    {
        playlist=malloc(sizeof(ThemePlaylist));
        memset(playlist,0,sizeof(ThemePlaylist));

        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"table"))) 
            {
                playlist->Table=XML_ParseTable(playlist->Table,doc,cur);
            }
            cur=cur->next;
        }
    }
    else
    {
        return NULL;
    }
    return playlist;



    
}

ThemeFont *AddFont(ThemeFont *font,xmlDocPtr doc, xmlNodePtr cur)
{

    if(font==NULL)
    {
        font=malloc(sizeof(ThemeFont));
        memset(font,0,sizeof(ThemeFont));
        StorePropertyAsString(cur,"id",&font->id);
        StorePropertyAsString(cur,"filename",&font->filename);
        RebuildFilename(&font->filename);
    }
    else
    {
        ThemeFont *last;
        last=font;
        while(last->next)
            last=last->next;

        last->next=malloc(sizeof(ThemeFont));
        memset(last->next,0,sizeof(ThemeFont));

        StorePropertyAsString(cur,"id",&last->next->id);
        StorePropertyAsString(cur,"filename",&last->next->filename);
        RebuildFilename(&last->next->filename);
    }
    return font;
}

ThemeFont *XML_ParseFont(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeFont *font;
    font=NULL;

    cur = cur->xmlChildrenNode;

    if(cur)
    {
        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"bitmapfont"))) 
            {
                font=AddFont(font,doc,cur);
            }
            cur=cur->next;
        }
    }
    else
    {
        return NULL;
    }
    return font;
    

} 

SDL_Font *THEME_Font(char *fontid)
{
    ThemeFont* font=THEME_GetActive()->Font;
    
    while(font && fontid)
    {
        if(!strcmp(fontid,font->id))
        {
            if(font->font==NULL)
                font->font=SDL_FontInit(font->filename);

            if(font->font==NULL)
            {
                ERROR("Font not found %s",font->filename);
            }
            return font->font;
        }
        font=font->next;
    }
    printf("ERROR font not found\n");
    return NULL;

}
