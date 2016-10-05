/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
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

//r_alias.c -- alias model rendering



#include "quakedef.h"



//up to 16 color translated skins
gltexture_t *playertextures[MAX_COLORMAP_SKINS];

#define NUMVERTEXNORMALS	162

float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

vec3_t	shadevector;

extern vec3_t	lightcolor; //johnfitz -- replaces "float shadelight" for lit support

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anorm_dots.h"
;

extern	vec3_t			lightspot;

float	*shadedots = r_avertexnormal_dots[0];

float	entalpha; //johnfitz

cbool	overbright; //johnfitz

cbool shading = true; //johnfitz -- if false, disable vertex shading for various reasons (fullbright, r_lightmap, showtris, etc)


/*
=============
GL_DrawAliasFrame -- johnfitz -- rewritten to support colored light, lerping, entalpha, multitexture, and r_drawflat
=============
*/
#ifdef DIRECT3D_WRAPPER
cbool direct3d_external_textures_workaround;
#endif // DIRECT3D_WRAPPER
void GL_DrawAliasFrame (aliashdr_t *paliashdr, lerpdata_t lerpdata)
{
	float 	vertcolor[4];
    trivertx_t *verts1, *verts2;
	int		*commands;
	int		count;
	float	u,v;
	float	blend, iblend;
	cbool lerping;

	if (lerpdata.pose1 != lerpdata.pose2)
	{
		lerping = true;
		verts1  = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
		verts2  = verts1;
		verts1 += lerpdata.pose1 * paliashdr->poseverts;
		verts2 += lerpdata.pose2 * paliashdr->poseverts;
		blend = lerpdata.blend;
		iblend = 1.0f - blend;
	}
	else // poses the same means either 1. the entity has paused its animation, or 2. r_lerpmodels is disabled
	{
		lerping = false;
		verts1  = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
		verts2  = verts1; // avoid bogus compiler warning
		verts1 += lerpdata.pose1 * paliashdr->poseverts;
		blend = iblend = 0; // avoid bogus compiler warning
	}

#ifdef DIRECT3D_WRAPPER
	if (!direct3d_external_textures_workaround)
	commands = (int *)((byte *)paliashdr + paliashdr->commands_d3d_no_external_skins);
	else
#endif // DIRECT3D_WRAPPER
	commands = (int *)((byte *)paliashdr + paliashdr->commands);

	vertcolor[3] = entalpha; //never changes, so there's no need to put this inside the loop

	while (1)
	{
		// get the vertex count and primitive type
		count = *commands++;
		if (!count)
			break;		// done

		if (count < 0)
		{
			count = -count;
			eglBegin (GL_TRIANGLE_FAN);
		}
		else
			eglBegin (GL_TRIANGLE_STRIP);

		do
		{
			u = ((float *)commands)[0];
			v = ((float *)commands)[1];
			if (mtexenabled)
			{
				renderer.GL_MTexCoord2fFunc (renderer.TEXTURE0, u, v);
				renderer.GL_MTexCoord2fFunc (renderer.TEXTURE1, u, v);
			}
			else
				eglTexCoord2f (u, v);

			commands += 2;

			if (shading)
			{
				if (r_drawflat_cheatsafe)
				{
					srand(count * (unsigned int)(src_offset_t) commands);
					eglColor3f (rand()%256/255.0, rand()%256/255.0, rand()%256/255.0);
				}
				else if (lerping)
				{
					vertcolor[0] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * lightcolor[0];
					vertcolor[1] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * lightcolor[1];
					vertcolor[2] = (shadedots[verts1->lightnormalindex]*iblend + shadedots[verts2->lightnormalindex]*blend) * lightcolor[2];
					eglColor4fv (vertcolor);
				}
				else
				{
					vertcolor[0] = shadedots[verts1->lightnormalindex] * lightcolor[0];
					vertcolor[1] = shadedots[verts1->lightnormalindex] * lightcolor[1];
					vertcolor[2] = shadedots[verts1->lightnormalindex] * lightcolor[2];
					eglColor4fv (vertcolor);
				}
			}

			if (lerping)
			{
				eglVertex3f (verts1->v[0]*iblend + verts2->v[0]*blend,
							verts1->v[1]*iblend + verts2->v[1]*blend,
							verts1->v[2]*iblend + verts2->v[2]*blend);
				verts1++;
				verts2++;
			}
			else
			{
				eglVertex3f (verts1->v[0], verts1->v[1], verts1->v[2]);
				verts1++;
			}
		} while (--count);

		eglEnd ();
	}

	rs_aliaspasses += paliashdr->numtris;
}


