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

#include "config.h"
#include "osa.h"
#include "configfile.h"

int THEME_Init()
{
    BFList *dir;
    int NoOfThemes;
    ConfigFile *themecfg;
    char themepath[255];
    int fullscreen;


    dir=OSA_FindDirectories(THEME_DIR);
    if(dir == NULL)
        return 0;

    NoOfThemes=LLIST_NoOfEntries(dir);
    if(NoOfThemes == 1)
    {
        /* use it */
        sprintf(themepath,"%s%s\\theme.cfg",THEME_DIR,dir->data);
        themecfg=bf_cfg_open_file (themepath);
        
        bf_cfg_read_int(themecfg,"Screen","Fullscreen", &fullscreen);
        

    }
    
    return 1;
}