/*
   BeatForce
   err.h  - error numbers
   
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


#ifndef __ERROR_NR_H__
#define __ERROR_NR_H__


/*
 * 
 * ERROR
 *
 */
#define ERROR_START_NUMBER			   (-0xf000)

#define ERROR_UNKNOWN				   (-0xf000)
#define ERROR_NO_MEMORY 					   (-0xf001)
#define ERROR_OUT_OF_MEMORY 		   ERROR_NO_MEMORY
#define ERROR_OUT_OF_BOUNDS 		   (-0xf002)
#define ERROR_INVALID_ARG	 (-0xf003)

/* audio output */
#define ERROR_ALREADY_OPEN			   (-0xf100)

#define ERROR_UNKNOWN_CHANNEL	(-0xf102)
#define ERROR_FRAG_TO_LARGE 		   (-0xf103)
#define ERROR_FRAG_TO_SMALL 		   (-0xf104)
#define ERROR_WRONG_CH_NUMBER  (-0xf105)
#define ERROR_INVALID_FORMAT		   (-0xf106)

#define ERROR_NO_OUTPUT_SELECTED	   (-0xf107)

/* plugins */
#define ERROR_NOT_SUPPORTED 		   (-0xff00)
#define ERROR_EOF							   (-0xff01)
#define ERROR_NOT_OPEN				   (-0xff02)
#define ERROR_NOT_PLAYING			   (-0xff03)
#define ERROR_UNKNOWN_FILE			   (-0xff04)
#define ERROR_NO_FILE_LOADED   (-0xff05)
#define ERROR_OUTPUT_ERROR			   (-0xff06)
#define ERROR_OPEN_ERROR			   (-0xff07)
#define ERROR_AUDIO_ERROR			   (-0xff08)
#define ERROR_FILE_FORMAT_ERROR (-0xff09)
#define ERROR_READ_SEEK_ERROR		   (-0xff0a)

#endif
