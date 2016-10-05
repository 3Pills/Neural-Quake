/*
Copyright (C) 2009-2013 Baker

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
// vid_wgl.c -- server code for moving users


#include "quakedef.h"
#include "winquake.h"
#include "resource.h" // IDI_ICON2


//
// miscelleanous init
//

void VID_Local_Window_PreSetup (void)
{
	WNDCLASS		wc;
	sysplat.hIcon = LoadIcon (sysplat.hInstance, MAKEINTRESOURCE (IDI_ICON1));


	// Register the frame class
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)WIN_MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = sysplat.hInstance;
//  wc.hIcon         = sysplat.hIcon;
	wc.hIcon		 = ExtractIcon (sysplat.hInstance, Folder_Binary_URL(), 0);
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = NULL;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = ENGINE_NAME;

    if (!RegisterClass (&wc) )
		System_Error ("Couldn't register window class");

#pragma message ("Baker: See if we can get multisample to work for Direct3D")
#ifndef DIRECT3D_WRAPPER
	 // Baker: Multisample support
	if (sysplat.hwnd_dialog)
	{
		if (vid_multisample.value)
		{
			// Poke into it for the PFD
			HDC	hdc			= GetDC(sysplat.hwnd_dialog);
			int unused		= WIN_SetupPixelFormat (hdc);
			HGLRC wglHRC	= ewglCreateContext( hdc );
			HDC wglHDC 		= ewglGetCurrentDC();
			int unused2		= ewglMakeCurrent( hdc, wglHRC);
			int ask_samples	= (int)vid_multisample.value;

			if (ask_samples != 2 && ask_samples != 4 && ask_samples != 8)
			{
				Con_Warning ("Multisamples requested \"%i\" is invalid, trying 4\n", ask_samples);
				ask_samples = 4;
			}

			// Do it.  We already have desktop properties
			sysplat.multisamples = WIN_InitMultisample (sysplat.hInstance, sysplat.hwnd_dialog, sysplat.pfd, ask_samples, &sysplat.forcePixelFormat);

			// Your mission is complete.  You may leave now ...
			ewglMakeCurrent(NULL, NULL);
			ewglDeleteContext(wglHRC);
			ReleaseDC(sysplat.hwnd_dialog, wglHDC);
			ReleaseDC(sysplat.hwnd_dialog, hdc);

			if (sysplat.multisamples)
			{
				Con_SafePrintf ("Multisample x %i Enabled (Requested %i, Received %i).\n", sysplat.multisamples, ask_samples, sysplat.multisamples);
				vid.multisamples = sysplat.multisamples;
			}
			else Con_Warning ("Multisample: Requested but not available.\n");
		} else Con_SafePrintf ("Note: Multisample not requested\n");

		// Post teardown
		DestroyWindow (sysplat.hwnd_dialog);
		sysplat.hwnd_dialog = NULL;
	}
	// Baker: End multisample support
#endif // ! DIRECT3D_WRAPPER
}

vmode_t VID_Local_GetDesktopProperties (void)
{
	DEVMODE	devmode;
	vmode_t desktop = {0};

	if (!EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &devmode))
	{
		System_Error ("VID_UpdateDesktopProperties: EnumDisplaySettings failed");
		return desktop;
	}

	desktop.type		=	MODE_FULLSCREEN;
	desktop.width		=	devmode.dmPelsWidth;
	desktop.height		=	devmode.dmPelsHeight;
	desktop.bpp			=	devmode.dmBitsPerPel;

	return desktop;
}

//
// vsync
//


cbool VID_Local_Vsync_Init (const char *gl_extensions_str)
{
#ifdef DIRECT3D_WRAPPER
	return true;
#else
	if (strstr(gl_extensions_str, "GL_EXT_swap_control") || strstr(gl_extensions_str, "GL_WIN_swap_hint"))

	{
		sysplat.wglSwapIntervalEXT = (SETSWAPFUNC) ewglGetProcAddress("wglSwapIntervalEXT");
		sysplat.wglGetSwapIntervalEXT = (GETSWAPFUNC) ewglGetProcAddress("wglGetSwapIntervalEXT");

		if (sysplat.wglSwapIntervalEXT && sysplat.wglGetSwapIntervalEXT && sysplat.wglSwapIntervalEXT(0) &&
			sysplat.wglGetSwapIntervalEXT() != -1)
				return true;
	}
	return false;
#endif
}

void VID_Local_Vsync (void)
{
#ifndef DIRECT3D_WRAPPER
	if (renderer.gl_swap_control)
	{
		if (vid_vsync.value)
		{
			if (!sysplat.wglSwapIntervalEXT(1))
				Con_Printf ("VID_Vsync_f: failed on wglSwapIntervalEXT\n");
		}
		else
		{
			if (!sysplat.wglSwapIntervalEXT(0))
				Con_Printf ("VID_Vsync_f: failed on wglSwapIntervalEXT\n");
		}
	}
#endif // DIRECT3D_WRAPPER
}

void VID_Local_Vsync_f (cvar_t *var)
{
#ifdef DIRECT3D_WRAPPER
	if (host_post_initialized)
		Con_Printf ("Direct3D: vid_vsync takes effect after mode change\n          vsync only works for fullscreen\n");
#endif // DIRECT3D_WRAPPER
	VID_Local_Vsync ();
}


void VID_Local_Multisample_f (cvar_t *var)
{
#pragma message ("Baker: What if it isn't supported?  Like if we don't bother on a Mac?")
	if (host_initialized)
		Con_Printf ("%s set to \"%s\".  requires engine restart.\n"
				    "Note settings are: 2, 4, 8 and 0\n", var->name, var->string);
}

//
// vid modes
//


void VID_Local_AddFullscreenModes (void)
{

	BOOL		stat;						// Used to test mode validity
	DEVMODE		devmode = {0};
	int			hmodenum = 0;				// Hardware modes start at 0

	// Baker: Run through every display mode and get information

	while ( (stat = EnumDisplaySettings (NULL, hmodenum++, &devmode)) && vid.nummodes < MAX_MODE_LIST )
	{
#ifdef SUPPORTS_REFRESHRATE
		vmode_t test		= { MODE_FULLSCREEN, devmode.dmPelsWidth, devmode.dmPelsHeight, devmode.dmBitsPerPel, devmode.dmDisplayFrequency };
#else
		vmode_t test		= { MODE_FULLSCREEN, devmode.dmPelsWidth, devmode.dmPelsHeight, devmode.dmBitsPerPel };
#endif // ! SUPPORTS_REFRESHRATE
		cbool bpp_ok		= (int)devmode.dmBitsPerPel == vid.desktop.bpp;
		cbool width_ok	= INBOUNDS (MIN_MODE_WIDTH, devmode.dmPelsWidth, MAX_MODE_WIDTH);
		cbool height_ok	= INBOUNDS (MIN_MODE_HEIGHT, devmode.dmPelsHeight, MAX_MODE_HEIGHT);
		cbool qualified	= (bpp_ok && width_ok && height_ok);

#ifdef SUPPORTS_REFRESHRATE
		devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
#else
		devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
#endif // ! SUPPORTS_REFRESHRATE

		if (qualified && !VID_Mode_Exists(&test, NULL) && eChangeDisplaySettings (&devmode, CDS_TEST | CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		{
			// Not a dup and test = ok ---> add it
			memcpy (&vid.modelist[vid.nummodes++], &test, sizeof(vmode_t) );
//			Con_SafePrintf ("Added %i x %i %i\n", vid.modelist[vid.nummodes-1].width, vid.modelist[vid.nummodes-1].height, vid.modelist[vid.nummodes-1].bpp);
		}
	}
}

// Baker: begin resize window on the fly
void VID_Local_Resize_Act (void)
{
	WINDOWINFO windowinfo;
	windowinfo.cbSize = sizeof (WINDOWINFO);
	GetWindowInfo (sysplat.mainwindow, &windowinfo); // Client screen

	// Need to catch minimized scenario
	// Fill in top left, bottom, right, center
	vid.client_window.left = windowinfo.rcClient.left;
	vid.client_window.right = windowinfo.rcClient.right;
	vid.client_window.bottom = windowinfo.rcClient.bottom;
	vid.client_window.top = windowinfo.rcClient.top;
	vid.client_window.width = vid.client_window.right - vid.client_window.left;
	vid.client_window.height = vid.client_window.bottom - vid.client_window.top;

#ifndef DIRECT3D_WRAPPER
	if (COM_CheckParm ("-resizable"))
	{
		vid.screen.width = vid.client_window.width;
		vid.screen.height = vid.client_window.height;
		vid.consize_stale = true;
	}
#endif // DIRECT3D_WRAPPER
}
// End resize window on the fly

void WIN_Construct_Or_Resize_Window (DWORD style, DWORD exstyle, RECT window_rect)
{
	const char *nm = ENGINE_NAME;

	int x = window_rect.left, y = window_rect.top;
	int w = RECT_WIDTH(window_rect), h = RECT_HEIGHT(window_rect);

// Baker: begin resize window on the fly
	VID_Resize_Check ();
// End resize window on the fly
	if (sysplat.mainwindow)
	{
		SetWindowLong (sysplat.mainwindow, GWL_EXSTYLE, exstyle);
		SetWindowLong (sysplat.mainwindow, GWL_STYLE, style);
		SetWindowPos  (sysplat.mainwindow, NULL, x, y, w, h, SWP_DRAWFRAME);
		return;
	}

	sysplat.mainwindow = CreateWindowEx (exstyle, nm, nm, style, x, y, w, h, NULL, NULL, sysplat.hInstance, NULL);

	if (!sysplat.mainwindow) System_Error ("Couldn't create DIB window");
}

void WIN_Change_DisplaySettings (int modenum)
{
	// Change display settings
	sysplat.gdevmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	sysplat.gdevmode.dmPelsWidth = vid.modelist[modenum].width;
	sysplat.gdevmode.dmPelsHeight = vid.modelist[modenum].height;
	sysplat.gdevmode.dmBitsPerPel = vid.modelist[modenum].bpp;
	sysplat.gdevmode.dmSize = sizeof (DEVMODE);

	if (eChangeDisplaySettings (&sysplat.gdevmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		System_Error ("Couldn't set fullscreen mode %i x %i @ %i bpp", vid.modelist[modenum].width, vid.modelist[modenum].height, vid.modelist[modenum].bpp);
}

// Returns false if need to do GL setup again.
cbool VID_Local_SetMode (int modenum)
{
	cbool reuseok = false;
	RECT client_rect	= {0,0,vid.modelist[modenum].width, vid.modelist[modenum].height};
	RECT window_rect	= client_rect;
	cbool bordered	= vid.modelist[modenum].type   == MODE_WINDOWED &&
						  (vid.modelist[modenum].width  != vid.desktop.width ||
						  vid.modelist[modenum].height != vid.desktop.height);

	DWORD ExWindowStyle = 0;
	DWORD WindowStyle	= bordered ? DW_BORDERED : DW_BORDERLESS;
	cbool restart	= (sysplat.mainwindow != NULL);

	// Preserve these for hopeful reuse.
	HDC wglHDC 		= restart ? ewglGetCurrentDC() : 0;
	HGLRC wglHRC 	= restart ? ewglGetCurrentContext() : 0;

// Baker: begin resize window on the fly
#ifndef DIRECT3D_WRAPPER
	if (bordered && COM_CheckParm ("-resizable"))
		WindowStyle = WindowStyle | WS_SIZEBOX;
#endif // DIRECT3D_WRAPPER

// End resize window on the fly
	if (restart)
		VID_Local_Window_Renderer_Teardown (TEARDOWN_NO_DELETE_GL_CONTEXT);

	if (vid.modelist[modenum].type == MODE_FULLSCREEN)
		WIN_Change_DisplaySettings (modenum);

// Baker: begin resize window on the fly
	AdjustWindowRectEx (&window_rect, WindowStyle, FALSE, ExWindowStyle);  // Adds the borders
	// Window width - Client width
	vid.border_width = (window_rect.right - window_rect.left) - client_rect.right;
	vid.border_height = (window_rect.bottom - window_rect.top) - client_rect.bottom;
// End resize window on the fly
	WIN_AdjustRectToCenterScreen(&window_rect);

#if 1
	// Windows 8 introduces chaos :(
	if (restart && vid.modelist[modenum].type != vid.screen.type)
	{
		DestroyWindow (sysplat.mainwindow);
		sysplat.mainwindow = 0;
	}
#endif

	WIN_Construct_Or_Resize_Window (WindowStyle, ExWindowStyle, window_rect);

	if (vid.modelist[modenum].type == MODE_WINDOWED)
		eChangeDisplaySettings (NULL, 0);

	// clear to black so it isn't empty
	sysplat.draw_context = GetDC(sysplat.mainwindow);
	#pragma message ("Baker: Oddly PaintBlackness does not seem to be doing anything now that I have multisample")
	PatBlt (sysplat.draw_context, 0, 0, vid.modelist[modenum].width,vid.modelist[modenum].height, BLACKNESS);

// Get focus if we can, get foreground, finish setup, pump messages.
// then sleep a little.

	ShowWindow (sysplat.mainwindow, SW_SHOWDEFAULT);
	UpdateWindow (sysplat.mainwindow);
	SetWindowPos (sysplat.mainwindow, HWND_TOP, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOCOPYBITS);
	SetForegroundWindow (sysplat.mainwindow);

	System_Process_Messages_Sleep_100 ();

	WIN_SetupPixelFormat (sysplat.draw_context);

#ifdef DIRECT3D_WRAPPER
	Direct3D_SetVsync (vid_vsync.value); // Baker
	Direct3D_SetFullscreen (vid.modelist[modenum].type == MODE_FULLSCREEN); // Baker
	Direct3D_SetBPP (vid.desktop.bpp);
#endif // DIRECT3D_WRAPPER

	if (wglHRC && (reuseok = ewglMakeCurrent (sysplat.draw_context, wglHRC)) == 0)
	{
		// Tried to reuse context and it failed
		ewglDeleteContext (wglHRC);
		wglHRC = NULL;
		Con_DPrintf ("Context reuse failed.  Must reload textures.\n");
	}


	if (!wglHRC)
	{
		// Must create a context.
		wglHRC = ewglCreateContext( sysplat.draw_context );

#ifdef DIRECT3D_WRAPPER
		Direct3D_SetVsync (vid_vsync.value); // Baker
		Direct3D_SetFullscreen (vid.modelist[modenum].type == MODE_FULLSCREEN); // Baker
		Direct3D_SetBPP (vid.desktop.bpp);
#endif // DIRECT3D_WRAPPER

		if (!wglHRC)
			System_Error ("Could not initialize GL (wglCreateContext failed).\n\nMake sure you in are 65535 color mode, and try running -window.");
		if (!ewglMakeCurrent( sysplat.draw_context, wglHRC ))
			System_Error ("VID_Init: wglMakeCurrent failed");
	}

#if 1
	if (!restart)
	{
		eglClear (GL_COLOR_BUFFER_BIT);
		VID_SwapBuffers ();
	}
#endif
	return reuseok;
}

//
// in game
//

void VID_Local_SwapBuffers (void)
{
#ifdef DIRECT3D_WRAPPER
	Direct3D_SwapBuffers ();
#else
	if (SwapBuffers (sysplat.draw_context) == 0)
		System_MessageBox ("Quake", "Swapbuffers failed");
#endif // DIRECT3D_WRAPPER
}


void VID_Local_Suspend (cbool bSuspend)
{
	if (bSuspend == false)
	{
		eChangeDisplaySettings (&sysplat.gdevmode, CDS_FULLSCREEN);
		ShowWindow(sysplat.mainwindow, SW_SHOWNORMAL);
		MoveWindow(sysplat.mainwindow, 0, 0, sysplat.gdevmode.dmPelsWidth, sysplat.gdevmode.dmPelsHeight, false); //johnfitz -- alt-tab fix via Baker
#ifdef GLQUAKE_HARDWARE_GAMMA
		VID_Gamma_Clock_Set (); // Baker: Don't trust windows to do the right thing.
#endif // GLQUAKE_HARDWARE_GAMMA
	} else  eChangeDisplaySettings (NULL, 0);
}

//
// window setup
//


BOOL WIN_SetupPixelFormat (HDC hDC)
{
    static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
	1,						// version number
	PFD_DRAW_TO_WINDOW |	// support window
	PFD_SUPPORT_OPENGL |	// support OpenGL
	PFD_DOUBLEBUFFER,		// double buffered
	PFD_TYPE_RGBA,			// RGBA type
	24,						// 24-bit color depth
	0, 0, 0, 0, 0, 0,		// color bits ignored
	0,						// no alpha buffer
	0,						// shift bit ignored
	0,						// no accumulation buffer
	0, 0, 0, 0, 			// accum bits ignored
	32,						// 32-bit z-buffer
	8,						// 8-bit stencil buffer
	0,						// no auxiliary buffer
	PFD_MAIN_PLANE,			// main layer
	0,						// reserved
	0, 0, 0					// layer masks ignored
    };
    int pixelformat;
	PIXELFORMATDESCRIPTOR  test; //johnfitz

	if (!sysplat.multisamples)
	{
		if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 )
		{
			System_Error ("Video: ChoosePixelFormat failed");
			return FALSE;
		}
	} else pixelformat = sysplat.forcePixelFormat; // Multisample overrride

	DescribePixelFormat(hDC, pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &test);

    if (eSetPixelFormat(hDC, pixelformat, &pfd) == FALSE)
    {
        System_Error ("SetPixelFormat failed");
        return FALSE;
    }

	memcpy (&sysplat.pfd, &pfd, sizeof(pfd) );

    return TRUE;
}

void WIN_AdjustRectToCenterScreen (RECT *in_windowrect)
{
	vmode_t desktop = VID_Local_GetDesktopProperties ();
	int nwidth  = in_windowrect->right - in_windowrect->left;
	int nheight = in_windowrect->bottom - in_windowrect->top;

	in_windowrect->left = 0 + (desktop.width - nwidth) / 2;
	in_windowrect->top =  0 + (desktop.height - nheight) / 2;
	in_windowrect->right = in_windowrect->left + nwidth;
	in_windowrect->bottom = in_windowrect->top + nheight;
}

//
// window teardown
//

void VID_Local_Window_Renderer_Teardown (int destroy)
{
	// destroy = 1 = TEARDOWN_FULL else TEARDOWN_NO_DELETE_GL_CONTEXT (don't destroy the context or destroy window)
	HGLRC hRC = ewglGetCurrentContext();
    HDC	  hDC = ewglGetCurrentDC();

    ewglMakeCurrent(NULL, NULL);

    if (hRC && destroy)		ewglDeleteContext(hRC);
	if (hDC)				ReleaseDC(sysplat.mainwindow, hDC);

	if (sysplat.draw_context)
	{
		ReleaseDC (sysplat.mainwindow, sysplat.draw_context);
		sysplat.draw_context = NULL;
	}

	if (destroy)
	{
		DestroyWindow (sysplat.mainwindow);
		sysplat.mainwindow = NULL;
	}

	eChangeDisplaySettings (NULL, 0);
}

// Baker: Multisample support ...

#include "vid_wglext.h"		//WGL extensions

int	arbMultisampleSupported	= false;
int	arbMultisampleFormat	= 0;

cbool WGLisExtensionSupported(const char *extension)
{
#ifndef DIRECT3D_WRAPPER
	const size_t extlen = strlen(extension);
	const char *supported = NULL;
	const char *p;

	// Try To Use wglGetExtensionStringARB On Current DC, If Possible
	PROC wglGetExtString = ewglGetProcAddress("wglGetExtensionsStringARB");

	if (wglGetExtString)
		supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());

	// If That Failed, Try Standard Opengl Extensions String
	if (supported == NULL)
		supported = (char*)eglGetString(GL_EXTENSIONS);

	// If That Failed Too, Must Be No Extensions Supported
	if (supported == NULL)
		return false;

	// Begin Examination At Start Of String, Increment By 1 On False Match
	for (p = supported; ; p++)
	{
		// Advance p Up To The Next Possible Match
		p = strstr(p, extension);

		if (p == NULL)
			return false;															// No Match

		// Make Sure That Match Is At The Start Of The String Or That
		// The Previous Char Is A Space, Or Else We Could Accidentally
		// Match "wglFunkywglExtension" With "wglExtension"

		// Also, Make Sure That The Following Character Is Space Or NULL
		// Or Else "wglExtensionTwo" Might Match "wglExtension"
		if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
			return true;															// Match
	}
#else
	return false;

#endif // DIRECT3D_WRAPPER
}

// InitMultisample: Used To Query The Multisample Frequencies
int WIN_InitMultisample (HINSTANCE hInstance, HWND hWnd,PIXELFORMATDESCRIPTOR pfd, int ask_samples, int* pixelForceFormat)
{
	 // See If The String Exists In WGL!
	if (!WGLisExtensionSupported("WGL_ARB_multisample"))
	{
		return (arbMultisampleSupported = 0);
	}

	{
		// Get Our Pixel Format
		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)ewglGetProcAddress("wglChoosePixelFormatARB");
		if (!wglChoosePixelFormatARB)
		{
			arbMultisampleSupported=false;
			return false;
		}


		{
			// Get Our Current Device Context
			HDC hDC = GetDC(hWnd);

			int		pixelFormat;
			int		valid;
			UINT	numFormats;
			float	fAttributes[] = {0,0};

			// These Attributes Are The Bits We Want To Test For In Our Sample
			// Everything Is Pretty Standard, The Only One We Want To
			// Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
			// These Two Are Going To Do The Main Testing For Whether Or Not
			// We Support Multisampling On This Hardware.
			int iAttributes[] =
			{
				WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
				WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
				WGL_COLOR_BITS_ARB, 24 /*currentbpp? Nah */, // Baker: Mirror current bpp color depth?
				WGL_ALPHA_BITS_ARB,8,
				WGL_DEPTH_BITS_ARB,16,
				WGL_STENCIL_BITS_ARB,8, // Baker: Stencil bits
				WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
				WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
				WGL_SAMPLES_ARB, ask_samples /*multisample bits*/,
				0,0
			};


			while (ask_samples == 8 || ask_samples == 4 || ask_samples == 2)
			{
				iAttributes[19] = ask_samples;

				// First We Check To See If We Can Get A Pixel Format For 4 Samples
				valid = wglChoosePixelFormatARB(hDC,iAttributes,fAttributes,1,&pixelFormat,&numFormats);

				// If We Returned True, And Our Format Count Is Greater Than 1
				if (valid && numFormats >= 1)
				{
					*pixelForceFormat = arbMultisampleFormat = pixelFormat;
					return (arbMultisampleSupported = ask_samples);
				}

				ask_samples >>= 1; // Divide by 2
			}

			// Return Fail
			return  (arbMultisampleSupported = 0);
		}
	}
}

