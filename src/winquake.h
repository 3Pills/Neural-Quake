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
// winquake.h: Win32-specific Quake header file

#ifndef __WINQUAKE_H__
#define __WINQUAKE_H__

#include <core_windows.h>

#pragma comment(lib, "winmm.lib") // cd_win (mci ..), in_win (joy_Get ...), snd_win (waveOut ...)

//stephenkoren
#if _MSC_VER && _MSC_VER == 1800 // Visual C++ 2013
// cd audio
#define WM_GRAPHNOTIFY  				WM_USER + 13

// Vsync
typedef BOOL(APIENTRY * SETSWAPFUNC) (int);
typedef int (APIENTRY * GETSWAPFUNC) (void);
#endif
//stephenkoren

////////////////////////////////////////////////////////////////////
// Actual shared
////////////////////////////////////////////////////////////////////

// General ...
typedef struct
{
	HINSTANCE	hInstance;
	HICON		hIcon;
	HWND		mainwindow;
	HDC			draw_context;

	DEVMODE		gdevmode;

	SETSWAPFUNC wglSwapIntervalEXT;
	GETSWAPFUNC wglGetSwapIntervalEXT;

#ifdef GLQUAKE_RENDERER_SUPPORT // Baker: for multisample
	HWND		hwnd_dialog;

	int			multisamples;
	int			forcePixelFormat;

	PIXELFORMATDESCRIPTOR pfd;
#endif // GLQUAKE_RENDERER_SUPPORT


// Baker: Dedicated console
	HANDLE		hinput, houtput;

	HANDLE		tevent;
	HANDLE		hFile;
	HANDLE		heventParent;
	HANDLE		heventChild;

	cbool	sc_return_on_enter;
} sysplat_t;

extern sysplat_t sysplat;

// Sound ...

#include <dsound.h>
extern LPDIRECTSOUND pDS;
extern LPDIRECTSOUNDBUFFER pDSBuf;
extern DWORD gSndBufSize;

// Window setup/video

#define DW_BORDERLESS	(WS_POPUP) // Window covers entire screen; no caption, borders etc
#define DW_BORDERED		(WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)

#define RECT_WIDTH(_rect)	(_rect.right - _rect.left)
#define RECT_HEIGHT(_rect)  (_rect.bottom - _rect.top)

// Window / vid

void WIN_AdjustRectToCenterScreen (RECT *in_windowrect);
void WIN_AdjustRectToCenterUsable (RECT *in_windowrect);
void WIN_Change_DisplaySettings (int modenum);
void WIN_Construct_Or_Resize_Window (DWORD style, DWORD exstyle, RECT window_rect);
// Various functions passed around.

LRESULT CALLBACK WIN_MainWndProc (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

BOOL WIN_SetupPixelFormat(HDC hDC);
LONG WIN_CDAudio_MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // WinQuake
LONG WIN_MediaPlayer_MessageHandler (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
cbool WIN_IN_ReadInputMessages (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

#ifdef GLQUAKE_RENDERER_SUPPORT // Baker: Multisample support
int WIN_InitMultisample (HINSTANCE hInstance,HWND hWnd,PIXELFORMATDESCRIPTOR pfd, int ask_samples, int *pixelForceFormat);
#endif // GLQUAKE_RENDERER_SUPPORT


// Dedicated Console ...

DWORD RequestProc (DWORD dwNichts);
LPVOID GetMappedBuffer (HANDLE hfileBuffer);
void ReleaseMappedBuffer (LPVOID pBuffer);
BOOL GetScreenBufferLines (int *piLines);
BOOL SetScreenBufferLines (int iLines);
BOOL ReadText (LPTSTR pszText, int iBeginLine, int iEndLine);
BOOL WriteText (LPCTSTR szText);
int CharToCode (char c);
BOOL SetConsoleCXCY(HANDLE hStdout, int cx, int cy);

/*

Baker: List of libraries

advapi32.lib
dsound.lib
dxguid.lib
gdi32.lib
kernel32.lib
ole32.lib
oleaut32.lib
shell32.lib
user32.lib
winmm.lib
wsock32.lib

advapi32.lib dsound.lib dxguid.lib gdi32.lib kernel32.lib ole32.lib oleaut32.lib shell32.lib user32.lib winmm.lib wsock32.lib

// GLQUAKE:		opengl32.lib and glu32.lib
// DX8QUAKE:	d3d8.lib and d3dx8.lib
// MP3:			strmiids.lib // dxsdk/sdk8/lib
// Optional:	libcurl.lib

// fmod.dll is a run-time loaded library

Baker: List of include folders


..\sdk\										// general
..\sdk\core									// core
..\sdk\dxsdk\sdk8\include					// dxsdk
..\sdk\libcurl-7.18.0-win32-msvc\include    // curl

Include aggregate:
../sdk/,../sdk/core,../sdk/dxsdk/sdk8/include,../sdk/libcurl-7.18.0-win32-msvc/include

Baker: List of lib folders

../sdk/dxsdk/sdk8/lib  ../sdk/libcurl-7.18.0-win32-msvc

In CodeBlocks/MinGW32, this is used for Direct3D, DirectInput, DirectSound due
to limitations of CodeBlocks/MinGW32 using their own slightly incompatible Window headers
..\sdk\dxsdk\sdk8\include					// dxsdk

*/

#endif // __WINQUAKE_H__
