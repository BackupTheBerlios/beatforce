/*
   BeatForce
   configfile.c  - config & config file stuff

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "osa.h"
#include "configfile.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static ConfigSection *bf_cfg_create_section (ConfigFile * cfg, char * name);
static ConfigLine *bf_cfg_create_string (ConfigSection * section, char * key,
					 char * value);
static ConfigSection *bf_cfg_find_section (ConfigFile * cfg, char * name);
static ConfigLine *bf_cfg_find_string (ConfigSection * section, char * key);


AudioConfig *
bf_cfg_read_AudioConfig (ConfigFile * cfgfile)
{
    AudioConfig *cfg;
    char sect[] = "Audio";
    int i;

    cfg = malloc   (sizeof (AudioConfig));
    memset (cfg, 0, sizeof (AudioConfig));

/* Input */
    if (!CONFIGFILE_ReadInt(cfgfile, sect, "RingBufferSize", &cfg->RingBufferSize))
        cfg->RingBufferSize = 5;
    if (!CONFIGFILE_ReadInt (cfgfile, sect, "FragmentSize", &cfg->FragmentSize))
        cfg->FragmentSize = 832;


/* Output */
    if (!CONFIGFILE_ReadInt (cfgfile, sect, "LowWatermark", &cfg->LowWatermark))
        cfg->LowWatermark = 3;

    if (!CONFIGFILE_ReadInt (cfgfile, sect, "HighWatermark", &cfg->HighWatermark))
        cfg->HighWatermark = 10;

/* FFTW */
    if (!CONFIGFILE_ReadInt (cfgfile, sect, "FFTW_Policy", &cfg->FFTW_Policy))
        cfg->FFTW_Policy = 0;	/* estimate */

    if (!bf_cfg_read_boolean
        (cfgfile, sect, "FFTW_UseWisdom", &cfg->FFTW_UseWisdom))
        cfg->FFTW_UseWisdom = FALSE;


    for (i = 0; i <= 2; i++)
    {
        char groupname[12];
        char name[32];

        switch (i)
        {
        case 0:
            sprintf (groupname, "MASTER");
            break;
        case 1:
            sprintf (groupname, "GroupA");
            break;
        case 2:
            sprintf (groupname, "GroupB");
            break;
        default:
            break;
        }

        if(i==0)
        {
            sprintf (name, "Output_%s", groupname);
            if (!bf_cfg_read_string (cfgfile, sect, name, &cfg->output_id[i]))
                cfg->output_id[i] = "oss";
        
            sprintf (name, "Device_%s", groupname);
            if (!bf_cfg_read_string (cfgfile, sect, name, &cfg->device_id[i]))
                cfg->device_id[i] = "/dev/dsp";
        }
        else
        {
            sprintf (name, "Output_%s", groupname);
            if (!bf_cfg_read_string (cfgfile, sect, name, &cfg->output_id[i]))
                cfg->output_id[i] = "none";
            
            sprintf (name, "Device_%s", groupname);
            if (!bf_cfg_read_string (cfgfile, sect, name, &cfg->device_id[i]))
                cfg->device_id[i] = "none";

        }
    }

    return cfg;
}

SongDBConfig *
bf_cfg_read_SongDBConfig (ConfigFile * cfgfile)
{
    SongDBConfig *cfg;
    char sect[] = "SongDB";
    int i;
    char string[255];

    cfg = malloc (sizeof (SongDBConfig));
    memset (cfg, 0, sizeof (SongDBConfig));
    
   
    if(!CONFIGFILE_ReadInt(cfgfile, sect, "Tabs", &cfg->Tabs))
    {
        cfg->Tabs = 1;
    }

    cfg->TabTitle  = malloc(cfg->Tabs * sizeof(char*));
    cfg->TabString = malloc(cfg->Tabs * sizeof(char*));

    for(i=0;i<cfg->Tabs;i++)
    {
        sprintf(string,"TabTitle%d",i);
        if(!bf_cfg_read_string(cfgfile, sect,string, &cfg->TabTitle[i]))
        {
            cfg->TabTitle[0]=(char*)malloc(255*sizeof(char));
            sprintf(cfg->TabTitle[i],"Beatforce");
        }
        sprintf(string,"TabString%d",i);
        if(!bf_cfg_read_string(cfgfile, sect,string, &cfg->TabString[i]))
        {
            cfg->TabString[i]=(char*)malloc(255*sizeof(char));
            memset(cfg->TabString[0],0,255);
        }
    }

    if (!bf_cfg_read_string(cfgfile, sect, "Database_File", &cfg->database_file))
    {
        cfg->database_file = (char*)malloc(255);
        sprintf(cfg->database_file,"%s%s",OSA_GetConfigDir(),"database");    
    }

    if (!bf_cfg_read_boolean (cfgfile, sect, "DB_Autoload", &cfg->db_autoload))
        cfg->db_autoload = TRUE;

    if (!bf_cfg_read_boolean (cfgfile, sect, "DB_Autosave", &cfg->db_autosave))
        cfg->db_autosave = TRUE;

    if (!bf_cfg_read_boolean (cfgfile, sect, "DB_Compress", &cfg->db_compress))
        cfg->db_compress = FALSE;

    bf_cfg_read_PositionConfig (cfgfile, sect, &cfg->pos);

    return cfg;
}


