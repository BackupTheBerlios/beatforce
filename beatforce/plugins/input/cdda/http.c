/*
   BeatForce plugin
   http.c - Basic http support needed for cddb
   
   Copyright (c) 2003, John Beuving (john.beuving@home.nl)

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

#include "http.h"

int HTTP_OpenConnection(char * server, int port)
{
    int sock;
    struct hostent *host;
    struct sockaddr_in address;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;

    if (!(host = gethostbyname(server)))
        return 0;

    memcpy(&address.sin_addr.s_addr, *(host->h_addr_list), sizeof(address.sin_addr.s_addr));
    address.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *) &address, sizeof (struct sockaddr_in)) == -1)
        return 0;

    return sock;
}

void HTTP_CloseConnection(int sock)
{
    shutdown(sock, 2);
    close(sock);
}

int HTTP_ReadLine(int sock, char * buf, int size)
{
    int i = 0;

    while (i < size - 1)
    {
        if (recv(sock, buf + i, 1,0) <= 0)
        {
            if (i == 0)
                return -1;
            else
                break;
        }
        
        if (buf[i] == '\n')
            break;
        if (buf[i] != '\r')
            i++;
    }
    buf[i] = '\0';
    return i;
}

int HTTP_ReadFirstLine(int sock, char * buf, int size)
{
    /* Skips the HTTP-header, if there is one, and reads the first line into buf.
       Returns number of bytes read. */
	
    int i;
    /* Skip the HTTP-header */
    if ((i = HTTP_ReadLine(sock, buf, size)) < 0)
        return 0;

    /* Check to make sure its not HTTP/0.9 */
    if (!strncmp(buf, "HTTP", 4)) 
    {
        while (HTTP_ReadLine(sock, buf, size) > 0)
            /* nothing */;
        if ((i = HTTP_ReadLine(sock, buf, size)) < 0)
            return 0;
    }
	
    return i;
}

