/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske
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

#ifndef __VID_H__
#define __VID_H__

// vid.h -- video driver defs

// moved here for global use -- kristian
typedef enum { MODE_UNINIT = -1, MODE_WINDOWED = 0, MODE_FULLSCREEN = 1 } modestate_t;

#ifdef WINQUAKE_RENDERER_SUPPORT

#define VID_CBITS	6
#define VID_GRADES	(1 << VID_CBITS)
typedef byte pixel_t; // a pixel can be one, two, or four bytes

#define WINQUAKE_MAX_WIDTH  3000	// MAXWIDTH in r_shared.h is 3000.  Does not cascade into other definitions as much as MAXHEIGHT
#define WINQUAKE_MAX_HEIGHT 1080	// Must also change MAXHEIGHT 1080 in r_shared.h, d_ifacea.h.  This affects asm.
#endif // WINQUAKE_RENDERER_SUPPORT


typedef struct vrect_s
{
	int			x,y,width,height;
	struct vrect_s	*pnext;	// Baker: ASM expects this in struct
} vrect_t;

enum {TEARDOWN_NO_DELETE_GL_CONTEXT = 0, TEARDOWN_FULL = 1};

enum {USER_SETTING_FAVORITE_MODE = 0, ALT_ENTER_TEMPMODE = 1};

#define MAX_MODE_LIST				600
#define MAX_MODE_WIDTH				10000
#define MAX_MODE_HEIGHT				10000
#define MIN_MODE_WIDTH				640
#define MIN_MODE_HEIGHT				400

#define MIN_WINDOWED_MODE_WIDTH		320
#define MIN_WINDOWED_MODE_HEIGHT	200

typedef struct
{
	modestate_t	type;
#ifdef PLATFORM_OSX
	void*		ptr;	// Baker: I use this for OS X
#endif // PLATFORM_OSX
	int			width;
	int			height;
	int			bpp;
#ifdef SUPPORTS_REFRESHRATE
	int			refreshrate;
#endif // SUPPORTS_REFRESHRATE
} vmode_t;


#define VID_MIN_CONTRAST 1.0
#define VID_MAX_CONTRAST 2.0

#define VID_MIN_POSSIBLE_GAMMA 0.5
#define VID_MAX_POSSIBLE_GAMMA 4.0

#define VID_MIN_MENU_GAMMA 0.5
#define VID_MAX_MENU_GAMMA 1.0


typedef struct mrect_s
{
	int				left, right, bottom, top;
	int				center_x, center_y;
	int				width, height;
} mrect_t;

typedef struct
{
	vmode_t		modelist[MAX_MODE_LIST];
	int			nummodes; // The number of them filled in

	vmode_t		screen;

#ifdef GLQUAKE_RESIZABLE_WINDOW // Windows resize on the fly
	int				border_width;
	int				border_height;
	mrect_t			client_window;
#endif // GLQUAKE_RESIZABLE_WINDOW
	int			modenum_screen;		// mode # on-screen now
	int			modenum_user_selected;	// mode # user intentionally selected (i.e. not an ALT-ENTER toggle)

	int			conwidth;
	int			conheight;

#ifdef WINQUAKE_RENDERER_SUPPORT
// These need to be set when screen changes
	unsigned	rowbytes;		// may be > width if displayed in a window
	float		stretch_x;		// If we scaled for large resolution  WINDOWS StretchBlt usage
	float		stretch_y;		// If we scaled for large resolution  WINDOWS StretchBlt usage
	float		aspect;			// width / height -- < 0 is taller than wide

	pixel_t		*buffer;		// invisible buffer, the main vid.buffer!
	byte		*basepal;		// host_basepal
	pixel_t		*colormap;		// 256 * VID_GRADES size
	byte		altblack;

	byte		*surfcache;
	int			surfcachesize;
	int			highhunkmark;

#ifdef PLATFORM_OSX
	short*			pzbuffer;

	unsigned int	texture;
	unsigned int	texture_actual_width, texture_actual_height; //POW2 upsized
	float			texture_s1, texture_t1;

	unsigned int    rgbapal[256];
	unsigned int	*bitmap;

	cbool		texture_initialized;

#endif // PLATFORM_OSX

#endif // WINQUAKE_RENDERER_SUPPORT

// Color
#ifdef GLQUAKE_RENDERER_SUPPORT
	unsigned int	d_8to24table[256];  // Palette representation in RGB color
#endif // GLQUAKE_RENDERER_SUPPORT

	byte		gammatable[256];		// Palette gamma ramp (gamma, contrast)
	unsigned char	curpal[256*3];	// Palette RGB with gamma ramp

	int			numpages;
	int			recalc_refdef;		// if true, recalc vid-based stuff

	vmode_t		desktop;
	cbool		canalttab;
	cbool		wassuspended;
	cbool		ActiveApp;
	cbool		Hidden;
	cbool		Minimized;
	cbool		sound_suspended;
	cbool		initialized;

#ifdef GLQUAKE_RENDERER_SUPPORT // Windows resize on the fly
	cbool		resized; // Baker: resize on fly, if resized this flag gets cleared after resize actions occur
	cbool		warp_stale;
	cbool		consize_stale;
#endif  // GLQUAKE_RENDERER_SUPPORT

	int			direct3d;
	int			multisamples;
} viddef_t;

