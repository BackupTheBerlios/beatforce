/*
   BeatForce
   theme.c  - theme manager

   Copyright (C) 2003-2004 John Beuving (john.beuving@beatforce.org)

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
#include <sys/stat.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "theme.h"
#include "config.h"
#include "osa.h"
#include "configfile.h"


#define MODULE_ID THEME
#include "debug.h"

#define DEFAULT_THEME "beatforce"

/* Help function */
static int StorePropertyAsString(xmlNodePtr cur,char *property,char **string);
static int StorePropertyAsShort(xmlNodePtr cur,char *property,short *value);
static void RebuildFilename(char **string);

/* Widget Parser */
ThemeImage  *XML_ParseImage(ThemeImage *image,xmlDocPtr doc, xmlNodePtr cur);
ThemeFont   *XML_ParseFont(xmlDocPtr doc, xmlNodePtr cur);
ThemeText   *XML_ParseText(ThemeText *text,xmlDocPtr doc, xmlNodePtr cur);
ThemeButton *XML_ParseButton(ThemeButton *button,xmlDocPtr doc, xmlNodePtr cur);
ThemeTable  *XML_ParseTable(ThemeTable *pp,xmlDocPtr doc, xmlNodePtr cur);
ThemeTree   *XML_ParseTree(xmlDocPtr doc, xmlNodePtr cur);

/* Window parsers */
ThemeMainWindow   *XML_ParseMainwindow(xmlDocPtr doc, xmlNodePtr cur);
ThemeSearchWindow *XML_ParseSearchwindow(xmlDocPtr doc, xmlNodePtr cur);
ThemeFileWindow   *XML_ParseFilewindow(xmlDocPtr doc, xmlNodePtr cur);
ThemeConfigWindow *XML_ParseConfigwindow(xmlDocPtr doc, xmlNodePtr cur);

/* Mainwindow group parser */
ThemePlayer   *XML_ParsePlayer(ThemePlayer *player,xmlDocPtr doc, xmlNodePtr cur);
ThemePlaylist *XML_ParsePlaylist(ThemePlaylist *playlist,xmlDocPtr doc, xmlNodePtr cur);
ThemeClock    *XML_ParseClock(xmlDocPtr doc, xmlNodePtr cur);
ThemeMixer    *XML_ParseMixer(xmlDocPtr doc, xmlNodePtr cur);
ThemeSongdb   *XML_ParseSongdb(xmlDocPtr doc, xmlNodePtr cur);

static ThemeConfig *active;
static BFList      *ThemeList;

ThemeImage *XML_ParseImage(ThemeImage *image,xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeImage *pImage;

    if(image == NULL)
    {
        pImage=malloc(sizeof(ThemeImage));
        memset(pImage,0,sizeof(ThemeImage));
                    
        StorePropertyAsString(cur,"filename",&pImage->filename);
        RebuildFilename(&pImage->filename);
        
        StorePropertyAsShort(cur,"x",&pImage->x);
        StorePropertyAsShort(cur,"y",&pImage->y);
        StorePropertyAsShort(cur,"w",&pImage->w);
        StorePropertyAsShort(cur,"h",&pImage->h);
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
        StorePropertyAsShort(cur,"x",&last->next->x);
        StorePropertyAsShort(cur,"y",&last->next->y);
        StorePropertyAsShort(cur,"w",&last->next->w);
        StorePropertyAsShort(cur,"h",&last->next->h);
        return image;    
    }
    
}

