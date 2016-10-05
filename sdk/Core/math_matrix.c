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
// math_matrix.c

#include "core.h"
#include "math_matrix.h"

#include <math.h>
#include <stdlib.h> // malloc, etc.
#include <string.h> // memcpy, etc.



/*
============================================================================================================

	MATRIX OPS  (Baker: From MH)

	These happen in pace on the matrix and update it's current values

	These are D3D style matrix functions; sorry OpenGL-lovers but they're more sensible, usable
	and intuitive this way...

============================================================================================================
*/

float VectorNormalize3f (float *x, float *y, float *z)
{
	float	length, ilength;

	length = x[0] * x[0] + y[0] * y[0] + z[0] * z[0];
	length = sqrt (length);		// FIXME

	if (length)
	{
		ilength = 1 / length;

		x[0] *= ilength;
		y[0] *= ilength;
		z[0] *= ilength;
	}

	return length;
}


glmatrix *GL_LoadMatrix (glmatrix *dst, glmatrix *src)
{
	memcpy (dst, src, sizeof (glmatrix));

	return dst;
}


glmatrix *GL_IdentityMatrix (glmatrix *m)
{
	m->m16[0] = m->m16[5] = m->m16[10] = m->m16[15] = 1;
	m->m16[1] = m->m16[2] = m->m16[3] = m->m16[4] = m->m16[6] = m->m16[7] = m->m16[8] = m->m16[9] = m->m16[11] = m->m16[12] = m->m16[13] = m->m16[14] = 0;

	return m;
}


glmatrix *GL_MultiplyMatrix (glmatrix *out, glmatrix *m1, glmatrix *m2)
{
	int i, j;
	glmatrix tmp;

	// do it this way because either of m1 or m2 might be the same as out...
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			tmp.m4x4[i][j] = m1->m4x4[i][0] * m2->m4x4[0][j] +
							 m1->m4x4[i][1] * m2->m4x4[1][j] +
							 m1->m4x4[i][2] * m2->m4x4[2][j] +
							 m1->m4x4[i][3] * m2->m4x4[3][j];
		}
	}

	memcpy (out, &tmp, sizeof (glmatrix));

	return out;
}


glmatrix *GL_TranslateMatrix (glmatrix *m, float x, float y, float z)
{
	glmatrix tmp;
	GL_IdentityMatrix (&tmp);

	tmp.m16[12] = x;
	tmp.m16[13] = y;
	tmp.m16[14] = z;

	GL_MultiplyMatrix (m, &tmp, m);

	return m;
}


glmatrix *GL_ScaleMatrix (glmatrix *m, float x, float y, float z)
{
	glmatrix tmp;
	GL_IdentityMatrix (&tmp);

	tmp.m16[0] = x;
	tmp.m16[5] = y;
	tmp.m16[10] = z;

	GL_MultiplyMatrix (m, &tmp, m);

	return m;
}


glmatrix *GL_RotateMatrix (glmatrix *m, float a, float x, float y, float z)
{
	// i prefer spaces around my operators because it makes stuff like a = b * -c clearer and easier on the eye. ;)
	glmatrix tmp;
	float c = cos (a * M_PI / 180.0);
	float s = sin (a * M_PI / 180.0);

	// http://www.opengl.org/sdk/docs/man/xhtml/glRotate.xml
	// this should normalize the vector before rotating
	VectorNormalize3f (&x, &y, &z);

	tmp.m16[0] = x * x * (1 - c) + c;
	tmp.m16[4] = x * y * (1 - c) - z * s;
	tmp.m16[8] = x * z * (1 - c) + y * s;
	tmp.m16[12] = 0;

	tmp.m16[1] = y * x * (1 - c) + z * s;
	tmp.m16[5] = y * y * (1 - c) + c;
	tmp.m16[9] = y * z * (1 - c) - x * s;
	tmp.m16[13] = 0;

	tmp.m16[2] = x * z * (1 - c) - y * s;
	tmp.m16[6] = y * z * (1 - c) + x * s;
	tmp.m16[10] = z * z * (1 - c) + c;
	tmp.m16[14] = 0;

	tmp.m16[3] = 0;
	tmp.m16[7] = 0;
	tmp.m16[11] = 0;
	tmp.m16[15] = 1;

	GL_MultiplyMatrix (m, &tmp, m);

	return m;
}