PositionConfig *
bf_cfg_read_PositionConfig (ConfigFile * cfgfile, char *sect,
			    PositionConfig * pos)
{

    if (!CONFIGFILE_ReadInt (cfgfile, sect, "PositionX", &pos->x))
        pos->x = -1;

    if (!CONFIGFILE_ReadInt (cfgfile, sect, "PositionY", &pos->y))
        pos->y = -1;

    if (!CONFIGFILE_ReadInt (cfgfile, sect, "Width", &pos->width))
        pos->width = -1;

    if (!CONFIGFILE_ReadInt (cfgfile, sect, "Height", &pos->height))
        pos->height = -1;

    if (!CONFIGFILE_ReadInt (cfgfile, sect, "Show", &pos->show))
        pos->show = 0;

    return pos;
}

PlayerConfig *
bf_cfg_read_PlayerConfig (ConfigFile * cfgfile, int nr)
{
    PlayerConfig *cfg;
    char sect[16];

    cfg = malloc (sizeof (PlayerConfig));
    memset (cfg, 0, sizeof (PlayerConfig));

    sprintf (sect, "Player%d", nr);

    if (!bf_cfg_read_boolean
        (cfgfile, sect, "RemoveAfterPlay", &cfg->RemoveAfterPlay))
        cfg->RemoveAfterPlay = FALSE;

    bf_cfg_read_PositionConfig (cfgfile, sect, &cfg->pos);

    return cfg;
}

SamplerConfig *
bf_cfg_read_SamplerConfig (ConfigFile * cfgfile)
{
    SamplerConfig *cfg;
    char sect[] = "Sampler";

    cfg = malloc (sizeof (SamplerConfig));
    memset (cfg, 0, sizeof (SamplerConfig));

    bf_cfg_read_PositionConfig (cfgfile, sect, &cfg->pos);

    return cfg;
}


MixerConfig *
bf_cfg_read_MixerConfig (ConfigFile * cfgfile)
{
    MixerConfig *cfg;
    char sect[] = "Mixer";

    cfg = malloc (sizeof (MixerConfig));
    memset (cfg, 0, sizeof (MixerConfig));

    if (!bf_cfg_read_boolean
        (cfgfile, sect, "TForwAfterFadeNow", &cfg->TForwAfterFadeNow))
        cfg->TForwAfterFadeNow = TRUE;

    bf_cfg_read_PositionConfig (cfgfile, sect, &cfg->pos);

    return cfg;
}


int
bf_cfg_write_AudioConfig (ConfigFile * cfgfile, AudioConfig * cfg)
{
    char sect[] = "Audio";
    int i;

    bf_cfg_write_int (cfgfile, sect, "RingBufferSize", cfg->RingBufferSize);
    bf_cfg_write_int (cfgfile, sect, "FragmentSize"  , cfg->FragmentSize);

    bf_cfg_write_int (cfgfile, sect, "LowWatermark"  , cfg->LowWatermark);
    bf_cfg_write_int (cfgfile, sect, "HighWatermark" , cfg->HighWatermark);

    bf_cfg_write_int (cfgfile, sect, "FFTW_Policy"   , cfg->FFTW_Policy);
    bf_cfg_write_boolean (cfgfile, sect, "FFTW_UseWisdom", cfg->FFTW_UseWisdom);

    for (i = 0; i < 3; i++)
    {
        char groupname[12];
        char name[32];

        switch (i)
        {
        case 0:
            sprintf (groupname, "MASTER");
            break;
        case 1:
            sprintf (groupname, "GroupA");
            break;
        case 2:
            sprintf (groupname, "GroupB");
            break;
        default:
            break;
        }

        sprintf (name, "Output_%s", groupname);
        bf_cfg_write_string (cfgfile, sect, name, cfg->output_id[i]);

        sprintf (name, "Device_%s", groupname);
        bf_cfg_write_string (cfgfile, sect, name, cfg->device_id[i]);

    }

    return 0;
}

