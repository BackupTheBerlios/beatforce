/*
  Beatforce/ Startup of beatforce

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
#ifndef __BFLIST_H__
#define __BFLIST_H__

typedef struct BFList
{
    void *data;        
    void *next;
    void *prev;
}BFList;

BFList* LLIST_Combine(BFList *list1,BFList *list2);

BFList* LLIST_Prepend (BFList *list,void* data);
BFList* LLIST_Append  (BFList *list,void* data);
BFList* LLIST_Remove  (BFList *list,void* data);

BFList* LLIST_Last    (BFList *list);

int LLIST_NoOfEntries(BFList *list);
#endif
