/*
  BeatForce
  songdb.c  -	song database
   
  Copyright (c) 2003, John Beuving (john.beuving@home.nl)

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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#include "configfile.h"
#include "songdb.h"
#include "player.h"
#include "osa.h"

//temp
#include "input.h"
#include "playlist.h"

/* xml input/output */
#include <libxml/tree.h>
#include <libxml/parser.h>

#define MODULE_ID SONGDB
#include "debug.h"


SongDBGroup *MainGroup;

/* search return */
struct SongDBEntry **search_results;
long n_search_results;
long n_index;

/* local prototypes */
struct SongDBEntry *SONGDB_AllocEntry (void);
void songdb_FreeEntry (struct SongDBEntry *e);

/* prototypes ued for searching */
static int SONGDB_JumpToFileMatch(char* song, char * keys[], int nw);
int SONGDB_AddSubgroup(struct SongDBGroup *group,char *title);
struct SongDBSubgroup *songdb_AddSubgroup(struct SongDBSubgroup *sg,char *title);


static int SONGDB_LoadXMLDatabase(); /* Load entire database from xml file */
static int SONGDB_SaveXMLDatabase(); /* Save entire database to xml file */

static int subcount;


int parsesubgroup(xmlDocPtr doc, xmlNodePtr cur)
{
    xmlChar *key;
    long time;
    struct SongDBEntry *e;
    struct SongDBSubgroup *sg;

    key = xmlGetProp(cur, "name");
    if(key)
        SONGDB_AddSubgroup(MainGroup,key);
    
    sg=SONGDB_GetSubgroup();
    /* Go the the last added subgroup */
    while(sg->next)
        sg=sg->next;

    cur = cur->xmlChildrenNode;
    while(cur != NULL)
    {
        if((!xmlStrcmp(cur->name, (const xmlChar *)"song")))                         
        {
            key = xmlGetProp(cur, "filename");
            if(key)
            {
                SONGDB_AddFileToSubgroup(sg,key);
            }
            key = xmlGetProp(cur, "time");
            if(key)
            {
                time= atol(key);
                SONGDB_SetActiveSubgroup(sg);
                e=SONGDB_GetEntryID(SONGDB_GetNoOfEntries()-1);
                if(e)
                    e->time=time;
            }
            
        }
        cur=cur->next;
    }
    subcount++;
    return 1;
}


int SONGDB_Init (SongDBConfig * our_cfg)
{
    MainGroup=malloc(sizeof(SongDBGroup));
    memset(MainGroup,0,sizeof(SongDBGroup));

    search_results = NULL;
    n_search_results = 0;
    n_index = 0;

    srand(time(NULL));
    SONGDB_LoadXMLDatabase();

    MainGroup->Changed  = 1;
    MainGroup->Active   = MainGroup->Subgroup;
    
    return 1;
}

int SONGDB_Exit()
{
    return SONGDB_SaveXMLDatabase();
}

int SONGDB_SubgroupSetVolatile(struct SongDBSubgroup *subgroup)
{
    subgroup->Volatile=1;
    return 1;
}

int SONGDB_AddSubgroup(struct SongDBGroup *group,char *title)
{
    if(title == NULL)
        return 0;

    group->Subgroup=songdb_AddSubgroup(group->Subgroup,title);
    group->Changed=1;
    group->SubgroupCount ++;
    return 1;
}

/* 
 * Return the active group 
 */
SongDBGroup *SONGDB_GetActiveGroup()
{
    return MainGroup;
}

int SONGDB_GetSubgroupCount()
{
    return MainGroup->SubgroupCount;
}

int SONGDB_RenameSubgroup(struct SongDBSubgroup *sg, char *title)
{
    if(sg)
    {
        if(sg->Name)
            free(sg->Name);

        sg->Name=strdup(title);
        MainGroup->Changed=1;
        return 1;
    }
    return 0;

}

