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

#ifndef __MODEL__
#define __MODEL__

#include "modelgen.h"
#include "spritegn.h"

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/

// entity effects

#define	EF_BRIGHTFIELD			1
#define	EF_MUZZLEFLASH 			2
#define	EF_BRIGHTLIGHT 			4
#define	EF_DIMLIGHT 			8


/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vec3_t		position;
} mvertex_t;

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2

// plane_t structure
// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct mplane_s
{
	vec3_t	normal;
	float	dist;
	byte	type;			// for texture axis selection and fast side tests
	byte	signbits;		// signx + signy<<1 + signz<<1
	byte	pad[2];
} mplane_t;

extern	mplane_t frustum[4];

typedef struct texture_s
{
	char		name[16];
	unsigned	width, height;
#ifdef GLQUAKE_RENDERER_SUPPORT
	struct gltexture_s	*gltexture; //johnfitz -- pointer to gltexture
	struct gltexture_s	*fullbright; //johnfitz -- fullbright mask texture
	struct gltexture_s	*warpimage; //johnfitz -- for water animation
	cbool			update_warp; //johnfitz -- update warp this frame
	struct msurface_s	*texturechain;	// for texture chains
#endif // GLQUAKE_RENDERER_SUPPORT
	int			anim_total;				// total tenths in sequence ( 0 = no)
	int			anim_min, anim_max;		// time for this frame min <=time< max
	struct texture_s *anim_next;		// in the animation sequence
	struct texture_s *alternate_anims;	// bmodels in frmae 1 use these
#ifdef WINQUAKE_RENDERER_SUPPORT
	unsigned	offsets[MIPLEVELS];		// four mip maps stored
#else // .. GLQuake doesn't use the mipmaps
	unsigned			offset0;
#endif // !WINQUAKE_RENDERER_SUPPORT
} texture_t;


#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40
#define SURF_UNDERWATER		0x80
#define SURF_NOTEXTURE		0x100 //johnfitz
#define SURF_DRAWFENCE		0x200 // Not implemented in software (yet)
#define SURF_DRAWLAVA		0x400
#define SURF_DRAWSLIME		0x800
#define SURF_DRAWTELE		0x1000
#define SURF_DRAWWATER		0x2000
#define SURF_DRAWENVMAP     0x4000  // Not implemented in software
#define SURF_SCROLLX		0x8000  // Not implemented in software
#define SURF_SCROLLY		0x10000 // Not implemented in software
#define SURF_DRAWMIRROR		0x20000 // Not implemented in software (GL but not D3D yet)
#define SURF_SELECTED		0x40000 // Baker: Texture pointer

#define SURF_WINQUAKE_DRAWTRANSLUCENT (SURF_DRAWLAVA | SURF_DRAWSLIME | SURF_DRAWWATER)

#ifdef WINQUAKE_SOFTWARE_SKYBOX
#define SURF_DRAWSKYBOX		0x80000	// Manoel Kasimier - skyboxes // Code taken from the ToChriS engine - Author: Vic (vic@quakesrc.org) (http://hkitchen.quakesrc.org/)
#endif


// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	unsigned int	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct
{
	float		vecs[2][4];
#ifdef WINQUAKE_RENDERER_SUPPORT
	float		mipadjust; // Baker: GLQuake doesn't use
#endif // WINQUAKE_RENDERER_SUPPORT
	texture_t	*texture;
	int			flags;
} mtexinfo_t;

#ifdef GLQUAKE_RENDERER_SUPPORT
#define	VERTEXSIZE	7

typedef struct glpoly_s
{
	struct	glpoly_s	*next;
	struct	glpoly_s	*chain;
	int		numverts;
	float	verts[4][VERTEXSIZE];	// variable sized (xyz s1t1 s2t2)
} glpoly_t;
#endif // GLQUAKE_RENDERER_SUPPORT

