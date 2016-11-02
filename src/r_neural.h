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
// r_neural.h -- Helper functions for debug rendering within the neural network.

#ifndef __NEURAL_RENDER_H__
#define __NEURAL_RENDER_H__

void Draw_Line(int x1, int y1, int x2, int y2, float thickness, int c, int alpha);
void Draw_Square(int x, int y, int w, int h, float thickness, int c, int alpha);

void R_DrawPoint(vec3_t origin, int size, int c);
void R_DrawWireBox(vec3_t origin, vec3_t mins, vec3_t maxs, int c);

#endif//!__NEURAL_RENDER_H__