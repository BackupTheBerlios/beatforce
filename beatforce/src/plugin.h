/*
  Beatforce/ Startup of beatforce

  Copyright (C) 2003-2004 John Beuving (john.beuving@beatforce.org)

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
#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "config.h"
#include "llist.h"

#ifndef PLUGIN_DIR
#define PLUGIN_DIR                      /usr/share/beatforce/plugins/
#endif

#ifndef PLUGIN_ROOT_DIR
#define PLUGIN_ROOT_DIR 		PLUGIN_DIR
#endif

#ifndef INPUT_DIR
#define INPUT_DIR				"input"
#endif

#ifndef OUTPUT_DIR
#define OUTPUT_DIR				"output"
#endif

#define PLUGIN_TYPE_INPUT		1
#define PLUGIN_TYPE_OUTPUT		2
#define PLUGIN_TYPE_EFFECT              3

int PLUGIN_Init(int type);
BFList *PLUGIN_GetList (int type);
int PLUGIN_Cleanup();

#endif
