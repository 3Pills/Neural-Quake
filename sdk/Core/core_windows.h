/*
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
// core_windows.h - <windows.h> plus fix up defines


#ifndef __CORE_WINDOWS_H__
#define __CORE_WINDOWS_H__

#include <windows.h>

#if !defined(__WINDOWS_VC6_BANDAGES__) && defined(_MSC_VER) && _MSC_VER < 1400
    #define __WINDOWS_VC6_BANDAGES__
    // Bandages to cover things that must require a service pack for Visual Studio 6 ..
    // Except Microsoft doesn't make the service packs available any more so we work around

	#ifdef IS_INTRESOURCE
		#define __WINDOWS_VC6_HAVE_PSDK_2003__
	#endif

    // GetFileAttributes ...
    #define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

	#ifndef __WINDOWS_VC6_HAVE_PSDK_2003__
		// Baker: I am using whether or not this is defined to detect
		// Windows February 2003 PSDK installation.

		
		// Defined in 2003 February SDK winuser.h 
		typedef struct tagWINDOWINFO
		{
		  DWORD cbSize;
		  RECT  rcWindow;
		  RECT  rcClient;
		  DWORD dwStyle;
		  DWORD dwExStyle;
		  DWORD dwWindowStatus;
		  UINT  cxWindowBorders;
		  UINT  cyWindowBorders;
		  ATOM  atomWindowType;
		  WORD  wCreatorVersion;
		} WINDOWINFO, *PWINDOWINFO, *LPWINDOWINFO;

		BOOL WINAPI GetWindowInfo(HWND hwnd, PWINDOWINFO pwi);
	
		//
		// Input ...
		//

		typedef ULONG ULONG_PTR;

		#define WH_KEYBOARD_LL 13
		#define LLKHF_UP			(KF_UP >> 8)

		#define MAPVK_VK_TO_VSC 0
		#define MAPVK_VSC_TO_VK 1
		#define MAPVK_VK_TO_CHAR 2
		#define MAPVK_VSC_TO_VK_EX 3

//		#define WM_INITMENUPOPUP                0x0117  // defined in winuser.h  it should be defined

	#endif // ! __WINDOWS_VC6_HAVE_PSDK_2003__

	// MSVC6 ONLY -- Do not do for CodeBlocks/MinGW/GCC
	typedef struct
	{
		DWORD		vkCode;
		DWORD		scanCode;
		DWORD		flags;
		DWORD		time;
		ULONG_PTR dwExtraInfo;
	} *PKBDLLHOOKSTRUCT;
		
	#define WM_MOUSEWHEEL                   0x020A // winuser.h, unsure how not defined
	#define MK_XBUTTON1 					0x0020 // winuser.h, unsure how not defined
	#define MK_XBUTTON2 					0x0040 // winuser.h, unsure how not defined

	#define WM_GRAPHNOTIFY  				WM_USER + 13

    // Vsync
    typedef BOOL (APIENTRY * SETSWAPFUNC) (int);
    typedef int (APIENTRY * GETSWAPFUNC) (void);

#endif // ! __WINDOWS_VC6_BANDAGES__

#if !defined(__GCC_VC6_BANDAGES__) && defined(__GNUC__)
    #define __GCC_BANDAGES__
    #define MK_XBUTTON1 					0x0020
    #define MK_XBUTTON2 					0x0040

       // Vsync
    typedef BOOL (APIENTRY * SETSWAPFUNC) (int);
    typedef int (APIENTRY * GETSWAPFUNC) (void);
#endif // ! __GCC_VC6_BANDAGES__


#if !defined(__WINDOWS_VC2008_BANDAGES__) && _MSC_VER && _MSC_VER == 1500 // Visual C++ 2008
	#define __WINDOWS_VC2008_BANDAGES__
       // Vsync
    typedef BOOL (APIENTRY * SETSWAPFUNC) (int);
    typedef int (APIENTRY * GETSWAPFUNC) (void);
	#define WM_GRAPHNOTIFY  				WM_USER + 13
#endif // ! __WINDOWS_VC2008_BANDAGES__

#endif // !__CORE_WINDOWS_H__

