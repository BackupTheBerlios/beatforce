/*
  Beatforce/ Startup of beatforce

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
#include "types.h"
#include "audio_output.h"
#include "input.h"


InputInterface beatforce_if = 
{
    AUDIOOUTPUT_Open,
    output_write,
    AUDIOOUTPUT_Pause,
    AUDIOOUTPUT_BufferFree,
    output_get_time,
    AUDIOOUTPUT_Close,
    INPUT_EOF
};
