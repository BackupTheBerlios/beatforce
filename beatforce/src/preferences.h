/*
   BeatForce
   preferences.h  - preferences dialog
   
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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

int preferences_init (GtkWidget *);
int preferences_show (ConfigFile *);


void preferences_ok_clicked (GnomeDialog *);
void preferences_close_clicked (GnomeDialog *);
void preferences_help_clicked (GnomeDialog *);

int preferences_general_init (GtkWidget *, ConfigFile *);
int preferences_audio_init (GtkWidget *, ConfigFile *);
int preferences_player_mixer_init (GtkWidget *, ConfigFile *);
int preferences_plugins_init (GtkWidget *, ConfigFile *);
void preferences_plugins_select_row (GtkCList *, gint, gint, gpointer);


int preferences_general_save (GtkWidget *, ConfigFile *);
int preferences_audio_save (GtkWidget *, ConfigFile *);
int preferences_player_mixer_save (GtkWidget *, ConfigFile *);
int preferences_plugins_save (GtkWidget *, ConfigFile *);


#endif