int THEME_Load(char *theme)
{
    char path[255];

    xmlNodePtr cur;
    xmlDocPtr doc=NULL;
    xmlChar *key;
    ThemeConfig *current;

    TRACE("THEME_Load");
    current=(ThemeConfig*)malloc(sizeof(ThemeConfig));
    memset(current,0,sizeof(ThemeConfig));

    if(theme == NULL)
    {
        return 0;
    }

    sprintf(path,"%s/%s/skin.xml",THEME_DIR,theme);

    doc = xmlParseFile(path);    
    if(doc)
    {
        /* use it */
            
        /*
         * build an XML tree from a the file;
         */
        cur = xmlDocGetRootElement(doc);
        if (cur == NULL) 
        {
            ERROR("empty document");
            xmlFreeDoc(doc);
            return 0;
        }
        
        /* CHeck the root node */
        if (xmlStrcmp(cur->name, (const xmlChar *) "Beatforce")) 
        {
            ERROR("document of the wrong type, root node != Beatforce");
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"configwindow"))) 
            {
                current->ConfigWindow=XML_ParseConfigwindow(doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"filewindow"))) 
            {
                current->FileWindow=XML_ParseFilewindow(doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mainwindow"))) 
            {
                current->MainWindow=XML_ParseMainwindow(doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"searchwindow"))) 
            {
                current->SearchWindow=XML_ParseSearchwindow(doc,cur);
            }
            cur = cur->next;
        }
        active=current;
        return 1;
    }
    else
    {
        return 0;
    }

}

int THEME_Init()
{
    BFList *dir=NULL;
    int NoOfThemes=0;

    TRACE("THEME_Init");
    ThemeList = NULL;

    dir=OSA_FindDirectories(THEME_DIR);
    if(dir == NULL)
    {
        ERROR("Couldn't list directories in %s",THEME_DIR);
        return 0;
    }

    while(dir)
    {
        char path[255];
        FILE *fd;
        sprintf(path,"%s/%s/skin.xml",THEME_DIR,(char*)dir->data);

        if((fd=fopen(path,"rb")))
        {  
            fclose(fd);
            ThemeList=LLIST_Append(ThemeList,strdup(dir->data));
            NoOfThemes++;
        }
        free(dir->data);
        dir=dir->next;
    }
    if(NoOfThemes == 0)
    {
        ERROR("No themes found");
        return 0;
    }
    if(NoOfThemes == 1)
        return THEME_Load(ThemeList->data);
    else
        return THEME_Load(DEFAULT_THEME);

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

static int StorePropertyAsShort(xmlNodePtr cur,char *property,short *value)
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

static int StorePropertyAsString(xmlNodePtr cur,char *property,char **string)
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

static void RebuildFilename(char **string)
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

ThemeConfigWindow *XML_ParseConfigwindow(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeConfigWindow *configwindow;

    configwindow=NULL;
    cur = cur->xmlChildrenNode;

    if(cur)
    {
        configwindow=malloc(sizeof(ThemeConfigWindow));
        memset(configwindow,0,sizeof(ThemeConfigWindow));

        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"clock"))) 
            {
                configwindow->Clock=XML_ParseClock(doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"image"))) 
            {
                configwindow->Image=XML_ParseImage(configwindow->Image,doc,cur);
            }
            cur=cur->next;
        }
    }
    return configwindow;
}

ThemeFileWindow *XML_ParseFilewindow(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeFileWindow *filewindow;

    filewindow=NULL;

    filewindow=malloc(sizeof(ThemeFileWindow));
    memset(filewindow,0,sizeof(ThemeFileWindow));

    StorePropertyAsShort(cur,"x",&filewindow->x);
    StorePropertyAsShort(cur,"y",&filewindow->y);
    StorePropertyAsShort(cur,"w",&filewindow->w);
    StorePropertyAsShort(cur,"h",&filewindow->h);
    
    cur = cur->xmlChildrenNode;

    if(cur)
    {
        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"clock"))) 
            {
                filewindow->Clock=XML_ParseClock(doc,cur);
            }
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"tree"))) 
            {
                filewindow->Tree=XML_ParseTree(doc,cur);
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
                mainwindow->Player=XML_ParsePlayer(mainwindow->Player,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"songdb"))) 
            {
                mainwindow->Songdb=XML_ParseSongdb(doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"playlist"))) 
            {
                mainwindow->Playlist=XML_ParsePlaylist(mainwindow->Playlist,doc,cur);
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

    StorePropertyAsShort(cur,"x",&clock->x);
    StorePropertyAsShort(cur,"y",&clock->y);
    StorePropertyAsShort(cur,"w",&clock->w);
    StorePropertyAsShort(cur,"h",&clock->h);

    StorePropertyAsString(cur,"bgcolor",&color);
    clock->bgcolor=getcolor(color);
    free(color);
    StorePropertyAsString(cur,"fgcolor",&color);

    clock->fgcolor=getcolor(color);
    free(color);

    StorePropertyAsString(cur,"font",&clock->font);
    return clock;
}

