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
#include "sdlquake.h"
//#include "resource.h" // IDI_ICON2

void VID_Local_Window_PreSetup (void)
{
    // I don't think we need anything here
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
        System_Error("Could not initialize SDL Video");

    // Attributes -- stencil, multisample, anything else?
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);	// Set depth 24 bits
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);	// Set depth 24 bits

}


vmode_t VID_Local_GetDesktopProperties (void)
{
    SDL_DisplayMode mode;
    vmode_t desktop = {0};

    if (SDL_GetDesktopDisplayMode(0, &mode) != 0)
        System_Error("Could not get desktop display mode");

	desktop.type		=	MODE_FULLSCREEN;
	desktop.width		=	mode.w;
	desktop.height		=	mode.h;
	desktop.bpp			=	SDL_BITSPERPIXEL(mode.format);

	return desktop;
}

//
// vsync
//


cbool VID_Local_Vsync_Init (const char *gl_extensions_str)
{
#if 0
    int ret;
    if (!gl_extensions_str)
    {
        System_Alert ("No extension string");
        return false;
    }

	if (!strstr(gl_extensions_str, "GL_EXT_swap_control") && !strstr(gl_extensions_str, "GL_WIN_swap_hint"))
        return true; // Baker: Are these the extensions?  What about Linux GLX?

    if (SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0) == -1)
        return false;

    if (SDL_GL_GetAttribute(SDL_GL_SWAP_CONTROL, &ret) == -1)
        return false;
#endif
	return true;
}

void VID_Local_Vsync (void)
{
	if (!renderer.gl_swap_control)
        return;
// Doesn't work
//    if (SDL_GLSetSwapInterval(!!vid_vsync.value) == -1)
//        Con_Printf ("VID_Vsync_f: failed to set attribute\n");
//    else Con_Printf ("Set Vsync successfully");
}

void VID_Local_Vsync_f (cvar_t *var)
{
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
    const int num_sdl_modes = SDL_GetNumDisplayModes(0);
	int	i;

	for (i = 0;  i < num_sdl_modes && vid.nummodes < MAX_MODE_LIST; i++)
	{
	    SDL_DisplayMode mode;

		if (SDL_GetDisplayMode(0, i, &mode) == 0)
		{
            vmode_t test	= { MODE_FULLSCREEN, mode.w, mode.h, SDL_BITSPERPIXEL(mode.format)};
            cbool width_ok	= INBOUNDS (MIN_MODE_WIDTH,  test.width, MAX_MODE_WIDTH );
            cbool height_ok	= INBOUNDS (MIN_MODE_HEIGHT, test.height, MAX_MODE_HEIGHT);
            cbool bpp_ok	= (test.bpp == vid.desktop.bpp);
            cbool qualified	= (bpp_ok && width_ok && height_ok);

            if (qualified && !VID_Mode_Exists(&test, NULL))
            {
                memcpy (&vid.modelist[vid.nummodes++], &test, sizeof(vmode_t) );
//                System_Alert ("Added %i x %i %i\n", vid.modelist[vid.nummodes-1].width, vid.modelist[vid.nummodes-1].height, vid.modelist[vid.nummodes-1].bpp);
            }

		} 
		else System_Error ("Couldn't get display mode");

	}
}

#if 0
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
#endif //


// Returns false if need to do GL setup again.
cbool VID_Local_SetMode (int modenum)
{
//	cbool reuseok = false;
//	cbool bordered	= vid.modelist[modenum].type   == MODE_WINDOWED &&
	//					  (vid.modelist[modenum].width  != vid.desktop.width ||
		//				  vid.modelist[modenum].height != vid.desktop.height);



// End resize window on the fly
//	if (sysplat.draw_context)
//		VID_Local_Window_Renderer_Teardown (TEARDOWN_NO_DELETE_GL_CONTEXT);
    cbool restart = (sysplat.draw_context != NULL);
    vmode_t *p = &vid.modelist[modenum];
    int flags = SDL_WINDOW_OPENGL;  // SDL_WINDOW_RESIZABLE  SDL_WINDOW_OPENGL|

    if (p->type == MODE_FULLSCREEN)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
//        System_Alert ("Asking for fullscreen");
    }
/*
    sysplat.main_window = SDL_CreateWindow(ENGINE_NAME,
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED,
                          640, 480,
                          SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
  */
    sysplat.mainwindow = SDL_CreateWindow(ENGINE_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, p->width, p->height, flags);
    if (  !(sysplat.draw_context = SDL_GL_CreateContext(sysplat.mainwindow))  )
        System_Error ("Couldn't create context fullscreen mode %i x %i @ %i bpp", p->width, p->height, p->bpp);

