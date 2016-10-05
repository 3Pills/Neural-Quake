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
// math_general.h -- basic


#ifndef __MATH_GENERAL_H__
#define __MATH_GENERAL_H__


#define RANDOM_FLOAT ((float)rand()/(float)RAND_MAX)
      
cbool isPowerOfTwo (unsigned int x);
unsigned int NextPowerOfTwo (unsigned int v);
unsigned int PowerOfTwoSize (unsigned int x);

int hex_char_to_int (char ch);

#endif // ! __MATH_GENERAL_H__



