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
// r_part.c

#include "quakedef.h"

#define DEFAULT_NUM_PARTICLES			2048	// default max # of particles at one
										//  time
#define ABSOLUTE_MIN_PARTICLES	512		// no fewer than this no matter what's
										//  on the command line

static int		ramp1[8] = {0x6f, 0x6d, 0x6b, 0x69, 0x67, 0x65, 0x63, 0x61};
static int		ramp2[8] = {0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x68, 0x66};
static int		ramp3[8] = {0x6d, 0x6b, 6, 5, 4, 3};

particle_t	*active_particles, *free_particles, *particles;

vec3_t r_pright /* qasm */, r_pup /* qasm */, r_ppn /* qasm */;

int			r_numparticles;

#ifdef GLQUAKE_DRAW_PARTICLES
gltexture_t *particletexture, *particletexture1, *particletexture2, *particletexture3, *particletexture4; //johnfitz
float texturescalefactor; //johnfitz -- compensate for apparent size of different particle textures


/*
===============
R_ParticleTextureLookup -- johnfitz -- generate nice antialiased 32x32 circle for particles
===============
*/
int R_ParticleTextureLookup (int x, int y, int sharpness)
{
	int r; //distance from point x,y to circle origin, squared
	int a; //alpha value to return

	x -= 16;
	y -= 16;
	r = x * x + y * y;
	r = r > 255 ? 255 : r;
	a = sharpness * (255 - r);
	a = c_min(a,255);
	return a;
}

/*
===============
R_InitParticleTextures -- johnfitz -- rewritten
===============
*/
void R_InitParticleTextures (void)
{
	int			x,y;
	static byte	particle1_data[64*64*4];
	static byte	particle2_data[2*2*4];
	static byte	particle3_data[64*64*4];
	byte		*dst;

	// particle texture 1 -- circle
	dst = particle1_data;
	for (x=0 ; x<64 ; x++)
		for (y=0 ; y<64 ; y++)
		{
			*dst++ = 255;
			*dst++ = 255;
			*dst++ = 255;
			*dst++ = R_ParticleTextureLookup(x, y, 8);
		}
	particletexture1 = TexMgr_LoadImage (NULL, -1, "particle1", 64, 64, SRC_RGBA, particle1_data, "", (src_offset_t)particle1_data, TEXPREF_PERSIST | TEXPREF_ALPHA | TEXPREF_LINEAR);

	// particle texture 2 -- square
	dst = particle2_data;
	for (x=0 ; x<2 ; x++)
		for (y=0 ; y<2 ; y++)
		{
			*dst++ = 255;
			*dst++ = 255;
			*dst++ = 255;
			*dst++ = x || y ? 0 : 255;
		}
	particletexture2 = TexMgr_LoadImage (NULL, -1, "particle2", 2, 2, SRC_RGBA, particle2_data, "", (src_offset_t)particle2_data, TEXPREF_PERSIST | TEXPREF_ALPHA | TEXPREF_NEAREST);

	// particle texture 3 -- blob
	dst = particle3_data;
	for (x=0 ; x<64 ; x++)
		for (y=0 ; y<64 ; y++)
		{
			*dst++ = 255;
			*dst++ = 255;
			*dst++ = 255;
			*dst++ = R_ParticleTextureLookup(x, y, 2);
		}
	particletexture3 = TexMgr_LoadImage (NULL, -1, "particle3", 64, 64, SRC_RGBA, particle3_data, "", (src_offset_t)particle3_data, TEXPREF_PERSIST | TEXPREF_ALPHA | TEXPREF_LINEAR);


	//set default
	particletexture = particletexture1;
	texturescalefactor = 1.27;
}

#endif // GLQUAKE_DRAW_PARTICLES

