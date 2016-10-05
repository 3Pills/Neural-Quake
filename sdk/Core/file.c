/*
Copyright (C) 2012-2014 Baker

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// file.c -- file functions

#define CORE_LOCAL
#define FILE_LOCAL

#include "core.h"

#include "file.h"
#include "stringlib.h"

#include <stdio.h> // fopen, etc.
#include <stdlib.h> // malloc, etc.


///////////////////////////////////////////////////////////////////////////////
//  FILE INFORMATION: Baker - These functions operate on a path_to_file
///////////////////////////////////////////////////////////////////////////////


cbool File_Exists (const char *path_to_file)
{
	return System_File_Exists (path_to_file);
}

cbool File_Is_Folder (const char* path_to_file)
{
	return System_File_Is_Folder (path_to_file);
}

size_t File_Length (const char* path_to_file)
{
	return System_File_Length (path_to_file);
}

double File_Time (const char *path_to_file)
{
	return System_File_Time (path_to_file);

}

///////////////////////////////////////////////////////////////////////////////
//  FILE ACTION: Baker - These functions operate on a path_to_file
///////////////////////////////////////////////////////////////////////////////

void File_Mkdir (const char* path_url)
{
	if (File_Exists(path_url)) // If it exists, no point in trying to make it
		return;

	System_mkdir (path_url);
}

void File_Chdir (const char* path_url)
{
	System_chdir (path_url);
}

const char* File_Getcwd (void)
{
	return System_getcwd ();
}

void File_Mkdir_Recursive (const char *path_to_file)
{
	char	path_string[MAX_OSPATH];
	char	*cursor;

	Core_DPrintf ("Making a path \"%s\"\n", path_to_file);

	c_strlcpy (path_string, path_to_file);

	for (cursor = &path_string[1]; *cursor ; cursor ++)
	{
		if (*cursor != '/')
			continue;

		// create the directory
		*cursor = 0;
		File_Mkdir (path_string);
		*cursor = '/';
	}
}

cbool File_Rmdir (const char* path_url)
{
	//_rmdir returns 0 if successfully deleted.  We return true if successfully deleted
	return !System_rmdir (path_url);
}


void File_Rmdir_List (clist_t *list_of_urls)
{
	clist_t *cur;
	for (cur = list_of_urls; cur; cur = cur->next)
		File_Rmdir (cur->name);
}


cbool File_Rename (const char* path_to_file, const char* new_name_url)
{
	cbool success = (rename(path_to_file, new_name_url) == 0);

	if (success == false)
		perrorf ("Error in renaming file.");

	return success;
}

cbool File_Delete (const char *path_to_file)
{
	cbool success = remove(path_to_file) != -1;

	return success;
}

void File_Delete_List (clist_t *list_of_urls)
{
	clist_t *cur;
	for (cur = list_of_urls; cur; cur = cur->next)
		File_Delete (cur->name);

}





///////////////////////////////////////////////////////////////////////////////
//  FILE MEMORY:  Memory_To_File and Memory_From_File
///////////////////////////////////////////////////////////////////////////////


cbool File_Memory_To_File (const char* path_to_file, byte *data, size_t numbytes)
{
	FILE* fout = core_fopen_write (path_to_file, "wb");

	if (!fout)											return false;
	if ( fwrite (data, 1, numbytes, fout) != numbytes)	return false;

	core_fclose (fout);

	return true;
}


void* File_To_Memory_Offset_Alloc (const char *path_to_file, size_t *numbytes, size_t offset_into_file, size_t read_len)
{
	size_t filesize, read_end;
	FILE *f;

	// If we find the file size is 0, we get out
	filesize = File_Length (path_to_file);
	if (!filesize) return NULL;

	// If len = 0 is specified, we read as much as possible
	if (read_len == 0) read_len = (filesize - offset_into_file );

	// Calc the read end, make sure file is long enough for read we want
	read_end = offset_into_file + read_len;
	if (filesize < read_end) return NULL;

	// Open file
	f = core_fopen_read (path_to_file, "rb");
	if (!f)
		return NULL;
	else
	{
		int ret = fseek (f, offset_into_file, SEEK_SET);
		byte *membuf = malloc (read_len);
		size_t bytes_read = fread (membuf, 1, read_len, f);

		// If numbytes read did not meet expectations say it
		if (bytes_read != read_len) Core_Warning ("bytesread != filesize: did not read entire file!");

		// Close file
		core_fclose (f);

		// If caller requested to know amount read, report it back
		if (numbytes != NULL) *numbytes = bytes_read;

		return membuf;
	}
}


void* File_To_Memory_Alloc (const char* path_to_file, size_t *numbytes)
{
	return File_To_Memory_Offset_Alloc (path_to_file, numbytes, 0 /* offset of none*/, 0 /*len 0 means read all*/);

