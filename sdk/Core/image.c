/*
Copyright (col) 1996-2001 Id Software, Inc.
Copyright (col) 2002-2009 John Fitzgibbons and others
Copyright (col) 2010-2014 Quakespasm
Copyright (col) 2012-2014 Baker

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
// image.col -- image manipulation and loading

#define CORE_LOCAL

#include "core.h"
#include "image.h"



///////////////////////////////////////////////////////////////////////////////
//  IMAGE: OPERATIONS
///////////////////////////////////////////////////////////////////////////////

void Image_Flip_RedGreen (void *rgba, size_t numbytes)
{
	byte	*byte_rep = (byte *)rgba;
	byte	temp;

	// RGBA to BGRA so clipboard will take it
	size_t i; for (i = 0 ; i < numbytes ; i += RGBA_4)
	{
		temp = byte_rep[i + 0];

		byte_rep[i + 0] = byte_rep[i + 2];
		byte_rep[i + 2] = temp;
	}
}

void Image_Flip_RedGreen_Alpha_255 (void *rgba, size_t numbytes)
{
	byte	*byte_rep = (byte *)rgba;
	byte	temp;

	// RGBA to BGRA so clipboard will take it
	size_t i; for (i = 0 ; i < numbytes ; i += RGBA_4)
	{
		temp = byte_rep[i + 0];

		byte_rep[i + 0] = byte_rep[i + 2];
		byte_rep[i + 2] = temp;
		byte_rep[i + 3] = 255;
	}
}


void Image_Flip_Buffer (void *in_buffer, int columns, int rows, int bytes_per_pixel)
{
	byte	*buffer = (byte *)in_buffer;
	int		bufsize = columns * bytes_per_pixel; // bufsize = widthBytes;
	byte*	tb1 = malloc (bufsize); // Flip buffer
	byte*	tb2 = malloc (bufsize); // Flip buffer2
	int		i, offset1, offset2;

	for (i = 0; i < (rows + 1) / 2; i ++)
	{
		offset1 = i * bufsize;
		offset2 = ((rows - 1) - i) * bufsize;

		memcpy(tb1,				buffer + offset1, bufsize);
		memcpy(tb2,				buffer + offset2, bufsize);
		memcpy(buffer+offset1,	tb2,			  bufsize);
		memcpy(buffer+offset2,	tb1,			  bufsize);
	}

	free (tb1);
	free (tb2);
	return;
}

///////////////////////////////////////////////////////////////////////////////
//  IMAGE: MISCELLANEOUS
///////////////////////////////////////////////////////////////////////////////

int Image_Power_Of_Two_Size (int n)
{
	int upsized_pow2;
	for (upsized_pow2 = 1 ; upsized_pow2 < n ; upsized_pow2 <<= 1) //  <<= 1 is a bitwise double, same as "times two"
		;

	Core_DPrintf ("Upsized %i to %i\n", n, upsized_pow2);
	return upsized_pow2;
}

///////////////////////////////////////////////////////////////////////////////
//  IMAGE: GAMMA
///////////////////////////////////////////////////////////////////////////////

void Image_Build_Gamma_Table (float g, float col, byte gammatable_256[])
{
	int	i, inf;

	g = CLAMP(0.3, g, 3);
	col = CLAMP(1, col, 3);

	if (g == 1 && col == 1)
	{
		for (i = 0 ; i < 256 ; i++)
			gammatable_256[i] = i;
		return;
	}

	for (i = 0 ; i < 256 ; i++)
	{
		inf = 255 * pow((i + 0.5) / 255.5 * col, g) + 0.5;
		gammatable_256[i] = CLAMP(0, inf, 255);
	}
}

void Image_Apply_Gamma_To_RGBA_Pixels (unsigned *rgba, int width, int height, byte gammatable_256[])
{
	byte* data = (byte *)rgba;
	int imagesize = width * height;
	int i;
	for (i = 0 ; i < imagesize ; i++)
	{
		data[4*i+0] = gammatable_256[data[4*i+0]];
		data[4*i+1] = gammatable_256[data[4*i+1]];
		data[4*i+2] = gammatable_256[data[4*i+2]];
	}
}


///////////////////////////////////////////////////////////////////////////////
//  PALETTE REPLACEMENT
///////////////////////////////////////////////////////////////////////////////

static byte *fastpal;
static byte	 fastblack;

void Image_Quick_Palette_256_Free (void)
{
	free (fastpal);
	fastpal = NULL;
}

void Image_Quick_Palette_256_Alloc (int black_index)
{
	// Check to see if it was already allocated
	if (fastpal)
	{
		Core_Printf ("Quick palette wasn't closed. Freeing ...\n");
		Image_Quick_Palette_256_Free ();
	}

	// Baker: Allocate a buffer for all possible RGB combinations, calloc zeros the memory
	fastpal = (byte*)calloc (256*256*256, 1);
	fastblack = (byte)black_index;
}


byte Palette_Best_Color_Index_For_Pixel (unsigned mypixel, byte palette768[], int numpalcolors)
{
	static byte red, green, blue; //, alpha;
	static int red_distance, green_distance, blue_distance;
	static int color_distance, best_color_index;
	static unsigned best_color_distance;
	static byte *current_palette_index;
	static int i;

	unsigned rgbpix = mypixel & 0xFFFFFF; // Remove alpha from the equation

	byte fasteval = fastpal ? fastpal[rgbpix] : 0;

	if (fasteval)
	{
		if (mypixel == 0)
			return fastblack;

		return fasteval;
	}

	red		= (mypixel      )  & 0xFF;
	green	= (mypixel >> 8 )  & 0xFF;
	blue	= (mypixel >> 16)  & 0xFF;
//	alpha	= (mypixel >> 24)  & 0xFF;

	best_color_index		=  -1;
	best_color_distance		=  0 - 1; // Should be a big number
	current_palette_index	= palette768;

	for (i = 0; i < numpalcolors; i++, current_palette_index += 3 )
	{
		red_distance	= red   - current_palette_index[0];
		green_distance	= green - current_palette_index[1];
		blue_distance	= blue  - current_palette_index[2];
		color_distance	= SQUARED(red_distance) + SQUARED(green_distance) + SQUARED(blue_distance); // Could sqrt this but no reason to do so

		if ((unsigned)color_distance < best_color_distance)
		{
			best_color_distance	= color_distance;
			best_color_index		= i;

			if (best_color_distance == 0)
				break; // Can't beat an exact match
		}
	}

	if (fastpal)
		fastpal[rgbpix] = (byte)best_color_index;

	return (byte)best_color_index;
}

void Image_Convert_Palette_To_Palette (byte *pixels_indexed, int len, byte old_palette_768[], byte new_palette_768[] )
{
	byte	newpal[256]; // For each HL color, we will fill this in
	int i, j;
	unsigned mycolor;
	byte	qpal;

	for (i = 0, j = 0; i < 256; i++, j+=3)
	{
		mycolor = ((unsigned)old_palette_768[j + 0] << 0)
				+ ((unsigned)old_palette_768[j + 1] << 8)
				+ ((unsigned)old_palette_768[j + 2] << 16);

		newpal[i] = qpal = Palette_Best_Color_Index_For_Pixel (mycolor, new_palette_768, 224);
	}

	// Baker: I think Half-Life color #255 is special like Quake
	newpal[255] = 255;

	// Now convert the pixels
	for (i = 0; i < len; i ++)
	{
		pixels_indexed[i] = newpal[pixels_indexed[i]];
	}
}

byte *Image_Buffer_RGBA_To_Palette_Alpha_Threshold_Alloc (unsigned *pixels_rgba, int width, int height, byte palette_768[], int alpha_threshold)
{
	int size = width * height;
	byte *buf = malloc (size);
	byte *dst;
	unsigned *src;
	int i;
	unsigned mypixel;
	byte red, green, blue, alpha;

	for (i = 0, src = (unsigned *) pixels_rgba, dst = buf; i < size; i++, dst++, src++)
	{
		mypixel = *src;
		red		= (mypixel      )  & 0xFF;
		green	= (mypixel >> 8 )  & 0xFF;
		blue	= (mypixel >> 16)  & 0xFF;
		alpha	= (mypixel >> 24)  & 0xFF;

		if (alpha <= alpha_threshold)
			*dst = 255; // Quake transparent color
		else
			*dst = Palette_Best_Color_Index_For_Pixel (*src, palette_768, 255); // First 255 colors, not 256
	}
	return buf;
}



unsigned *fastpal_rgb4;
void Image_Quick_Palette_RGBA_Free (void)
{
	free (fastpal_rgb4);
	fastpal_rgb4 = NULL;
}

void Image_Quick_Palette_RGBA_Alloc (void)
{
	// Check to see if it was already allocated
	if (fastpal_rgb4)
	{
		Core_Printf ("Quick palette wasn't closed. Freeing ...\n");
		Image_Quick_Palette_RGBA_Free ();
	}

	// Baker: Allocate a buffer for all possible RGB combinations, calloc zeros the memory
	fastpal_rgb4 = (unsigned *)calloc (256*256*256, 4);
}


///////////////////////////////////////////////////////////////////////////////
//  IMAGE: BUFFERED READ (STATIC)
///////////////////////////////////////////////////////////////////////////////


typedef struct stdio_buffer_s
{
	FILE *f;
	unsigned char buffer[1024];
	int size;
	int pos;
} stdio_buffer_t;

static stdio_buffer_t *Buf_Alloc(FILE *f)
{
	stdio_buffer_t *buf = calloc (1, sizeof(stdio_buffer_t));
	buf->f = f;
	return buf;
}

static void Buf_Free(stdio_buffer_t *buf)
{
	free (buf);
}

static inline int Buf_GetC(stdio_buffer_t *buf)
{
	if (buf->pos >= buf->size)
	{
		buf->size = fread(buf->buffer, 1, sizeof(buf->buffer), buf->f);
		buf->pos = 0;

		if (buf->size == 0)
			return EOF;
	}

	return buf->buffer[buf->pos++];
}


///////////////////////////////////////////////////////////////////////////////
//  IMAGE: TGA with bgra and rgba support
///////////////////////////////////////////////////////////////////////////////

typedef struct targaheader_s
{
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} targaheader_t;

#define TARGAHEADERSIZE 18 //size on disk

targaheader_t targa_header;

static int fgetLittleShort (FILE *f)
{
	byte	b1, b2;

	b1 = fgetc(f);
	b2 = fgetc(f);

	return (short)(b1 + b2*256);
}

/* unused
static int fgetLittleLong (FILE *f)
{
	byte	b1, b2, b3, b4;

	b1 = fgetc(f);
	b2 = fgetc(f);
	b3 = fgetc(f);
	b4 = fgetc(f);

	return b1 + (b2<<8) + (b3<<16) + (b4<<24);
}
*/


