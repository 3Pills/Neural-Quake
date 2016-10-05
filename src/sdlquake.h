/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2012 John Fitzgibbons and others
Copyright (C) 2009-2014 Baker and others

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
// sdlquake.h: Win32-specific Quake header file

#ifndef __SDLQUAKE_H__
#define __SDLQUAKE_H__

#ifdef _MSC_VER

#pragma warning( disable : 4142 ) // SDL likes to define int8_t, uintptr_t, etc.
#pragma comment (lib, "src\sdl2.lib")
#pragma comment (lib, "src\sdl2main.lib")
#include <SDL.h> 

#include <core_windows.h> // LLWinKeyHook
#else
#include <SDL2/SDL.h>
#endif

////////////////////////////////////////////////////////////////////
// Actual shared
////////////////////////////////////////////////////////////////////

// General ...
typedef struct
{
    SDL_Window	    *mainwindow;

    SDL_GLContext	*draw_context;


#if 0 // def GLQUAKE_RENDERER_SUPPORT // Baker: for multisample
	HWND		hwnd_dialog;

	int			multisamples;
	int			forcePixelFormat;

	PIXELFORMATDESCRIPTOR pfd;
#endif // GLQUAKE_RENDERER_SUPPORT

} sysplat_t;

extern sysplat_t sysplat;

// Important:  link to mingw32 or equivalent FIRST in linker section for SDL
// Otherwise you get undefined reference to 'WinMain@16'

#endif // __SDLQUAKE_H__
