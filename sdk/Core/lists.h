/*
Copyright (C) 2009-2013 Baker

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// lists.h

#ifndef __LISTS_H__
#define __LISTS_H__

// Lists are alphabetical with forced lower case.
// Normal add sorts and will not add duplicates
typedef struct clist_s
{
	struct clist_s		*next;
	char				*name;
} clist_t;


void List_Free (clist_t** headnode);
void List_Add (clist_t** headnode, const char *name);
void List_Add_No_Case_To_Lower (clist_t** headnode, const char *name); // Doesn't lcase

int List_Count (clist_t *headnode);
void List_Print (clist_t *headnode);
clist_t *List_String_Split (const char *s, int ch_delimiter); // sorts results
cbool List_Find (const clist_t *headnode, const char *s_find); // Optimized for sorted list.  Doesn't work on an unsorted list.
clist_t *List_Find_Item (const clist_t *headnode, const char *s_find); // Like find, except it locates the item

void List_Concat_Unsorted (clist_t** headnode, clist_t *list2);
void List_Add_Unsorted (clist_t** headnode, const char *name);

void List_Add_Raw_Unsorted (clist_t** headnode, const void *buf, size_t bufsiz); // Adds bytes  (Black sheep, incompatible with Find) Used for message lists.
int List_Compare (clist_t *list1, clist_t *list2);

clist_t *List_From_String_Lines (const char *s); // Splits on newline. Whitespace to spaces, then trimmed.  All lower-cased.



#endif // ! __LISTS_H__




