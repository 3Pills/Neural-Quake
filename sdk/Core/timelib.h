/*
Copyright (C) 2011-2014 Baker

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
// timelib.h -- time functions


#ifndef __TIMELIB_H__
#define __TIMELIB_H__

#include "environment.h"

///////////////////////////////////////////////////////////////////////////////
//  CORE: Time functions
///////////////////////////////////////////////////////////////////////////////

int Time_Minutes (int seconds);
int Time_Seconds (int seconds);
double Time_Now (void); // seconds precision time since 1970
void Time_Wait (double duration);
void Time_Sleep (unsigned long milliseconds);
// Double self-initializes now
//double Time_Precise_Now (void); // no set metric
#define Time_Now_Precise System_Time_Now_Precise



char *Time_To_String (double seconds_since_1970); // static buffer versions
char *Time_To_String_GMT (double seconds_since_1970); 

char *Time_To_Buffer (double seconds_since_1970, char *buf, size_t bufsize); // buffer fill versions
char *Time_To_Buffer_GMT (double seconds_since_1970, char *buf, size_t bufsize);

double Time_String_To_Time (const char *s);





#endif	// ! __TIMELIB_H__