//    SDL_WM_SetCaption("hi", "lo");

// Get focus if we can, get foreground, finish setup, pump messages.
// then sleep a little.

	if (!restart)
	{
		eglClear (GL_COLOR_BUFFER_BIT);
		VID_SwapBuffers ();
	}

// Are we able to re-use the context?
	return false;  //reuseok;
}

//
// in game
//

void VID_Local_SwapBuffers (void)
{
    SDL_GL_SwapWindow (sysplat.mainwindow);
}


void VID_Local_Suspend (cbool bSuspend)
{
	if (bSuspend == false)
	{
//		eChangeDisplaySettings (&sysplat.gdevmode, CDS_FULLSCREEN);
//		ShowWindow(sysplat.mainwindow, SW_SHOWNORMAL);
//		MoveWindow(sysplat.mainwindow, 0, 0, sysplat.gdevmode.dmPelsWidth, sysplat.gdevmode.dmPelsHeight, false); //johnfitz -- alt-tab fix via Baker
#ifdef GLQUAKE_HARDWARE_GAMMA
		VID_Gamma_Clock_Set (); // Baker: Don't trust windows to do the right thing.
#endif // GLQUAKE_HARDWARE_GAMMA
	} //else  eChangeDisplaySettings (NULL, 0);
	// case SDL_ACTIVEEVENT: SDL handles the gamma
}

//
// window setup
//


cbool SDL_SetupPixelFormat (void)
{
//	24,						// 24-bit color depth
//	32,						// 32-bit z-buffer
//	8,						// 8-bit stencil buffer
//	if (!sysplat.multisamples)
/*	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, fsaa > 0 ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, fsaa);

	draw_context = SDL_SetVideoMode(width, height, bpp, flags);
	if (!draw_context) { // scale back fsaa
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		draw_context = SDL_SetVideoMode(width, height, bpp, flags);
	}
	if (!draw_context) { // scale back SDL_GL_DEPTH_SIZE
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		draw_context = SDL_SetVideoMode(width, height, bpp, flags);
		if (!draw_context)
			Sys_Error ("Couldn't set video mode");
	}
// read the obtained z-buffer depth
	if (SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depthbits) == -1)
		depthbits = 0;

// read obtained fsaa samples
	if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &fsaa_obtained) == -1)
		fsaa_obtained = 0;

// GLMaxSize?*/
    return true;
}



cbool VID_Local_IsGammaAvailable (unsigned short* ramps)
{
	if (!SDL_GetWindowGammaRamp (sysplat.mainwindow, &ramps[0], &ramps[256], &ramps[512]) )
		return false;

	return true;
}


void VID_Local_Gamma_Set (unsigned short* ramps)
{
	SDL_SetWindowGammaRamp (sysplat.mainwindow, &ramps[0], &ramps[256], &ramps[512]);
}

int VID_Local_Gamma_Reset (void)
{
    // Does SDL need?  I don't think so.
	unsigned short gammaramps[3][256];
    int i;
	for (i = 0;i < 256; i++)
		gammaramps[0][i] = gammaramps[1][i] = gammaramps[2][i] = (i * 65535) / 255;

	return !!SDL_SetWindowGammaRamp (sysplat.mainwindow, gammaramps[0], gammaramps[1], gammaramps[2]);
}

// Baker: Starting Quake Dialog
void VID_Local_Startup_Dialog (void)
{
    // Not for SDL.
}


//
//  Window
//


void VID_Local_Set_Window_Caption (const char *text)
{
    const char *new_caption = text ? text : ENGINE_NAME;

    if (!sysplat.mainwindow)
        return;

    SDL_SetWindowTitle (sysplat.mainwindow, text);
}


void VID_Local_Shutdown (void)
{
//    VID_Local_Window_Renderer_Teardown (TEARDOWN_FULL);

#ifdef GLQUAKE_HARDWARE_GAMMA
	VID_Gamma_Shutdown (); // Needed?  I bet it is?
#endif // GLQUAKE_HARDWARE_GAMMA

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

// This function gets called before anything happens
void VID_Local_Init (void)
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
        System_Error("Could not initialize SDL Video");

//    SDL_putenv("SDL_VIDEO_CENTERED=center");
// Early
#ifdef GLQUAKE_RENDERER_SUPPORT
	VID_Renderer_Setup (); // Hooks up our GL functions
#endif // GLQUAKE_RENDERER_SUPPORT

}