int
bf_cfg_write_SongDBConfig (ConfigFile * cfgfile, SongDBConfig * cfg)
{
    char sect[] = "SongDB";
    int i;
    char string[255];

    bf_cfg_write_int     (cfgfile, sect, "Tabs",cfg->Tabs);
    for(i=0;i<cfg->Tabs;i++)
    {
        sprintf(string,"TabTitle%d",i);
        bf_cfg_write_string  (cfgfile, sect, string , cfg->TabTitle[i]);
        sprintf(string,"TabString%d",i);
        bf_cfg_write_string  (cfgfile, sect, string, cfg->TabString[i]);
    }
    bf_cfg_write_string  (cfgfile, sect, "Database_File", cfg->database_file);
    bf_cfg_write_boolean (cfgfile, sect, "DB_Autoload"  , cfg->db_autoload);
    bf_cfg_write_boolean (cfgfile, sect, "DB_Autosave"  , cfg->db_autosave);
    bf_cfg_write_boolean (cfgfile, sect, "DB_Compress"  , cfg->db_compress);
    return 0;
}


int
bf_cfg_write_PlayerConfig (ConfigFile * cfgfile, PlayerConfig * cfg, int nr)
{
    char sect[16];

    sprintf (sect, "Player%d", nr);

    bf_cfg_write_boolean (cfgfile, sect, "RemoveAfterPlay",
                          cfg->RemoveAfterPlay);
    return 0;
}



int
bf_cfg_write_MixerConfig (ConfigFile * cfgfile, MixerConfig * cfg)
{
    char sect[] = "Mixer";

    bf_cfg_write_boolean (cfgfile, sect, "TForwAfterFadeNow",
                          cfg->TForwAfterFadeNow);

    return 0;
}






ConfigFile *
bf_cfg_new (void)
{
    ConfigFile *cfg;

    cfg = malloc (sizeof (ConfigFile));
    memset(cfg, 0 ,sizeof(ConfigFile));
    return cfg;
}

ConfigFile *
bf_cfg_open_file (char * filename)
{
    ConfigFile *cfg;

    FILE *file;
    char *buffer, *tmp;
    char *line,*tmpbuffer;
    char *endline;
    long filesize;
    
    ConfigSection *section = NULL;

    if (!(file = fopen (filename, "rb")))
        return NULL;
    
    /* Determine the filesize */
    if (fseek(file,0,SEEK_END))
        return NULL;
    filesize=ftell(file);
    if (fseek(file,0,SEEK_SET))
        return NULL;
    
    buffer = malloc (filesize + 1);
    if (fread (buffer, 1, filesize, file) != filesize)
    {
        free (buffer);
        fclose (file);
        return NULL;
    }
    fclose (file);
    buffer[filesize] = '\0';
    cfg = malloc(sizeof (ConfigFile));
    memset(cfg,0,sizeof (ConfigFile));

    tmpbuffer=buffer;

    while((endline = strchr(buffer,'\n')))
    {
        line    = buffer;
        buffer  = endline + 1;
        *endline = '\0';
        if(line[0] == '[')
        {
            if ((tmp = strchr (line, ']')))
            {
                *tmp = '\0';
                section = bf_cfg_create_section (cfg, line+1);
            }
        }
        else if (line[0] != '#' && section)
        {
            if ((tmp = strchr (line, '=')))
            {
                *tmp = '\0';
                tmp++;
                bf_cfg_create_string (section, line, tmp);
            }
        }
    }
    
    buffer=tmpbuffer;
    free (buffer);
    return cfg;
}

char *
bf_cfg_get_default_filename (void)
{
    static char *filename = NULL;
    if (!filename)
    {
        filename = malloc(255);
        sprintf(filename,"%s%s",OSA_GetConfigDir(),"config");
    }
    return filename;
}


