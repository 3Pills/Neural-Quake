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
// gl_common.c -- server code for moving users




#include "quakedef.h"

renderer_t renderer;

/*
===============
GL_MakeNiceExtensionsList -- johnfitz
===============
*/
static char *GL_MakeNiceExtensionsList (const char *in)
{
	char *copy, *token, *out;
	int i, count;
	size_t bufsize;

	if (!in) return Z_Strdup("(none)");

	//each space will be replaced by 4 chars, so count the spaces before we malloc
	for (i = 0, count = 1; i < (int)strlen(in); i++)
	{
		if (in[i] == ' ')
			count++;
	}

	bufsize = strlen(in) + count*3 + 1; //usually about 1-2k
	out = (char *) Z_Malloc (bufsize /*strlen(in) + count*3 + 1*/); //usually about 1-2k
	out[0] = 0;

	copy = (char *) Z_Strdup(in);
	for (token = strtok(copy, " "); token; token = strtok(NULL, " "))
	{
		strlcat (out, "\n   ", bufsize);
		strlcat (out, token, bufsize);
	}

	Z_Free (copy);
	return out;
}


/*
===============
GL_Info_f -- johnfitz
===============
*/
void GL_Info_f (lparse_t *unused)
{
	Con_SafePrintf ("GL_VENDOR: %s\n", renderer.gl_vendor);
	Con_SafePrintf ("GL_RENDERER: %s\n", renderer.gl_renderer);
	Con_SafePrintf ("GL_VERSION: %s\n", renderer.gl_version);
	Con_SafePrintf ("GL_EXTENSIONS: %s\n", renderer.gl_extensions_nice);
}

/*
===============
GL_CheckExtensions
===============
*/
static cbool GL_ParseExtensionList (const char *list, const char *name)
{
	const char	*start;
	const char	*where, *terminator;

	if (!list || !name || !*name)
		return false;
	if (strchr(name, ' ') != NULL)
		return false;	// extension names must not have spaces

	start = list;
	while (1) {
		where = strstr (start, name);
		if (!where)
			break;
		terminator = where + strlen (name);
		if (where == start || where[-1] == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return true;
		start = terminator;
	}
	return false;
}

static void GL_CheckExtensions (void)
{
	int		(*print_fn) (const char *fmt, ...) __fp_attribute__((__format__(__printf__,1,2))) = Con_Printf;
	int		(*warn_fn) (const char *fmt, ...) __fp_attribute__((__format__(__printf__,1,2))) = Con_Warning;

	if (host_initialized)
	{
		print_fn = Con_DPrintf;
		warn_fn = Con_DPrintf;
	}


	//
	// multitexture
	//
	if (COM_CheckParm("-nomtex"))
		warn_fn ("Multitexture disabled at command line\n");
	else if (GL_ParseExtensionList(renderer.gl_extensions, "GL_ARB_multitexture"))
	{
		renderer.GL_MTexCoord2fFunc = (PFNGLMULTITEXCOORD2FARBPROC) eSystem_GL_GetProcAddress("glMultiTexCoord2fARB");
		renderer.GL_SelectTextureFunc = (PFNGLACTIVETEXTUREARBPROC) eSystem_GL_GetProcAddress("glActiveTextureARB");
		if (renderer.GL_MTexCoord2fFunc && renderer.GL_SelectTextureFunc)
		{
			print_fn ("FOUND: ARB_multitexture\n");
			renderer.TEXTURE0 = GL_TEXTURE0;
			renderer.TEXTURE1 = GL_TEXTURE1;
			renderer.gl_mtexable = true;
		}
		else warn_fn ("Couldn't link to multitexture functions\n");
	}
	else warn_fn ("multitexture not supported (extension not found)\n");

	//
	// texture_env_combine
	//
	if (COM_CheckParm("-nocombine"))
		warn_fn ("texture_env_combine disabled at command line\n");
	else if (GL_ParseExtensionList(renderer.gl_extensions, "GL_ARB_texture_env_combine"))
	{
		print_fn ("FOUND: ARB_texture_env_combine\n");
		renderer.gl_texture_env_combine = true;
	}
	else if (GL_ParseExtensionList(renderer.gl_extensions, "GL_EXT_texture_env_combine"))
	{
		print_fn ("FOUND: EXT_texture_env_combine\n");
		renderer.gl_texture_env_combine = true;
	}
	else warn_fn ("texture_env_combine not supported\n");

	//
	// texture_env_add
	//
	if (COM_CheckParm("-noadd"))
		warn_fn ("texture_env_add disabled at command line\n");
	else if (GL_ParseExtensionList(renderer.gl_extensions, "GL_ARB_texture_env_add"))
	{
		print_fn ("FOUND: ARB_texture_env_add\n");
		renderer.gl_texture_env_add = true;
	}
	else if (GL_ParseExtensionList(renderer.gl_extensions, "GL_EXT_texture_env_add"))
	{
		print_fn ("FOUND: EXT_texture_env_add\n");
		renderer.gl_texture_env_add = true;
	}
	else warn_fn ("texture_env_add not supported\n");

	//
	// gl_texture_non_power_of_two
	//
	if (COM_CheckParm("-nonpot"))
		warn_fn ("texture_non_power_of_two disabled at command line\n");
	else if (GL_ParseExtensionList(renderer.gl_extensions, "GL_ARB_texture_non_power_of_two"))
	{
		print_fn ("FOUND: GL_ARB_texture_non_power_of_two\n");
		renderer.gl_texture_non_power_of_two = true;
	}
	else warn_fn ("texture_non_power_of_two not supported\n");


	//
	// anisotropic filtering
	//
	if (GL_ParseExtensionList(renderer.gl_extensions, "GL_EXT_texture_filter_anisotropic"))
	{
		float test1,test2;
		GLuint tex;

		// test to make sure we really have control over it
		// 1.0 and 2.0 should always be legal values
		eglGenTextures(1, &tex);
		eglBindTexture (GL_TEXTURE_2D, tex);
		eglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
		eglGetTexParameterfv (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &test1);
		eglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0f);
		eglGetTexParameterfv (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &test2);
		eglDeleteTextures(1, &tex);

		if (test1 == 1 && test2 == 2)
		{
			print_fn ("FOUND: EXT_texture_filter_anisotropic\n");
			renderer.gl_anisotropy_able = true;
		}
		else warn_fn ("anisotropic filtering locked by driver. Current driver setting is %f\n", test1);

		//get max value either way, so the menu and stuff know it
		eglGetFloatv (GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &renderer.gl_max_anisotropy);

	} else warn_fn ("texture_filter_anisotropic not supported\n");

	//
	// swap control
	//

#ifdef PLATFORM_OSX
	renderer.gl_swap_control = true; // This should be true even if using SDL
#else
	renderer.gl_swap_control = VID_Local_Vsync_Init (renderer.gl_extensions);
	if (renderer.gl_swap_control)
		print_fn  ("Swap control enabled\n");
	else warn_fn ("Swap control not available\n");
#endif // PLATFORM_OSX

}