/*
=================
R_SetupAliasLighting -- johnfitz -- broken out from R_DrawAliasModel and rewritten
=================
*/
void R_SetupAliasLighting (entity_t	*e)
{
	vec3_t		dist;
	float		add;
	int			i;

	R_LightPoint (e->origin);

	//add dlights
	for (i=0 ; i<MAX_FITZQUAKE_DLIGHTS ; i++)
	{
		if (cl.dlights[i].die >= cl.time)
		{
			VectorSubtract (currententity->origin, cl.dlights[i].origin, dist);
			add = cl.dlights[i].radius - VectorLength(dist);
			if (add > 0)
				VectorMA (lightcolor, add, cl.dlights[i].color, lightcolor);
		}
	}

	// minimum light value on gun (24)
	if (e == &cl.viewent)
	{
		add = 72.0f - (lightcolor[0] + lightcolor[1] + lightcolor[2]);
		if (add > 0.0f)
		{
			lightcolor[0] += add / 3.0f;
			lightcolor[1] += add / 3.0f;
			lightcolor[2] += add / 3.0f;
		}
	}

	// minimum light value on players (8)
	if (currententity > cl_entities && currententity <= cl_entities + cl.maxclients)
	{
		add = 24.0f - (lightcolor[0] + lightcolor[1] + lightcolor[2]);
		if (add > 0.0f)
		{
			lightcolor[0] += add / 3.0f;
			lightcolor[1] += add / 3.0f;
			lightcolor[2] += add / 3.0f;
		}
	}

	// clamp lighting so it doesn't overbright as much (96)
	if (overbright)
	{
		add = 288.0f / (lightcolor[0] + lightcolor[1] + lightcolor[2]);
		if (add < 1.0f)
			VectorScale(lightcolor, add, lightcolor);
	}

	//hack up the brightness when fullbrights but no overbrights (256)
	if (gl_fullbrights.value && !gl_overbright_models.value)
		if (e->model->flags & MOD_FBRIGHTHACK)
		{
			lightcolor[0] = 256.0f;
			lightcolor[1] = 256.0f;
			lightcolor[2] = 256.0f;
		}

	shadedots = r_avertexnormal_dots[((int)(e->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
	VectorScale(lightcolor, 1.0f / 200.0f, lightcolor);
}

/*
=================
R_DrawAliasModel -- johnfitz -- almost completely rewritten
=================
*/



void R_DrawAliasModel (entity_t *e)
{
	aliashdr_t	*paliashdr;
	int			anim, skinnum;
	gltexture_t	*tx, *fb;
	lerpdata_t	lerpdata;
	
	//
	// setup pose/lerp data -- do it first so we don't miss updates due to culling
	//
	paliashdr = (aliashdr_t *)Mod_Extradata (e->model);
	R_SetupAliasFrame (paliashdr, e->frame, &lerpdata);
	R_SetupEntityTransform (e, &lerpdata);

	//
	// cull it
	//
	if (e != &cl.viewent && R_CullModelForEntity(e))
		return;

	//
	// transform it
	//
    eglPushMatrix ();
	R_RotateForEntity (lerpdata.origin, lerpdata.angles);

	// Special handling of view model to keep FOV from altering look.  Pretty good.  Not perfect but rather close.
	if (e == &cl.viewent && r_viewmodel_fov.value)
	{
		float scale = 1.0f / tan (Degree_To_Radians (scr_fov.value / 2.0f) ) * r_viewmodel_fov.value / 90.0f; // Reverse out fov and do fov we want

		eglTranslatef (paliashdr->scale_origin[0] * scale, paliashdr->scale_origin[1], paliashdr->scale_origin[2]);
		eglScalef (paliashdr->scale[0] * scale, paliashdr->scale[1], paliashdr->scale[2]);

	}
	else
	{
		eglTranslatef (paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2]);
		eglScalef (paliashdr->scale[0], paliashdr->scale[1], paliashdr->scale[2]);
	}

	//
	// random stuff
	//
	if (gl_smoothmodels.value && !r_drawflat_cheatsafe)
		eglShadeModel (GL_SMOOTH);
	if (gl_affinemodels.value)
		eglHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	overbright = gl_overbright_models.value;
	shading = true;

	//
	// set up for alpha blending
	//
	if (r_drawflat_cheatsafe || r_lightmap_cheatsafe) //no alpha in drawflat or lightmap mode
		entalpha = 1;
	else
		entalpha = ENTALPHA_DECODE(e->alpha);
	if (entalpha == 0)
	{
		tx = NULL; // Let's us know tx was never bound at all. (nor assigned!)
		goto cleanup;
	}

	if (entalpha < 1)
	{
		if (!renderer.gl_texture_env_combine) overbright = false; //overbright can't be done in a single pass without combiners
		if (e != &cl.viewent) // Baker: If we don't do this, looks real stupid
			eglDepthMask(GL_FALSE);
		eglEnable(GL_BLEND);
	}

	//
	// set up lighting
	//
	rs_aliaspolys += paliashdr->numtris;
	R_SetupAliasLighting (e);

	//
	// set up textures
	//
	anim = (int)(cl.ctime*10) & 3;
	
#if 1
	skinnum = e->skinnum;
	if ((skinnum >= paliashdr->numskins) || (skinnum < 0))
	{
		Con_DPrintf ("R_DrawAliasModel: no such skin # %d\n", skinnum);
		skinnum = 0;
	}
	tx = paliashdr->gltextures[skinnum][anim];
	fb = paliashdr->fbtextures[skinnum][anim];	
#else	
	tx = paliashdr->gltextures[e->skinnum][anim];
	fb = paliashdr->fbtextures[e->skinnum][anim];
#endif

	if (e->colormap && !gl_nocolors.value && e->coloredskin)
		tx = e->coloredskin;
	if (!gl_fullbrights.value)
		fb = NULL;

	if (!tx)
		tx = whitetexture; //Baker: Part of missing skins fix

// Baker: MH's tip to use texture matrix
	GL_DisableMultitexture();

	if (tx->flags & TEXPREF_ALPHA) // EF_ALPHA_MASKED_MDL
		eglEnable (GL_ALPHA_TEST);

#ifdef DIRECT3D_WRAPPER
// Baker: The Direct3D wrapper doesn't have texture matrix support
// so I am doing a workaround in the event external textures
// are used for an alias model (aka a Quake .mdl)
	if (tx->source_format != SRC_RGBA)
		direct3d_external_textures_workaround = false;
	else direct3d_external_textures_workaround = true;
#else
#pragma message ("Baker: Direct3D wrapper can't handle texture matrix")
	if (tx->source_format != SRC_RGBA)
	{
		float width_scale = (float) tx->source_width / tx->width;
		float height_scale = (float) tx->source_height / tx->height;
		eglMatrixMode (GL_TEXTURE);
		eglLoadIdentity ();
		eglScalef (width_scale, height_scale, 1.0f);
		if (fb)
		{
			float fbwidth_scale = (float) fb->source_width / fb->width;
			float fbheight_scale = (float) fb->source_height / fb->height;
			GL_EnableMultitexture();
			eglMatrixMode (GL_TEXTURE);
			eglLoadIdentity ();
			eglScalef (fbwidth_scale, fbheight_scale, 1.0f);
			GL_DisableMultitexture();
		}
		eglMatrixMode (GL_MODELVIEW);
	}
#endif // !DIRECT3D_WRAPPER



	//
	// draw it
	//
	if (r_drawflat_cheatsafe)
	{
		eglDisable (GL_TEXTURE_2D);
		GL_DrawAliasFrame (paliashdr, lerpdata);
		eglEnable (GL_TEXTURE_2D);
		srand((int) (cl.ctime * 1000)); //restore randomness
	}
	else if (r_fullbright_cheatsafe)
	{
		GL_Bind (tx);
		shading = false;
		eglColor4f(1,1,1,entalpha);
		GL_DrawAliasFrame (paliashdr, lerpdata);
		if (fb)
		{
			eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			GL_Bind(fb);
			eglEnable(GL_BLEND);
			eglBlendFunc (GL_ONE, GL_ONE);
			eglDepthMask(GL_FALSE);
			eglColor3f(entalpha,entalpha,entalpha);
			Fog_StartAdditive ();
			GL_DrawAliasFrame (paliashdr, lerpdata);
			Fog_StopAdditive ();
			eglDepthMask(GL_TRUE);
			eglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			eglDisable(GL_BLEND);
		}
	}
	else if (r_lightmap_cheatsafe)
	{
		eglDisable (GL_TEXTURE_2D);
		shading = false;
		eglColor3f(1,1,1);
		GL_DrawAliasFrame (paliashdr, lerpdata);
		eglEnable (GL_TEXTURE_2D);
	}
	else if (overbright)
	{
		if  (renderer.gl_texture_env_combine && renderer.gl_mtexable && renderer.gl_texture_env_add && fb) //case 1: everything in one pass
		{
			GL_Bind (tx);
			eglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			eglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			eglTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			eglTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			eglTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
			GL_EnableMultitexture(); // selects TEXTURE1
			GL_Bind (fb);
			eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
			eglEnable(GL_BLEND);
			GL_DrawAliasFrame (paliashdr, lerpdata);
			eglDisable(GL_BLEND);
			GL_DisableMultitexture();
			eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}
		else if (renderer.gl_texture_env_combine) //case 2: overbright in one pass, then fullbright pass
		{
		// first pass
			GL_Bind(tx);
			eglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			eglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			eglTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			eglTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			eglTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
			GL_DrawAliasFrame (paliashdr, lerpdata);
			eglTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.0f);
			eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		// second pass
			if (fb)
			{
				eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				GL_Bind(fb);
				eglEnable(GL_BLEND);
				eglBlendFunc (GL_ONE, GL_ONE);
				eglDepthMask(GL_FALSE);
				shading = false;
				eglColor3f(entalpha,entalpha,entalpha);
				Fog_StartAdditive ();
				GL_DrawAliasFrame (paliashdr, lerpdata);
				Fog_StopAdditive ();
				eglDepthMask(GL_TRUE);
				eglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				eglDisable(GL_BLEND);
				eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			}
		}
		else //case 3: overbright in two passes, then fullbright pass
		{

		// first pass
			GL_Bind(tx);
			eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			GL_DrawAliasFrame (paliashdr, lerpdata);
		// second pass -- additive with black fog, to double the object colors but not the fog color
			eglEnable(GL_BLEND);
			eglBlendFunc (GL_ONE, GL_ONE);
			eglDepthMask(GL_FALSE);
			Fog_StartAdditive ();
			GL_DrawAliasFrame (paliashdr, lerpdata);
			Fog_StopAdditive ();
			eglDepthMask(GL_TRUE);
			eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			eglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			eglDisable(GL_BLEND);
		// third pass
			if (fb)
			{
				eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				GL_Bind(fb);
				eglEnable(GL_BLEND);
				eglBlendFunc (GL_ONE, GL_ONE);
				eglDepthMask(GL_FALSE);
				shading = false;
				eglColor3f(entalpha,entalpha,entalpha);
				Fog_StartAdditive ();
				GL_DrawAliasFrame (paliashdr, lerpdata);
				Fog_StopAdditive ();
				eglDepthMask(GL_TRUE);
				eglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				eglDisable(GL_BLEND);
				eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			}
		}
	}
	else
	{
		if (renderer.gl_mtexable && renderer.gl_texture_env_add && fb) //case 4: fullbright mask using multitexture
		{
			GL_DisableMultitexture(); // selects TEXTURE0
			GL_Bind (tx);
			eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			GL_EnableMultitexture(); // selects TEXTURE1
			GL_Bind (fb);
			eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
			eglEnable(GL_BLEND);
			GL_DrawAliasFrame (paliashdr, lerpdata);
			eglDisable(GL_BLEND);
			GL_DisableMultitexture();
			eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}
		else //case 5: fullbright mask without multitexture
		{
		// first pass
			GL_Bind(tx);
			eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			GL_DrawAliasFrame (paliashdr, lerpdata);
		// second pass
			if (fb)
			{
				GL_Bind(fb);
				eglEnable(GL_BLEND);
				eglBlendFunc (GL_ONE, GL_ONE);
				eglDepthMask(GL_FALSE);
				shading = false;
				eglColor3f(entalpha,entalpha,entalpha);
				Fog_StartAdditive ();
				GL_DrawAliasFrame (paliashdr, lerpdata);
				Fog_StopAdditive ();
				eglDepthMask(GL_TRUE);
				eglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				eglDisable(GL_BLEND);
			}
		}
	}

cleanup:
	if (tx->flags & TEXPREF_ALPHA) // EF_ALPHA_MASKED_MDL
		eglDisable (GL_ALPHA_TEST);

	eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	eglHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	eglShadeModel (GL_FLAT);
	eglDepthMask(GL_TRUE);
	eglDisable(GL_BLEND);
	eglColor3f(1,1,1);
	eglPopMatrix ();

#ifndef DIRECT3D_WRAPPER
	if (tx && tx->source_format != SRC_RGBA)
	{
		if (fb)
		{
			GL_EnableMultitexture ();
			eglMatrixMode (GL_TEXTURE);
			eglLoadIdentity ();
		}
		GL_DisableMultitexture ();
		eglMatrixMode (GL_TEXTURE);
		eglLoadIdentity ();
		eglMatrixMode (GL_MODELVIEW);
	} 
	else 
#endif // !DIRECT3D_WRAPPER
		GL_DisableMultitexture ();
}

//johnfitz -- values for shadow matrix
#define SHADOW_SKEW_X -0.7 //skew along x axis. -0.7 to mimic glquake shadows
#define SHADOW_SKEW_Y 0 //skew along y axis. 0 to mimic glquake shadows
#define SHADOW_VSCALE 0 //0=completely flat
#define SHADOW_HEIGHT 0.1 //how far above the floor to render the shadow
//johnfitz

/*
=============
GL_DrawAliasShadow -- johnfitz -- rewritten

TODO: orient shadow onto "lightplane" (a global mplane_t*)
=============
*/
void GL_DrawAliasShadow (entity_t *e)
{
	float	shadowmatrix[16] = {1,				0,				0,				0,
								0,				1,				0,				0,
								SHADOW_SKEW_X,	SHADOW_SKEW_Y,	SHADOW_VSCALE,	0,
								0,				0,				SHADOW_HEIGHT,	1};
	float		lheight;
	aliashdr_t	*paliashdr;
	lerpdata_t	lerpdata;

//	if (R_CullModelForEntity(e))  // Baker: Just because entity is culled doesn't mean shadow is!
//		return;

	if (e == &cl.viewent || e->model->flags & MOD_NOSHADOW)
		return;

	entalpha = ENTALPHA_DECODE(e->alpha);
	if (entalpha == 0) return;

	paliashdr = (aliashdr_t *)Mod_Extradata (e->model);
	R_SetupAliasFrame (paliashdr, e->frame, &lerpdata);
	R_SetupEntityTransform (e, &lerpdata);
	R_LightPoint (e->origin);
	lheight = currententity->origin[2] - lightspot[2];

// set up matrix
    eglPushMatrix ();
	eglTranslatef (lerpdata.origin[0],  lerpdata.origin[1],  lerpdata.origin[2]);
	eglTranslatef (0,0,-lheight);
	eglMultMatrixf (shadowmatrix);
	eglTranslatef (0,0,lheight);
	eglRotatef (lerpdata.angles[1],  0, 0, 1);
	eglRotatef (-lerpdata.angles[0],  0, 1, 0);
	eglRotatef (lerpdata.angles[2],  1, 0, 0);
	eglTranslatef (paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2]);
	eglScalef (paliashdr->scale[0], paliashdr->scale[1], paliashdr->scale[2]);

// draw it
	eglDepthMask(GL_FALSE);
	eglEnable (GL_BLEND);
	GL_DisableMultitexture ();
	eglDisable (GL_TEXTURE_2D);
	shading = false;
	eglColor4f(0,0,0,entalpha * 0.5);
	GL_DrawAliasFrame (paliashdr, lerpdata);
	eglEnable (GL_TEXTURE_2D);
	eglDisable (GL_BLEND);
	eglDepthMask(GL_TRUE);

//clean up
	eglPopMatrix ();


}

/*
=================
R_DrawAliasModel_ShowTris -- johnfitz
=================
*/
void R_DrawAliasModel_ShowTris (entity_t *e)
{
	aliashdr_t	*paliashdr;
	lerpdata_t	lerpdata;

	if (R_CullModelForEntity(e))
		return;

	paliashdr = (aliashdr_t *)Mod_Extradata (e->model);
	R_SetupAliasFrame (paliashdr, e->frame, &lerpdata);
	R_SetupEntityTransform (e, &lerpdata);

    eglPushMatrix ();
	R_RotateForEntity (lerpdata.origin,lerpdata.angles);
	eglTranslatef (paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2]);
	eglScalef (paliashdr->scale[0], paliashdr->scale[1], paliashdr->scale[2]);

	shading = false;
	eglColor3f(1,1,1);
	GL_DrawAliasFrame (paliashdr, lerpdata);

	eglPopMatrix ();
}


