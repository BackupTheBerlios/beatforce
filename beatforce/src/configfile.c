/*
   BeatForce
   configfile.c  - config & config file stuff
   
   Copyright (C) 2004 John Beuving (john.beuving@wanadoo.nl)

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

#include <config.h>

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "osa.h"
#include "configfile.h"

#define MODULE_ID CONFIGFILE
#include "debug.h"

static BeatforceConfig *gBeatforceConfig;

AudioConfig *
CONFIGFILE_ReadAudioConfig (BeatforceConfig *cfgfile)
{
    if(cfgfile->Audio == NULL)
    {
        cfgfile->Audio=malloc(sizeof(AudioConfig));
        memset(cfgfile->Audio,0,sizeof(AudioConfig));
        cfgfile->Audio->RingBufferSize=5;
        cfgfile->Audio->FragmentSize = 832;
        cfgfile->Audio->LowWatermark = 3;
        cfgfile->Audio->HighWatermark = 10;
        cfgfile->Audio->FFTW_Policy = 0;	/* estimate */
        cfgfile->Audio->FFTW_UseWisdom = 0;
        cfgfile->Audio->output_id[0]="oss";
        cfgfile->Audio->device_id[0]="/dev/dsp";
        cfgfile->Audio->output_id[1]="none";
        cfgfile->Audio->device_id[1]="none";
        cfgfile->Audio->output_id[2]="none";
        cfgfile->Audio->device_id[2]="none";
    }
    
    return cfgfile->Audio;
}


BeatforceConfig*
CONFIGFILE_New (void)
{
    BeatforceConfig *cfg;
    cfg = malloc (sizeof (BeatforceConfig));
    memset(cfg, 0 ,sizeof(BeatforceConfig));
    return cfg;
}

AudioConfig *CONFIGFILE_GetCurrentAudioConfig()
{
    if(gBeatforceConfig && gBeatforceConfig->Audio)
        return gBeatforceConfig->Audio;
    return NULL;
}

BeatforceConfig *
CONFIGFILE_OpenFile(char * filename)
{
    BeatforceConfig *cfg;

    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr cur = NULL;
    cfg=NULL;

    LIBXML_TEST_VERSION;
    xmlKeepBlanksDefault(0);
   
    /*
     * build an XML tree from a the file;
     */
    doc = xmlParseFile(filename);
    if (doc == NULL) 
        return NULL;

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

    cfg=malloc(sizeof(BeatforceConfig));
    memset(cfg,0,sizeof(BeatforceConfig));

    cur = cur->xmlChildrenNode;
    while (cur != NULL) 
    {
        if((!xmlStrcmp(cur->name, (const xmlChar *)"Audio"))) 
        {
            if(cfg->Audio == NULL)
            {
                cfg->Audio=malloc(sizeof(AudioConfig));
                memset(cfg->Audio,0,sizeof(AudioConfig));
            }
            cur = cur->xmlChildrenNode;
            while(cur != NULL)
            {
                if(!xmlStrcmp(cur->name,"RingBufferSize"))
                    cfg->Audio->RingBufferSize=atoi(xmlNodeListGetString(doc,cur->xmlChildrenNode,1));
                if(!xmlStrcmp(cur->name,"FragmentSize"))
                    cfg->Audio->FragmentSize=atoi(xmlNodeListGetString(doc,cur->xmlChildrenNode,1));
                if(!xmlStrcmp(cur->name,"LowWatermark"))
                    cfg->Audio->LowWatermark=atoi(xmlNodeListGetString(doc,cur->xmlChildrenNode,1));
                if(!xmlStrcmp(cur->name,"HighWatermark"))
                    cfg->Audio->HighWatermark=atoi(xmlNodeListGetString(doc,cur->xmlChildrenNode,1));
                if(!xmlStrcmp(cur->name,"MasterOutput"))
                    cfg->Audio->output_id[0]=xmlNodeListGetString(doc,cur->xmlChildrenNode,1);
                if(!xmlStrcmp(cur->name,"MasterDevice"))
                    cfg->Audio->device_id[0]=xmlNodeListGetString(doc,cur->xmlChildrenNode,1);

                
                if(cur)
                    cur=cur->next;
            }
        }
        if(cur)
            cur=cur->next;
    }
    xmlCleanupParser();

    return cfg;
}

