/*
  Beatforce/Debug

  one line to give the program's name and an idea of what it does.
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

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

char module_id[40];
char msg[40];

int IGR_Write_to_log(char* id,char* text)
{
    int size=0;
    int i;
    unsigned char Log_string[255];
   
    size=strlen(text);
    size=80-size;
    sprintf(Log_string,"%s",text);

    for(i=0;i<size;i++)
        sprintf(Log_string,"%s ",Log_string);

    sprintf(Log_string,"%s%s\n",Log_string,id);

    printf("%s",Log_string);
    return 0;
}

void noprint(char *fmt,...)
{

}

void traceprintf(char *fmt,...)
{
    va_list ap;
    int d,i=0,j=0;
    char *s;
    char tempbuf[200];
    char tmpbuf[200];
    char getal[10];

    if(strlen(fmt)>200)
        return;
    va_start(ap, fmt);
    while (*fmt)
    {
        switch (*fmt) 
        {
        case '%':
            *fmt++;
            switch(*fmt++)
            {
            case 's':                       /* string */
                s=va_arg(ap, char *);
                if(s != NULL)
                {
                    tempbuf[i++]='<';     /* will be set between < and > */
                    while(*s)
                        tempbuf[i++]=*s++;
                    tempbuf[i++]='>';
                }
                break;
            case 'd':                       /* int */
                d = va_arg(ap, int);
                sprintf(getal,"%d",d);
                while(getal[j])
                    tempbuf[i++]=getal[j++];
                j=0;
                break;
            }
            break;
        default:
            tempbuf[i++]=*fmt++;
            break;
            
        }
    }
    va_start(ap, fmt);
    tempbuf[i]=0;
    sprintf(tmpbuf,"%s - %s",msg,tempbuf);
    IGR_Write_to_log(module_id,tmpbuf);

}

void printid(char *id,int line,char *message)
{
    sprintf(msg,"%s",message);
    sprintf(module_id,"%s (line %04d)",id,line);
}