int
bf_cfg_create_dir (void)
{
    char *dirname;
    dirname = strdup(OSA_GetConfigDir());
    {
        if (mkdir (dirname, 0755) != 0)
        {
            fprintf(stderr,"Couldn´t create directory %s.", dirname);
        }
        printf ("Created Directory %s .", dirname);
    }
    free (dirname);
    return TRUE;
}


ConfigFile *
bf_cfg_create_default_file (void)
{
    ConfigFile *cfgfile;

    cfgfile = bf_cfg_new ();
    bf_cfg_create_dir ();


    bf_cfg_write_AudioConfig   (cfgfile, bf_cfg_read_AudioConfig   (cfgfile));
    bf_cfg_write_SongDBConfig  (cfgfile, bf_cfg_read_SongDBConfig  (cfgfile));
    bf_cfg_write_PlayerConfig  (cfgfile, bf_cfg_read_PlayerConfig  (cfgfile, 0), 0);
    bf_cfg_write_PlayerConfig  (cfgfile, bf_cfg_read_PlayerConfig  (cfgfile, 1), 1);
    bf_cfg_write_MixerConfig   (cfgfile, bf_cfg_read_MixerConfig   (cfgfile));

    if (!bf_cfg_write_default_file (cfgfile))
    {
        bf_cfg_free (cfgfile);
        fprintf(stderr,"Couldn't write default config %s", bf_cfg_get_default_filename());
        return NULL;
    }

    return cfgfile;
}

ConfigFile *
bf_cfg_open_default_file (void)
{
    ConfigFile *ret;
    
    ret = bf_cfg_open_file (bf_cfg_get_default_filename ());

    if (!ret)
    {
//        char str[255];

        /*sprintf("This seems to be your first start of BeatForce,\n"
          "At least I could not find a config file in %s\n"
                "Please edit your Preferences and choose your output.\n"
                "Please restart BeatForce after this is done.\n"
                " AND NOW: Have a lot of fun with BeatForce!\n",
                bf_cfg_get_default_filename ());
        bf_error_dialog (str);*/

        ret = bf_cfg_create_default_file ();
    }

    return ret;
}

int
bf_cfg_write_file (ConfigFile * cfg, char * filename)
{
    FILE *file;
    BFList *section_list, *line_list;
    ConfigSection *section;
    ConfigLine *line;

    if (!(file = fopen (filename, "w")))
        return FALSE;

    section_list = cfg->sections;
    while (section_list)
    {
        section = (ConfigSection *) section_list->data;
        if (section->lines)
        {
            fprintf (file, "[%s]\n", section->name);
            line_list = section->lines;
            while (line_list)
            {
                line = (ConfigLine *) line_list->data;
                fprintf(file, "%s=%s\n", line->key, line->value);
                line_list = line_list->next;
            }
            fprintf (file, "\n");
        }
        section_list = section_list->next;
    }
    fclose (file);
    return TRUE;
}

int
bf_cfg_write_default_file (ConfigFile * cfg)
{
    return bf_cfg_write_file (cfg, bf_cfg_get_default_filename ());
}

int
bf_cfg_read_string (ConfigFile * cfg, char * section, char * key,
		    char ** value)
{
    ConfigSection *sect;
    ConfigLine    *line;

    if (!(sect = bf_cfg_find_section (cfg, section)))
        return FALSE;
    if (!(line = bf_cfg_find_string (sect, key)))
        return FALSE;
    *value = strdup (line->value);
    return TRUE;
}

int CONFIGFILE_ReadInt(ConfigFile * cfg, char * section, char * key, int *value)
{
    char *str;

    if (!bf_cfg_read_string (cfg, section, key, &str))
        return FALSE;
    *value = atoi (str);
    free (str);

    return TRUE;
}

int
bf_cfg_read_boolean (ConfigFile * cfg, char * section, char * key,
		     int * value)
{
    char *str;

    if (!bf_cfg_read_string (cfg, section, key, &str))
        return FALSE;
    if (!strcmp (str, "TRUE"))
        *value = TRUE;
    else
        *value = FALSE;
    free (str);
    return TRUE;
}

int
bf_cfg_read_float (ConfigFile * cfg, char * section, char * key,
		   float * value)
{
    char *str;

    if (!bf_cfg_read_string (cfg, section, key, &str))
        return FALSE;

    *value = (float)strtod(str, NULL);
    free (str);

    return TRUE;
}