/*
===============
R_SetParticleTexture_f -- johnfitz
===============
*/
void R_SetParticleTexture_f (cvar_t *var)
{
#ifdef GLQUAKE_DRAW_PARTICLES
	switch ((int)(gl_particles.value))
	{
	case 1:
		particletexture = particletexture1;
		texturescalefactor = 1.27;
		break;
	case 2:
		particletexture = particletexture2;
		texturescalefactor = 1.0;
		break;
//	case 3:
//		particletexture = particletexture3;
//		texturescalefactor = 1.5;
//		break;
	}
#endif // GLQUAKE_DRAW_PARTICLES
}



/*
===============
R_InitParticles
===============
*/
void R_InitParticles (void)
{
	int		i;

	i = COM_CheckParm ("-particles");

	if (i  && i+1 < com_argc)
	{
		r_numparticles = (int)(atoi(com_argv[i+1]));
		if (r_numparticles < ABSOLUTE_MIN_PARTICLES)
			r_numparticles = ABSOLUTE_MIN_PARTICLES;
	}
	else
	{
		r_numparticles = DEFAULT_NUM_PARTICLES;
	}

	particles = (particle_t *)
			Hunk_AllocName (r_numparticles * sizeof(particle_t), "particles");

#ifdef GLQUAKE_DRAW_PARTICLES
	R_InitParticleTextures (); //johnfitz
#endif // GLQUAKE_DRAW_PARTICLES
}

/*
===============
R_EntityParticles
===============
*/

#define NUMVERTEXNORMALS	162
extern	float	r_avertexnormals[NUMVERTEXNORMALS][3];
vec3_t	avelocities[NUMVERTEXNORMALS];
float	beamlength = 16;

void R_EntityParticles (entity_t *ent)
{
	int			i;
	particle_t	*p;
	float		angle;
	float		sp, sy, cp, cy;
//	float		sr, cr;
//	int		count;
	vec3_t		forward;
	float		dist;

	dist = 64;
//	count = 50;

	if (!avelocities[0][0])
	{
		for (i = 0; i < NUMVERTEXNORMALS; i++)
		{
			avelocities[i][0] = (rand() & 255) * 0.01;
			avelocities[i][1] = (rand() & 255) * 0.01;
			avelocities[i][2] = (rand() & 255) * 0.01;
		}
	}

	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		angle = cl.time * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = cl.time * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
// Baker: These results aren't actually used
	//	angle = cl.time * avelocities[i][2];
	//	sr = sin(angle);
	//	cr = cos(angle);

		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 0.01;
		p->color = 0x6f;
		p->type = pt_explode;

		p->org[0] = ent->origin[0] + r_avertexnormals[i][0]*dist + forward[0]*beamlength;
		p->org[1] = ent->origin[1] + r_avertexnormals[i][1]*dist + forward[1]*beamlength;
		p->org[2] = ent->origin[2] + r_avertexnormals[i][2]*dist + forward[2]*beamlength;
	}
}

/*
===============
R_ClearParticles
===============
*/
void R_ClearParticles (void)
{
	int		i;

	free_particles = &particles[0];
	active_particles = NULL;

	for (i=0 ;i<r_numparticles ; i++)
		particles[i].next = &particles[i+1];
	particles[r_numparticles-1].next = NULL;
}

/*
===============
R_ReadPointFile_f
===============
*/
void R_ReadPointFile_f (lparse_t *unused)
{
	FILE	*f;
	vec3_t	org;
	int		r;
	int		c;
	particle_t	*p;
	char	name[MAX_QPATH];

	if (cls.state != ca_connected)
		return;			// need an active map.

	c_snprintf (name, "maps/%s.pts", cl.worldname);

	COM_FOpenFile (name, &f);
	if (!f)
	{
		Con_Printf ("couldn't open %s\n", name);
		return;
	}

	Con_Printf ("Reading %s...\n", name);
	c = 0;
	for ( ;; )
	{
		r = fscanf (f,"%f %f %f\n", &org[0], &org[1], &org[2]);
		if (r != 3)
			break;
		c++;

		if (!free_particles)
		{
			Con_Printf ("Not enough free particles\n");
			break;
		}
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = 99999;
		p->color = (-c)&15;
		p->type = pt_static;
		VectorCopy (vec3_origin, p->vel);
		VectorCopy (org, p->org);
	}

	FS_fclose (f);
	Con_Printf ("%i points read\n", c);
}

