/*
Copyright (C) 2001-2012 Axel 'awe' Wefers (Fruitz Of Dojo)
Copyright (C) 2010-2011 MH
Copyright (C) 2009-2014 Baker

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
// vid.c

#include "quakedef.h"
#include "winquake.h"
#include "resource.h" // IDI_ICON2

#pragma comment (lib, "gdi32.lib") // CreateCompatibleDC, BitBlt, etc.

//
// miscelleanous init
//

void VID_Local_Window_PreSetup (void)
{
	WNDCLASS		wc;
	sysplat.hIcon = LoadIcon (sysplat.hInstance, MAKEINTRESOURCE (IDI_ICON1));


	// Register the frame class
	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC) WIN_MainWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = sysplat.hInstance;
//	wc.hIcon         = sysplat.hIcon;
	wc.hIcon		 = ExtractIcon (sysplat.hInstance, Folder_Binary_URL(), 0);
	wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = 0;  // We can change this later
	wc.lpszClassName = ENGINE_NAME;

	if (!RegisterClass (&wc))
		System_Error ("Couldn't register window class");

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
		vmode_t test		= { MODE_FULLSCREEN, devmode.dmPelsWidth, devmode.dmPelsHeight, devmode.dmBitsPerPel };
/* Baker: No more!
		cbool bpp_ok		= (devmode.dmBitsPerPel >= 16);
*/
		cbool bpp_ok		= (int)devmode.dmBitsPerPel == vid.desktop.bpp;
		cbool width_ok	= INBOUNDS (MIN_MODE_WIDTH, devmode.dmPelsWidth, MAX_MODE_WIDTH);
		cbool height_ok	= INBOUNDS (MIN_MODE_HEIGHT, devmode.dmPelsHeight, MAX_MODE_HEIGHT);
		cbool qualified	= (bpp_ok && width_ok && height_ok);

		devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

		if (qualified && !VID_Mode_Exists(&test, NULL) && ChangeDisplaySettings (&devmode, CDS_TEST | CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		{
			// Not a dup and test = ok ---> add it
			memcpy (&vid.modelist[vid.nummodes++], &test, sizeof(vmode_t) );
#if 0
			Con_SafePrintf ("Added %i: %i x %i %i\n", vid.nummodes -1, vid.modelist[vid.nummodes-1].width, vid.modelist[vid.nummodes-1].height, vid.modelist[vid.nummodes-1].bpp);
#endif
		}
	}
}


