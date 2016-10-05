/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2012 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske
Copyright (C) 2009-2014 Baker and others

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
// sys.c -- system

#include "quakedef.h"
#include "sdlquake.h"
sysplat_t sysplat;

///////////////////////////////////////////////////////////////////////////////
//  CLOCK: Baker
///////////////////////////////////////////////////////////////////////////////


double System_DoubleTime (void)
{
    return SDL_GetTicks() / 1000.0;
}


///////////////////////////////////////////////////////////////////////////////
//  FILE IO: Baker
///////////////////////////////////////////////////////////////////////////////


#define	MAX_HANDLES		100 //johnfitz -- was 10

FILE	*sys_handles[MAX_HANDLES];

int findhandle (void)
{
	int		i;

	for (i = 1 ; i < MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;

	System_Error ("out of handles");
	return -1;
}

int System_FileOpenRead (const char *path_to_file, int *pHandle)
{
	FILE	*f;
	int		i, retval;

	i = findhandle ();

	f = FS_fopen_read(path_to_file, "rb");

	if (!f)
	{
		*pHandle = -1;
		retval = -1;
	}
	else
	{
		sys_handles[i] = f;
		*pHandle = i;
		//retval = (int) FileHandle_GetLength (f);
		retval = (int) File_Length (path_to_file);
	}

	return retval;
}



int System_FileOpenWrite (const char *path_to_file)
{
	FILE	*f;
	int		i;

	i = findhandle ();

	f = FS_fopen_write(path_to_file, "wb");
	if (!f)
		System_Error ("Error opening %s: %s", path_to_file, strerror(errno));
	sys_handles[i] = f;

	return i;
}

void System_FileClose (int handle)
{
	FS_fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}


void System_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}


int System_FileRead (int handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int System_FileWrite (int handle, const void *pdata, int numbytes)
{
	return fwrite (pdata, 1, numbytes, sys_handles[handle]);
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM ERROR: Baker
///////////////////////////////////////////////////////////////////////////////


void System_Error (const char *fmt, ...)
{
	static int	in_sys_error = 0;
	int was_in_error = in_sys_error;

	VA_EXPAND (text, SYSTEM_STRING_SIZE_1024, fmt)

	in_sys_error = 1;

	if (was_in_error == false)
		Input_Shutdown ();

	System_MessageBox (was_in_error ? "Recursive Double Quake Error" : "Quake Error", text);

	Host_Shutdown ();

	exit (1);
}



void System_SendKeyEvents (void)
{
	extern inline int IN_SDL2_ScancodeToQuakeKey(SDL_Scancode scancode);
    SDL_Event event;

    SDL_PumpEvents();
    while (SDL_PollEvent (&event))
    {
		int scancode;
        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
				scancode = IN_SDL2_ScancodeToQuakeKey(event.key.keysym.scancode);
				Key_Event (scancode, event.key.type == SDL_KEYDOWN);
                return;
            case SDL_QUIT:
                System_Quit();
                break;
            default:
                SDL_PumpEvents();
                break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM MAIN LOOP
///////////////////////////////////////////////////////////////////////////////

void System_Quit (void)
{

	Host_Shutdown();

//	if (isDedicated)
//		FreeConsole ();

// shut down QHOST hooks if necessary
//	DeinitConProc ();

	exit (0);
}


void System_Init (void)
{
// What goes in here?
	// Floating point precision
// Anything else?
// What's a Mac need?
// What windows need?
//Dedicated
// Linux?
}

void System_Printf (const char *fmt, ...)
{
//	va_list		argptr;
//	char		text[1024];
//	DWORD		dummy;
//
//	va_start (argptr,fmt);
//	q_vsnprintf (text, sizeof(text), fmt, argptr);
//	va_end (argptr);
//
//	if (isDedicated)
//	{
//		WriteFile(houtput, text, strlen(text), &dummy, NULL);
//	}
//	else
//	{
//	/* SDL will put these into its own stdout log,
//	   so print to stdout even in graphical mode. */
//		fputs (text, stdout);
//	}
}


const char *Dedicated_ConsoleInput (void)
{
    char *pText = NULL;

//#ifdef SERVER_ONLY
//
//    if (stdin_ready != 0)
//    {
//        static char     text[256];
//        const int		length = read (0, text, FD_SIZE_OF_ARRAY (text));
//
//        stdin_ready = 0;
//
//        if (length > 0)
//        {
//			Con_Printf ("%i\n",(int)length);
//            text[length - 1]    = '\0';
//            pText               = &(text[0]);
//        }
//    }
//
//#endif // SERVER_ONLY

    return pText;
}





