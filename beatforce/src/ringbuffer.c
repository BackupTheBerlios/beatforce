/*
   BeatForce
   ringbuffer.c  - ring buffer's
   
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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <stdio.h>
#include <errno.h>

#include <vrb.h>

#include "ringbuffer.h"

#define MODULE_ID RINGBUFFER
#include "debug.h"


#define mutex_unlock( m )   (m=0)
#ifdef __USE_RB_MUTEX__
#define mutex_lock( m )     while( m );  m = 1
#else
#define mutex_lock( m )
#endif

int rb_mutex;

/* call before output thread is active !!!!! */
int
rb_init (struct OutRingBuffer **rb, int size)
{
  struct OutRingBuffer *ring;

  if(rb==NULL || size <= 1024)
  {
      return 0;
  }

  rb_mutex = 0;

  ring = malloc (sizeof (struct OutRingBuffer));
  if(ring == NULL)
  {
      ERROR("Not enough memory");
      return 0;
  }
  memset (ring, 0, sizeof (struct OutRingBuffer));

  ring->vrb_buf = vrb_new (size, NULL);
  if(ring->vrb_buf == NULL)
  {
      ERROR("Not enough memory");
      return 0;
  }
  ring->size = size;
  *rb = ring;



  return 1;
}


int
rb_write (struct OutRingBuffer *rb, unsigned char * buf, int len)
{
  int nwritten;

  mutex_lock (rb_mutex);
  nwritten =  vrb_put (rb->vrb_buf, (char *) buf, (size_t) len);
  mutex_unlock (rb_mutex);

  return nwritten;
}

int
rb_free (struct OutRingBuffer *rb)
{
  int free = 0;



  mutex_lock (rb_mutex);
  free = (int) vrb_space_len (rb->vrb_buf);
  mutex_unlock (rb_mutex);
  return free;
}


int
rb_read (struct OutRingBuffer *rb, unsigned char * buf, int max)
{
  int nread = 0;



  mutex_lock (rb_mutex);
  nread = (int) vrb_get (rb->vrb_buf, (char *) buf, (size_t) max);
  mutex_unlock (rb_mutex);

  return nread;

}

int
rb_data_size (struct OutRingBuffer *rb)
{

  return (vrb_capacity (rb->vrb_buf) - vrb_space_len (rb->vrb_buf));
}

int
rb_clear (struct OutRingBuffer *rb)
{
    char *temp;
    temp=malloc(rb->size);
    memset(temp,0,rb->size);
    rb_write(rb,temp,rb->size);
    free(temp);
    return 0;
}