/* Obsolete but may be nice for reference because is simple
	FILE *f = core_fopen_read (path_to_file, "rb");

	if (f)
	{
		size_t	filesize	= FileHandle_GetLength (f);
		byte	*membuf		= malloc (filesize);
		size_t	bytes_read	= fread (membuf, 1, filesize, f);

		if (bytes_read != filesize)
			Core_Warning ("bytesread != filesize: did not read entire file!");

		core_fclose (f);

		if (numbytes != NULL)  // A function might not request this be written
			*numbytes = bytes_read; //filesize; // Optimistic?  What if fread didn't read whole thing (rare I know )...

		return membuf;
	}

	return NULL;
*/
}

///////////////////////////////////////////////////////////////////////////////
//  FILE HANDLE OPS:  File handle manipulation
///////////////////////////////////////////////////////////////////////////////

#if 0  // My notes say use File_Length instead
size_t FileHandle_GetLength (FILE* filehandle)
{
	size_t filelen;

	fseek (filehandle, 0, SEEK_END);	// Go to end
	filelen = ftell(filehandle);		// Read position, that is file length

	fseek (filehandle, 0, SEEK_SET);	// Set to start
	return filelen;
}
#endif

size_t FileHandle_Block_Copy (FILE* fdst, FILE* fsrc, size_t len)
{
	char   buf[4096];
	size_t bufsize = sizeof(buf);
	size_t written, remaining, bytes_this_pass;
	size_t bytes_in, bytes_out;

	for (remaining = len, written = 0; remaining > 0; /* nothing */ )
	{
		bytes_this_pass = (remaining < bufsize) ? remaining : bufsize;

		remaining -= (bytes_in  = fread  ( buf, 1, bytes_this_pass, fsrc));  // read 4096 or less
		written   += (bytes_out = fwrite ( buf, 1, bytes_this_pass, fdst));  // write 4096 bytes or less

		if (!bytes_in || !bytes_out || bytes_in !=bytes_this_pass || bytes_out != bytes_this_pass)
		{
			// Core_Printf ("Couldn't write all data");
			return 0; // Something bad happened
		}
	}

	return written;
}

size_t FileHandle_Block_To_File (FILE* fsrc, size_t len, const char *path_to_file)
{
	FILE *fdst = core_fopen_write (path_to_file, "wb");

	if (fdst)
	{
		size_t written = FileHandle_Block_Copy (fdst, fsrc, len);

		core_fclose (fdst);
		return written;
	}
	else return 0; // Error
}


size_t FileHandle_Append_File (FILE* fdst, size_t len, const char *path_to_file)
{
	FILE *fsrc = core_fopen_write (path_to_file, "rb");

	if (fsrc)
	{
		size_t written = FileHandle_Block_Copy (fdst, fsrc, len);

		core_fclose (fsrc);
		return written;
	}  else return 0; // Error
}


///////////////////////////////////////////////////////////////////////////////
//  FILE STRING OPERATIONS:  Baker - string operations for file URLs
///////////////////////////////////////////////////////////////////////////////

// If no extension, add it
void File_URL_Edit_Default_Extension (char *path_to_file, const char *dot_new_extension, size_t len)
{
	const char *extension = File_URL_GetExtension (path_to_file);

	if (extension[0] == 0)
	{
		// No extension so default it
		strlcat (path_to_file, dot_new_extension, len);
	}
}

// Removes and changes the extension if necessary
void File_URL_Edit_Change_Extension (char *path_to_file, const char *dot_new_extension, size_t len)
{
	const char *extension = File_URL_GetExtension (path_to_file);

	if (extension[0] == 0 || strcasecmp (extension, dot_new_extension))
	{
		File_URL_Edit_Remove_Extension (path_to_file);
		strlcat (path_to_file, dot_new_extension, len);
	}
}

// If no extension or the extension isn't right, add it
void File_URL_Edit_Force_Extension (char *path_to_file, const char *dot_new_extension, size_t len)
{
	const char *extension = File_URL_GetExtension (path_to_file);

	if (extension[0] == 0 || strcasecmp (extension, dot_new_extension))
	{
		// 1) No extension so default it
		// 2) Doesn't match so append it
		strlcat (path_to_file, dot_new_extension, len);
	}
}



