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
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "types.h"
#include "audio_channel.h"
#include "audio_output.h"
#include "input.h"


InputInterface beatforce_if = 
{
    AUDIOCHANNEL_Open,
    AUDIOCHANNEL_Write,
    AUDIOCHANNEL_Pause,
    AUDIOCHANNEL_BufferFree,
    AUDIOCHANNEL_GetTimePlayed,
    AUDIOCHANNEL_Close,
    INPUTPLUGIN_EOF
};

#endif /* __INTERFACE_H__ */