extern	viddef_t	vid;				// global video state

extern	int clx, cly, clwidth, clheight;


//cmd void VID_Test (void);
//cmd void VID_Restart_f (void);
void VID_Alt_Enter_f (void);


// During run ...
void VID_AppActivate(cbool fActive, cbool minimize, cbool hidden);
void VID_Local_Suspend (cbool bSuspend);
void VID_BeginRendering (int *x, int *y, int *width, int *height);
void VID_EndRendering (void);
void VID_SwapBuffers (void);
void VID_Local_SwapBuffers (void);

// Platform localized video setup ...
vmode_t VID_Local_GetDesktopProperties (void);
void VID_Local_Window_PreSetup (void);

// Main
void VID_Init (void);
void VID_Local_Init (void);
int VID_SetMode (int modenum);
cbool VID_Local_SetMode (int modenum);
void VID_Shutdown (void);
void VID_Local_Shutdown (void);
void VID_Local_Window_Renderer_Teardown (int full);
void VID_Local_Set_Window_Caption (const char *text);

// Video modes
cbool VID_Mode_Exists (vmode_t *test, int *outmodenum);
void VID_Local_AddFullscreenModes (void);


// Cvars and modes
vmode_t VID_Cvars_To_Mode (void);
void VID_Cvars_Sync_To_Mode (vmode_t *mymode);
void VID_Cvars_Set_Autoselect_Temp_Fullscreen_Mode (int favoritemode);
void VID_Cvars_Set_Autoselect_Temp_Windowed_Mode (int favoritemode);

#ifdef GLQUAKE_RENDERER_SUPPORT
void VID_Local_Startup_Dialog (void);
void VID_Renderer_Setup (void);
void VID_Local_Multisample_f (cvar_t *var);
void VID_BrightenScreen (void); // Non-hardware gamma

// Gamma Table
void VID_Gamma_Init (void);
void VID_Gamma_Think (void);
void VID_Gamma_Shutdown (void);
cbool VID_Local_IsGammaAvailable (unsigned short* ramps);
void VID_Local_Gamma_Set (unsigned short* ramps);
int VID_Local_Gamma_Reset (void);
void VID_Gamma_Clock_Set (void); // Initiates a "timer" to ensure gamma is good in fullscreen

#endif // GLQUAKE_RENDERER_SUPPORT


#ifdef GLQUAKE_RESIZABLE_WINDOW
// Baker: resize on the fly
void VID_Resize_Check (void);
void VID_Resize_Think (void);
void VID_Local_Resize_Act (void);
// Baker: end resize on the fly
#endif // GLQUAKE_RESIZABLE_WINDOW

#ifdef WINQUAKE_RENDERER_SUPPORT
void VID_Local_SetPalette (unsigned char *palette);
// called after any gamma correction

void VID_ShiftPalette (unsigned char *palette);
// called for bonus and pain flashes, and for underwater color changes

void VID_Update (vrect_t *rects); // Equivalent of swap buffers for WinQuake


void VID_Local_Modify_Palette (unsigned char *palette);
cbool VID_CheckGamma (void);  // Equivalent of VID_Gamma_Think
void VID_Palette_NewGame (void); // New game needs to reload palette (a few rare mods use custom palette / colormap)

#endif // WINQUAKE_RENDERER_SUPPORT


// Vsync on Windows doesn't work for software renderer
// But could probably be made to work
void VID_Local_Vsync (void);
void VID_Local_Vsync_f (cvar_t *var);

// Baker: Doesn't apply on a Mac
cbool VID_Local_Vsync_Init (const char *gl_extensions_str);

unsigned *VID_GetBuffer_RGBA_Malloc (int *width, int *height, cbool bgra);



#endif	// ! __VID_H__

