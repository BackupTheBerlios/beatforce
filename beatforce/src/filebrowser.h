/*
   BeatForce
   filebrowser.h  -  file browser

   This code was taken from XMMS (http://www.xmms.org) and modified
   for beatforce and is therefore
   Copyright (C) 1998-2000  Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies
   
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


#ifndef __FILEBROWSER_H__
#define __FILEBROWSER_H__

#ifdef __FILEBROWSER_PRIVATE__
static int int_compare_func (gconstpointer, gconstpointer);
static void filebrowser_changed (GtkWidget *, GtkFileSelection *);

gboolean beatforce_filebrowser_is_dir (GtkFileSelection *);

static void filebrowser_add_files (GtkFileSelection *);

static void filebrowser_ok (GtkWidget *, GtkWidget *);

static void filebrowser_add_selected_files (GtkWidget *, gpointer);


static void filebrowser_add_all_files (GtkWidget *, gpointer);
#endif


GtkWidget *beatforce_create_filebrowser (GtkCTreeNode *);

#endif