typedef struct msurface_s
{
	int			visframe;		// should be drawn when node is crossed
	cbool	culled;			// johnfitz -- for frustum culling
	float		mins[3];		// johnfitz -- for frustum culling
	float		maxs[3];		// johnfitz -- for frustum culling

	mplane_t	*plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges

#ifdef WINQUAKE_RENDERER_SUPPORT
// surface generation data
	struct surfcache_s	*cachespots[MIPLEVELS];
#endif // WINQUAKE_RENDERER_SUPPORT

	short		texturemins[2];
	short		extents[2];

	int			smax;
	int			tmax;

	cbool	stained;
#ifdef GLQUAKE_RENDERER_SUPPORT
	int			stainnum; // WinQuake doesn't use
#endif // GLQUAKE_RENDERER_SUPPORT


	int			light_s, light_t;	//gl lightmap coordinates

#ifdef GLQUAKE_RENDERER_SUPPORT
	glpoly_t	*polys;				// multiple if warped
	struct	msurface_s	*texturechain;
#endif // GLQUAKE_RENDERER_SUPPORT
	mtexinfo_t	*texinfo;

// lighting info
	int			dlightframe;
	int			dlightbits[(MAX_FITZQUAKE_DLIGHTS + 31) >> 5];	// mh - 128 dynamic lights

	int			lightmaptexturenum;
	byte		styles[MAXLIGHTMAPS];
#ifdef GLQUAKE_RENDERER_SUPPORT
	int			cached_light[MAXLIGHTMAPS];	// values currently used in lightmap
	cbool	cached_dlight;				// true if dynamic light in cache
#endif // GLQUAKE_RENDERER_SUPPORT
	byte		*samples;		// [numstyles*surfsize]
} msurface_t;

typedef struct mnode_s
{
// common with leaf
	int			contents;		// 0, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current

	float		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// node specific
	mplane_t	*plane;
	struct mnode_s	*children[2];
#ifdef SUPPORTS_BSP2_IMPROVEMENTS
	unsigned int		firstsurface;
	unsigned int		numsurfaces;
#else
	unsigned short		firstsurface;
	unsigned short		numsurfaces;
#endif // !SUPPORTS_BSP2_IMPROVEMENTS
} mnode_t;



typedef struct mleaf_s
{
// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	float		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	byte		*compressed_vis;
	efrag_t		*efrags;

	msurface_t	**firstmarksurface;
	int			nummarksurfaces;
	int			key;			// BSP sequence number for leaf's contents
	byte		ambient_sound_level[NUM_AMBIENTS];
} mleaf_t;

//johnfitz -- for clipnodes>32k
typedef struct mclipnode_s
{
	int			planenum;
	int			children[2]; // negative numbers are contents
} mclipnode_t;
//johnfitz

// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct
{
	mclipnode_t	*clipnodes; //johnfitz -- was dclipnode_t
	mplane_t	*planes;
	int			firstclipnode;
	int			lastclipnode;
	vec3_t		clip_mins;
	vec3_t		clip_maxs;
	int			available;
} hull_t;

/*
==============================================================================

SPRITE MODELS

==============================================================================
*/


// FIXME: shorten these?
typedef struct mspriteframe_s
{
	int		width, height;
//	void	*pcachespot;			// remove?
	float	up, down, left, right;
#ifdef GLQUAKE_RENDERER_SUPPORT
	float				smax, tmax; //johnfitz -- image might be padded
	struct gltexture_s	*gltexture;
#endif // GLQUAKE_RENDERER_SUPPORT
#ifdef WINQUAKE_RENDERER_SUPPORT
	byte	pixels[4];
#endif // WINQUAKE_RENDERER_SUPPORT
} mspriteframe_t;

typedef struct
{
	int				numframes;
	float			*intervals;
	mspriteframe_t	*frames[1];
} mspritegroup_t;

typedef struct
{
	spriteframetype_t	type;
	mspriteframe_t		*frameptr;
} mspriteframedesc_t;

typedef enum
{
	sprite_render_normal,
	sprite_render_additive,
	sprite_render_filter,
} sprite_rend_type_t;

typedef struct
{
	int					type;
	int					maxwidth;
	int					maxheight;
	int					numframes;
	float				beamlength;		// remove?
//	void				*cachespot;		// remove?
	int					version;
	sprite_rend_type_t	sprite_render_type;
	mspriteframedesc_t	frames[1];
} msprite_t;


/*
==============================================================================

ALIAS MODELS

Alias models are position independent, so the cache manager can move them.
==============================================================================
*/

// normalizing factor so player model works out to about
//  1 pixel per triangle
#define ALIAS_BASE_SIZE_RATIO		(1.0 / 11.0)
#define MAX_LBM_HEIGHT	480

typedef struct
{
#ifdef WINQUAKE_RENDERER_SUPPORT
	aliasframetype_t	type;
#endif // WINQUAKE_RENDERER_SUPPORT
	int					firstpose;
	int					numposes;
	float				interval;
	trivertx_t			bboxmin;
	trivertx_t			bboxmax;
	int					frame;
	char				name[16];
} maliasframedesc_t;

