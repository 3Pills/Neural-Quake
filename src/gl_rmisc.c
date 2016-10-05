/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
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
// r_misc.c

#include "quakedef.h"



/*
====================
GL_Fullbrights_f -- johnfitz
====================
*/
void GL_Fullbrights_f (cvar_t *var)
{
	TexMgr_ReloadNobrightImages ();
}

/*
====================
R_SetClearColor_f -- johnfitz
====================
*/
void R_SetClearColor_f (cvar_t *var)
{
	byte	*rgb;
	int		s;

	s = (int)r_clearcolor.value & 0xFF;
	rgb = (byte*)(vid.d_8to24table + s);
	eglClearColor (rgb[0]/255.0,rgb[1]/255.0,rgb[2]/255.0,0);
}

/*
====================
R_Novis_f -- johnfitz
====================
*/
void R_VisChanged (cvar_t *var)
{
	extern int vis_changed;
	vis_changed = 1;
}


/*
===============
R_Envmap_f

Grab six views for environment mapping tests
===============
*/
void R_Envmap_f (lparse_t *unused)
{
	unsigned	buffer[256*256];

	if (vid.direct3d)
	{
		// Direct3D wrapper doesn't seem to support size on the fly at this time.
		Con_Printf ("Not supported for Direct3D at this time\n");
		return;
	}

	if (cls.state != ca_connected)
	{
		Con_Printf ("No map running\n");
		return;
	}

	envmap = true;

	clx = cly = r_refdef.vrect.x = r_refdef.vrect.y = 0;
	clwidth = clheight = r_refdef.vrect.width = r_refdef.vrect.height = 256;

	r_refdef.viewangles[0] = 0;
	r_refdef.viewangles[1] = 0;
	r_refdef.viewangles[2] = 0;

	R_RenderView ();
	eglFinish (); // Force update before grabbing pixels.
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	
	Image_Save_PNG_QPath ("env_0.png", buffer, 256, 256);

	r_refdef.viewangles[1] = 90;
	R_RenderView ();
	eglFinish ();
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	
	Image_Save_PNG_QPath ("env_1.png", buffer, 256, 256);

	r_refdef.viewangles[1] = 180;
	R_RenderView ();
	eglFinish ();
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	Image_Save_PNG_QPath ("env_2.png", buffer, 256, 256);

	r_refdef.viewangles[1] = 270;
	R_RenderView ();
	eglFinish ();
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	
	Image_Save_PNG_QPath ("env_3.png", buffer, 256, 256);

	r_refdef.viewangles[0] = -90;
	r_refdef.viewangles[1] = 0;
	R_RenderView ();
	eglFinish ();
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	Image_Save_PNG_QPath ("env_4.png", buffer, 256, 256);

	r_refdef.viewangles[0] = 90;
	r_refdef.viewangles[1] = 0;
	eglFinish ();
	R_RenderView ();
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	Image_Save_PNG_QPath ("env_5.png", buffer, 256, 256);

	envmap = false;

	Recent_File_Set_QPath ("env_0.png");
	Con_Printf ("Envmaps env files created\n");
	vid.recalc_refdef = true; // Recalc the refdef next frame
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////

/*
===============
R_Init
===============
*/

void R_Init_Local (void)
{	
	Cmd_AddCommands (R_Init_Local);

	R_Init_FlashBlend_Bubble ();

	Entity_Inspector_Init ();
	TexturePointer_Init ();


	Fog_Init (); //johnfitz
}

/*
===============
R_NewGame -- johnfitz -- handle a game switch
===============
*/
void R_NewGame (void)
{
	int i;

	//clear playertexture pointers (the textures themselves were freed by texmgr_newgame)
	for (i = 0; i < MAX_COLORMAP_SKINS; i ++)
		playertextures[i] = NULL;
}

/*
===============
R_NewMap_Local
===============
*/

void R_NewMap_Local (void)
{
	int i;
	Stains_WipeStains_NewMap ();
//	TexturePointer_ClearCaches_NewMap ();
	GL_BuildLightmaps_Upload_All_NewMap ();

#pragma message ("Baker: Clean up some of the mirrors code, it is messy in places")
#ifdef GLQUAKE_MIRRORS // Baker change
// mirror:  1. Identify the mirror textures in the map (DONE)
	mirrortexturenum = -1;
	for (i = 0 ; i < cl.worldmodel->numtextures ; i++)
	{
//		if (cl.worldmodel->textures[i]->name && !strncmp(cl.worldmodel->textures[i]->name,"window02_1",10) )
		if (cl.worldmodel->textures[i]->name)
		{
			cbool Is_Texture_Prefix (const char *texturename, const char *prefixstring);
			if (Is_Texture_Prefix (cl.worldmodel->textures[i]->name, gl_texprefix_mirror.string) )
			{
				mirrortexturenum = i;
				Con_Printf ("Located a mirror texture at %i\n", i);
			}
		}
	}
// mirror end 1
#endif // Baker change + #ifdef GLQUAKE_MIRRORS // Baker change


	Fog_NewMap (); //johnfitz -- global fog in worldspawn

	TexturePointer_Reset ();

	load_subdivide_size = gl_subdivide_size.value; //johnfitz -- is this the right place to set this?
}



/*
===============
R_TranslateNewModelSkinColormap

Looks through our table and returns an existing model/skin/pants/shirt combination already uploaded.
If does not exist.  Uploads it.

Since this re-uses colormapped skins (2 red players = same texture) this will upload fewer skins
than the GLQuake way, but will colormap everything.

r_nocolormap_list provides a means to exclude colormapping trivial things like gibs.  Although it
supports 1024 combinations, it will rarely use more than 10 to 30 in practice.  All skins are
deleted on a new map.
===============
*/

// This is for an entity WITH an existing skin texture
// So we know it is an alias model (or at least was until now!)
cbool R_SkinTextureChanged (entity_t *cur_ent)
{
	gltexture_t *skintexture	= cur_ent->coloredskin;
	int entnum = cur_ent - cl_entities;

	if (skintexture->owner != cur_ent->model)
	{
#if 0
		Con_Printf ("ent %i Model changed\n", entnum);
#endif
		return true;	// Model changed
	}

	do
	{
		int playerslot				= cur_ent->colormap - 1;
		int shirt_color				= (cl.scores[playerslot].colors & 0xf0) >> 4;
		int pants_color				= cl.scores[playerslot].colors & 15;

		if (skintexture->pants != pants_color)
		{
#if 0
			Con_Printf ("ent %i: Pants changed\n", entnum);		// Pants changed
#endif
			return true;
		}

		if (skintexture->shirt != shirt_color)
		{
#if 0
			Con_Printf ("ent %i: Shirt changed\n", entnum);		// Shirt changed
#endif
			return true;
		}

		if (skintexture->skinnum != cur_ent->skinnum)
		{
#if 0
			Con_Printf ("ent %i: Player skin changed\n", entnum);		// Skin changed
#endif
			return true; // Skin changed
		}

		// NOTE: Baker --> invalid skin situation can persistently trigger "skin changed"
		return false;

	} while (0);

}


gltexture_t *R_TranslateNewModelSkinColormap (entity_t *cur_ent)
{
	int entity_number = cur_ent - cl_entities; // If player, this will be 1-16
	int shirt_color, pants_color, skinnum, matchingslot;
	aliashdr_t	*paliashdr;

	do	// REJECTIONS PHASE
	{
		// No model or it isn't an alias model
		if (!cur_ent->model || cur_ent->model->type != mod_alias)
			return NULL;

		// No color map or it is invalid
		if (cur_ent->colormap <= 0 || cur_ent->colormap > cl.maxclients)
			return NULL;

		// Certain models just aren't worth trying to colormap
		if (cur_ent->model->flags & MOD_NOCOLORMAP)
			return NULL;

		//TODO: move these tests to the place where skinnum gets received from the server
		paliashdr = (aliashdr_t *)Mod_Extradata (cur_ent->model);
		skinnum = cur_ent->skinnum;

		if (skinnum < 0 || skinnum >= paliashdr->numskins)
		{
			// Baker: Note I do not believe this ever happens!!!
			Con_DPrintf("(%d): Invalid player skin #%d\n", entity_number, skinnum);
			skinnum = 0;
		}

	} while (0);


	do // SEE IF WE HAVE SKIN + MODEL + COLOR ALREADY PHASE
	{
		int playerslot = cur_ent->colormap - 1;

		shirt_color = (cl.scores[playerslot].colors & 0xf0) >> 4;
		pants_color = cl.scores[playerslot].colors & 15;

		for (matchingslot = 0; matchingslot < MAX_COLORMAP_SKINS; matchingslot ++)
		{
			gltexture_t *curtex = playertextures[matchingslot];

			if (playertextures[matchingslot] == NULL)
				break; // Not found, but use this slot

			if (curtex->shirt != shirt_color) continue;
			if (curtex->pants != pants_color) continue;
			if (curtex->skinnum != skinnum) continue;
			if (curtex->owner != cur_ent->model) continue;

			// Found an existing translation for this
			return curtex;

		}

		if (matchingslot == MAX_COLORMAP_SKINS)
		{
			Host_Error ("Color Slots Full");
			return NULL;
		}

		// If we are here matchingslot is our new texture slot

	} while (0);

	do // UPLOAD THE NEW SKIN + MODEL PHASE (MAYBE COLOR)
	{
		aliashdr_t	*paliashdr = (aliashdr_t *)Mod_Extradata (cur_ent->model);
		byte		*pixels = (byte *)paliashdr + paliashdr->texels[skinnum]; // This is not a persistent place!
		char		name[64];

		c_snprintf(name, "player_%i", entity_number);

//		Con_Printf ("New upload\n");

		//upload new image
		playertextures[matchingslot] = TexMgr_LoadImage (cur_ent->model, -1 /*not bsp texture*/, name, paliashdr->skinwidth, paliashdr->skinheight,
		SRC_INDEXED, pixels, paliashdr->gltextures[skinnum][0]->source_file, paliashdr->gltextures[skinnum][0]->source_offset, TEXPREF_PAD | TEXPREF_OVERWRITE);

		if (playertextures[matchingslot])
		{
			playertextures[matchingslot]->skinnum = skinnum;
			TexMgr_ReloadImage (playertextures[matchingslot], shirt_color, pants_color);
		}

	} while (0);

	return playertextures[matchingslot];
}


