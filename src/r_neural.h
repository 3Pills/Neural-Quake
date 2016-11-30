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

typedef struct gene_s gene_t;

void Draw_Line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, float thickness, unsigned char c, float alpha);
void Draw_Square(unsigned short x, unsigned short y, unsigned short w, unsigned short h, float thickness, unsigned char c, float alpha);

void R_DrawPoint(vec3_t origin, double size, unsigned char c);

// Struct used for storing neural graph ui data for drawing.
typedef struct uinode_s
{
	unsigned short x, y; // Node coords
	unsigned short sizex, sizey; // Node size.
	unsigned char color; // Used for hidden nodes.
} uinode_t;

// Struct used for displaying links between two uinodes.
typedef struct uilink_s
{
	unsigned int inode; // The index of the start uinode.
	unsigned int onode; // The index of the end uinode.
	unsigned char color; // Color denoted by the link's weight.
	unsigned char opacity; // Alpha of line, between 0-255.
	unsigned int gene; // Refers to gene index within genome.
} uilink_t;

#endif//!__NEURAL_RENDER_H__