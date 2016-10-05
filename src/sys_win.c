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
// sys.c -- system

#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h> // Baker: Removes a warning
#include <time.h>

#include "quakedef.h"
#include "winquake.h"
#include "resource.h"
#include "dedicated_win.h"

sysplat_t sysplat;


///////////////////////////////////////////////////////////////////////////////
//  CLOCK: Baker
///////////////////////////////////////////////////////////////////////////////

// Double self-initializes now
double System_DoubleTime (void)
{
	static	__int64	startcount;
	static double	pfreq;
	static cbool	first = true;

	__int64		pcount;

	QueryPerformanceCounter ((LARGE_INTEGER *)&pcount);
	if (first)
	{
		__int64	freq;
		QueryPerformanceFrequency ((LARGE_INTEGER *)&freq);
		if (freq <= 0)
			System_Error ("Hardware timer not available");

		pfreq = (double)freq;
		first = false;
		startcount = pcount;
		return 0.0;
	}

	// TODO: check for wrapping
	return (pcount - startcount) / pfreq;
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
//  SYSTEM IO
///////////////////////////////////////////////////////////////////////////////


#if id386

/*
================
System_MakeCodeWriteable
================
*/
void System_MakeCodeWriteable (unsigned long startaddr, unsigned long len)
{
	DWORD  flOldProtect;

	if (!VirtualProtect((LPVOID)startaddr, len, PAGE_READWRITE, &flOldProtect))
   		System_Error("Protection change failed");
}
#endif // id386




///////////////////////////////////////////////////////////////////////////////
//  SYSTEM ERROR: Baker
///////////////////////////////////////////////////////////////////////////////


void System_Error (const char *fmt, ...)
{
	static int	in_sys_error0 = 0;
	static int	in_sys_error1 = 0;
	static int	in_sys_error2 = 0;
	static int	in_sys_error3 = 0;

	VA_EXPAND (text, SYSTEM_STRING_SIZE_1024, fmt)

	if (!in_sys_error3) in_sys_error3 = 1;

	switch (isDedicated)
	{
	case true:
		ConProc_Error (text);
		break;

	case false:

#ifdef DIRECT3D_WRAPPER
		// Baker: Direct3D hides popups, although this assumes window is setup which might be bad assumption
		if (vid.initialized && vid.screen.type == MODE_FULLSCREEN)
		{
			VID_Shutdown ();
			Input_Shutdown ();
			System_Process_Messages_Sleep_100 ();
		}
#endif // DIRECT3D_WRAPPER

		switch (in_sys_error0)
		{
		case true: // Recursive error, like occurred during shutdown
			MessageBox(NULL, text, "Double Quake Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
			break;

		case false:
			in_sys_error0 = 1;
			Input_Shutdown ();
			MessageBox(NULL, text, "Quake Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
			break;
		}
		break;
	} // End of dedicated vs. non-dedicated switch statement

	if (!in_sys_error1)
	{
		in_sys_error1 = 1;
		Host_Shutdown ();
	}

// shut down QHOST hooks if necessary
	if (!in_sys_error2)
	{
		in_sys_error2 = 1;
		DeinitConProc ();
	}

	exit (1);
}



///////////////////////////////////////////////////////////////////////////////
//  SYSTEM EVENTS
///////////////////////////////////////////////////////////////////////////////


void System_Process_Messages_Sleep_100 (void)
{
	MSG				msg;
	while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	{
	    TranslateMessage (&msg);
	    DispatchMessage (&msg);
	}

	Sleep (100);
}

//
//
//
//
//
//
//
void System_SendKeyEvents (void)
{
	Input_Local_SendKeyEvents ();

}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM MAIN LOOP
///////////////////////////////////////////////////////////////////////////////

void System_Quit (void)
{
	Host_Shutdown();

	if (sysplat.tevent)
	{
		CloseHandle (sysplat.tevent);
		sysplat.tevent = NULL;
	}

	if (isDedicated)
		FreeConsole ();

// shut down QHOST hooks if necessary
	DeinitConProc ();

	exit (0);
}


void System_Init (void)
{
	OSVERSIONINFO	vinfo;

#if id386
	MaskExceptions ();
	Sys_SetFPCW ();
#endif // id386

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if (!GetVersionEx (&vinfo))
		System_Error ("Couldn't get OS info");

	if ((vinfo.dwMajorVersion < 4) ||
		(vinfo.dwPlatformId == VER_PLATFORM_WIN32s))
	{
		System_Error ("WinQuake requires at least Win95 or NT 4.0");
	}
#pragma message ("Baker: I'm not sure we can run on Windows 98 any more or even Windows 2000")

#if defined(GLQUAKE_RENDERER_SUPPORT) && !defined (DIRECT3D_WRAPPER)
	// This is the "starting Quake" dialog which we abuse for multisample
	if (!isDedicated)
		VID_Local_Startup_Dialog ();
#endif // GLQUAKE_RENDERER_SUPPORT + ! DIRECT3D_WRAPPER

	if (!(sysplat.tevent = CreateEvent(NULL, FALSE, FALSE, NULL)))
		System_Error ("Couldn't create event");

	if (isDedicated)
		Dedicated_Init ();
}


void System_SleepUntilInput (int time)
{
	MsgWaitForMultipleObjects(1, &sysplat.tevent, FALSE, time, QS_ALLINPUT);
}




int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	
	sysplat.hInstance = hInstance;

    return Main_Central (lpCmdLine, &sysplat.mainwindow, true /* perform loop */);
}

/* main window procedure */
LRESULT CALLBACK WIN_MainWndProc (
	HWND    hWnd,
	UINT    Msg,
	WPARAM  wParam,
	LPARAM  lParam)
{
	int fActive, fMinimized;

	// check for input messages
	if (WIN_IN_ReadInputMessages (hWnd, Msg, wParam, lParam)) return 0;

    switch (Msg)
    {
	// events we want to discard
	case WM_CREATE:		return 0;
	case WM_ERASEBKGND: return 1; // MH: treachery!!! see your MSDN!
	case WM_SYSCHAR:	return 0;

	case WM_KILLFOCUS:
		// Baker: Plus this makes it survive a Windows firewall warning "better"
		if (vid.screen.type == MODE_FULLSCREEN)
			ShowWindow(sysplat.mainwindow, SW_SHOWMINNOACTIVE);
		break;

	case WM_SYSCOMMAND:
		switch (wParam & ~0x0F)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			// prevent from happening
			return 0;
		}
		break;

   	case WM_CLOSE:
		//if (MessageBox (sysplat.mainwindow, "Are you sure you want to quit?", "Confirm Exit", MB_YESNO | MB_SETFOREGROUND | MB_ICONQUESTION) == IDYES)
		System_Quit ();

	    return 0;

	case WM_ACTIVATE:
		fActive = LOWORD(wParam);
		fMinimized = (BOOL) HIWORD(wParam);
		VID_AppActivate(!(fActive == WA_INACTIVE), fMinimized, false);
#pragma message ("Baker: If we lost the context due a Windows firewall warming we might be able to rebuild it here")

		return 0;

   	case WM_DESTROY:
        PostQuitMessage (0);
        return 0;

#ifdef GLQUAKE_RESIZABLE_WINDOW  // Baker: Optional resizeable GL window start
	case WM_GETMINMAXINFO:	// Sent before size change; can be used to override default mins/maxs
		//if (host.isAltTabCapable) // If we aren't ALT-TAB capable, we cannot resize.

		if (sysplat.mainwindow)
		{
			MINMAXINFO *mmi = (MINMAXINFO *) lParam;

			mmi->ptMinTrackSize.x = 320 + (int)vid.border_width;
			mmi->ptMinTrackSize.y = 200 + (int)vid.border_height;
		}
		return 0;

	case WM_SIZE:

		if (vid.Minimized || !vid.ActiveApp)
			return 0;

		if (wParam == SIZE_MINIMIZED)
			return 0;

		VID_Resize_Check ();
		return 0;
#endif  // GLQUAKE_RESIZABLE_WINDOW // Baker: Optional resizable window


#ifdef SUPPORTS_MP3_MUSIC // Baker change
	case WM_GRAPHNOTIFY:
		return WIN_MediaPlayer_MessageHandler (hWnd, Msg, wParam, lParam);
#endif // Baker change +

	case MM_MCINOTIFY:
        return WIN_CDAudio_MessageHandler (hWnd, Msg, wParam, lParam);

#ifdef WINQUAKE_RENDERER_SUPPORT
	case WM_PAINT:
		// Baker: GL does not seem to need.  Makes me wonder if we are
		// constantly drawing the sbar or something in GL
		// In WinQuake, I need this if I keep moving a window over the
		// top, it won't paint certain areas if I am tricky about it
		// without this scr_fullupdate. GLQuake won't do it.
		winquake_scr_fullupdate = 0;
		break;
#endif // WINQUAKE_RENDERER_SUPPORT

    default:
		break;
    }

	// pass all unhandled messages to DefWindowProc
	return DefWindowProc (hWnd, Msg, wParam, lParam);
}

