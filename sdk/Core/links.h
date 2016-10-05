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
// links.h -- links


#ifndef __LINKS_H__
#define __LINKS_H__

/*
 * Linear links add to the tail only, no sorting.
 */

// __LIST_UNSORTED_ITEM_REQUIRED__(struct mystruct) // Supplies *prev, *next
#define __LIST_UNSORTED_ITEM_REQUIRED__(_my_struct_name)	\
	_my_struct_name *prev, *next;	

// __LIST_UNSORTED_LIST_REQUIRED__(struct mystruct) // Supplies *first, *last
#define __LIST_UNSORTED_LIST_REQUIRED__(_my_struct_name)	\
	_my_struct_name *first, *last;	

struct list_unsorted_item_s {
	struct list_unsorted_item_s *prev, *next;
};

struct list_unsorted_list_s {
	struct list_unsorted_item_s *first, *last;
};

void List_Unsorted_Add (void *_list, void *_item);
void List_Unsorted_Remove (void *_list, void *_item);

/*
 * Sorted links uses the name to add
 */

struct list_sorted_item_s {
	struct list_sorted_item_s *prev, *next;
	const char *name;
};

struct list_sorted_list_s {
	struct list_sorted_item_s *first, *last;
};


// __LIST_SORTED_ITEM_REQUIRED__(struct mystruct) // Supplies *prev, *next, *name
#define __LIST_SORTED_ITEM_REQUIRED__(_my_struct_name)	\
	_my_struct_name *prev, *next; \
	const char *name;	

void List_Sorted_Add (void *_list, void *_item);

// No difference in removing and structure can degrade to unsorted list
#define __LIST_SORTED_LIST_REQUIRED__ __LIST_UNSORTED_LIST_REQUIRED__  
#define List_Sorted_Remove List_Unsorted_Remove 


/*
** Void links - data payload.  List knows size.  Add/Remove allocate!
*/


struct voidlist_s {
	struct voiditem_s *first, *last;

	size_t	data_size;
	cbool	sorted;
};

struct voiditem_s {
	struct voiditem_s	*prev, *next;
	const char			*name_a;

// Knows parent
	struct voidlist_s	*parent;
	void				*data_a;
};


struct voiditem_s *VoidList_Add (struct voidlist_s *list, const char *name);
struct voiditem_s *VoidList_Remove (struct voiditem_s *item);

struct voidlist_s *VoidList_Create (size_t item_data_size, cbool is_sorted);
struct voidlist_s *VoidList_Shutdown (struct voidlist_s *me);




#endif // ! __LINKS_H__