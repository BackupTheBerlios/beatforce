/*
  Beatforce/ Linked list implementation

  Copyright (C) 2003-2004 John Beuving (john.beuving@wanadoo.nl)

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

#include <stdio.h>
#include <stdlib.h>
#include "llist.h"


BFList*
LLIST_Combine(BFList *list1,BFList *list2)
{
    BFList *temp;

    if(list1)
    {
        temp=list1;
        
        while(temp->next)
            temp=temp->next;
        
        
        temp->next=list2;
        return list1;
    }
    else
        return list2;
}

BFList*
LLIST_Prepend (BFList   *list,
		 void* data)
{
  BFList *new_list;

  new_list = malloc(sizeof(BFList));
  new_list->data = data;
  new_list->next = list;
  new_list->prev = NULL;
  return new_list;
}

BFList*
LLIST_Append (BFList *list,void* data)
{
    BFList *new_list;
    BFList *last;

    new_list = malloc (sizeof(BFList));
    
    new_list->data = data;
    new_list->next = NULL;
    new_list->prev = NULL;

    if (list)
    {
        last = LLIST_Last (list);
        last->next = new_list;
        new_list->prev = last;
        new_list->next = NULL;
        return list;
    }
    else
    {
        return new_list;
    }
}

BFList*
LLIST_Last (BFList *list)
{
    if (list)
    {
        while (list->next)
            list = list->next;
    }
    return list;
}

BFList*
LLIST_Remove (BFList *list, void* data)
{
    BFList *tmp;
  
    tmp = list;
    while (tmp)
    {
        if (tmp->data != data)
            tmp = tmp->next;
        else
        {
            if (tmp->prev)
                ((BFList*)(tmp->prev))->next = tmp->next;
            if (tmp->next)
                ((BFList*)(tmp->next))->prev = tmp->prev;
            
            if (list == tmp)
                list = list->next;

            free(tmp);
            break;    
        }
    }
    return list;
}

int
LLIST_NoOfEntries(BFList *list)
{
    BFList *tmp;
    int count=0;

    tmp=list;
    while(tmp)
    {
        count++;
        tmp=tmp->next;
    }
    return count;
}