cbool VID_Local_IsGammaAvailable (unsigned short* ramps)
{
	if (!GetDeviceGammaRamp (sysplat.draw_context, ramps))
		return false;

	return true;
}


void VID_Local_Gamma_Set (unsigned short* ramps)
{
	SetDeviceGammaRamp (sysplat.draw_context, ramps);
}

int VID_Local_Gamma_Reset (void)
{
	int i;
	HDC hdc = GetDC (NULL);
	WORD gammaramps[3][256];

	for (i = 0;i < 256; i++)
		gammaramps[0][i] = gammaramps[1][i] = gammaramps[2][i] = (i * 65535) / 255;

	i = SetDeviceGammaRamp(hdc, &gammaramps[0][0]);
	ReleaseDC (NULL, hdc);

	return !!i;
}

// Baker: Starting Quake Dialog
void VID_Local_Startup_Dialog (void)
{
	sysplat.hwnd_dialog = CreateDialog(sysplat.hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, NULL);

	if (sysplat.hwnd_dialog)
	{
		RECT rect;
		GetWindowRect (sysplat.hwnd_dialog, &rect);
		WIN_AdjustRectToCenterScreen (&rect);
		SetWindowPos
		(
			sysplat.hwnd_dialog,
			NULL,
			rect.left,
			rect.top,
			0,
			0,
			SWP_NOZORDER | SWP_NOSIZE
		);
		ShowWindow (sysplat.hwnd_dialog, SW_SHOWDEFAULT);
		UpdateWindow (sysplat.hwnd_dialog);
		SetForegroundWindow (sysplat.hwnd_dialog);
	}

}


//
//  Window
//


void VID_Local_Set_Window_Caption (const char *text)
{
	if (!sysplat.mainwindow)
		return;

	if (!text)
		SetWindowText (sysplat.mainwindow, ENGINE_NAME);
	else
		SetWindowText (sysplat.mainwindow, text);
}


void VID_Local_Shutdown (void)
{
    VID_Local_Window_Renderer_Teardown (TEARDOWN_FULL);

#ifdef GLQUAKE_HARDWARE_GAMMA
	VID_Gamma_Shutdown ();
#endif // GLQUAKE_HARDWARE_GAMMA



}

void VID_Local_Init (void)
{
// Early
#ifdef GLQUAKE_RENDERER_SUPPORT
	VID_Renderer_Setup ();
#endif // GLQUAKE_RENDERER_SUPPORT

}