int SONGDB_RemoveSubgroup(struct SongDBSubgroup *sg)
{
    int count=0;
    SongDBSubgroup *list;

    list=MainGroup->Subgroup;

    while(list)
    {
        if(list==sg)
        {
            if(list->prev == NULL && list->next)
            {
                MainGroup->Subgroup = list->next;
            }
            else if (list->prev)
            {
                list->prev->next=list->next;
            }
            else
            {
                MainGroup->Subgroup = NULL;
            }

            if(list->next)
                list->next->prev=list->prev;

            free(list->Name);
            free(list);
            MainGroup->Changed=1;
            return 1;
        }
        
        list=list->next;
        count++;
    }
    return 0;
}

int SONGDB_RemovePlaylistEntry(struct SongDBSubgroup *sg,struct SongDBEntry *e)
{
    struct SongDBEntry *list,*prev;

    if(e == NULL)
        return 0;

    if(sg->Playlist)
    {
        list=sg->Playlist;
        prev=NULL;
        while(list)
        {
            if(list==e)
            {
                if(prev)
                    prev->next=list->next;
                else
                    sg->Playlist=list->next;
               
                sg->Songcount--;
                return 1;
            }
            prev=list;
            list=list->next;
            
        }
    }
    return 0;
}
struct SongDBSubgroup *SONGDB_GetSubgroup()
{
    return MainGroup->Subgroup;
}



int SONGDB_AddFileToSubgroup(struct SongDBSubgroup *sg,char *filename)
{
    if(sg)
    {
        struct SongDBEntry *e;
        struct SongDBEntry *Playlist;
        
        if (INPUT_WhoseFile (PLAYER_GetData(0)->ip_plugins, filename) != NULL)
        {
            e = SONGDB_AllocEntry ();
            
            if(e)
            {
                e->filename = strdup (filename);
                e->id       = sg->Songcount++;
                e->next     = NULL;
                
                /* If this is a cd try to get the tag immediatly */
                if(sg->Volatile)
                    INPUT_GetTag(PLAYER_GetData(0)->ip_plugins, filename,e);

                /* Add the created entry to the active database */
                if(sg->Playlist == NULL)
                    sg->Playlist = e;
                else
                {
                    Playlist=sg->Playlist;
                    while(Playlist->next)
                        Playlist=Playlist->next;
                    Playlist->next = e;
                }
            }
        }
        else
        {
            ERROR("No plugin for this file");
            return 0;
        }
    }
    return 1;
}

int SONGDB_GroupChanged()
{
    int retval;
    
    retval = MainGroup->Changed;
    MainGroup->Changed = 0;
    return retval;
}


struct SongDBSubgroup *songdb_AddSubgroup(struct SongDBSubgroup *sg,char *title)
{
    if(sg == NULL)
    {
        sg=malloc(sizeof(SongDBSubgroup));
        memset(sg,0,sizeof(SongDBSubgroup));
        sg->Name=strdup(title);
        sg->Songcount=0;
        sg->Playlist=NULL;
        sg->Volatile=0;
    }
    else
    {
        struct SongDBSubgroup *last;
        last=sg;
        while(last->next)
            last=last->next;

        last->next=malloc(sizeof(SongDBSubgroup));
        memset(last->next,0,sizeof(SongDBSubgroup));
        last->next->Name=strdup(title);
        last->next->Songcount=0;
        last->next->Playlist=NULL;
        last->next->Volatile=0;
        last->next->prev=last;
        
    }
    return sg;
}

struct SongDBEntry *SONGDB_AllocEntry (void)
{
    struct SongDBEntry *e;

    e = malloc (DBENTRY_LEN);
    if (e == NULL)
    {
        perror ("songdb_alloc_entry");
        return NULL;
    }
    memset (e, 0, DBENTRY_LEN);

    return e;
}

void SONGDB_FreeActiveList()
{
    if(MainGroup->Active == NULL)
        return;
    
    while(MainGroup->Active->Playlist)
    {
        songdb_FreeEntry(MainGroup->Active->Playlist);
        MainGroup->Active->Playlist = MainGroup->Active->Playlist->next;
    }
    MainGroup->Active->Songcount=0;

}

