/*
  Beatforce/ 

  Copyright (C) 2003 John Beuving (john.beuving@home.nl)

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

#ifndef __SAMPLER_H__
#define __SAMPLER_H__


struct SamplerPrivate
{



  int lock1;
  int lock2;
  int rec;

  char *sample_buffer[20];

};

struct SamplerButton
{
  int state;


  char *buffer;
  int len;

  char *file;
  FILE *fd;

};


#define SAMPLER_STATE_LOCKED	1
#define SAMPLER_STATE_RECORDED	2

#define SAMPLER_BUTTON_LOCKED( _btn )		((_btn->state & SAMPLER_STATE_LOCKED) > 0)
#define SAMPLER_BUTTON_RECORDED( _btn )		((_btn->state & SAMPLER_STATE_RECORDED) > 0)


#define SAMPLER_PRIVATE_LEN    (sizeof( struct SamplerPrivate ))

#define SAMPLER_PRIVATE( win )	((struct SamplerPrivate*)gtk_object_get_data( GTK_OBJECT(win), "private" ))


#define SAMPLER_COLOR_CLEAR		0
#define SAMPLER_COLOR_RED		1
#define SAMPLER_COLOR_GREEN		2
#define SAMPLER_COLOR_YELLOW    3
#define SAMPLER_COLOR_BLUE      4

#define SAMPLER_N_COLORS		5


int sampler_init (GtkWidget *);
int sampler_finalize (GtkWidget *);
GtkWidget *sampler_get_window ();



int sampler_init_widgets (void);
int sampler_button_set_color (char, int);
GdkPixmap *sampler_get_color (int);


#endif