char *
CONFIGFILE_GetDefaultFilename (void)
{
    static char *filename = NULL;
    if (!filename)
    {
        filename = malloc(255);
        sprintf(filename,"%s%s",OSA_GetConfigDir(),"config.xml");
    }
    return filename;
}


int CONFIGFILE_CreateDir (void)
{
    char *dirname;
    dirname = strdup(OSA_GetConfigDir());
    {
        if (mkdir (dirname, 0755) != 0)
        {
            ERROR("Couldn´t create directory %s.", dirname);
            return 0;
        }
    }
    free (dirname);
    return 1;
}


BeatforceConfig *
CONFIGFILE_CreateDefaultFile (void)
{
    BeatforceConfig *cfgfile;

    cfgfile = CONFIGFILE_New();
    CONFIGFILE_CreateDir ();
    CONFIGFILE_ReadAudioConfig(cfgfile);
    
//    CONFIGFILE_WriteAudioConfig(cfgfile, CONFIGFILE_ReadAudioConfig(cfgfile));

    if (!CONFIGFILE_WriteDefaultFile (cfgfile))
    {
        CONFIGFILE_Free (cfgfile);
        fprintf(stderr,"Couldn't write default config %s", CONFIGFILE_GetDefaultFilename());
        return NULL;
    }

    return cfgfile;
}

void
CONFIGFILE_OpenDefaultFile(void)
{
    BeatforceConfig *ret;
    
    ret = CONFIGFILE_OpenFile(CONFIGFILE_GetDefaultFilename());

    if (!ret)
        ret = CONFIGFILE_CreateDefaultFile();

    gBeatforceConfig=ret;
    
}

int
CONFIGFILE_WriteFile (BeatforceConfig * cfg, char * filename)
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL; /* node pointers */
    xmlNodePtr audio;
    char buff[256];


    LIBXML_TEST_VERSION;

    /* 
     * Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "Beatforce");
    xmlDocSetRootElement(doc, root_node);

    /* 
     * xmlNewChild() creates a new node, which is "attached" as child node
     * of root_node node. 
     */
    audio=xmlNewChild(root_node, NULL, BAD_CAST "Audio",NULL);

    sprintf(buff,"%d",cfg->Audio->RingBufferSize);
    node=xmlNewChild(audio,NULL,BAD_CAST "RingBufferSize",buff);

    sprintf(buff,"%d",cfg->Audio->FragmentSize);
    node=xmlNewChild(audio,NULL,BAD_CAST "FragmentSize",buff);

#if 0
    sprintf(buff,"%d",cfg->Audio->LowWaterMark);
    node=xmlNewChild(audio,NULL,BAD_CAST "LowWaterMark",buff);

    sprintf(buff,"%d",cfg->Audio->HighWaterMark);
    node=xmlNewChild(audio,NULL,BAD_CAST "HighWaterMark",buff);
#endif

    sprintf(buff,"%d",cfg->Audio->FFTW_Policy);
    node=xmlNewChild(audio,NULL,BAD_CAST "FFTWPolicy",buff);

    sprintf(buff,"%d",cfg->Audio->FFTW_UseWisdom);
    node=xmlNewChild(audio,NULL,BAD_CAST "FFTWUseWisdom",buff);

    node=xmlNewChild(audio,NULL,BAD_CAST "MasterOutput",cfg->Audio->output_id[0]);
    node=xmlNewChild(audio,NULL,BAD_CAST "MasterDevice",cfg->Audio->device_id[0]);
    
    node=xmlNewChild(audio,NULL,BAD_CAST "GroupAOutput",cfg->Audio->output_id[1]);
    node=xmlNewChild(audio,NULL,BAD_CAST "GroupADevice",cfg->Audio->device_id[1]);
 
    node=xmlNewChild(audio,NULL,BAD_CAST "GroupBOutput",cfg->Audio->output_id[2]);
    node=xmlNewChild(audio,NULL,BAD_CAST "GroupBDevice",cfg->Audio->device_id[2]);

    /* 
     * Dumping document to stdio or file
     */
    xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();

    return 1;
}

int
CONFIGFILE_WriteDefaultFile (BeatforceConfig * cfg)
{
    return CONFIGFILE_WriteFile (cfg, CONFIGFILE_GetDefaultFilename ());
}

void
CONFIGFILE_Free (BeatforceConfig * cfg)
{

    free (cfg);
    cfg = NULL;
}