void songdb_FreeEntry (struct SongDBEntry *e)
{
    if(e)
    {
        if(e->filename)
            free(e->filename);
        free (e);
    }
}


long SONGDB_GetNoOfEntries (void)
{
    if(MainGroup->Active)
        return MainGroup->Active->Songcount;
    else
        return 0;
}

long SONGDB_GetNoOfSearchResults(void)
{
    return n_search_results;
}

struct SongDBEntry *SONGDB_GetSearchEntry(long id)
{
    if (id >= n_search_results || id < 0 || id == SONGDB_ID_UNKNOWN)
    {
        return NULL;
    }
    return search_results[id];
}

struct SongDBEntry *SONGDB_GetEntryID(long id)
{
    struct SongDBEntry *e = NULL;
    struct SongDBEntry *Playlist = NULL;

    if(MainGroup && MainGroup->Active)
        Playlist = MainGroup->Active->Playlist;
    else
        return NULL;
    if(id < 0)
        return NULL;

    
    if(id < MainGroup->Active->Songcount)
    {
        while(id)
        {
            Playlist=Playlist->next;
            id--;
        }
        e = Playlist;
    }

    if (e != NULL)
        return e;
    return NULL;
}

int SONGDB_SetActiveSubgroup(struct SongDBSubgroup *sg)
{
    if(sg)
        MainGroup->Active=sg;
    else
        return 0;
    
    return 1;
}

struct SongDBSubgroup *SONGDB_GetActiveSubgroup()
{
    return MainGroup->Active;
}


static int SONGDB_JumpToFileMatch(char* song, char * keys[], int nw)
{
    int i;
    
    for (i = 0; i < nw; i++)
    {
        if (!strstr(song, keys[i]))
            return 0;
    }
    return 1;
}

int SONGDB_FindSubgroup(struct SongDBEntry *e)
{
    struct SongDBSubgroup *Subgroup = NULL;
    struct SongDBEntry *Playlist = NULL;
    int count=0;

    if(e==NULL)
    {
        printf("Cannot search\n");
        return 0;
    }
        

    Subgroup=MainGroup->Subgroup;

    while(Subgroup)
    {
        Playlist=Subgroup->Playlist;

        while(Playlist)
        {
            if(Playlist == e)
                return count;
            
            Playlist=Playlist->next;
        }
        count++;
        Subgroup=Subgroup->next;
    }
    return 0;
}


/* Jump to file function */
void SONGDB_FindEntry(char *search_string)
{
    char *ptr;
    int nw=0;
    int i=0;
    char *words[20];

    struct SongDBEntry *e = NULL;
    struct SongDBEntry *Playlist = NULL;

    struct SongDBSubgroup *Subgroup = NULL;
    
    /* lowercase the string */
    for(ptr=search_string;*ptr;ptr++)
        *ptr=tolower(*ptr);

    /* chop the searchstring in pieces */
    for(ptr=search_string; nw < 20; ptr=strchr(ptr,' '))
    {
        if(!ptr)
            break;
        else if(*ptr == ' ')
        {
            while(*ptr == ' ')
            {
                *ptr = '\0';
                ptr++;
            }
            words[nw++] = ptr;
        }
        else
        {
            words[nw++] = ptr;
        }
    }

    n_search_results = 0;
    free(search_results);
    search_results = NULL;

    Subgroup=MainGroup->Subgroup;

    while(Subgroup)
    {
        if(Subgroup->Volatile == 0)
        {
            Playlist=Subgroup->Playlist;
            
            while(Playlist)
            {
                int match = 0; 
                e = Playlist;
                
                if (nw == 0   ||        /* No words yet */
                    (nw == 1 && words[0][0] == '\0') || /* empty word */
                    (nw == 1 && strlen(words[0]) == 1 && /* only one character */
                     ((e->title &&
                       strchr(e->title,words[0][0])) || strchr(e->filename,words[0][0]))))
                {
                    match=1;
                }
                else
                    if (nw == 1 && strlen(words[0]) == 1)
                        match=0;
                    else
                    {
                        char song[256];
 
                        for(ptr = e->title,i=0   ; ptr && *ptr && i < 254; i++, ptr++)
                            song[i]=tolower(*ptr);
                        for(ptr= e->filename,i=0 ; ptr && *ptr && i < 254; i++, ptr++)
                            song[i]=tolower(*ptr);
                        song[i] = '\0';
                        match=SONGDB_JumpToFileMatch(song,words,nw);
                    }

                if(match)
                {
                    n_search_results++;
                    search_results = realloc (search_results, n_search_results * DBENTRY_PTR_LEN);
                    search_results[n_search_results - 1] = e;
                }
                Playlist=Playlist->next;
            }
        }
        Subgroup=Subgroup->next;
    }
}