int
bf_cfg_read_double (ConfigFile * cfg, char * section, char * key,
		    double * value)
{
    char *str;

    if (!bf_cfg_read_string (cfg, section, key, &str))
        return FALSE;

    *value = strtod(str, NULL);
    free (str);

    return TRUE;
}

void
bf_cfg_write_string (ConfigFile * cfg, char * section, char * key,
		     char * value)
{
    ConfigSection *sect;
    ConfigLine *line;

    sect = bf_cfg_find_section (cfg, section);
    if (!sect)
    {
        sect = bf_cfg_create_section (cfg, section);
    }
    if ((line = bf_cfg_find_string (sect, key)))
    {
        free (line->value);
        line->value = strdup(value);
    }
    else
    {
        bf_cfg_create_string (sect, key, value);
    }
}

void
bf_cfg_write_int (ConfigFile * cfg, char * section, char * key, int value)
{
    char strvalue[50];

    sprintf(strvalue,"%d", value);
    bf_cfg_write_string (cfg, section, key, strvalue);
}

void
bf_cfg_write_boolean (ConfigFile * cfg, char * section, char * key,
		      int value)
{
    if (value)
        bf_cfg_write_string (cfg, section, key, "TRUE");
    else
        bf_cfg_write_string (cfg, section, key, "FALSE");
}

void
bf_cfg_write_float (ConfigFile * cfg, char * section, char * key,
		    float value)
{
    char strvalue[50];

    sprintf(strvalue,"%g",value);
    bf_cfg_write_string (cfg, section, key, strvalue);
}

void
bf_cfg_write_double (ConfigFile * cfg, char * section, char * key,
		     double value)
{
    char strvalue[50];

    sprintf(strvalue,"%g",value);
    bf_cfg_write_string (cfg, section, key, strvalue);
}

void
bf_cfg_remove_key (ConfigFile * cfg, char * section, char * key)
{
    ConfigSection *sect;
    ConfigLine *line;

    if ((sect = bf_cfg_find_section (cfg, section)) != NULL)
    {
        if ((line = bf_cfg_find_string (sect, key)) != NULL)
        {
            free (line->key);
            free (line->value);
            free (line);
            sect->lines = LLIST_Remove (sect->lines, line);
        }
    }
}

void
bf_cfg_free (ConfigFile * cfg)
{
    ConfigSection *section;
    ConfigLine *line;
    BFList *section_list, *line_list;

    section_list = cfg->sections;
    while (section_list)
    {
        section = (ConfigSection *) section_list->data;
        free (section->name);

        line_list = section->lines;
        while (line_list)
        {
            line = (ConfigLine *) line_list->data;
            free (line->key);
            free (line->value);
            free (line);
            line_list = line_list->next;
        }
        //g_list_free (section->lines);
        free (section->lines);
        free (section);

        section_list = section_list->next;
    }
    //g_list_free (cfg->sections);
    free (cfg->sections);
    free (cfg);
    cfg = NULL;
}

static ConfigSection *
bf_cfg_create_section (ConfigFile * cfg, char * name)
{
    ConfigSection *section;

    section = malloc (sizeof (ConfigSection));
    memset(section,0,(sizeof (ConfigSection)));
    section->name = strdup (name);
    cfg->sections = LLIST_Append (cfg->sections, section);

    return section;
}

static ConfigLine *
bf_cfg_create_string (ConfigSection * section, char * key, char * value)
{
    ConfigLine *line;

    line = malloc (sizeof (ConfigLine));
    memset(line,0,(sizeof (ConfigLine)));
    line->key   = strdup(key);
    line->value = strdup(value);
    section->lines = LLIST_Append (section->lines, line);
    return line;
}

static ConfigSection *
bf_cfg_find_section (ConfigFile * cfg, char * name)
{
    ConfigSection *section;
    BFList *list;
    
    list = cfg->sections;
    while (list)
    {
        section = (ConfigSection *) list->data;
        if (!strcmp (section->name, name))
            return section;
        list=list->next;
    }
    return NULL;
}

static ConfigLine *
bf_cfg_find_string (ConfigSection * section, char * key)
{
    ConfigLine *line;
    BFList *list;

    list = section->lines;
    while (list)
    {
        line = (ConfigLine *) list->data;
        if (!strcmp (line->key, key))
            return line;
        list = list->next;
    }
    return NULL;
}
