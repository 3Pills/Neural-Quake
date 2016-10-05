/*
Copyright (C) 2014-2014 Baker

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
// links.c

//#define CORE_LOCAL

#include "core.h"
#include "links.h" // Courtesy

// Not needed at this time.
// #include <stddef.h> // offsetof  ... //	list->data_offset = offsetof(struct voiditem_s, data_a); // We don't need that.

void List_Sorted_Add (void *_list, void *_item)
{	
	struct list_sorted_list_s *list = _list;
	struct list_sorted_item_s *item = _item;

	struct list_sorted_item_s *cursor, *prev = NULL;

	for (cursor = list->first, prev = NULL; cursor; prev = cursor, cursor = cursor->next )
		if (strcasecmp(cursor->name, item->name)  > 0)
			break;

	// Cursor is first item we beat, making prev worst item we didn't beat
	item->prev = prev, item->next = cursor;
	
	if (item->prev) item->prev->next = item;
	else list->first = item;  // We are first
	
	if (item->next) item->next->prev = item;
 	else list->last = item; // We are last
}


void List_Unsorted_Add (void *_list, void *_item)
{
	// First cast to struct type
	struct list_unsorted_list_s *list = _list;
	struct list_unsorted_item_s *item = _item;

	item->prev = list->last, item->next = NULL;

	if (item->prev) item->prev->next = item;
	else list->first = item; // We are first

	list->last = item; // We are last
}


void List_Unsorted_Remove (void *_list, void *_item)
{
	// First cast to struct type
	struct list_unsorted_list_s *list = _list;
	struct list_unsorted_item_s *item = _item;

	if (item->prev) 
		item->prev->next = item->next;
	else list->first = item->next; // I was first, so my next takes the place

	if (item->next) 
		item->next->prev = item->prev;
	else list->last = item->prev; // I was last, so update the last

	item->prev = item->next = NULL;
}


/*
 * Void links - data payload.  List knows size.  Add/Remove allocate!
 */

// Void payload list
// Conforms to a sorted and unsorted list



// Should data be ready to copy in?  No.  Prevents the convenience factor.
struct voiditem_s *VoidList_Add (struct voidlist_s *list, const char *name)
{
	struct voiditem_s *item = calloc(sizeof(struct voiditem_s), 1);

	item->name_a = strdup (name);
	item->parent = list;
	item->data_a = calloc(item->parent->data_size, 1);
	
	if (item->parent->sorted)
		List_Sorted_Add (item->parent, item);
	else List_Unsorted_Add (item->parent, item);

	return item;
}

struct voiditem_s *VoidList_Remove (struct voiditem_s *item)
{
	struct voidlist_s *list = item->parent;
	
	if (list->sorted)
		List_Sorted_Remove (list, item);
	else List_Unsorted_Remove (list, item);

	free ((void *)item->name_a);
	free (item->data_a);
	free (item);
	return NULL;
}

struct voidlist_s *VoidList_Shutdown (struct voidlist_s *list)
{ // Remove each one LIFO
	while (list->last)
		VoidList_Remove (list->last);
	
	free (list);
	return NULL;
}


struct voidlist_s *VoidList_Create (size_t item_data_size, cbool is_sorted)
{
	struct voidlist_s *list = calloc (sizeof(struct voidlist_s), 1);
	list->data_size = item_data_size;
	list->sorted = is_sorted;
	return list;
}