/*
===============
GL_SetupState -- johnfitz

does all the stuff from GL_Init that needs to be done every time a new GL render context is created
GL_Init will still do the stuff that only needs to be done once
===============
*/
void GL_SetupState (void)
{
	eglClearColor (0.15, 0.15, 0.15, 0); //johnfitz -- originally 1,0,0,0
	eglCullFace(GL_BACK); //johnfitz -- glquake used CCW with backwards culling -- let's do it right
	eglFrontFace(GL_CW); //johnfitz -- glquake used CCW with backwards culling -- let's do it right
	eglEnable(GL_TEXTURE_2D);
	eglEnable(GL_ALPHA_TEST);
	eglAlphaFunc(GL_GREATER, 0.666);
	eglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	eglShadeModel (GL_FLAT);
	eglHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); //johnfitz
	eglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	eglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	eglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	eglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	eglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

// Baker: OLD: eglDepthRange (0, 1);
// Baker: NEW: eglDepthRange (0, 0.5); // So mirror can use other depth range
//johnfitz -- moved here becuase gl_ztrick is gone.
	eglDepthRange (Q_GLDEPTHRANGE_MIN, Q_GLDEPTHRANGE_MAX);
	eglDepthFunc (GL_LEQUAL); //johnfitz -- moved here becuase gl_ztrick is gone.
	eglClearStencil (0); // Stencil Sky Fix - 2015 April 13 Per Spike

///////////////////////////////////////////////////////////////////////////////
// Baker: In the hope it will make pre-emptively make npot play nice with strange
// widths with "imagedump" which could occur with a replacement texture made by
// someone unknowledgable.
//
// This is in addition to the benefits of screenshots and video capture.
// Note: Direct3D wrapper does not support glPixelStorei, but doesn't support npot
// either so this will not affect "imagedump" because textures will be pow2 upscaled
// (hence, no alignment issues).
//
// Khronos: glPixelStorei
// "... parameter affects how pixel data is returned to client memory:"
// glReadPixels, glTexImage2D, glTexSubImage2D
//
///////////////////////////////////////////////////////////////////////////////

	eglPixelStorei (GL_PACK_ALIGNMENT, 1);
}

/*
===============
GL_Evaluate_Renderer
===============
*/
void GL_Evaluate_Renderer (void)
{
	static cbool first_init_complete = false;
	int		(*printf_fn) (const char *fmt, ...) __fp_attribute__((__format__(__printf__,1,2)));
	printf_fn = first_init_complete ? Con_DPrintf : Con_SafePrintf;
	first_init_complete = true;

	// Free extensions string before wiping the struct
	if (renderer.gl_extensions_nice != NULL)
		Z_Free (renderer.gl_extensions_nice);

	// Wipe the struct
	memset (&renderer, 0, sizeof(renderer));

	renderer.gl_vendor = (const char *) eglGetString (GL_VENDOR);
	renderer.gl_renderer = (const char *) eglGetString (GL_RENDERER);
	renderer.gl_version = (const char *) eglGetString (GL_VERSION);
	renderer.gl_extensions = (const char *) eglGetString (GL_EXTENSIONS);

	renderer.gl_extensions_nice = GL_MakeNiceExtensionsList (renderer.gl_extensions);

	GL_CheckExtensions (); //johnfitz

	//johnfitz -- intel video workarounds from Baker
	if (!strcmp(renderer.gl_vendor, "Intel"))
	{
		printf_fn ("Intel Display Adapter detected\n");
		renderer.isIntelVideo = true;
	}
	//johnfitz

	//johnfitz -- confirm presence of stencil buffer
	eglGetIntegerv(GL_STENCIL_BITS, &renderer.gl_stencilbits);
	if(!renderer.gl_stencilbits)
		Con_Warning ("Could not create stencil buffer\n");
	else Con_DPrintf ("%i bit stencil buffer\n", renderer.gl_stencilbits);
}

