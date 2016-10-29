#include "quakedef.h"
#include "r_neural.h"

#if defined(DIRECT3D_WRAPPER)
void Draw_Line(int x1, int y1, int x2, int y2, float thickness, int c, int alpha)
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

void Draw_Square(int x, int y, int w, int h, float thickness, int c, int alpha)
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
void Draw_Line(int x1, int y1, int x2, int y2, float thickness, int c, int alpha)
{
	byte *pal = (byte *)vid.d_8to24table;

	eglDisable(GL_TEXTURE_2D);
	eglDisable(GL_ALPHA_TEST);
	eglEnable(GL_BLEND);
	eglColor4f(pal[c * 4] / 255.0, pal[c * 4 + 1] / 255.0, pal[c * 4 + 2] / 255.0, alpha);

	eglLineWidth(thickness);
	eglBegin(GL_LINES);
	eglVertex2f(x1, y1);
	eglVertex2f(x2, y2);
	eglEnd();

	eglColor3f(1, 1, 1);
	eglDisable(GL_BLEND);
	eglEnable(GL_ALPHA_TEST);
	eglEnable(GL_TEXTURE_2D);
}

void Draw_Square(int x, int y, int w, int h, float thickness, int c, int alpha)
{
	byte *pal = (byte *)vid.d_8to24table;

	eglDisable(GL_TEXTURE_2D);
	eglDisable(GL_ALPHA_TEST);
	eglEnable(GL_BLEND);
	eglColor4f(pal[c * 4] / 255.0, pal[c * 4 + 1] / 255.0, pal[c * 4 + 2] / 255.0, alpha);

	eglLineWidth(thickness);
	eglBegin(GL_LINE_LOOP);
	eglVertex2f(x, y);
	eglVertex2f(x + w, y);
	eglVertex2f(x + w, y + h);
	eglVertex2f(x, y + h);
	eglEnd();

	eglColor3f(1, 1, 1);
	eglDisable(GL_BLEND);
	eglEnable(GL_ALPHA_TEST);
	eglEnable(GL_TEXTURE_2D);
}
#else

//{
//	// Clamping?
//	// Baker: Alpha is ignored at this time
//	int x = _x + canvas.x; // Baker: Canvas
//	int y = _y + canvas.y; // Baker: Canvas
//#if 0
//	int x = CLAMP(0, _x + canvas.x, vid.width - 1)
//		int y = CLAMP(0, _y + canvas.y, vid.height - 1)
//		int w = CLAMP(0, _w, vid.width - x);  // draw at 350 w 100 vidwidth 400 so 399 is max.  w = 400-1-350=49
//	int h = CLAMP(0, _h, vid.height - y);
//#endif
//	byte			*dest;
//	int				u, v;
//
//	dest = vid.buffer + y*vid.rowbytes + x;
//	for (v = 0; v < h; v++, dest += vid.rowbytes)
//		for (u = 0; u < w; u++)
//			dest[u] = c;
//}

void Draw_Line(int _x1, int _y1, int _x2, int _y2, float thickness, int c, int alpha)
/*{
	int dx, dy, p, end;
	float x, y;

	int x1 = _x1 + canvas.x;
	int y1 = _y1 + canvas.y;
	int x2 = _x2 + canvas.x;
	int y2 = _y2 + canvas.y;

	dx = abs(x1 - x2);
	dy = abs(y1 - y2);
	p = 2 * dy - dx;
	if (x1 > x2)
	{
		x = x2;
		y = y2;
		end = x1;
	}
	else
	{
		x = x1;
		y = y1;
		end = x2;
	}

	byte *dest = vid.buffer + (int)y*vid.rowbytes;
	dest[(int)x] = c;
	while (x < end)
	{
		x = x + 1;
		if (p < 0)
		{
			p = p + 2 * dy;
		}
		else
		{
			y = y + 1;
			p = p + 2 * (dy - dx);
		}
		putpixel(x, y, 10);
	}
}

*/
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

void Draw_Square(int _x, int _y, int w, int h, float thickness, int c, int alpha)
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