/*
===============
R_ParseParticleEffect

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect (void)
{
	vec3_t		org, dir;
	int			i, count, msgcount, color;

	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	for (i=0 ; i<3 ; i++)
		dir[i] = MSG_ReadChar () * (1.0/16);
	msgcount = MSG_ReadByte ();
	color = MSG_ReadByte ();

if (msgcount == 255)
	count = 1024;
else
	count = msgcount;

	R_RunParticleEffect (org, dir, color, count);
}

/*
===============
R_ParticleExplosion

===============
*/
void R_ParticleExplosion (vec3_t org)
{
	int			i, j;
	particle_t	*p;

	for (i=0 ; i<1024 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 5;
		p->color = ramp1[0];
		p->ramp = rand()&3;
		if (i & 1)
		{
			p->type = pt_explode;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
		else
		{
			p->type = pt_explode2;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
	}
}

/*
===============
R_ParticleExplosion2

===============
*/
void R_ParticleExplosion2 (vec3_t org, int colorStart, int colorLength)
{
	int			i, j;
	particle_t	*p;
	int			colorMod = 0;

	for (i=0; i<512; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 0.3;
		p->color = colorStart + (colorMod % colorLength);
		colorMod++;

		p->type = pt_blob;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand()%32)-16);
			p->vel[j] = (rand()%512)-256;
		}
	}
}

/*
===============
R_BlobExplosion

===============
*/
void R_BlobExplosion (vec3_t org)
{
	int			i, j;
	particle_t	*p;

	for (i=0 ; i<1024 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 1 + (rand()&8)*0.05;

		if (i & 1)
		{
			p->type = pt_blob;
			p->color = 66 + rand()%6;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
		else
		{
			p->type = pt_blob2;
			p->color = 150 + rand()%6;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
	}
}

/*
===============
R_RunParticleEffect

===============
*/
void R_RunParticleEffect (vec3_t org, const vec3_t dir, int color, int count)
{
	int			i, j;
	particle_t	*p;

	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		if (count == 1024)
		{
			// rocket explosion
			p->die = cl.time + 5;
			p->color = ramp1[0];
			p->ramp = rand()&3;
			if (i & 1)
			{
				p->type = pt_explode;
				for (j=0 ; j<3 ; j++)
				{
					p->org[j] = org[j] + ((rand()%32)-16);
					p->vel[j] = (rand()%512)-256;
				}
			}
			else
			{
				p->type = pt_explode2;
				for (j=0 ; j<3 ; j++)
				{
					p->org[j] = org[j] + ((rand()%32)-16);
					p->vel[j] = (rand()%512)-256;
				}
			}
		}
		else
		{
			p->die = cl.time + 0.1*(rand()%5);
			p->color = (color&~7) + (rand()&7);
			p->type = pt_slowgrav;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()&15)-8);
				p->vel[j] = dir[j]*15;// + (rand()%300)-150;
			}
		}
	}
}


/*
===============
R_LavaSplash

===============
*/
void R_LavaSplash (vec3_t org)
{
	int			i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i=-16 ; i<16 ; i++)
	{
		for (j=-16 ; j<16 ; j++)
		{
			for (k=0 ; k<1 ; k++)
			{
				if (!free_particles)
					return;
				p = free_particles;
				free_particles = p->next;
				p->next = active_particles;
				active_particles = p;

				p->die = cl.time + 2 + (rand()&31) * 0.02;
				p->color = 224 + (rand()&7);
				p->type = pt_slowgrav;

				dir[0] = j*8 + (rand()&7);
				dir[1] = i*8 + (rand()&7);
				dir[2] = 256;

				p->org[0] = org[0] + dir[0];
				p->org[1] = org[1] + dir[1];
				p->org[2] = org[2] + (rand()&63);

				VectorNormalize (dir);
				vel = 50 + (rand()&63);
				VectorScale (dir, vel, p->vel);
			}
}
	}
}

