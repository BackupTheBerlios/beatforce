/*
  BeatForce
  songdb.c  -	song database
   
  Copyright (c) 2001, Patrick Prasse (patrick.prasse@gmx.net)

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
struct SongDBEntry *songdb_AllocEntry (void);
void songdb_FreeEntry (struct SongDBEntry *e);
int songdb_JumpToFileMatch(char* song, char * keys[], int nw);
struct SongDBSubgroup *songdb_AddSubgroup(struct SongDBSubgroup *sg,char *title);
static int count;

int parsesubgroup(xmlDocPtr doc, xmlNodePtr cur)
{
    xmlChar *key;

    key = xmlGetProp(cur, "name");
    if(key)
        MainGroup->Subgroup = songdb_AddSubgroup(MainGroup->Subgroup,key);
    cur = cur->xmlChildrenNode;
    while(cur != NULL)
    {
        if((!xmlStrcmp(cur->name, (const xmlChar *)"song")))                         
        {
            key = xmlGetProp(cur, "filename");
            if(key)
                SONGDB_AddFileTo(count,key);
        }
        cur=cur->next;
    }
    count++;

}

int parxml()
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr cur = NULL;
    xmlChar *key;

    count=0;

    LIBXML_TEST_VERSION;
    xmlKeepBlanksDefault(0);
   
    
    /*
     * build an XML tree from a the file;
     */
    doc = xmlParseFile("music.xml");
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

}

int SONGDB_Init (SongDBConfig * our_cfg)
{
    MainGroup=malloc(sizeof(SongDBGroup));
    memset(MainGroup,0,sizeof(SongDBGroup));

    search_results = NULL;
    n_search_results = 0;
    n_index = 0;


    
    parxml();

    MainGroup->Changed = 1;
    MainGroup->Active   = MainGroup->Subgroup;
    return 1;
}

int SONGDB_Exit()
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;/* node pointers */
    xmlNodePtr maingroup = NULL, subgroup = NULL, file = NULL;
    char buff[256];
    int i, j;

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
        subgroup=xmlNewChild(maingroup, NULL, BAD_CAST "subgroup", NULL);
        xmlNewProp(subgroup, BAD_CAST "name", BAD_CAST MainGroup->Subgroup->Name);
        for(i=0;i<MainGroup->Subgroup->Songcount;i++)
        {
            file=xmlNewChild(subgroup,NULL,"song",NULL);
            xmlNewProp(file, BAD_CAST "filename", BAD_CAST MainGroup->Subgroup->Playlist[i]->filename);
        }
        MainGroup->Subgroup=MainGroup->Subgroup->next;
    }

    /* 
     * Dumping document to stdio or file
     */
    xmlSaveFormatFileEnc("music.xml", doc, "UTF-8", 1);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return(0);

}


int SONGDB_AddSubgroup(char *title)
{
    if(title == NULL)
        return 0;
    MainGroup->Subgroup=songdb_AddSubgroup(MainGroup->Subgroup,title);
    MainGroup->Changed=1;
    return 1;
}

int SONGDB_RenameSubgroup(int which, char *title)
{
    SongDBSubgroup *sg=SONGDB_GetSubgroup(which);

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

int SONGDB_RemoveSubgroup(int which)
{
    int count=0;
    SongDBSubgroup *list;

    list=MainGroup->Subgroup;

    while(list)
    {
        if(count==which)
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
            return 1;
            MainGroup->Changed=1;
        }
        
        list=list->next;
        count++;
    }
    return 0;
}

struct SongDBSubgroup *SONGDB_GetSubgroup(int which)
{
    int count=0;
    struct SongDBSubgroup *list;
    if(count < 0)
        return NULL;

    list=MainGroup->Subgroup;

    while(list)
    {
        if(count==which)
            return list;
        
        list=list->next;
        count++;
    }
    return NULL;
}

int SONGDB_SubgroupCount()
{
    struct SongDBSubgroup *list;
    int count=0;

    list=MainGroup->Subgroup;

    while(list)
    {
        list=list->next;
        count++;
    }
    return count;
}

int SONGDB_AddFileTo(int which,char *filename)
{
    struct SongDBSubgroup *sg;
    
    sg=SONGDB_GetSubgroup(which);
    if(sg)
    {
        struct SongDBEntry *e;
        
        e = songdb_AllocEntry ();
        
        e->filename = strdup (filename);
        e->id       = sg->Songcount++;
        
        if (input_whose_file (PLAYER_GetData(0)->ip_plugins, filename) != NULL)
        {
            /* Add the created entry to the active database */
            sg->Playlist = realloc (sg->Playlist, sg->Songcount * DBENTRY_PTR_LEN);
            if(sg->Playlist==NULL)
            {
                ERROR("Realloc failed");
            }
            sg->Playlist[sg->Songcount-1] = e;
            
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
        last->next->prev=last;

    }
    return sg;
}

struct SongDBEntry *songdb_AllocEntry (void)
{
    struct SongDBEntry *e;

    e = malloc (DBENTRY_LEN);
    if (e == NULL)
    {
        perror ("songdb_alloc_entry");
    }
    memset (e, 0, DBENTRY_LEN);

    return e;
}

void SONGDB_FreeActiveList()
{
    int i=0;
    if(MainGroup->Active == NULL)
        return;
    for(i=0; i < MainGroup->Active->Songcount; i++)
    {
        songdb_FreeEntry(MainGroup->Active->Playlist[i]);
        MainGroup->Active->Playlist[i]=NULL;
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
    if (id >= n_search_results || id == SONGDB_ID_UNKNOWN)
    {
        return NULL;
    }
    return search_results[id];
}

struct SongDBEntry *SONGDB_GetEntryID(long id)
{
    struct SongDBEntry *e = NULL;

    if(id < 0)
        return NULL;

    if(id < MainGroup->Active->Songcount)
        e = MainGroup->Active->Playlist[id];

    if (e != NULL)
        return e;
    return NULL;
}

int SONGDB_SetActiveSubgroup(int which)
{
    struct SongDBSubgroup *sg;
    sg=SONGDB_GetSubgroup(which);
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


int songdb_JumpToFileMatch(char* song, char * keys[], int nw)
{
    int i;
    
    for (i = 0; i < nw; i++)
    {
        if (!strstr(song, keys[i]))
            return 0;
    }
    return 1;
}

void SONGDB_FindEntry(char *search_string)
{
    char *ptr;
    int nw=0;
    int entry;
    int i=0;
    char *words[20];

    struct SongDBEntry *e = NULL;

    
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

    for(entry=0; entry < MainGroup->Active->Songcount; entry++)
    {
        int match = 0; 
        e = MainGroup->Active->Playlist[entry];
        
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
            match=songdb_JumpToFileMatch(song,words,nw);
        }

        if(match)
        {
            n_search_results++;
            search_results = realloc (search_results, n_search_results * DBENTRY_PTR_LEN);
            search_results[n_search_results - 1] = e;
        }
    }
}