ThemeTree *XML_ParseTree(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeTree *tree;
#if 0
    char *color;
#endif

    TRACE("XML_ParseTree");
    tree=malloc(sizeof(ThemeTree));
    memset(tree,0,sizeof(ThemeTree));

    StorePropertyAsShort(cur,"x",&tree->x);
    StorePropertyAsShort(cur,"y",&tree->y);
    StorePropertyAsShort(cur,"w",&tree->w);
    StorePropertyAsShort(cur,"h",&tree->h);

#if 0
    StorePropertyAsString(cur,"bgcolor",&color);
    tree->bgcolor=getcolor(color);
    free(color);
    StorePropertyAsString(cur,"fgcolor",&color);

    tree->fgcolor=getcolor(color);
    free(color);

    StorePropertyAsString(cur,"font",&tree->Font);
#endif
    return tree;
}



ThemeEdit *XML_ParseEdit(ThemeEdit *edit,xmlDocPtr doc, xmlNodePtr cur)
{
    if(edit == NULL)
    {
        edit=malloc(sizeof(ThemeEdit));
        memset(edit,0,sizeof(ThemeEdit));
                    
        StorePropertyAsShort(cur,"x",&edit->x);
        StorePropertyAsShort(cur,"y",&edit->y);
        StorePropertyAsShort(cur,"w",&edit->w);
        StorePropertyAsShort(cur,"h",&edit->h);
        
    }
    else
    {
        ThemeEdit *last;
        last=edit;
        while(last->next)
            last=last->next;

        last->next=malloc(sizeof(ThemeEdit));
        memset(last->next,0,sizeof(ThemeEdit));

        StorePropertyAsShort(cur,"x",&last->next->x);
        StorePropertyAsShort(cur,"y",&last->next->y);
        StorePropertyAsShort(cur,"w",&last->next->w);
        StorePropertyAsShort(cur,"h",&last->next->h);
        
    }
    return edit;
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
            if(!strcmp(action,"INFO"))
                button->action=BUTTON_INFO;
            if(!strcmp(action,"PAUSE"))
                button->action=BUTTON_PAUSE;
            if(!strcmp(action,"RESET_FADER"))
                button->action=BUTTON_RESET_FADER;
            if(!strcmp(action,"EDIT_GROUP"))
                button->action=BUTTON_EDIT_GROUP;
            if(!strcmp(action,"REMOVE"))
                button->action=BUTTON_REMOVE;
            if(!strcmp(action,"ADD"))
                button->action=BUTTON_CREATE;
            if(!strcmp(action,"RENAME"))
                button->action=BUTTON_RENAME;
            if(!strcmp(action,"ADDSELECTED"))
                button->action=BUTTON_ADDSELECTED;
            if(!strcmp(action,"ADDALL"))
                button->action=BUTTON_ADDALL;
            if(!strcmp(action,"DELETESELECTED"))
                button->action=BUTTON_DELETESELECTED;
            free(action);
        }

        StorePropertyAsShort(cur,"x",&button->x);
        StorePropertyAsShort(cur,"y",&button->y);
        StorePropertyAsShort(cur,"w",&button->w);
        StorePropertyAsShort(cur,"h",&button->h);
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
            if(!strcmp(action,"INFO"))
                button->action=BUTTON_INFO;
            if(!strcmp(action,"PAUSE"))
                last->next->action=BUTTON_PAUSE;
            if(!strcmp(action,"RESET_FADER"))
                last->next->action=BUTTON_RESET_FADER;
            if(!strcmp(action,"EDIT_GROUP"))
                last->next->action=BUTTON_EDIT_GROUP;
            if(!strcmp(action,"REMOVE"))
                last->next->action=BUTTON_REMOVE;
            if(!strcmp(action,"ADD"))
                last->next->action=BUTTON_CREATE;
            if(!strcmp(action,"ADDALL"))
                last->next->action=BUTTON_ADDALL;
            if(!strcmp(action,"RENAME"))
                last->next->action=BUTTON_RENAME;
            if(!strcmp(action,"ADDSELECTED"))
                last->next->action=BUTTON_ADDSELECTED;
            if(!strcmp(action,"DELETESELECTED"))
                last->next->action=BUTTON_DELETESELECTED;

            free(action);
        }

        StorePropertyAsShort(cur,"x",&last->next->x);
        StorePropertyAsShort(cur,"y",&last->next->y);
        StorePropertyAsShort(cur,"w",&last->next->w);
        StorePropertyAsShort(cur,"h",&last->next->h);

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

        StorePropertyAsShort(cur,"x",&text->x);
        StorePropertyAsShort(cur,"y",&text->y);
        StorePropertyAsShort(cur,"w",&text->w);
        StorePropertyAsShort(cur,"h",&text->h);

        
        
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

        StorePropertyAsShort(cur,"x",&last->next->x);
        StorePropertyAsShort(cur,"y",&last->next->y);
        StorePropertyAsShort(cur,"w",&last->next->w);
        StorePropertyAsShort(cur,"h",&last->next->h);
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
        

        StorePropertyAsShort(cur,"x",&volumebar->x);
        StorePropertyAsShort(cur,"y",&volumebar->y);
        StorePropertyAsShort(cur,"w",&volumebar->w);
        StorePropertyAsShort(cur,"h",&volumebar->h);

        
        
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
        
        StorePropertyAsShort(cur,"x",&last->next->x);
        StorePropertyAsShort(cur,"y",&last->next->y);
        StorePropertyAsShort(cur,"w",&last->next->w);
        StorePropertyAsShort(cur,"h",&last->next->h);
    }
    return volumebar;    

}

