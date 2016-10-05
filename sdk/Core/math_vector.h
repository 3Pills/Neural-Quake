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

#ifndef __MATH_VECTOR_H__
#define __MATH_VECTOR_H__

// mathlib.h
#include "environment.h"
#include <math.h>


enum q_pitch_e
{
	Q_PITCH = 0, // up / down
	Q_YAW   = 1, // left / right
	Q_ROLL  = 2, // fall over
};

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];



#define M_PI_DIV_180 (M_PI / 180.0)
#define Degree_To_Radians(a) (a * M_PI) / 180.0F





void PerpendicularVector( vec3_t dst, const vec3_t src );
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
#define DotProduct(x,y) (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}
#define VectorAdd(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
#define VectorCopy(a,b) {b[0]=a[0];b[1]=a[1];b[2]=a[2];}
#define VectorClear(a)		((a)[0] = (a)[1] = (a)[2] = 0)
#define VectorNegate(a, b)	((b)[0] = -(a)[0], (b)[1] = -(a)[1], (b)[2] = -(a)[2])

//johnfitz -- courtesy of lordhavoc
// QuakeSpasm: To avoid strict aliasing violations, use a float/int union instead of type punning.
#define VectorNormalizeFast(_v)\
{\
	union { float f; int i; } _y, _number;\
	_number.f = DotProduct(_v, _v);\
	if (_number.f != 0.0)\
	{\
		_y.i = 0x5f3759df - (_number.i >> 1);\
		_y.f = _y.f * (1.5f - (_number.f * 0.5f * _y.f * _y.f));\
		VectorScale(_v, _y.f, _v);\
	}\
}

void TurnVector (vec3_t out, const vec3_t forward, const vec3_t side, float angle); //johnfitz
void VectorAngles (const vec3_t forward, vec3_t angles); //johnfitz

void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);

vec_t _DotProduct (vec3_t v1, vec3_t v2);
void _VectorSubtract (vec3_t veca, vec3_t vecb, vec3_t out);
void _VectorAdd (vec3_t veca, vec3_t vecb, vec3_t out);
void _VectorCopy (vec3_t in, vec3_t out);

int VectorCompare (vec3_t v1, vec3_t v2);
vec_t VectorLength (vec3_t v);
void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross);
float VectorNormalize (vec3_t v); // returns vector length
void VectorInverse (vec3_t v);
void VectorScale (vec3_t in, vec_t scale, vec3_t out);
void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);


#define VectorSet(v, x, y, z) ((v)[0] = (x), (v)[1] = (y), (v)[2] = (z))
#define VectorAddFloat(a, b, c)				((c)[0] = (a)[0] + (b), (c)[1] = (a)[1] + (b), (c)[2] = (a)[2] + (b))

vec_t DistanceBetween2Points (vec3_t v1, vec3_t v2);

void VectorAverage (vec3_t v1, vec3_t v2, vec3_t out);
void VectorExtendLimits (vec3_t newvalue, vec3_t minlimit, vec3_t maxlimit);


int ParseFloats(const char *s, float *f, int *f_size);
cbool PointInCube(vec3_t point, vec3_t cube_mins, vec3_t cube_maxs);



#endif	// ! __MATH_VECTOR_H__



