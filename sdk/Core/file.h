/*
Copyright (C) 2012-2014 Baker

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
// file.h -- file functions

#ifndef __FILE_H__
#define __FILE_H__

#include "environment.h"
#include <stdio.h> // fopen, etc.

///////////////////////////////////////////////////////////////////////////////
//  FILE INFORMATION: Baker - These functions operate on a path_to_file
///////////////////////////////////////////////////////////////////////////////

cbool File_Exists (const char *path_to_file);
cbool File_Is_Folder (const char *path_to_file);
size_t File_Length (const char *path_to_file);
double File_Time (const char *path_to_file); // Returns the seconds since midnight 1970

///////////////////////////////////////////////////////////////////////////////
//  FILE ACTION: Baker - These functions operate on a path_to_file
///////////////////////////////////////////////////////////////////////////////

void File_Chdir (const char *path_url); // Change directory
const char *File_Getcwd (void); // Get current directory
void File_Mkdir (const char *path_url); // Make directory
void File_Mkdir_Recursive (const char *path_to_file); // Recursive mkdir, adding trailing "/" for entire path
cbool File_Rmdir (const char* path_url); // Returns true if successful.
void File_Rmdir_List (clist_t *list_of_urls);

cbool File_Rename (const char *path_to_file, const char *new_name_url);
cbool File_Delete (const char *path_to_file);

///////////////////////////////////////////////////////////////////////////////
//  FILE MEMORY:  Memory_To_File and Memory_From_File
///////////////////////////////////////////////////////////////////////////////

cbool File_Memory_To_File (const char *path_to_file, byte *data, size_t numbytes);
void* File_To_Memory_Alloc (const char *path_to_file, size_t *numbytes);

// Zero means read everything possible
void* File_To_Memory_Offset_Alloc (const char *path_to_file, size_t *numbytes, size_t offset_into_file, size_t len);

///////////////////////////////////////////////////////////////////////////////
//  FILE HANDLE OPS:  File handle manipulation
///////////////////////////////////////////////////////////////////////////////

//size_t FileHandle_GetLength (FILE* filehandle);  DO NOT USE.  Use file system method File_Length

// Takes an open file and writes out to new file.  Set position before calling
size_t FileHandle_Block_To_File (FILE* fsrc, size_t len, const char *path_to_file);

// Takes an open file and writes out to new file.  Set position before calling
size_t FileHandle_Append_File (FILE* fdst, size_t len, const char *path_to_file);

// Open file to open file
size_t FileHandle_Block_Copy (FILE* fdst, FILE* fsrc, size_t len);

///////////////////////////////////////////////////////////////////////////////
//  FILE STRING OPERATIONS:  Baker - string operations for file URLs
///////////////////////////////////////////////////////////////////////////////


void File_URL_Copy_StripExtension (char *dst, const char *src, size_t siz);
const char *File_URL_GetExtension (const char *path_to_file);
void File_URL_Edit_Default_Extension (char *path_to_file, const char *dot_extension, size_t len); // This appends if there is no extension
void File_URL_Edit_Change_Extension (char *path_to_file, const char *dot_new_extension, size_t len); // This removes and appends
void File_URL_Edit_Force_Extension (char *path_to_file, const char *dot_extension, size_t len); // This appends if no extension or wrong extension
cbool File_URL_Is_Relative (const char *path_to_file);
char *File_URL_Edit_Reduce_To_Parent_Path (char *path_to_file);
char *File_URL_Edit_Remove_Extension (char *path_to_file);
char *File_URL_Edit_Remove_Any_Trailing_Slash (char *path_to_file); // Removes a trailing unix slash if found
char *File_URL_Edit_SlashesForward_Like_Unix (char *windows_path_to_file);
char *File_URL_Edit_SlashesBack_Like_Windows (char *unix_path_to_file);
#define File_URL_Has_Extension(_filename, _extension) (String_Does_End_With_Caseless(_filename, _extension)

// Duplicators
char* File_URL_Strdup_SlashesBack_Like_Windows (const char *unix_path_to_file);
char* File_URL_Strdup_SlashesForward_Like_Unix (const char *windows_path_to_file);

enum image_type_e
{
	image_type_invalid,
	image_type_rgba, // Do not use
	image_type_png,
	image_type_tga,
	image_type_pcx,
	image_type_jpeg,
};

enum image_type_e File_URL_Image_Type (const char *path_to_file);

const char *File_URL_GetExtension (const char *path_to_file);
const char *File_URL_SkipPath (const char *path_to_file);

///////////////////////////////////////////////////////////////////////////////
//  FILE LIST OPERATIONS:  Returns a list
///////////////////////////////////////////////////////////////////////////////

void File_Delete_List (clist_t *list_of_urls);

clist_t * File_List_Alloc (const char *folder_url, const char *extension);
clist_t * File_List_Relative_Alloc (const char *folder_url, const char *extension);
clist_t * File_List_Recursive_Alloc (const char *folder_url); // No extension yet!  Is ok, will use pattern later.
clist_t * File_List_Recursive_Relative_Alloc (const char *folder_url); // No extension yet!


clist_t * File_List_Dirs_Recursive_Alloc (const char *folder_url);

///////////////////////////////////////////////////////////////////////////////
//  FILE DIALOG OPERATIONS: PROMPT FOR A FILE OR FOLDER
///////////////////////////////////////////////////////////////////////////////

const char * File_Dialog_Open_Directory (const char *title, const char *starting_folder_url);
const char * File_Dialog_Open_Type (const char *title, const char * starting_folder_url, const char *extensions_comma_delimited);

// starting_file_url is default save name, add a "/" merely to suggest starting folder
const char * File_Dialog_Save_Type (const char *title, const char * starting_file_url, const char *extensions_comma_delimited);


#endif	// ! __FILE_H__


