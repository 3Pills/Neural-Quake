/*
Copyright (C) 2010-2012 MH
Copyright (C) 2013-2014 Baker

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
// math_matrix.h

#ifndef __MATH_MATRIX_H__
#define __MATH_MATRIX_H__

#include "environment.h"

typedef struct _glmatrix
{
    union
	{
		// put first because gcc barfs a little
		float m16[16];
        float m4x4[4][4];

        struct
		{
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
    };
} glmatrix;


struct _glmatrix *GL_MultiplyMatrix (struct _glmatrix *out, struct _glmatrix *m1, struct _glmatrix *m2);
struct _glmatrix *GL_TranslateMatrix (struct _glmatrix *m, float x, float y, float z);
struct _glmatrix *GL_ScaleMatrix (struct _glmatrix *m, float x, float y, float z);
struct _glmatrix *GL_RotateMatrix (struct _glmatrix *m, float a, float x, float y, float z);
struct _glmatrix *GL_IdentityMatrix (struct _glmatrix *m);
struct _glmatrix *GL_LoadMatrix (struct _glmatrix *dst, struct _glmatrix *src);


#endif // ! __MATH_MATRIX_H__


