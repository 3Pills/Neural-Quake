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
// image.h -- image manipulation and loading

#ifndef __IMAGE_H__
#define __IMAGE_H__


/*
** Allocations in Core SHALL be made with C allocators calloc, free, etc.
** 
** image.c is part of Core.  
** Functions may provide a method for caller to supply their own allocator.
**
*/

// This keeps things real clear!  And communicates the return type and input quite well
// Rule: indexed ---> byte *data		
// Rule: rgba/bgra--> unsigned *rgba, unsigned *bgra
// Rule: unknwon ---> void *data


#define CORE_LOCAL
#include "core.h"
#include "environment.h"
#include <stdio.h> // fopen, etc.
#include <stdlib.h> // malloc, etc.
#include <string.h> // mem
#include "lodepng.h"

///////////////////////////////////////////////////////////////////////////////
//  IMAGE: OPERATIONS
///////////////////////////////////////////////////////////////////////////////

void Image_Flip_Buffer (void *buffer, int columns, int rows, int bytes_per_pixel);
void Image_Flip_RedGreen (void *rgba, size_t numbytes);
void Image_Flip_RedGreen_Alpha_255 (void *rgba, size_t numbytes);

///////////////////////////////////////////////////////////////////////////////
//  IMAGE: MISCELLANEOUS
///////////////////////////////////////////////////////////////////////////////

int Image_Power_Of_Two_Size (int myInteger);


///////////////////////////////////////////////////////////////////////////////
//  IMAGE: GAMMA
///////////////////////////////////////////////////////////////////////////////

void Image_Build_Gamma_Table (float g, float c, byte gammatable_256[]);
void Image_Apply_Gamma_To_RGBA_Pixels (unsigned *rgba, int width, int height, byte gammatable_256[]);

///////////////////////////////////////////////////////////////////////////////
//  PALETTE REPLACEMENT
///////////////////////////////////////////////////////////////////////////////

// 256 color functions
void Image_Quick_Palette_256_Free (void);
void Image_Quick_Palette_256_Alloc (int black_index);
byte Palette_Best_Color_Index_For_Pixel (unsigned mypixel, byte palette768[], int numpalcolors);
void Image_Convert_Palette_To_Palette (byte *pixels_indexed, int len, byte old_palette_768[], byte new_palette_768[] );
byte *Image_Buffer_RGBA_To_Palette_Alpha_Threshold_Alloc (unsigned *rgba, int width, int height, byte palette_768[], int alpha_threshold);

// RGB functions
extern unsigned *fastpal_rgb4;
void Image_Quick_Palette_RGBA_Free (void);
void Image_Quick_Palette_RGBA_Alloc (void);

///////////////////////////////////////////////////////////////////////////////
//  IMAGE: PNG, PCX (READ ONLY), TGA -- RGBA input and output
///////////////////////////////////////////////////////////////////////////////

// TODO: Detect PNG and JFIF headers (89 + PNG = first 4 bytes, 6 bytes + JFIF, TGA read the first 18 and calc?)

unsigned *Image_Load_TGA_FileHandle (FILE *fin, int *width, int *height, malloc_fn_t Malloc_Fn, const char *description);
unsigned *Image_Load_PCX_FileHandle (FILE *fin, int *width, int *height, size_t filelen, malloc_fn_t Malloc_Fn, const char *description);


cbool Image_Save_TGA (const char *path_to_file, const void *pels, int width, int height, int bits_per_pixel, cbool upsidedown);
cbool Image_Save_PNG (const char *path_to_file, const unsigned *rgba, int width, int height);
cbool Image_Save_Auto (const char *path_to_file, const unsigned *rgba, int width, int height);

char *Image_Base64Encode_PNG_Alloc (const unsigned *rgba, int width, int height);
unsigned *Image_Base64Decode_Alloc (const char *s, int *width, int *height);

unsigned *Image_Load_PNG_Alloc (const char *path_to_file, int *width, int *height);
unsigned *Image_Load_PNG_Memory_Alloc (const byte *png_data, size_t len, int *width, int *height);
unsigned *Image_Load_PNG_File_Offset_Alloc (const char *path_to_file, size_t offset_into_file, size_t len, int *width, int *height);

unsigned *Image_Load_Auto_Alloc (const char *path_to_file, int *width, int *height);

// Loads from a blob and will be a type of image (not rgba at this point)
unsigned *Image_Load_File_Offset_Alloc (const char *path_to_file, enum image_type_e blob_type, size_t offset_into_file, size_t len, int *width, int *height);
unsigned *Image_Load_Memory_Alloc (enum image_type_e image_type, const byte *data, size_t len, int *width, int *height);

// Opposite is Image_Load_PNG_Memory_Alloc
byte *Image_PNG_Compress_Alloc (const unsigned *rgba, int width, int height, size_t *bytes_out);

///////////////////////////////////////////////////////////////////////////////
//  IMAGE: Query functions (has alpha, etc.)
///////////////////////////////////////////////////////////////////////////////

cbool Image_Has_Alpha_Channel (const unsigned *rgba, int width, int height);

#define RGBA_4 4 // Used to indicate a 4 x is bytes per pixel
#define RGB_3  3 // Used to indicate a 3 x is bytes per pixel 


///////////////////////////////////////////////////////////////////////////////
//  IMAGE: Color Functions
///////////////////////////////////////////////////////////////////////////////

unsigned HTML_Color_RGBA (const char *s); // AliceBlue or #ff0000 to rgba
const char* HTML_Color_From_RGBA (unsigned rgba); // rgba to color string (loses alpha)

///////////////////////////////////////////////////////////////////////////////
//  IMAGE: Resample (speed issues, reliability, quality?)
///////////////////////////////////////////////////////////////////////////////

unsigned *Image_Bilinear_Resize_Alloc (unsigned *rgba, int w, int h, int new_width, int new_height);

void Image_Paste (int dst_x, int dst_y, int src_x, int src_y, int paste_width, int paste_height, void *dst, int dst_width, int dst_height, void *src, int src_width, int src_height, int pixel_bytes);
void *Image_Crop_Alloc (void *pels, int width, int height, int pixel_bytes, int x, int y, int new_width, int new_height);
void *Image_Enlarge_Canvas_Alloc (void *pels, int width, int height, int pixel_bytes, int new_width, int new_height, unsigned fillcolor, cbool centered);

void Image_Rect_Fill (unsigned fillcolor, int x, int y, int paint_width, int paint_height, byte *buf, int pix_width, int pix_height, int pixel_bytes);

#endif	// ! __IMAGE_H__


