/*
 *  Copyright 1999 Håvard Kvålen <havardk@sol.no>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __HTTP_H__
#define __HTTP_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>


int HTTP_OpenConnection(char * server, int port);
void HTTP_CloseConnection(int sock);
int HTTP_ReadLine(int sock, char * buf, int size);
int HTTP_read_first_line(int sock, char * buf, int size);
char * HTTP_Get(char * url);

#endif /* __HTTP_H__ */



