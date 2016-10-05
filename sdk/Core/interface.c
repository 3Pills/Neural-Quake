/*
Copyright (C) 2012-2014 Baker

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
// interface.c -- platform interface

#define CORE_LOCAL

#include "core.h"
#include "environment.h"
#include "interface.h"
#include <stdio.h> // fopen, etc.
#include "file.h"
#include <time.h> // time_t, etc.

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: URL QUERY
///////////////////////////////////////////////////////////////////////////////


const char * Folder_Binary_URL (void)
{
	return System_URL_Binary ();
}

const char * Folder_Binary_Folder_URL (void)
{
	return System_URL_Binary_Folder ();
}

const char * Folder_Caches_URL (void)
{
	return System_URL_Caches ();
}


///////////////////////////////////////////////////////////////////////////////
//  OS AND FOLDER INTERACTION: Baker
///////////////////////////////////////////////////////////////////////////////

cbool Folder_Open (const char *path_url)
{
	return System_Folder_Open (path_url);
}

cbool Folder_Open_Highlight (const char *path_to_file)
{
	return System_Folder_Open_Highlight(path_to_file);
}

cbool Folder_Open_Highlight_Binary (void)
{
	return System_Folder_Open_Highlight_Binary();
}

///////////////////////////////////////////////////////////////////////////////
//  CLIPBOARD -- Baker
///////////////////////////////////////////////////////////////////////////////

// gets image off clipboard
unsigned *Clipboard_Get_Image_Alloc (int *outwidth, int *outheight)
{
	return System_Clipboard_Get_Image_RGBA_Alloc (outwidth, outheight);
}

// gets text from clipboard, spaces out whitespace, terms at newline
const char* Clipboard_Get_Text_Line (void)
{
	return System_Clipboard_Get_Text_Line ();
}

const char* Clipboard_Get_Text_Alloc (void)
{
	return System_Clipboard_Get_Text_Alloc ();
}


// copies given image to clipbard
cbool Clipboard_Set_Image (unsigned *rgba, int width, int height)
{
	return System_Clipboard_Set_Image_RGBA (rgba, width, height);
}

// copies given text to clipboard
cbool Clipboard_Set_Text (const char * text_to_clipboard)
{
	return System_Clipboard_Set_Text (text_to_clipboard);
}