/*
===============
R_TeleportSplash

===============
*/
void R_TeleportSplash (vec3_t org)
{
	int			i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i=-16 ; i<16 ; i+=4)
	{
		for (j=-16 ; j<16 ; j+=4)
		{
			for (k=-24 ; k<32 ; k+=4)
			{
				if (!free_particles)
					return;
				p = free_particles;
				free_particles = p->next;
				p->next = active_particles;
				active_particles = p;

				p->die = cl.time + 0.2 + (rand()&7) * 0.02;
				p->color = 7 + (rand()&7);
				p->type = pt_slowgrav;

				dir[0] = j*8;
				dir[1] = i*8;
				dir[2] = k*8;

				p->org[0] = org[0] + i + (rand()&3);
				p->org[1] = org[1] + j + (rand()&3);
				p->org[2] = org[2] + k + (rand()&3);

				VectorNormalize (dir);
				vel = 50 + (rand()&63);
				VectorScale (dir, vel, p->vel);
			}
}
	}
}

/*
===============
R_RocketTrail

FIXME -- rename function and use #defined types instead of numbers
===============
*/
void R_RocketTrail (vec3_t start, vec3_t end, vec3_t *trail_origin, trail_type_t type)
{
	vec3_t		vec;
	float		len;
	int			j;
	particle_t	*p;
	int			dec;
	static int	tracercount;

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	if (type < 128)
		dec = 3;
	else
	{
		dec = 1;
		type -= 128;
	}

	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		VectorCopy (vec3_origin, p->vel);
		p->die = cl.time + 2;

		switch (type)
		{
			case ROCKET_TRAIL_0: // 0 rocket trail
				p->ramp = (rand()&3);
				p->color = ramp3[(int)p->ramp];
				p->type = pt_fire;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				break;

			case GRENADE_TRAIL_1: // 1 smoke smoke
				p->ramp = (rand()&3) + 2;
				p->color = ramp3[(int)p->ramp];
				p->type = pt_fire;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				break;

			case BLOOD_TRAIL_2: // 2 blood
				p->type = pt_grav;
				p->color = 67 + (rand()&3);
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				break;

			case TRACER1_TRAIL_3: // 3
			case TRACER2_TRAIL_5: // 5 tracer
				p->die = cl.time + 0.5;
				p->type = pt_static;
				if (type == TRACER1_TRAIL_3) // 3 is tracer1_trail
					p->color = 52 + ((tracercount&4)<<1);
				else
					p->color = 230 + ((tracercount&4)<<1);

				tracercount++;

				VectorCopy (start, p->org);
				if (tracercount & 1)
				{
					p->vel[0] = 30*vec[1];
					p->vel[1] = 30*-vec[0];
				}
				else
				{
					p->vel[0] = 30*-vec[1];
					p->vel[1] = 30*vec[0];
				}
				break;

			case SLIGHT_BLOOD_TRAIL_4: // 4 slight blood
				p->type = pt_grav;
				p->color = 67 + (rand()&3);
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()%6)-3);
				len -= 3;
				break;

			case VOOR_TRAIL_6: // 6 voor trail
				p->color = 9*16 + 8 + (rand()&3);
				p->type = pt_static;
				p->die = cl.time + 0.3;
				for (j=0 ; j<3 ; j++)
					p->org[j] = start[j] + ((rand()&15)-8);
				break;
		}


		VectorAdd (start, vec, start);
	}
}


/*
===============
CL_RunParticles -- johnfitz -- all the particle behavior, separated from R_DrawParticles
WinQuake: R_DrawParticles (it draws them here too)
===============
*/

