/*
Copyright (C) 2013-2014 Baker

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
// math_general.c

#include "core.h"
#include "math_general.h" // Courtesy include

// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
unsigned int NextPowerOfTwo (unsigned int v) // compute the next highest power of 2 of 32-bit v
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}


cbool isPowerOfTwo (unsigned int x)
{
  return ((x != 0) && ((x & (~x + 1)) == x));
}

unsigned int PowerOfTwoSize (unsigned int x)
{
	if (isPowerOfTwo(x))
		return x;

	return NextPowerOfTwo(x);
}

// ch is a char variable holding a hexadecimal digit, some systems have digittoint
int hex_char_to_int (char ch)
{
	if (!isxdigit(ch)) { Core_Error ("%c is not a valid hex digit", ch); return 0; }
	
	// is decimal ? decimal  : lowercase hex eval
	return ( isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10  );
}
