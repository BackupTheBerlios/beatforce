/*
   BeatForce
   types.h	- type definitions
   
   Copyright (c) 2002, Patrick Prasse (patrick.prasse@gmx.net)

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


#ifndef __BF_TYPES_H__
#define __BF_TYPES_H__


typedef int boolean;

typedef void Private;

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef true
#define true  TRUE
#endif

#ifndef false
#define false  FALSE
#endif


/* Formats */
typedef enum
{
  FMT_U8,
  FMT_S8,

/* I do do do do hate unsigned sound . */
  FMT_U16_LE,
  FMT_U16_BE,
  FMT_U16_NE,

  FMT_S16_LE,
  FMT_S16_BE,
  FMT_S16_NE,

  FMT_S24_LE,                   /* unsupp */
  FMT_S24_BE,                   /* unsupp */
  FMT_S24_NE,                   /* supported */

  FMT_S32_LE,                   /* unsupp */
  FMT_S32_BE,                   /* unsupp */
  FMT_S32_NE,                   /* supported */

/* further enhancements */
  FMT_FLOAT32,                  /* float32 -1.0 to +1.0 */
  FMT_FIXED32,                  /* 32bit fixed-point integer as libMAD -1.0 to +1.0 */

  FMT_FLOAT64,

  FMT_UNKNOWN
}
AFormat;


typedef struct InputInterface
{
    int (*output_open)(int c, AFormat fmt, int rate, int nch, int *max_bytes);
    int (*output_write)(int c, void* buf, int len);
    int (*output_pause) (int c, int pause);
    long (*output_buffer_free) (int c);
    long (*output_get_time)    (int c);
    int (*output_close) (int c);
    int (*input_eof) (int ch_id);
}InputInterface;

#endif
