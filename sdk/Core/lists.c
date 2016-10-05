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
// lists.c

#define CORE_LOCAL
#include "core.h"
#include "lists.h"


// By design List_Add does not allow duplicates!
void List_Add (clist_t** headnode, const char *name)
{
	clist_t	*listent,*cursor,*prev;

	// ignore duplicate
	for (listent = *headnode; listent; listent = listent->next)
	{
		if (!strcmp (name, listent->name))
			return;
	}

	listent = (clist_t *)core_malloc (sizeof(clist_t));
	listent->name = core_strdup (name);
	String_Edit_To_Lower_Case (listent->name);

	//insert each entry in alphabetical order
    if (*headnode == NULL ||
	    strcasecmp(listent->name, (*headnode)->name) < 0) //insert at front
	{
        listent->next = *headnode;
        *headnode = listent;
    }
    else //insert later
	{
        prev = *headnode;
        cursor = (*headnode)->next;
		while (cursor && (strcasecmp(listent->name, cursor->name) > 0))
		{
            prev = cursor;
            cursor = cursor->next;
        }
        listent->next = prev->next;
        prev->next = listent;
    }
}

// By design List_Add does not allow duplicates!
void List_Add_No_Case_To_Lower (clist_t** headnode, const char *name)
{
	clist_t	*listent,*cursor,*prev;

	// ignore duplicate
	for (listent = *headnode; listent; listent = listent->next)
	{
		if (!strcmp (name, listent->name))
			return;
	}

	listent = (clist_t *)core_malloc (sizeof(clist_t));
	listent->name = core_strdup (name);
	//String_Edit_To_Lower_Case (listent->name);

	//insert each entry in alphabetical order
    if (*headnode == NULL ||
	    strcasecmp(listent->name, (*headnode)->name) < 0) //insert at front
	{
        listent->next = *headnode;
        *headnode = listent;
    }
    else //insert later
	{
        prev = *headnode;
        cursor = (*headnode)->next;
		while (cursor && (strcasecmp(listent->name, cursor->name) > 0))
		{
            prev = cursor;
            cursor = cursor->next;
        }
        listent->next = prev->next;
        prev->next = listent;
    }
}


cbool List_Find (const clist_t *headnode, const char *s_find)
{
	const clist_t *cur;
	int count;
	int result;

	for (cur = headnode, count = 0; cur; cur = cur->next)
	{
		result = strcasecmp (cur->name, s_find);
		if (result == 0)
			return true;
		if (result > 0)
			break;
	}
	return false;
}

// Depends on list being sorted
clist_t *List_Find_Item (const clist_t *headnode, const char *s_find)
{
	const clist_t *cur;
	int count;
	int result;

	for (cur = headnode, count = 0; cur; cur = cur->next)
	{
		result = strcasecmp (cur->name, s_find);
		if (result == 0)
			return (clist_t *)cur; // Returns non-const
		if (result > 0)
			break;
	}
	return NULL;
}


void List_Add_Raw_Unsorted (clist_t** headnode, const void *buf, size_t bufsiz)
{
	clist_t	*listent;
	
	listent = (clist_t *)core_malloc (sizeof(clist_t));
	listent->name = (char *)c_memdup (buf, bufsiz);
	listent->next = NULL;

	if (*headnode)
	{
		clist_t *cur;
		for (cur = *headnode; cur->next; cur = cur->next);
		cur->next = listent;

	} else *headnode = listent;

}

void List_Add_Unsorted (clist_t** headnode, const char *name)
{
	clist_t	*listent;
	listent = (clist_t *)core_malloc (sizeof(clist_t));
	listent->name = core_strdup (name);
	listent->next = NULL;
	String_Edit_To_Lower_Case (listent->name);

	if (*headnode)
	{
		clist_t *cur;
		for (cur = *headnode; cur->next; cur = cur->next);
		cur->next = listent;

	} else *headnode = listent;
}

void List_Concat_Unsorted (clist_t** headnode, clist_t *list2)
{
	clist_t *cur, *prev = NULL;
	// Find the trailer
	for (cur = *headnode; cur; prev = cur, cur = prev->next);
	
	if (prev)
		prev->next = list2;
	else *headnode = list2;
}


void List_Free (clist_t** headnode)
{
	clist_t *blah;

	while (*headnode)
	{
		blah = (*headnode)->next;
		core_free ((*headnode)->name);
		core_free (*headnode);

		*headnode = blah;
	}

	// headnode is NULL
}


int List_Count (clist_t *headnode)
{
	clist_t *cur;
	int count;

	for (cur = headnode, count = 0; cur; cur = cur->next, count ++);

	return count;
}

int List_Compare (clist_t *list1, clist_t *list2)
{
	clist_t *cur_1 = list1;
	clist_t *cur_2 = list2;
	int count = 1;  // Notice we start at 1.  So a 1 member list returns 1 instead 0 for the comparison fail.

	while (1)
	{
		if ( !cur_1 != !cur_2 ) // What an ugly line!!  Basically if one is null and the other isn't, this fails.
			return count; // Failed comparison @ count

		if (!cur_1 || !cur_2)	// They should both have to be null because of above statement.
			return 0;			// Comparison same!

		if (strcmp (cur_1->name, cur_2->name))
			return count;		// Strings don't match.
		
		cur_1 = cur_1->next;
		cur_2 = cur_2->next;
		count ++;
	}
	return count;
}



void List_Print (clist_t *headnode)
{
	clist_t *cur;
	int count;
	for (cur = headnode, count = 0; cur; cur = cur->next, count ++)
		Core_Printf ("%i: %s\n", count, cur->name);
}


clist_t *List_String_Split (const char *s, int ch_delimiter)
{
	clist_t *list = NULL;
	const char* linestart = s;
	const char* cursor = s;
	char *s2;
	size_t length =  0;

	for (linestart = s, cursor = s, length = 0; *cursor; cursor ++)
	{
		if (*cursor == ch_delimiter)
		{
			s2 = strndup (linestart, length);
			List_Add (&list, s2);
			free (s2);

			linestart = cursor + 1;
			length = 0;
		} else length++;
	}

	// Get the trailer
	s2 = strndup (linestart, length);
	List_Add (&list, s2);
	free (s2);
	
	return list;
}

// Will add a trailing empty line after a newline.
clist_t *List_From_String_Lines (const char *s)
{
	clist_t *out = NULL;
	char *buf = core_strdup (s); // Copy
	
	char *ch, *thisline;
	size_t count;

	for (ch = buf, thisline = ch, count = 0; *ch; ch++, count++)
	{
		if (*ch != '\n')
			continue;

		// New line char
		*ch = 0; // Null it out
		String_Edit_Whitespace_To_Space (thisline);
		String_Edit_Trim (thisline);

		List_Add_Unsorted (&out, thisline);

		thisline = &ch[1]; // Next character
		count = 0;
	}
	
	String_Edit_Whitespace_To_Space (thisline);
	String_Edit_Trim (thisline);
	List_Add_Unsorted (&out, thisline);

	buf = core_free (buf); // Free buffer
	return out;
}

