/*
Copyright (C) 2016 Stephen Koren

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
#include "quakedef.h"
#include "r_neural.h"

#if defined(DIRECT3D_WRAPPER)
void Draw_Line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, float thickness, unsigned char c, float alpha)
{
	byte *pal = (byte *)vid.d_8to24table;

	eglDisable(GL_TEXTURE_2D);
	eglColor3f(pal[c * 4] / 255.0, pal[c * 4 + 1] / 255.0, pal[c * 4 + 2] / 255.0);

	eglBegin(GL_LINES);
	eglVertex2f(x1, y1);
	eglVertex2f(x2, y2);
	eglEnd();

	eglColor3f(1, 1, 1);
	eglEnable(GL_TEXTURE_2D);
}

void Draw_Square(unsigned short x, unsigned short y, unsigned short w, unsigned short h, float thickness, unsigned char c, float alpha)
{
	byte *pal = (byte *)vid.d_8to24table;

	eglDisable(GL_TEXTURE_2D);
	eglColor3f(pal[c * 4] / 255.0, pal[c * 4 + 1] / 255.0, pal[c * 4 + 2] / 255.0);

	eglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	eglBegin(GL_QUADS);
	eglVertex2f(x, y);
	eglVertex2f(x + w, y);
	eglVertex2f(x + w, y + h);
	eglVertex2f(x, y + h);
	eglEnd();

	eglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	eglColor3f(1, 1, 1);
	eglEnable(GL_TEXTURE_2D);
}
#elif defined(GLQUAKE)
void Draw_Line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, float thickness, unsigned char c, float alpha)
{
	byte *pal = (byte *)vid.d_8to24table;

	eglDisable(GL_TEXTURE_2D);
	eglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	eglEnable(GL_BLEND); 
	eglDisable(GL_ALPHA_TEST); 
	eglColor4f(pal[c * 4] / 255.0, pal[c * 4 + 1] / 255.0, pal[c * 4 + 2] / 255.0, alpha);

	eglLineWidth(thickness);
	eglBegin(GL_LINES);
	eglVertex2f(x1, y1);
	eglVertex2f(x2, y2);
	eglEnd();

	eglColor3f(1, 1, 1);
	eglEnable(GL_ALPHA_TEST);
	eglDisable(GL_BLEND); 
	eglEnable(GL_TEXTURE_2D);
}

void Draw_Square(unsigned short x, unsigned short y, unsigned short w, unsigned short h, float thickness, unsigned char c, float alpha)
{
	byte *pal = (byte *)vid.d_8to24table;

	eglDisable(GL_TEXTURE_2D);
	eglDisable(GL_ALPHA_TEST);
	eglEnable(GL_BLEND);
	eglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	eglColor4f(pal[c * 4] / 255.0, pal[c * 4 + 1] / 255.0, pal[c * 4 + 2] / 255.0, alpha);

	eglLineWidth(thickness);
	eglBegin(GL_LINE_LOOP);
	eglVertex2f(x, y);
	eglVertex2f(x + w, y);
	eglVertex2f(x + w, y + h);
	eglVertex2f(x, y + h);
	eglEnd();

	eglDisable(GL_BLEND);
	eglEnable(GL_ALPHA_TEST);
	eglEnable(GL_TEXTURE_2D);
}

void R_DrawPoint (vec3_t origin, double size, unsigned char c)
{
	byte *pal = (byte *)vid.d_8to24table;
	eglColor3f(pal[c * 4] / 255.0, pal[c * 4 + 1] / 255.0, pal[c * 4 + 2] / 255.0);

	eglDisable(GL_TEXTURE_2D);
	eglDisable(GL_ALPHA_TEST);
	eglEnable(GL_BLEND);

	eglBegin (GL_LINES);
	eglVertex3f (origin[0]-size, origin[1], origin[2]);
	eglVertex3f (origin[0]+size, origin[1], origin[2]);
	eglVertex3f (origin[0], origin[1]-size, origin[2]);
	eglVertex3f (origin[0], origin[1]+size, origin[2]);
	eglVertex3f (origin[0], origin[1], origin[2]-size);
	eglVertex3f (origin[0], origin[1], origin[2]+size);
	eglEnd ();

	eglDisable(GL_BLEND);
	eglEnable(GL_ALPHA_TEST);
	eglEnable(GL_TEXTURE_2D);
}

void R_DrawWireBox(vec3_t origin, vec3_t mins, vec3_t maxs, unsigned char c)
{
	VectorAdd(origin, mins, mins);
	VectorAdd(origin, maxs, maxs);

	byte *pal = (byte *)vid.d_8to24table;
	eglColor3f(pal[c * 4] / 255.0, pal[c * 4 + 1] / 255.0, pal[c * 4 + 2] / 255.0);

	eglDisable(GL_TEXTURE_2D);
	eglDisable(GL_ALPHA_TEST);
	eglEnable(GL_BLEND);

	eglBegin (GL_LINE_LOOP);
	eglVertex3f(mins[0], mins[1], mins[2]);
	eglVertex3f(mins[0], mins[1], maxs[2]);
	eglVertex3f(maxs[0], mins[1], maxs[2]);
	eglVertex3f(maxs[0], mins[1], mins[2]);

	eglVertex3f(mins[0], mins[1], mins[2]);
	eglVertex3f(mins[0], mins[1], maxs[2]);
	eglVertex3f(mins[0], maxs[1], maxs[2]);
	eglVertex3f(mins[0], maxs[1], mins[2]);

	eglVertex3f(mins[0], maxs[1], mins[2]);
	eglVertex3f(mins[0], maxs[1], maxs[2]);
	eglVertex3f(maxs[0], maxs[1], maxs[2]);
	eglVertex3f(maxs[0], maxs[1], mins[2]);

	eglVertex3f(maxs[0], mins[1], mins[2]);
	eglVertex3f(maxs[0], mins[1], maxs[2]);
	eglVertex3f(maxs[0], maxs[1], maxs[2]);
	eglVertex3f(maxs[0], maxs[1], mins[2]);

	eglVertex3f(mins[0], maxs[1], mins[2]);
	eglEnd ();

	eglDisable(GL_BLEND);
	eglEnable(GL_ALPHA_TEST);
	eglEnable(GL_TEXTURE_2D);
}

#else
void Draw_Line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, float thickness, unsigned char c, float alpha)
{
	float x1 = CLAMP(0, _x1 + canvas.x, canvas.width);
	float y1 = CLAMP(0, _y1 + canvas.y, canvas.height);
	float x2 = CLAMP(0, _x2 + canvas.x, canvas.width);
	float y2 = CLAMP(0, _y2 + canvas.y, canvas.height);

	float xdiff = (x2 - x1);
	float ydiff = (y2 - y1);

	if (xdiff == 0.0f && ydiff == 0.0f) 
	{
		((byte*)vid.buffer + (int)y1*vid.rowbytes + (int)x1)[0] = c;
		return;
	}

	if (fabs(xdiff) > fabs(ydiff)) {
		float xmin, xmax;

		// set xmin to the lower x value given
		// and xmax to the higher value
		if (x1 < x2) {
			xmin = x1;
			xmax = x2;
		}
		else {
			xmin = x2;
			xmax = x1;
		}

		// draw line in terms of y slope
		float slope = ydiff / xdiff;
		for (float x = xmin; x <= xmax; x += 1.0f) {
			int y = y1 + ((x - xmin) * slope);
			((byte*)vid.buffer + (int)y*vid.rowbytes + (int)x)[0] = c;
		}
	}
	else {
		float ymin, ymax;

		// set ymin to the lower y value given
		// and ymax to the higher value
		if (y1 < y2) {
			ymin = y1;
			ymax = y2;
		}
		else {
			ymin = y2;
			ymax = y1;
		}

		// draw line in terms of x slope
		float slope = xdiff / ydiff;
		for (float y = ymin; y <= ymax; y += 1.0f) {
			int x = x1 + ((y - ymin) * slope);
			((byte*)vid.buffer + (int)y*vid.rowbytes + (int)x)[0] = c;
		}
	}
}

void Draw_Square(unsigned short x, unsigned short y, unsigned short w, unsigned short h, float thickness, unsigned char c, float alpha)
{
	int min_x = _x + canvas.x;
	int min_y = _y + canvas.y;
	int max_x = min_x + w;
	int max_y = min_y + h;

	Draw_Line(min_x, min_y, max_x, min_y, thickness, c, alpha);
	Draw_Line(max_x, min_y, max_x, max_y, thickness, c, alpha);
	Draw_Line(max_x, max_y, min_x, max_y, thickness, c, alpha);
	Draw_Line(min_x, max_y, min_x, min_y, thickness, c, alpha);
}
#endif