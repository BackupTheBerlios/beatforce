/*
  Beatforce/ Startup of beatforce

  Copyright (C) 2003-2004 John Beuving (john.beuving@wanadoo.nl)

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
#ifndef __MIXER_UI_H__
#define __MIXER_UI_H__

#include "theme.h" 

typedef struct MixerWidgets
{
    void *Fader;
    void *MainVolumeIndicator;
    void *MainVolume;
}MixerWidgets;

void *MIXERUI_CreateWindow(ThemeMixer *tm);
int   MIXERUI_Redraw(void *w);

/* for main window */
int MIXERUI_DecreaseMainVolume();
int MIXERUI_IncreaseMainVolume();
#endif __MIXER_UI_H__