#ifdef WINQUAKE_RENDERER_SUPPORT
typedef struct
{
	aliasskintype_t		type;
	void				*pcachespot;
	int					skin;
} maliasskindesc_t;
#endif // WINQUAKE_RENDERER_SUPPORT

typedef struct
{
	trivertx_t			bboxmin;
	trivertx_t			bboxmax;
	int					frame;
} maliasgroupframedesc_t;

typedef struct
{
	int						numframes;
	int						intervals;
	maliasgroupframedesc_t	frames[1];
} maliasgroup_t;

#ifdef WINQUAKE_RENDERER_SUPPORT
typedef struct
{
	int					numskins;
	int					intervals;
	maliasskindesc_t	skindescs[1];
} maliasskingroup_t;
#endif // WINQUAKE_RENDERER_SUPPORT

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct mtriangle_s {
	int					facesfront;
	int					vertindex[3];
} mtriangle_t;

#ifdef GLQUAKE_RENDERER_SUPPORT
#define	MAX_SKINS	32
#endif // GLQUAKE_RENDERER_SUPPORT
typedef struct
{
	int			ident;
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;

	int					numposes;
#ifdef GLQUAKE_RENDERER_SUPPORT
	int					poseverts;
	int					posedata;	// numposes*poseverts trivert_t
	int					commands;	// gl command list with embedded s/t
#ifdef DIRECT3D_WRAPPER
	int					commands_d3d_no_external_skins;	// gl command list with embedded s/t
#endif // DIRECT3D_WRAPPER
	struct gltexture_s	*gltextures[MAX_SKINS][4]; //johnfitz
	struct gltexture_s	*fbtextures[MAX_SKINS][4]; //johnfitz
	int					texels[MAX_SKINS];	// only for player skins
#endif // GLQUAKE_RENDERER_SUPPORT

#ifdef WINQUAKE_RENDERER_SUPPORT
	int					model;
	int					stverts;
	int					skindesc;
	int					triangles;
#endif // WINQUAKE_RENDERER_SUPPORT
	maliasframedesc_t	frames[1]; // variable sized
} aliashdr_t;

#define	MAXALIASVERTS	2000 //johnfitz -- was 1024
#define	MAXALIASFRAMES	256
#ifdef GLQUAKE_RENDERER_SUPPORT
#define	MAXALIASTRIS	2048
extern	aliashdr_t	*pheader;
extern	stvert_t	stverts[MAXALIASVERTS];
extern	mtriangle_t	triangles[MAXALIASTRIS];
extern	trivertx_t	*poseverts[MAXALIASFRAMES];
#endif // GLQUAKE_RENDERER_SUPPORT

//===================================================================

//
// Whole model
//

typedef enum {mod_brush, mod_sprite, mod_alias} modtype_t;

#define	EF_ROCKET	1			// leave a trail
#define	EF_GRENADE	2			// leave a trail
#define	EF_GIB		4			// leave a trail
#define	EF_ROTATE	8			// rotate (bonus items)
#define	EF_TRACER	16			// green split trail
#define	EF_ZOMGIB	32			// small blood trail
#define	EF_TRACER2	64			// orange split trail + rotate
#define	EF_TRACER3	128			// purple trail

#define EF_ALPHA_MASKED_MDL 16384	// Alpha masked model texture, only for .mdl models.

//johnfitz -- extra flags for rendering
#define	MOD_NOLERP		256		// don't lerp when animating
#define	MOD_NOSHADOW	512		// don't cast a shadow
#define	MOD_FBRIGHTHACK	1024	// when fullbrights are disabled, use a hack to render this model brighter
#define MOD_NOCOLORMAP	2048	// don't bother colormapping certain entities (wasteful) WinQuake does not use!

//#define MOD_PLAYER		4096	// player  (technically isn't model index 1 always player in Quake and required by the demo format to be so?)
#define MOD_EYES			8192	// eyes    (technically isn't model index 2 always eyes in Quake and required by the demo format to be so?)
#define MOD_GIBS			32768	// gibs, heads, bubbles
//johnfitz

typedef struct
{
	char	searchpath[MAX_OSPATH];
} loadinfo_t;