cbool Image_Save_TGA (const char *path_to_file, const void *pels, int width, int height, int bits_per_pixel, cbool upsidedown)
{
	int		i, size, temp, bytes;
	byte	header[TARGAHEADERSIZE];
	byte	*data = (byte *)pels;
	FILE *f;

	f = core_fopen_write (path_to_file, "wb");

	if (!f)
		return false;

	memset (&header, 0, TARGAHEADERSIZE);
	header[2] = 2; // uncompressed type
	header[12] = width&255;
	header[13] = width>>8;
	header[14] = height&255;
	header[15] = height>>8;
	header[16] = bits_per_pixel; // pixel size
	if (upsidedown)
		header[17] = 0x20; //upside-down attribute

	// swap red and blue bytes
	bytes = bits_per_pixel / 8;
	size = width * height * bytes;

	for (i = 0; i < size; i += bytes)
	{
		temp = data[i];
		data[i] = data[i+2];
		data[i+2] = temp;
	}

	fwrite (&header, 1, TARGAHEADERSIZE, f);
	fwrite (data, 1, size, f);

	core_fclose (f);

	return true;
}


unsigned *Image_Load_TGA_FileHandle (FILE *fin, int *width, int *height, malloc_fn_t Malloc_Fn, const char* description)
{
	int				columns, rows, numPixels;
	byte			*pixbuf;
	int				row, column;
	byte			*targa_rgba;
	int				realrow; //johnfitz -- fix for upside-down targas
	cbool			upside_down; //johnfitz -- fix for upside-down targas
	stdio_buffer_t	*buf;

	targa_header.id_length = fgetc(fin);
	targa_header.colormap_type = fgetc(fin);
	targa_header.image_type = fgetc(fin);

	targa_header.colormap_index = fgetLittleShort(fin);
	targa_header.colormap_length = fgetLittleShort(fin);
	targa_header.colormap_size = fgetc(fin);
	targa_header.x_origin = fgetLittleShort(fin);
	targa_header.y_origin = fgetLittleShort(fin);
	targa_header.width = fgetLittleShort(fin);
	targa_header.height = fgetLittleShort(fin);
	targa_header.pixel_size = fgetc(fin);
	targa_header.attributes = fgetc(fin);

	if (targa_header.image_type!=2 && targa_header.image_type!=10)
		Core_Error ("Image_LoadTGA: %s is not a type 2 or type 10 targa", description);

	if (targa_header.colormap_type !=0 || (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		Core_Error ("Image_LoadTGA: %s is not a 24bit or 32bit targa", description);

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;
	upside_down = !(targa_header.attributes & 0x20); //johnfitz -- fix for upside-down targas

	targa_rgba = (byte *)Malloc_Fn (numPixels*4);

	if (targa_header.id_length != 0)
		fseek (fin, targa_header.id_length, SEEK_CUR);  // skip TARGA image comment

	buf = Buf_Alloc(fin);

	if (targa_header.image_type == 2) // Uncompressed, RGB images
	{
		for(row=rows-1; row>=0; row--)
		{
			//johnfitz -- fix for upside-down targas
			realrow = upside_down ? row : rows - 1 - row;
			pixbuf = targa_rgba + realrow*columns*4;
			//johnfitz
			for(column=0; column<columns; column++)
			{
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size)
				{
				case 24:
					blue = Buf_GetC(buf);
					green = Buf_GetC(buf);
					red = Buf_GetC(buf);
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue = Buf_GetC(buf);
					green = Buf_GetC(buf);
					red = Buf_GetC(buf);
					alphabyte = Buf_GetC(buf);
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) // Runlength encoded RGB images
	{
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row = rows - 1; row >= 0; row--)
		{
			//johnfitz -- fix for upside-down targas
			realrow = upside_down ? row : rows - 1 - row;
			pixbuf = targa_rgba + realrow * columns * 4;
			//johnfitz
			for(column = 0; column < columns; )
			{
				packetHeader=Buf_GetC(buf);
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) // run-length packet
				{
					switch (targa_header.pixel_size)
					{
					case 24:
						blue = Buf_GetC(buf);
						green = Buf_GetC(buf);
						red = Buf_GetC(buf);
						alphabyte = 255;
						break;
					case 32:
						blue = Buf_GetC(buf);
						green = Buf_GetC(buf);
						red = Buf_GetC(buf);
						alphabyte = Buf_GetC(buf);
						break;
					default: /* avoid compiler warnings */
						blue = red = green = alphabyte = 0;
					}

					for(j = 0;j < packetSize; j++)
					{
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						column++;
						if (column == columns) // run spans across rows
						{
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							//johnfitz -- fix for upside-down targas
							realrow = upside_down ? row : rows - 1 - row;
							pixbuf = targa_rgba + realrow*columns*4;
							//johnfitz
						}
					}
				}
				else // non run-length packet
				{
					for(j=0;j<packetSize;j++)
					{
						switch (targa_header.pixel_size)
						{
						case 24:
							blue = Buf_GetC(buf);
							green = Buf_GetC(buf);
							red = Buf_GetC(buf);
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 32:
							blue = Buf_GetC(buf);
							green = Buf_GetC(buf);
							red = Buf_GetC(buf);
							alphabyte = Buf_GetC(buf);
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
						default: /* avoid compiler warnings */
							blue = red = green = alphabyte = 0;
						}
						column++;
						if (column == columns) // pixel packet run spans across rows
						{
							column = 0;
							if (row > 0)
								row--;
							else
								goto breakOut;
							//johnfitz -- fix for upside-down targas
							realrow = upside_down ? row : rows - 1 - row;
							pixbuf = targa_rgba + realrow*columns*4;
							//johnfitz
						}
					}
				}
			}
			breakOut:;
		}
	}

	Buf_Free(buf);
	core_fclose(fin);

	*width = (int)(targa_header.width);
	*height = (int)(targa_header.height);
	return (unsigned *)targa_rgba;
}


///////////////////////////////////////////////////////////////////////////////
//  IMAGE: PCX (READ ONLY)
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    char			signature;
    char			version;
    char			encoding;
    char			bits_per_pixel;
    unsigned short	xmin,ymin,xmax,ymax;
    unsigned short	hdpi,vdpi;
    byte			colortable[48];
    char			reserved;
    char			color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    char			filler[58];
} pcxheader_t;


unsigned *Image_Load_PCX_FileHandle (FILE *fin, int *width, int *height, size_t filelen, malloc_fn_t Malloc_Fn, const char* description)
{
	pcxheader_t	pcx;
	int			x, y, w, h, readbyte, runlength, start;
	byte		*p, *data;
	byte		palette[768];
	stdio_buffer_t  *buf;

	start = ftell (fin); //save start of file (since we might be inside a pak file, SEEK_SET might not be the start of the pcx)

	fread (&pcx, sizeof(pcx), 1, fin);
	pcx.xmin = (unsigned short)pcx.xmin;
	pcx.ymin = (unsigned short)pcx.ymin;
	pcx.xmax = (unsigned short)pcx.xmax;
	pcx.ymax = (unsigned short)pcx.ymax;
	pcx.bytes_per_line = (unsigned short)pcx.bytes_per_line;

	if (pcx.signature != 0x0A)
		Core_Error ("'%s' is not a valid PCX file", description);

	if (pcx.version != 5)
		Core_Error ("'%s' is version %i, should be 5", description, pcx.version);

	if (pcx.encoding != 1 || pcx.bits_per_pixel != 8 || pcx.color_planes != 1)
		Core_Error ("'%s' has wrong encoding or bit depth", description);

	w = pcx.xmax - pcx.xmin + 1;
	h = pcx.ymax - pcx.ymin + 1;

	data = (byte *)Malloc_Fn((w * h + 1) * 4); //+1 to allow reading padding byte on last line

	//load palette
	fseek (fin, start + filelen - 768, SEEK_SET);
	fread (palette, 1, 768, fin);

	//back to start of image data
	fseek (fin, start + sizeof(pcx), SEEK_SET);

	buf = Buf_Alloc(fin);

	for (y=0; y<h; y++)
	{
		p = data + y * w * 4;

		for (x=0; x < pcx.bytes_per_line; ) //read the extra padding byte if necessary
		{
			readbyte = Buf_GetC(buf);

			if(readbyte >= 0xC0)
			{
				runlength = readbyte & 0x3F;
				readbyte = Buf_GetC(buf);
			}
			else
				runlength = 1;

			while(runlength--)
			{
				p[0] = palette[readbyte*3];
				p[1] = palette[readbyte*3+1];
				p[2] = palette[readbyte*3+2];
				p[3] = 255;
				p += 4;
				x++;
			}
		}
	}

	Buf_Free(buf);
	core_fclose(fin);

	*width = w;
	*height = h;
	return (unsigned *)data;
}

///////////////////////////////////////////////////////////////////////////////
//  IMAGE: PNG
///////////////////////////////////////////////////////////////////////////////


#include "lodepng.h"

// no err handling
char *Image_Base64Encode_PNG_Alloc (const unsigned *rgba, int width, int height)
{
	size_t pngsize = 0; // Kills a gcc "may be used uninitialized warning
	byte *png = Image_PNG_Compress_Alloc (rgba, width, height, &pngsize);

	return base64_encode_a (png, pngsize, NULL);
}

// png only at this time, no err handling
unsigned *Image_Base64Decode_Alloc (const char *s, int *width, int *height)
{
	size_t png_len;
	byte *png_in = base64_decode_a (s, &png_len);
	return Image_Load_PNG_Memory_Alloc (png_in, png_len, width, height);
}


byte *Image_PNG_Compress_Alloc (const unsigned *rgba, int width, int height, size_t *bytes_out)
{
	byte *png = NULL;
	size_t len;
	// Baker: We are assuming we have write access to the buffer
	//double timex=Time_Now();
	unsigned error = lodepng_encode32 (&png, &len, (byte *)rgba, width, height);
	//double elapsed=Time_Now()-timex; System_Alert ("Elapsed %g",elapsed);
	if (error)
	{
		Core_Printf ("Error writing png to memory\n");
		return NULL;
	}

	if (bytes_out) *bytes_out = len;
	return png;
}


cbool Image_Save_PNG (const char *path_to_file, const unsigned *rgba, int width, int height)
{
	// Baker: We are assuming we have write access to the buffer
	unsigned error = lodepng_encode32_file(path_to_file, (byte *)rgba, width, height);
	if (error)
	{
		Core_Printf ("Error writing png\n");
		return 0;
	}

	return 1;
}


// Decode from memory
unsigned *Image_Load_PNG_Memory_Alloc (const byte *png_data, size_t len, int *width, int *height)
{
	byte *image = NULL;
	unsigned uwidth, uheight, error;

	error = lodepng_decode32(&image, &uwidth, &uheight, png_data, len);
	if(error) { Core_Error("error %u: %s\n", error, lodepng_error_text(error)); return NULL; }

	if (width) *width = (int)uwidth;
	if (height) *height = (int)uheight;
	return (unsigned *)image;
}


// Decode from memory
unsigned *Image_Load_PNG_File_Offset_Alloc (const char *path_to_file, size_t offset_into_file, size_t len, int *width, int *height)
{
	unsigned *image;
	byte *png = File_To_Memory_Offset_Alloc (path_to_file, NULL, offset_into_file, len);
	if (!png) { Core_Error ("Couldn't load PNG to memory"); return NULL; }

	image = Image_Load_PNG_Memory_Alloc (png, len, width, height);

	free (png);
	return image;
}


unsigned *Image_Load_PNG_Alloc (const char *path_to_file, int *width, int *height)
{
	byte *image = NULL;
	unsigned error = lodepng_decode32_file(&image, width, height, path_to_file);

	if (error) { Core_Error ("error %u: %s\n", error, lodepng_error_text(error)); return NULL; }

	return (unsigned *)image;
}


cbool Image_Save_Auto (const char *path_to_file, const unsigned *rgba, int width, int height)
{
	enum image_type_e image_type = File_URL_Image_Type (path_to_file);

	switch (image_type)
	{
	case image_type_png: return Image_Save_PNG(path_to_file, rgba, width, height);

	case image_type_tga:
	case image_type_pcx:
	case image_type_jpeg:
	default:
		Core_Error ("Image auto save for file type for %s not supported yet");
		return false;
	}
};

unsigned *Image_Load_Auto_Alloc (const char *path_to_file, int *width, int *height)
{
	enum image_type_e image_type = File_URL_Image_Type (path_to_file);

	switch (image_type)
	{
	case image_type_png: return Image_Load_PNG_Alloc (path_to_file, width, height);

	case image_type_tga:
	case image_type_pcx:
	case image_type_jpeg:
	default:
		Core_Error ("Image loading for file type for %s not supported yet");
		return NULL;
	}
};

// Loads from a blob
unsigned *Image_Load_File_Offset_Alloc (const char *path_to_file, enum image_type_e blob_type, size_t offset_into_file, size_t len, int *width, int *height)
{
	switch (blob_type)
	{
	case image_type_png: return Image_Load_PNG_File_Offset_Alloc (path_to_file, offset_into_file, len, width, height);

	case image_type_tga:
	case image_type_pcx:
	case image_type_jpeg:
	default:
		Core_Error ("Image loading for file type for %s not supported yet");
		return NULL;
	}
}


// Loads from a blob
unsigned *Image_Load_Memory_Alloc (enum image_type_e image_type, const byte *data, size_t len, int *width, int *height)
{
	switch (image_type)
	{
	case image_type_png: return Image_Load_PNG_Memory_Alloc (data, len, width, height);

	case image_type_tga:
	case image_type_pcx:
	case image_type_jpeg:
	default:
		Core_Error ("Image loading for file type for %s not supported yet");
		return NULL;
	}
}


cbool Image_Has_Alpha_Channel (const unsigned *rgba, int width, int height)
{
	int pels = width * height;
	int i;
	cbool found_black = false;
	cbool found_gray  = false;
	cbool found_white = false;
	byte *data;

	for (i = 0, data = (byte *) rgba; i < pels; i ++, data += RGBA_4)
	{
		switch (data[3])
		{
			case 0x00: found_black = true; break;
			case 0xFF: found_white = true; break;
			default  : found_gray  = true; return true; // Found gray, get out!
		}

		if (found_black && found_white) return true;
	}

	return false;
}






struct html_colors
{
	const char *name, *hex;
	unsigned	rgba; // With alpha = 1?
	float		rgb[RGB_3];
};


static struct html_colors HTML_Colors[] =
{
#include "html_colors.enum.h"  // { "AliceBlue", "#F0F8FF"}, etc.
//	{ "AliceBlue",      "#F0F8FF"},
	{NULL,		0} // Terminator
};

unsigned HTML_Color_RGBA (const char* s);
void HTML_Color_Clampf (const char* s, float v4[]);

static void sHTML_Colors_Init (void)
{
	struct html_colors *cur;
	int i;
	for (i = 0; ((cur = &HTML_Colors[i])->name) ; i++)
	{
		cur->rgba = HTML_Color_RGBA (cur->name);
		HTML_Color_Clampf (cur->name, cur->rgb);
	}
}

// unit interval
void HTML_Color_Clampf (const char* s, float v4[])
{
	unsigned rgba = HTML_Color_RGBA (s);
	byte red   = (rgba      )  & 0xFF;
	byte green = (rgba >> 8 )  & 0xFF;
	byte blue  = (rgba >> 16)  & 0xFF;
	byte alpha = (rgba >> 24)  & 0xFF;

// These are close but not perfect (no rounding, division range might need to be 255.99999 ?)
#define COLOR_BYTE_TO_UNIT_INTERVAL(_ch)		( (_ch) == 255 ? 1 : (_ch) / 255.0f )
#define UNIT_INTERVAL_TO_COLOR_BYTE_TO_(_fl)	( (_fl) >= 1.0 ? 255 : (_fl) <= 0.0 ? 0 : (_fl) / 255.0f )

	v4[0] = COLOR_BYTE_TO_UNIT_INTERVAL(red);  // byte / 256 or 1 if 255
	v4[1] = COLOR_BYTE_TO_UNIT_INTERVAL(green);
	v4[2] = COLOR_BYTE_TO_UNIT_INTERVAL(blue);
	v4[3] = COLOR_BYTE_TO_UNIT_INTERVAL(alpha);
}

unsigned HTML_Color_RGBA (const char *s)
{
	struct html_colors *cur;
	int i;

	if (s[0] == '#' && strlen (s) == 7)
	{ // HTML #rrggbb format
		int red_high	= hex_char_to_int(s[1]), red_low   = hex_char_to_int(s[2]);
		int green_high	= hex_char_to_int(s[3]), green_low = hex_char_to_int(s[4]);
		int blue_high	= hex_char_to_int(s[5]), blue_low  = hex_char_to_int(s[6]);

		byte red = red_high * 16 + red_low, green = green_high * 16 + green_low, blue = blue_high  * 16 + blue_low, alpha = 255;

		return ((unsigned)red + ((unsigned)green << 8) + ((unsigned)blue << 16) + ((unsigned)alpha << 24));
	}

	// HTML string eval like ("Teal")

	// Initialize table if it isn't, we check first entry to see rgba int is filled to determine
	if (HTML_Colors[0].rgba == 0) sHTML_Colors_Init ();

	for (i = 0; ((cur = &HTML_Colors[i])->name) ; i++)
		if (!strcasecmp (cur->name, s)) return cur->rgba;

	return (unsigned)(-1); // Error, return white I guess.
}


const char* HTML_Color_From_RGBA (unsigned rgba)
{
	struct html_colors *cur;
	int i;

	rgba |= ((unsigned)255 << 24); // Set alpha to 1

	// Initialize table if it isn't, we check first entry to see rgba int is filled to determine
	if (HTML_Colors[0].rgba == 0) sHTML_Colors_Init ();

	for (i = 0; ((cur = &HTML_Colors[i])->name) ; i++)
		if (cur->rgba == rgba) return cur->name;

	// Didn't hit an HTML color, convert to #rrggbb string and return
	{
		static char nomatch[8];  // #ffcc33 + null term
		byte red   = (rgba      )  & 0xFF;
		byte green = (rgba >> 8 )  & 0xFF;
		byte blue  = (rgba >> 16)  & 0xFF;
		byte alpha = (rgba >> 24)  & 0xFF;

		c_snprintf3 (nomatch, "#%02x%02x%02x", red, green, blue);
		return (const char *)nomatch;
	}
}


unsigned *Image_Bilinear_Resize_Alloc (unsigned *rgba, int width, int height, int new_width, int new_height)
{
    unsigned *temp_o = calloc (sizeof(unsigned), new_width * new_height);
    int a, b, col, d, x, y, index, i, j ;
    float x_ratio = ((float)(width-1))/new_width ;
    float y_ratio = ((float)(height-1))/new_height ;
    float x_diff, y_diff, blue, red, green ;
    int offset = 0 ;
    for (i=0;i<new_height;i++) {
        for (j=0;j<new_width;j++) {
            x = (int)(x_ratio * j) ;
            y = (int)(y_ratio * i) ;
            x_diff = (x_ratio * j) - x ;
            y_diff = (y_ratio * i) - y ;
            index = (y*width+x) ;
            a = rgba[index] ;
            b = rgba[index+1] ;
            col = rgba[index+width] ;
            d = rgba[index+width+1] ;

            // blue element
            // Yb = Ab(1-width)(1-height) + Bb(width)(1-height) + Cb(height)(1-width) + Db(wh)
            blue = (a&0xff)*(1-x_diff)*(1-y_diff) + (b&0xff)*(x_diff)*(1-y_diff) +
                   (col&0xff)*(y_diff)*(1-x_diff)   + (d&0xff)*(x_diff*y_diff);

            // green element
            // Yg = Ag(1-width)(1-height) + Bg(width)(1-height) + Cg(height)(1-width) + Dg(wh)
            green = ((a>>8)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>8)&0xff)*(x_diff)*(1-y_diff) +
                    ((col>>8)&0xff)*(y_diff)*(1-x_diff)   + ((d>>8)&0xff)*(x_diff*y_diff);

            // red element
            // Yr = Ar(1-width)(1-height) + Br(width)(1-height) + Cr(height)(1-width) + Dr(wh)
            red = ((a>>16)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>16)&0xff)*(x_diff)*(1-y_diff) +
                  ((col>>16)&0xff)*(y_diff)*(1-x_diff)   + ((d>>16)&0xff)*(x_diff*y_diff);

            temp_o[offset++] =
                    0xff000000 | // hardcode alpha
                    ((((int)red)<<16)&0xff0000) |
                    ((((int)green)<<8)&0xff00) |
                    ((int)blue) ;
        }
    }
    return temp_o ;
}