void CL_RunParticles (void)
{
	particle_t		*p, *kill;
	float			grav;
	int				i;
	float 			time2, time3;
	float			time1;
	float			dvel;
	float			frametime;

	frametime = fabs(cl.time - cl.oldtime);
	time3 = frametime * 15;
	time2 = frametime * 10; // 15;
	time1 = frametime * 5;
	grav = frametime * sv_gravity.value * 0.05;
	dvel = 4*frametime;

	for ( ;; )
	{
		kill = active_particles;
		if (kill && kill->die < cl.time)
		{
			active_particles = kill->next;
			kill->next = free_particles;
			free_particles = kill;
			continue;
		}
		break;
	}

	for (p=active_particles ; p ; p=p->next)
	{
		for ( ;; )
		{
			kill = p->next;
			if (kill && kill->die < cl.time)
			{
				p->next = kill->next;
				kill->next = free_particles;
				free_particles = kill;
				continue;
			}
			break;
		}

		p->org[0] += p->vel[0]*frametime;
		p->org[1] += p->vel[1]*frametime;
		p->org[2] += p->vel[2]*frametime;

		switch (p->type)
		{
		case pt_static:
			break;
		case pt_fire:
			p->ramp += time1;
			if (p->ramp >= 6)
				p->die = -1;
			else
				p->color = ramp3[(int)p->ramp];
			p->vel[2] += grav;
			break;

		case pt_explode:
			p->ramp += time2;
			if (p->ramp >=8)
				p->die = -1;
			else
				p->color = ramp1[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_explode2:
			p->ramp += time3;
			if (p->ramp >=8)
				p->die = -1;
			else
				p->color = ramp2[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] -= p->vel[i]*frametime;
			p->vel[2] -= grav;
			break;

		case pt_blob:
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_blob2:
			for (i=0 ; i<2 ; i++)
				p->vel[i] -= p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_grav:
		case pt_slowgrav:
			p->vel[2] -= grav;
			break;
		}
	}

}

#ifdef WINQUAKE_DRAW_PARTICLES

/*
===============
R_DrawParticles
===============
*/
void R_DrawParticles (void)
{
	particle_t		*p;

	VectorScale (vright, xscaleshrink, r_pright);
	VectorScale (vup, yscaleshrink, r_pup);
	VectorCopy (vpn, r_ppn);

	for (p = active_particles ; p ; p = p->next)
		D_DrawParticle (p);
}
#endif // WINQUAKE_DRAW_PARTICLES

#ifdef GLQUAKE_DRAW_PARTICLES
/*
===============
R_DrawParticles
===============
*/
void R_DrawParticles (void)
{
	particle_t		*p;
	float			scale;
	vec3_t			up, right, p_up, p_right, p_upright; //johnfitz -- p_ vectors
	byte			color[4], *c; //johnfitz -- particle transparency
//	float			alpha; //johnfitz -- particle transparency

	if (!gl_particles.value)
		return;

	VectorScale (vup, 1.5, up);
	VectorScale (vright, 1.5, right);

    GL_Bind(particletexture);
	eglEnable (GL_BLEND);
	eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	eglDepthMask (GL_FALSE); //johnfitz -- fix for particle z-buffer bug

	if (gl_quadparticles.value) //johnitz -- quads save fillrate
	{
		eglBegin (GL_QUADS);
		for (p=active_particles ; p ; p=p->next)
		{
			// hack a scale up to keep particles from disapearing
			scale = (p->org[0] - r_origin[0]) * vpn[0]
				  + (p->org[1] - r_origin[1]) * vpn[1]
				  + (p->org[2] - r_origin[2]) * vpn[2];
			if (scale < 20)
				scale = 1 + 0.08; //johnfitz -- added .08 to be consistent
			else
				scale = 1 + scale * 0.004;

			scale /= 2.0; //quad is half the size of triangle

			scale *= texturescalefactor; //johnfitz -- compensate for apparent size of different particle textures

			//johnfitz -- particle transparency and fade out
			c = (byte *) &vid.d_8to24table[(int)p->color];
			color[0] = c[0];
			color[1] = c[1];
			color[2] = c[2];
			//alpha = CLAMP(0, p->die + 0.5 - cl.time, 1);
			color[3] = 255; //(int)(alpha * 255);
			eglColor4ubv(color);
			//johnfitz

			eglTexCoord2f (0,0);
			eglVertex3fv (p->org);

			eglTexCoord2f (0.5,0);
			VectorMA (p->org, scale, up, p_up);
			eglVertex3fv (p_up);

			eglTexCoord2f (0.5,0.5);
			VectorMA (p_up, scale, right, p_upright);
			eglVertex3fv (p_upright);

			eglTexCoord2f (0,0.5);
			VectorMA (p->org, scale, right, p_right);
			eglVertex3fv (p_right);

			rs_particles++; //johnfitz //FIXME: just use r_numparticles
		}
		eglEnd ();
	}
	else //johnitz --  triangles save verts
	{
		eglBegin (GL_TRIANGLES);
		for (p=active_particles ; p ; p=p->next)
		{
			// hack a scale up to keep particles from disapearing
			scale = (p->org[0] - r_origin[0]) * vpn[0]
				  + (p->org[1] - r_origin[1]) * vpn[1]
				  + (p->org[2] - r_origin[2]) * vpn[2];
			if (scale < 20)
				scale = 1 + 0.08; //johnfitz -- added .08 to be consistent
			else
				scale = 1 + scale * 0.004;

			scale *= texturescalefactor; //johnfitz -- compensate for apparent size of different particle textures

			//johnfitz -- particle transparency and fade out
			c = (GLubyte *) &vid.d_8to24table[(int)p->color];
			color[0] = c[0];
			color[1] = c[1];
			color[2] = c[2];
			//alpha = CLAMP(0, p->die + 0.5 - cl.time, 1);
			color[3] = 255; //(int)(alpha * 255);
			eglColor4ubv(color);
			//johnfitz

			eglTexCoord2f (0,0);
			eglVertex3fv (p->org);

			eglTexCoord2f (1,0);
			VectorMA (p->org, scale, up, p_up);
			eglVertex3fv (p_up);

			eglTexCoord2f (0,1);
			VectorMA (p->org, scale, right, p_right);
			eglVertex3fv (p_right);

			rs_particles++; //johnfitz //FIXME: just use r_numparticles
		}
		eglEnd ();
	}

	eglDepthMask (GL_TRUE); //johnfitz -- fix for particle z-buffer bug
	eglDisable (GL_BLEND);
	eglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	eglColor3f(1,1,1);
}


/*
===============
R_DrawParticles_ShowTris -- johnfitz
===============
*/
void R_DrawParticles_ShowTris (void)
{
	particle_t		*p;
	float			scale;
	vec3_t			up, right, p_up, p_right, p_upright;

	if (!gl_particles.value)
		return;

	VectorScale (vup, 1.5, up);
	VectorScale (vright, 1.5, right);

	if (gl_quadparticles.value)
	{
		for (p=active_particles ; p ; p=p->next)
		{
			eglBegin (GL_TRIANGLE_FAN);

			// hack a scale up to keep particles from disapearing
			scale = (p->org[0] - r_origin[0]) * vpn[0]
				  + (p->org[1] - r_origin[1]) * vpn[1]
				  + (p->org[2] - r_origin[2]) * vpn[2];
			if (scale < 20)
				scale = 1 + 0.08; //johnfitz -- added .08 to be consistent
			else
				scale = 1 + scale * 0.004;

			scale /= 2.0; //quad is half the size of triangle

			scale *= texturescalefactor; //compensate for apparent size of different particle textures

			eglVertex3fv (p->org);

			VectorMA (p->org, scale, up, p_up);
			eglVertex3fv (p_up);

			VectorMA (p_up, scale, right, p_upright);
			eglVertex3fv (p_upright);

			VectorMA (p->org, scale, right, p_right);
			eglVertex3fv (p_right);

			eglEnd ();
		}
	}
	else
	{
		eglBegin (GL_TRIANGLES);
		for (p=active_particles ; p ; p=p->next)
		{
			// hack a scale up to keep particles from disapearing
			scale = (p->org[0] - r_origin[0]) * vpn[0]
				  + (p->org[1] - r_origin[1]) * vpn[1]
				  + (p->org[2] - r_origin[2]) * vpn[2];
			if (scale < 20)
				scale = 1 + 0.08; //johnfitz -- added .08 to be consistent
			else
				scale = 1 + scale * 0.004;

			scale *= texturescalefactor; //compensate for apparent size of different particle textures

			eglVertex3fv (p->org);

			VectorMA (p->org, scale, up, p_up);
			eglVertex3fv (p_up);

			VectorMA (p->org, scale, right, p_right);
			eglVertex3fv (p_right);
		}
		eglEnd ();
	}
}

#endif // GLQUAKE_DRAW_PARTICLES

int Effects_Bit_Flag (int flags)
{
	if (flags & EF_GIB) return EF_GIB;
	if (flags & EF_ZOMGIB) return EF_ZOMGIB;
	if (flags & EF_TRACER) return EF_TRACER;
	if (flags & EF_TRACER2) return EF_TRACER2;
	if (flags & EF_TRACER3) return EF_TRACER3;
	if (flags & EF_ROCKET) return EF_ROCKET;
	if (flags & EF_GRENADE) return EF_GRENADE;
	return 0;
}



#define NEHSMOKE 987
void Effects_Evaluate (int i, entity_t* ent, vec3_t oldorg)
{
	int effects = Effects_Bit_Flag (ent->model->flags);

	if (!effects)
		return;

#ifdef SUPPORTS_NEHAHRA
	if (nehahra_active && effects == EF_GRENADE && cl.time >= ent->smokepuff_time)
		effects = NEHSMOKE;
#endif // SUPPORTS_NEHAHRA

//	if (!ent->traildrawn || !VectorL2Compare(ent->trail_origin, ent->origin, 140))
//	{
//		VectorCopy (ent->origin, oldorg);	//not present last frame or too far away
//		ent->traildrawn = true;
//	}
//	else VectorCopy (ent->trail_origin, oldorg);

	switch (effects)
	{
	default:			return;

	case EF_GIB:		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, BLOOD_TRAIL_2);			return;
	case EF_ZOMGIB:		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, SLIGHT_BLOOD_TRAIL_4);	return;
	case EF_TRACER:		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, TRACER1_TRAIL_3);		return;
	case EF_TRACER2:	R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, TRACER2_TRAIL_5);		return;
	case EF_TRACER3:	R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, VOOR_TRAIL_6);			return;
	case EF_GRENADE:	R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, GRENADE_TRAIL_1);		return;

	case EF_ROCKET:		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, ROCKET_TRAIL_0);
						DLight_Add (i, ent->origin, 200, 0, cl.time + 0.01, /*rgb: */ 1,1,1);
						return;

#ifdef SUPPORTS_NEHAHRA
	case NEHSMOKE:		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, NEHAHRA_SMOKE_9);
						ent->smokepuff_time = cl.time + 0.14;
						return;
#endif // SUPPORTS_NEHAHRA
	}

}

void DLight_Add (int keyx, vec3_t originx, float radiusx, float minlightx, double dietimex, float redx, float greenx, float bluex)
{
	dlight_t *dl = CL_AllocDlight (keyx);
	dl->origin[0]=originx[0];
	dl->origin[1]=originx[1];
	dl->origin[2]=originx[2];
	dl->radius = radiusx;
	dl->minlight = minlightx;
	dl->die = dietimex;
#ifdef GLQUAKE_COLORED_LIGHTS
	dl->color[0] = redx;
	dl->color[1] = greenx;
	dl->color[2] = bluex;
#endif // GLQUAKE_COLORED_LIGHTS
}


