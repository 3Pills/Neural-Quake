/*
Copyright (C) 2012-2014 Baker

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// system.c -- totally neutralized system functions

#define CORE_LOCAL
#define FILE_LOCAL

#include "core.h"

#include "file.h"
#include "stringlib.h"

#include <stdio.h> // fopen, etc.
#include <stdlib.h> // malloc, etc.


const char *System_Clipboard_Get_Text_Line (void)
{
	static char out[SYS_CLIPBOARD_SIZE_256];
	const char *cliptext = System_Clipboard_Get_Text_Alloc ();

	out[0] = 0;
	if (cliptext)
	{
		const char *src = cliptext;
		char *dst = out;
		int remaining = sizeof out - 1;
		/*
		\e	Write an <escape> character.
		\a	Write a <bell> character.
		\b	Write a <backspace> character.
		\f	Write a <form-feed> character.
		\n	Write a <new-line> character.
		\r	Write a <carriage return> character.
		\t	Write a <tab> character.
		\v	Write a <vertical tab> character.
		\'	Write a <single quote> character.
		\\	Write a backslash character.
		*/

		// Truncate at any new line or carriage return or backspace character
		// BUT convert any whitespace characters that are not actual spaces into spaces.
		//while (*src && dst - cliptext < sizeof out - 1 && *src != '\n' && *src != '\r' && *src != '\b')
		while (*src && remaining > 0 && *src != '\n' && *src != '\r' && *src != '\b')
		{
			if (*src < ' ')
				*dst++ = ' ';
			else *dst++ = *src;
			src++;
			remaining --;
		}
		*dst = 0;

		core_free (cliptext);
	}

	return (const char *)out;
}

int System_Alert (const char *fmt, ...)
{
	VA_EXPAND (text, SYSTEM_STRING_SIZE_1024, fmt)
	System_MessageBox (NULL, text);
	return 0;
}