// Literal, if you want to clamp src/dst coords and width, do it in another function
void Image_Paste (int dst_x, int dst_y, int src_x, int src_y, int paste_width, int paste_height, void *dst, int dst_width, int dst_height, void *src, int src_width, int src_height, int pixel_bytes)
{
	int r;
	byte *bdst = (byte *)dst;
	byte *bsrc = (byte *)src;

	int dst_rowbytes = dst_width * pixel_bytes;
	int src_rowbytes = src_width * pixel_bytes;
	int paste_rowbytes = paste_width * pixel_bytes; // Amount to copy per row

	int dst_offset = dst_y * dst_rowbytes + dst_x * pixel_bytes;
	int src_offset = src_y * src_rowbytes + src_x * pixel_bytes;

	for (r = 0; r < paste_height; r++, dst_offset += dst_rowbytes, src_offset += src_rowbytes)
		memcpy (&bdst[dst_offset], &bsrc[src_offset], paste_rowbytes);
}

void Image_Rect_Fill (unsigned fillcolor, int x, int y, int paint_width, int paint_height, byte *buf, int pix_width, int pix_height, int pixel_bytes)
{
	int rowbytes = pix_width * pixel_bytes;
	int startoffset = y * rowbytes + x * pixel_bytes;
	int i, j, bufoffset;

	// Non-byte pixels must fill the first row immediately
	switch (pixel_bytes)
	{
	case 2: // Short
		for (i = 0; i < paint_width; i ++)
			((short *)buf)[y * pix_width + x + i] = (short)fillcolor; // Fill first row
		break;

	case RGBA_4: // 4, unsigned
		for (i = 0; i < paint_width; i ++)
			((unsigned *)buf)[y * pix_width + x + i] = fillcolor; // Fill first row
		break;

	default: // Hopefully default is 1
		break;
	}

	for (j = 0, bufoffset = startoffset; j < paint_height; y ++, j++, bufoffset += rowbytes)
	{
		// Single byte pixels: memset.  multi-byte pixels: skip first row we already filled, then memcpy it
		if (pixel_bytes == 1) memset (&buf[bufoffset], fillcolor, paint_width); // byte pixels just memset
		else if (j > 0) memcpy (&buf[bufoffset], &buf[startoffset], paint_width * pixel_bytes);
	}

}