ThemeProgressBar *XML_ParseProgressBar(xmlDocPtr doc, xmlNodePtr cur)
{
    ThemeProgressBar *progressbar;
    
    progressbar=malloc(sizeof(ThemeProgressBar));
    memset(progressbar,0,sizeof(ThemeProgressBar));


    StorePropertyAsShort(cur,"x",&progressbar->x);
    StorePropertyAsShort(cur,"y",&progressbar->y);
    StorePropertyAsShort(cur,"w",&progressbar->w);
    StorePropertyAsShort(cur,"h",&progressbar->h);
    
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

        StorePropertyAsShort(cur,"x",&slider->x);
        StorePropertyAsShort(cur,"y",&slider->y);
        StorePropertyAsShort(cur,"w",&slider->w);
        StorePropertyAsShort(cur,"h",&slider->h);
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
        StorePropertyAsShort(cur,"x",&last->next->x);
        StorePropertyAsShort(cur,"y",&last->next->y);
        StorePropertyAsShort(cur,"w",&last->next->w);
        StorePropertyAsShort(cur,"h",&last->next->h);
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


ThemePlayer *XML_ParsePlayer(ThemePlayer *player,xmlDocPtr doc, xmlNodePtr cur)
{
    ThemePlayer *pPlayer;
    pPlayer=NULL;

    cur = cur->xmlChildrenNode;

    if(cur)
    {

        pPlayer=malloc(sizeof(ThemePlayer));
        memset(pPlayer,0,sizeof(ThemePlayer));

        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"image"))) 
            {
                pPlayer->Image=XML_ParseImage(pPlayer->Image,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"button")))
            {
                pPlayer->Button=XML_ParseButton(pPlayer->Button,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"text")))
            {
                pPlayer->Text=XML_ParseText(pPlayer->Text,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"volumebar")))
            {
                pPlayer->VolumeBar=XML_ParseVolumeBar(pPlayer->VolumeBar,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"progressbar")))
            {
                pPlayer->ProgressBar=XML_ParseProgressBar(doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"slider")))
            {
                pPlayer->Slider=XML_ParseSlider(pPlayer->Slider,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"edit")))
            {
                pPlayer->Edit=XML_ParseEdit(pPlayer->Edit,doc,cur);
            }
            cur=cur->next;
        }
        
        if(player == NULL)
        {
            player=pPlayer;
        }
        else
        {
            ThemePlayer *last;
            last=player;
            while(last->Next)
                last=last->Next;
            last->Next=pPlayer;
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

        StorePropertyAsShort(cur,"x",&table->x);
        StorePropertyAsShort(cur,"y",&table->y);
        StorePropertyAsShort(cur,"w",&table->w);
        StorePropertyAsShort(cur,"h",&table->h);

        StorePropertyAsInt(cur,"columns",&table->Columns);
        StorePropertyAsInt(cur,"rows"   ,&table->Rows);
   
        StorePropertyAsString(cur,"contents",&contents);
        if(contents)
        {
            if(!strcmp(contents,"SUBGROUPS"))
                table->ContentType=CONTENTS_SUBGROUPS;
            if(!strcmp(contents,"FILESINDIRECTORY"))
                table->ContentType=CONTENTS_FILESINDIRECTORY;
            if(!strcmp(contents,"FILESINSUBGROUP"))
                table->ContentType=CONTENTS_FILESINSUBGROUP;
            if(!strcmp(contents,"DIRECTORIES"))
                table->ContentType=CONTENTS_DIRECTORIES;
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

        StorePropertyAsShort(cur,"x",&last->next->x);
        StorePropertyAsShort(cur,"y",&last->next->y);
        StorePropertyAsShort(cur,"w",&last->next->w);
        StorePropertyAsShort(cur,"h",&last->next->h);

        StorePropertyAsInt(cur,"columns",&last->next->Columns);
        StorePropertyAsInt(cur,"rows"   ,&last->next->Rows);
   
        StorePropertyAsString(cur,"contents",&contents);
        if(contents)
        {
            if(!strcmp(contents,"SUBGROUPS"))
                last->next->ContentType=CONTENTS_SUBGROUPS;
            if(!strcmp(contents,"FILESINDIRECTORY"))
                last->next->ContentType=CONTENTS_FILESINDIRECTORY;
            if(!strcmp(contents,"FILESINSUBGROUP"))
                last->next->ContentType=CONTENTS_FILESINSUBGROUP;
            if(!strcmp(contents,"DIRECTORIES"))
                last->next->ContentType=CONTENTS_DIRECTORIES;
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
            if((!xmlStrcmp(cur->name, (const xmlChar *)"button"))) 
            {
                songdb->Button=XML_ParseButton(songdb->Button,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"image"))) 
            {
                songdb->Image=XML_ParseImage(songdb->Image,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"table"))) 
            {
                songdb->Table=XML_ParseTable(songdb->Table,doc,cur);
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

ThemePlaylist *XML_ParsePlaylist(ThemePlaylist *playlist,xmlDocPtr doc, xmlNodePtr cur)
{
    ThemePlaylist *pPlaylist;
    pPlaylist=NULL;

    cur = cur->xmlChildrenNode;

    if(cur)
    {
        pPlaylist=malloc(sizeof(ThemePlaylist));
        memset(pPlaylist,0,sizeof(ThemePlaylist));

        while (cur != NULL) 
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"image"))) 
            {
                pPlaylist->Image=XML_ParseImage(pPlaylist->Image,doc,cur);
            }
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"table"))) 
            {
                pPlaylist->Table=XML_ParseTable(pPlaylist->Table,doc,cur);
            }
            cur=cur->next;
        }

        if(playlist == NULL)
        {
            playlist=pPlaylist;
        }
        else
        {
            ThemePlaylist *last;
            last=playlist;
            while(last->Next)
                last=last->Next;

            last->Next=pPlaylist;
        }
    }
    else
    {
        return NULL;
    }
    return playlist;



    
}

ThemeFont *THEME_AddFont(ThemeFont *font,xmlDocPtr doc, xmlNodePtr cur)
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
        while(last->Next)
            last=last->Next;

        last->Next=malloc(sizeof(ThemeFont));
        memset(last->Next,0,sizeof(ThemeFont));

        StorePropertyAsString(cur,"id",&last->Next->id);
        StorePropertyAsString(cur,"filename",&last->Next->filename);
        RebuildFilename(&last->Next->filename);
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
                font=THEME_AddFont(font,doc,cur);

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