char* File_URL_Edit_Reduce_To_Parent_Path (char* path_to_file)
{
	char* terminate_point = strrchr (path_to_file, '/');

	if (terminate_point)
		*terminate_point = '\0';

	return path_to_file;
}

char *File_URL_Edit_Remove_Any_Trailing_Slash (char *path_to_file)
{
	if (String_Does_End_With (path_to_file, "/"))
		path_to_file[strlen(path_to_file)-1] = 0; // Null it out

	return path_to_file;
}


char* File_URL_Edit_Remove_Extension (char *path_to_file)
{
	char* terminate_point = strrchr (path_to_file, '.');

	if (terminate_point)
		*terminate_point = '\0';

	return path_to_file;
}


// Turns c:\quake\id1 into c:/quake/id1
char* File_URL_Edit_SlashesForward_Like_Unix (char *windows_path_to_file)
{
	return String_Edit_Replace_Char (windows_path_to_file, '\\' /*find*/, '/' /*replace with*/, NULL /* don't want count */);
}


// Turns c:\quake\id1 into c:/quake/id1
char* File_URL_Strdup_SlashesForward_Like_Unix (const char *windows_path_to_file)
{
	char *s_o = strdup(windows_path_to_file);
	File_URL_Edit_SlashesForward_Like_Unix (s_o);

	return s_o;
}

// Turns c:/quake/id1 into c:\quake\id1
char* File_URL_Strdup_SlashesBack_Like_Windows (const char *unix_path_to_file)
{
	char *s_o = strdup(unix_path_to_file);
	File_URL_Edit_SlashesBack_Like_Windows (s_o);

	return s_o;
}

// Turns c:/quake/id1 into c:\quake\id1
char* File_URL_Edit_SlashesBack_Like_Windows (char *unix_path_to_file)
{
	// Translate "/" to "\"
	return String_Edit_Replace_Char (unix_path_to_file, '/' /*find*/, '\\' /*replace with*/, NULL /* don't want count */);
}


static const char *sFile_URL_SkipChar (const char *path_to_file, char skipme)
{
	const char* found_char = strrchr (path_to_file, skipme);

	if (found_char)
		return &found_char[1]; // path + 1

	return path_to_file; // Wasn't found
}

// Baker: This function does unix and window style paths
void File_URL_Copy_StripExtension (char *dst, const char *src, size_t siz)
{
	int	length;

	if (!*src)
	{
		*dst = '\0';
		return;
	}
	if (src != dst)	/* copy when not in-place editing */
		strlcpy (dst, src, siz);
	length = (int)strlen(dst) - 1;
	while (length > 0 && dst[length] != '.')
	{
		--length;
		if (dst[length] == '/' || dst[length] == '\\')
			return;	/* no extension */
	}
	if (length > 0)
		dst[length] = '\0';
}

// Doesn't return NULL rather emptystring, return the dot (".png", etc ..)
const char *File_URL_GetExtension (const char *path_to_file)
{
	const char *finddot = strrchr (path_to_file, '.');

	// Make sure we found it and that what we found wasn't a dot
	// in path
	if (finddot && strchr (finddot, '/') == NULL )
	{
		return finddot;
	}

	return "";
}

enum image_type_e File_URL_Image_Type (const char *path_to_file)
{
	const char *extension = File_URL_GetExtension (path_to_file);

	if (!strcasecmp (".png", extension)) return image_type_png;
	if (!strcasecmp (".tga", extension)) return image_type_tga;
	if (!strcasecmp (".pcx", extension)) return image_type_pcx;
	if (!strcasecmp (".jpg", extension)) return image_type_jpeg;
	if (!strcasecmp (".jpeg", extension)) return image_type_jpeg;
	return 0;
}



cbool File_URL_Is_Relative (const char *path_to_file)
{
	return System_File_URL_Is_Relative (path_to_file);
}


const char *File_URL_SkipPath (const char *path_to_file)
{
	return sFile_URL_SkipChar (path_to_file, '/');
}


///////////////////////////////////////////////////////////////////////////////
//  FILE STRING OPERATIONS:  Baker - string operations for file URLs
///////////////////////////////////////////////////////////////////////////////