static int SONGDB_LoadXMLDatabase()
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr cur = NULL;
    xmlChar *key;
    char filename[255];
    char *dir;

    subcount=0;

    LIBXML_TEST_VERSION;
    xmlKeepBlanksDefault(0);
   
    
    /*
     * build an XML tree from a the file;
     */
    dir=OSA_GetConfigDir();
    sprintf(filename,"%s/music.xml",dir);
    doc = xmlParseFile(filename);
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
    if (xmlStrcmp(cur->name, (const xmlChar *) "songdb")) 
    {
        fprintf(stderr,"document of the wrong type, root node != Helping");
        xmlFreeDoc(doc);
        return 0;
    }

    cur = cur->xmlChildrenNode;
    while (cur != NULL) 
    {
        if((!xmlStrcmp(cur->name, (const xmlChar *)"group"))) 
        {
            key = xmlGetProp(cur, "name");
            if(key)
                MainGroup->Name=strdup(key);
            cur = cur->xmlChildrenNode;
            while(cur != NULL)
            {
                if((!xmlStrcmp(cur->name, (const xmlChar *)"subgroup"))) 
                {
                    parsesubgroup(doc,cur);
                }
                if(cur)
                    cur=cur->next;
            }
        }
        if(cur)
            cur=cur->next;
    }
    return 1;


}

static int SONGDB_SaveXMLDatabase()
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL;
    xmlNodePtr maingroup = NULL, subgroup = NULL, file = NULL;
    struct SongDBEntry *Playlist;
    char *dir;
    char filename[255];
    char time[255];

    dir=OSA_GetConfigDir();
    sprintf(filename,"%s/music.xml",dir);

    LIBXML_TEST_VERSION;
    /* 
     * Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "songdb");
    xmlDocSetRootElement(doc, root_node);

    /* 
     * xmlNewChild() creates a new node, which is "attached" as child node
     * of root_node node. 
     */
    maingroup=xmlNewChild(root_node, NULL, BAD_CAST "group", NULL);
    xmlNewProp(maingroup, BAD_CAST "name", BAD_CAST MainGroup->Name);

    while(MainGroup->Subgroup)
    {
        if(MainGroup->Subgroup->Volatile == 0)
        {
            subgroup=xmlNewChild(maingroup, NULL, BAD_CAST "subgroup", NULL);
            xmlNewProp(subgroup, BAD_CAST "name", BAD_CAST MainGroup->Subgroup->Name);
            
            Playlist=MainGroup->Subgroup->Playlist;
            
            while(Playlist)
            {
                file=xmlNewChild(subgroup,NULL,"song",NULL);
                xmlNewProp(file, BAD_CAST "filename", BAD_CAST Playlist->filename);
                if(Playlist->time)
                {
                    sprintf(time,"%ld",Playlist->time);
                    xmlNewProp(file, BAD_CAST "time", BAD_CAST time);
                }
                Playlist=Playlist->next;
            }
        }
        MainGroup->Subgroup=MainGroup->Subgroup->next;
    }

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


    return 1;
}