void WIN_Construct_Or_Resize_Window (DWORD style, DWORD exstyle, RECT window_rect)
{
	const char *nm = ENGINE_NAME;

	int x = window_rect.left, y = window_rect.top;
	int w = RECT_WIDTH(window_rect), h = RECT_HEIGHT(window_rect);

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

		if (ChangeDisplaySettings (&sysplat.gdevmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		System_Error ("Couldn't set fullscreen mode %i x %i @ %i bpp", vid.modelist[modenum].width, vid.modelist[modenum].height, vid.modelist[modenum].bpp);
}

static cbool VID_AllocBuffers (int width, int height);
static void VID_CreateDIB (int width, int height, unsigned char *palette);

cbool VID_Local_SetMode (int modenum)
{
	RECT client_rect	= {0,0,vid.modelist[modenum].width, vid.modelist[modenum].height};
	RECT window_rect	= client_rect;
	cbool bordered	= vid.modelist[modenum].type   == MODE_WINDOWED;/* &&
						  (vid.modelist[modenum].width  != vid.desktop.width ||
						  vid.modelist[modenum].height != vid.desktop.height); */

	DWORD ExWindowStyle = 0;
	DWORD WindowStyle	= bordered ? DW_BORDERED : DW_BORDERLESS;
	cbool restart = (sysplat.mainwindow != NULL);


	if (restart)
		VID_Local_Window_Renderer_Teardown (TEARDOWN_NO_DELETE_GL_CONTEXT);

	if (vid.modelist[modenum].type == MODE_FULLSCREEN)
		WIN_Change_DisplaySettings (modenum);

	AdjustWindowRectEx (&window_rect, WindowStyle, FALSE, ExWindowStyle);
	WIN_AdjustRectToCenterScreen(&window_rect);


	WIN_Construct_Or_Resize_Window (WindowStyle, ExWindowStyle, window_rect);

	if (vid.modelist[modenum].type == MODE_WINDOWED)
		ChangeDisplaySettings (NULL, 0);


	// Get focus if we can, get foreground, finish setup, pump messages.
	// then sleep a little.

	ShowWindow (sysplat.mainwindow, SW_SHOWDEFAULT);
	UpdateWindow (sysplat.mainwindow);
	SetWindowPos (sysplat.mainwindow, HWND_TOP, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOCOPYBITS);
	SetForegroundWindow (sysplat.mainwindow);

	System_Process_Messages_Sleep_100 ();

	// Find best integral factor, set both the x and the y

	for (vid.stretch_x = 1; vid.modelist[modenum].width  / vid.stretch_x > WINQUAKE_MAX_WIDTH ; vid.stretch_x ++);
	for (vid.stretch_y = 1; vid.modelist[modenum].height / vid.stretch_y > WINQUAKE_MAX_HEIGHT; vid.stretch_y ++);

	vid.stretch_x = vid.stretch_y = c_max (vid.stretch_x, vid.stretch_y);
	vid.conwidth  = vid.modelist[modenum].width  / vid.stretch_x;
	vid.conheight  = vid.modelist[modenum].height  / vid.stretch_y;

	vid.aspect = ((float) vid.conwidth / (float) vid.conheight) * (320.0 / 240.0); // toxic

	VID_CreateDIB (vid.conwidth, vid.conheight, vid.curpal);

	if (!VID_AllocBuffers (vid.conwidth, vid.conheight))
	{
		System_Error ("VID_AllocBuffers failed.");
		return false;
	}

	D_InitCaches (vid.surfcache, vid.surfcachesize);

	return true;
}

//
// in game
//

typedef struct dibinfo_s
{
	BITMAPINFOHEADER	header;
	RGBQUAD				acolors[256];
} dibinfo_t;


typedef struct
{
	HGDIOBJ			previously_selected_GDI_obj;
	HBITMAP			hDIBSection;
	unsigned char	*pDIBBase;
	HDC				hdcDIBSection;
	HDC				maindc;
} tdibset_t;

tdibset_t tdibset;




void VID_Local_Suspend (cbool bSuspend)
{
	if (bSuspend == false)
	{
		ChangeDisplaySettings (&sysplat.gdevmode, CDS_FULLSCREEN);
		ShowWindow(sysplat.mainwindow, SW_SHOWNORMAL);
		MoveWindow(sysplat.mainwindow, 0, 0, sysplat.gdevmode.dmPelsWidth, sysplat.gdevmode.dmPelsHeight, false); //johnfitz -- alt-tab fix via Baker
	} else  ChangeDisplaySettings (NULL, 0);
}

//
// window setup
//

// Baker: Similar to GL Initial setup after window is constructed, needs to know palette
void VID_CreateDIB (int width, int height, unsigned char *palette)
{
	dibinfo_t   dibheader;
	BITMAPINFO *pbmiDIB = (BITMAPINFO *) &dibheader;
	int i;

	tdibset.maindc = GetDC (sysplat.mainwindow);
	memset (&dibheader, 0, sizeof (dibheader));

#if 1
	if (width & 7) width = (width - (width & 7)) + 8;

#endif

	// fill in the bitmap info
	pbmiDIB->bmiHeader.biSize          = sizeof (BITMAPINFOHEADER);
	pbmiDIB->bmiHeader.biWidth         = width;
	pbmiDIB->bmiHeader.biHeight        = height;
	pbmiDIB->bmiHeader.biPlanes        = 1;
	pbmiDIB->bmiHeader.biBitCount      = 8;
	pbmiDIB->bmiHeader.biCompression   = BI_RGB;
	pbmiDIB->bmiHeader.biSizeImage     = 0;
	pbmiDIB->bmiHeader.biXPelsPerMeter = 0;
	pbmiDIB->bmiHeader.biYPelsPerMeter = 0;
	pbmiDIB->bmiHeader.biClrUsed       = 256;
	pbmiDIB->bmiHeader.biClrImportant  = 256;

	// fill in the palette
	for (i = 0; i < 256; i++)
	{
		// d_8to24table isn't filled in yet so this is just for testing
		dibheader.acolors[i].rgbRed   = palette[i * 3];
		dibheader.acolors[i].rgbGreen = palette[i * 3 + 1];
		dibheader.acolors[i].rgbBlue  = palette[i * 3 + 2];
	}

	// create the DIB section
	tdibset.hDIBSection = CreateDIBSection (tdibset.maindc,
							pbmiDIB,
							DIB_RGB_COLORS,
							(void**)&tdibset.pDIBBase,
							NULL,
							0);

	// set video buffers
	if (pbmiDIB->bmiHeader.biHeight > 0)
	{
		// bottom up
		vid.buffer = tdibset.pDIBBase + (height - 1) * width;
		vid.rowbytes = -width;
	}
	else
	{
		// top down
		vid.buffer = tdibset.pDIBBase;
		vid.rowbytes = width;
	}

	// clear the buffer
	memset (tdibset.pDIBBase, 0xff, width * height);

	if ((tdibset.hdcDIBSection = CreateCompatibleDC (tdibset.maindc)) == NULL)
		System_Error ("DIB_Init() - CreateCompatibleDC failed");

	if ((tdibset.previously_selected_GDI_obj = SelectObject (tdibset.hdcDIBSection, tdibset.hDIBSection)) == NULL)
		System_Error ("DIB_Init() - SelectObject failed");
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
// Baker: Shuts down the DIB.   Similar to GL_Teardown
	{
		if (tdibset.hdcDIBSection)
		{
			SelectObject (tdibset.hdcDIBSection, tdibset.previously_selected_GDI_obj);
			DeleteDC (tdibset.hdcDIBSection);
			tdibset.hdcDIBSection = NULL;
		}

		if (tdibset.hDIBSection)
		{
			DeleteObject (tdibset.hDIBSection);
			tdibset.hDIBSection = NULL;
			tdibset.pDIBBase = NULL;
		}

		if (tdibset.maindc)
		{
			// if maindc exists sysplat.mainwindow must also be valid
			ReleaseDC (sysplat.mainwindow, tdibset.maindc);
			tdibset.maindc = NULL;
		}
	}

	if (destroy)
	{
		DestroyWindow (sysplat.mainwindow);
		sysplat.mainwindow = NULL;
	}

	ChangeDisplaySettings (NULL, 0);
}



/*
================
VID_AllocBuffers
================
*/
// Baker: Allocates memory for surface cache, called only by setmode
static cbool VID_AllocBuffers (int width, int height)
{
	int		tsize, tbuffersize;

	tbuffersize = width * height * sizeof (*d_pzbuffer);
	tsize = D_SurfaceCacheForRes (width, height);
	tbuffersize += tsize;

	vid.surfcachesize = tsize;

	if (d_pzbuffer)
	{
		D_FlushCaches ();
		Hunk_FreeToHighMark (vid.highhunkmark);
		d_pzbuffer = NULL;
	}

	vid.highhunkmark = Hunk_HighMark ();
	d_pzbuffer = Hunk_HighAllocName (tbuffersize, "video");
	vid.surfcache = (byte *) d_pzbuffer +
					width * height * sizeof (*d_pzbuffer);

	return true;
}



//
//
//
// Equivalent of swap buffers
//
//
//

void VID_Update (vrect_t *rects)
{
	// We've drawn the frame; copy it to the screen

	if (tdibset.hdcDIBSection)
	{
		int numrects = 0;

#if 0 // Baker: Unused
		while (rects)
#endif
		{
			if (vid.stretch_x !=1 || vid.stretch_y != 1)
				StretchBlt(
				  tdibset.maindc,
				  rects->x * vid.stretch_x, rects->y * vid.stretch_y,
				  rects->width * vid.stretch_x, rects->height * vid.stretch_y,
				  tdibset.hdcDIBSection,
				  rects->x, rects->y,
				  rects->width, rects->height,
				  SRCCOPY);
			else
				BitBlt (tdibset.maindc,
						rects->x, rects->y,
						rects->width,
						rects->height,
						tdibset.hdcDIBSection,
						rects->x, rects->y,
						SRCCOPY);

			numrects++;
#if 0 // Baker: multi-rect drawing unused
			rects = rects->pnext;
#endif
		}
	}
	// END FLIPPAGE

}


//
//
//
// Palette Set
//
//
//


void VID_Local_Modify_Palette (unsigned char *palette)
{
	int		i, bestmatch, bestmatchmetric, t, dr, dg, db;
	byte	*ptmp;
	// GDI doesn't let us remap palette index 0, so we'll remap color
	// mappings from that black to another one
	bestmatchmetric = 256 * 256 * 3;

	for (i = 1; i < 256; i++)
	{
		dr = palette[0] - palette[i*3];
		dg = palette[1] - palette[i*3+1];
		db = palette[2] - palette[i*3+2];
		t = (dr * dr) + (dg * dg) + (db * db);

		if (t < bestmatchmetric)
		{
			bestmatchmetric = t;
			bestmatch = i;

			if (t == 0)
				break;
		}
	}

	for (i = 0, ptmp = vid.colormap; i < (1 << (VID_CBITS + 8)); i++, ptmp++)
	{
		if (*ptmp == 0)
			*ptmp = bestmatch;
	}
	vid.altblack = bestmatch;
}

void VID_Local_SetPalette (unsigned char *palette)
{
	int			i;
	RGBQUAD		colors[256];
	unsigned char *pal = palette;

	if (tdibset.hdcDIBSection)
	{
		// incoming palette is 3 component
		for (i = 0; i < 256; i++, pal += 3)
		{
			colors[i].rgbRed   = pal[0];
			colors[i].rgbGreen = pal[1];
			colors[i].rgbBlue  = pal[2];
			colors[i].rgbReserved = 0;
		}

		colors[0].rgbRed = 0;
		colors[0].rgbGreen = 0;
		colors[0].rgbBlue = 0;
		colors[255].rgbRed = 0xff;
		colors[255].rgbGreen = 0xff;
		colors[255].rgbBlue = 0xff;

		if (SetDIBColorTable (tdibset.hdcDIBSection, 0, 256, colors) == 0)
		{
			Con_SafePrintf ("DIB_SetPalette() - SetDIBColorTable failed\n");
		}
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

// Unsupported
void VID_Local_Multisample_f (cvar_t *var) {}
void VID_Local_Vsync_f (cvar_t *var) {}

void VID_Local_Shutdown (void)
{

	VID_Local_Window_Renderer_Teardown (TEARDOWN_FULL);
}


void VID_Local_Init (void)
{
// Early
#ifdef WINQUAKE_RENDERER_SUPPORT
	VID_Palette_NewGame ();
#endif // WINQUAKE_RENDERER_SUPPORT


}