typedef struct qmodel_s
{
	char		name[MAX_QPATH];
	cbool	needload;		// bmodels and sprites don't cache normally

	modtype_t	type;
	int			numframes;
	synctype_t	synctype;

	int			flags;

//
// volume occupied by the model graphics
//
	vec3_t		mins, maxs;
	vec3_t		ymins, ymaxs; //johnfitz -- bounds for entities with nonzero yaw
	vec3_t		rmins, rmaxs; //johnfitz -- bounds for entities with nonzero pitch or roll
	//johnfitz -- removed float radius;

//
// solid volume for clipping
//
	cbool	clipbox;
	vec3_t		clipmins, clipmaxs;
#ifdef WINQUAKE_RENDERER_SUPPORT
	float		radius; // Baker: Still needed.  For now ...
#endif // WINQUAKE_RENDERER_SUPPORT

//
// brush model
//
	int			firstmodelsurface, nummodelsurfaces;

	int			numsubmodels;
	dmodel_t	*submodels;

	int			numplanes;
	mplane_t	*planes;

	int			numleafs;		// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numvertexes;
	mvertex_t	*vertexes;

	int			numedges;
	medge_t		*edges;

	int			numnodes;
	mnode_t		*nodes;

	int			numtexinfo;
	mtexinfo_t	*texinfo;

	int			numsurfaces;
	msurface_t	*surfaces;

	int			numsurfedges;
	int			*surfedges;

	int			numclipnodes;
	mclipnode_t	*clipnodes; //johnfitz -- was dclipnode_t

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	hull_t		hulls[MAX_MAP_HULLS];

	int			numtextures;
	texture_t	**textures;

	byte		*visdata;
	byte		*lightdata;
	char		*entities;

	cbool	isworldmodel;
	int			bspversion;

	loadinfo_t	loadinfo;

//
// additional model data
//

// !!!!! DO NOT place any struct members after this as the extra data may immediately follow it in memory !!!!!
	cache_user_t	cache;		// only access through Mod_Extradata

} qmodel_t;

//============================================================================

void	Mod_Init (void);
void	Mod_ClearAll (void);
void	Mod_ClearAll_Compact (void); // Baker: Used when gamedir changes to prevent use of corrupt data from same named (but different) models.
qmodel_t *Mod_ForName (const char *name, cbool crash);
void	*Mod_Extradata (qmodel_t *mod);	// handles caching
void	Mod_TouchModel (const char *name);

mleaf_t *Mod_PointInLeaf (float *p, qmodel_t *model);
byte	*Mod_LeafPVS (mleaf_t *leaf, qmodel_t *model);

#ifdef GLQUAKE_RENDERER_SUPPORT
void Mod_SetExtraFlags (qmodel_t *mod);
void Mod_FloodFillSkin( byte *skin, int skinwidth, int skinheight );
#endif // GLQUAKE_RENDERER_SUPPORT

//cmd void Mod_Print (void);
//cmd void Mod_PrintEx (void);

typedef struct
{
// Baker: Server doesn't bother to read these.  WinQuake client doesn't bother to read fog key.
	char		sky_key[MAX_QPATH];
	char		fog_key[MAX_QPATH];

// The following are definitely known by client/server immediately because they occur in model load.
	cbool	water;
	cbool	lava;
	cbool	slime;
	cbool	teleporter;
	cbool	sky;
//	cbool	mirror; // Server needs to know to send better model visibility
//	cbool	camera; // Server needs to know to send better model visibility
//	cbool	camera_screen; // Server needs to know to send better model visibility

// The following are not immediately known.  Although could probably have loader cross these off if no liquid textures.
	cbool	ever_been_away_from_water_portal;
	cbool	water_vis_known;
	cbool	water_vis;
//	cbool	sky_entities;
} level_info_t;

extern level_info_t level;

void Sky_LoadTexture (texture_t *mt);
void R_MarkLights (dlight_t *light, int num, mnode_t *node);

void Mod_SetExtraFlags (qmodel_t *mod);
void Mod_Flags_Refresh_f (cvar_t *var);

texture_t *R_TextureAnimation (texture_t *base, int frame); // Baker: Had to put this here due to texture_t

//johnfitz -- struct for passing lerp information to drawing functions
typedef struct {
	short pose1;
	short pose2;
	float blend;
	vec3_t origin;
	vec3_t angles;
} lerpdata_t;
//johnfitz

void R_SetupAliasFrame (aliashdr_t *paliashdr, int frame, lerpdata_t *lerpdata);
void R_SetupEntityTransform (entity_t *e, lerpdata_t *lerpdata);



#endif	// __MODEL__