clist_t* sFile_List_Alloc (const char *folder_url, const char *extension, cbool relative_url)
{
	// List files in folder

	DIR		*dir_p = opendir (folder_url);
	clist_t *list = NULL;
	const char *full_url;

	struct dirent	*dir_t;

	if (!dir_p)
		return NULL;

	while ((dir_t = readdir(dir_p)) != NULL)
	{
		if (dir_t->d_name[0] == '.')
			continue; // Do not want

		full_url = va("%s/%s", folder_url, dir_t->d_name);

		if (File_Is_Folder (full_url))
			continue; // Just files, not folders

		if (extension && !String_Does_End_With_Caseless(full_url, extension))
			continue; // Not what we are looking for

		List_Add (&list, relative_url ? dir_t->d_name : full_url);
	}
	closedir(dir_p);
	return list;
}

clist_t * File_List_Alloc (const char *folder_url, const char *extension)
{
	return sFile_List_Alloc (folder_url, extension, false /* not relative*/ );
}

clist_t * File_List_Relative_Alloc (const char *folder_url, const char *extension)
{
	return sFile_List_Alloc (folder_url, extension, true /* relative url */);
}




static void sFile_List_Recursive_Alloc (clist_t **list, const char *folder_url, int skipchars, const cbool wants_relative_url, cbool wants_dirs)
{
	char full_url[MAX_OSPATH];

	DIR		*dir_p = opendir (folder_url);
	struct dirent	*dir_t;

	if (dir_p)
	{
		while ( (dir_t = readdir(dir_p)) )
		{
			if (dir_t->d_name[0] == '.')
				continue; // Do not want

			c_snprintf2 (full_url, "%s/%s", folder_url, dir_t->d_name);

			switch (File_Is_Folder (full_url))
			{
			case true: // Folder
				sFile_List_Recursive_Alloc (list, full_url, skipchars, wants_relative_url, wants_dirs /* we don't want folders */);
				if (wants_dirs)
					List_Add (list, &full_url[skipchars]); // Adding after should allow us to more easily remove folders.

				break;
			case false: // File
				List_Add (list, &full_url[skipchars]);
			}
		}
		closedir(dir_p);
	}
}

clist_t * File_List_Recursive_Alloc (const char *folder_url)
{
	// 0123456789
	// c:/mydir  .. skip chars = 0 for full url, 9 for relative
	// 0123456789
	// c:/mydir/myfile.abc  [0] = full, [9] = myfile.abc
	clist_t *list = NULL;
	int skipchars = 0;//strlen (folder_url) + 1;

	sFile_List_Recursive_Alloc (&list, folder_url, skipchars, false /*absolute urls*/, false /* we don't want folders */);

	return list;
}

clist_t * File_List_Dirs_Recursive_Alloc (const char *folder_url)
{
	// 0123456789
	// c:/mydir  .. skip chars = 0 for full url, 9 for relative
	// 0123456789
	// c:/mydir/myfile.abc  [0] = full, [9] = myfile.abc
	clist_t *list = NULL;
	int skipchars = 0;//strlen (folder_url) + 1;

	sFile_List_Recursive_Alloc (&list, folder_url, skipchars, false /*absolute urls*/, true /* we don't want folders */);

	return list;
}


clist_t * File_List_Recursive_Relative_Alloc (const char *folder_url)
{
	// 0123456789
	// c:/mydir  .. skip chars = 0 for full url, 9 for relative
	// 0123456789
	// c:/mydir/myfile.abc  [0] = full, [9] = myfile.abc
	clist_t *list = NULL;
	int skipchars = strlen (folder_url) + 1;

	sFile_List_Recursive_Alloc (&list, folder_url, skipchars, true /*absolute urls*/, false /* we don't want folders */);

	return list;
}


///////////////////////////////////////////////////////////////////////////////
//  FILE DIALOG OPERATIONS: PROMPT FOR A FILE OR FOLDER
///////////////////////////////////////////////////////////////////////////////

const char * File_Dialog_Open_Directory (const char *title, const char *starting_folder_url)
{
	return System_Dialog_Open_Directory (title, starting_folder_url);
}

const char * File_Dialog_Open_Type (const char *title, const char * starting_folder_url, const char *extensions_comma_delimited)
{
	return System_Dialog_Open_Type (title, starting_folder_url, extensions_comma_delimited);
}

const char * File_Dialog_Save_Type (const char *title, const char * starting_file_url, const char *extensions_comma_delimited)
{
	return System_Dialog_Save_Type (title, starting_file_url, extensions_comma_delimited);
}



