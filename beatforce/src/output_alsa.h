/*
   BeatForce
   output_alsa.h  - ALSA audio output
   
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


#ifndef __OUTPUT_ALSA_H__
#define __OUTPUT_ALSA_H__


int output_alsa_init (AudioConfig *);
GList *output_alsa_get_devices (void);

int output_alsa_open (OutputDevice *, AudioConfig *);
int output_alsa_close (OutputDevice *);

int output_alsa_write (OutputDevice *, void *, int);

/*
int output_alsa_init_group( AudioConfig *, int );

*/

#endif