void sImage_Format_Buffer (byte *buf, int w, int h, int pixel_bytes, unsigned fillcolor)
{
	if (pixel_bytes == RGBA_4) Image_Rect_Fill (fillcolor,0,0,w,h,buf,w,h,pixel_bytes);  // unsigned
	else if (pixel_bytes == 1) memset (buf, fillcolor, w * h);
	else Core_Error ("Invalid pixel_bytes");
}

void *Image_Enlarge_Canvas_Alloc (void *pels, int width, int height, int pixel_bytes, int new_width, int new_height, unsigned fillcolor, cbool centered)
{
	byte *temp_o = calloc (pixel_bytes, new_width * new_height);

	if (fillcolor != 0) sImage_Format_Buffer (temp_o, new_width, new_height, pixel_bytes, fillcolor);

	if (centered)
	{
		int center_x = c_rint ((new_width - width) / 2.0);
		int center_y = c_rint ((new_height - height) / 2.0);
		Image_Paste (center_x,center_y,0,0,width,height,temp_o,new_width,new_height,pels,width,height,pixel_bytes);
	}
	else Image_Paste (0,0,0,0,width,height,temp_o,new_width,new_height,pels,width,height,pixel_bytes);

	return (void *)temp_o;
}



// Literal, if you want to clamp src/dst coords and width, do it in another function
void *Image_Crop_Alloc (void *pels, int width, int height, int pixel_bytes, int x, int y, int new_width, int new_height)
{
    byte *temp_o = calloc (pixel_bytes, new_width * new_height);

	Image_Paste (0, 0, x, y, new_width, new_height, temp_o, new_width, new_height, pels, width, height, pixel_bytes);

	return (void *)temp_o;
}




