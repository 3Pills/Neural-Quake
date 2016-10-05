/*
Copyright (C) 2011-2014 Baker

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
// core.h -- core functions

/* NOT True Core ...
core_opengl.h
gl_constants.h
libcurl.dll
math_matrix.c
math_vector.c
unzip_win.cpp
unzip_win.h
zip_win.cpp
zip_win.h
zlib1.dll
*/

#ifndef __CORE_H__
#define __CORE_H__

#ifdef __GNUC__
	#pragma GCC diagnostic ignored "-Wmissing-field-initializers" // Not a fan of disabling this but messes with struct x mystruct = {0};
#endif // __GNUC__

// #define CORE_LIBCURL

/*
** Allocations in Core SHALL be made with C allocators calloc, free, etc.
**
** Note that Corex and Corex_GL extensions to Core follow different rules.
*/


#define __CORE_INCLUDED__

#if defined (DEBUG) && !defined (_DEBUG)
	#define _DEBUG
#endif // Keep everything consistent across platforms

#include "environment.h"

#include <stdio.h> // fopen, etc.
#include <stdlib.h> // malloc, etc.

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> // _stat, etc.
#include <time.h>
#include <ctype.h> // islower, etc.

#ifdef PLATFORM_WINDOWS
	#include <io.h>
	#include <direct.h> // _chdir, etc.
	#include "dirent_win.h" // dirent.h for Windows
#endif // PLATFORM_WINDOWS

#ifndef PLATFORM_WINDOWS
#include <dirent.h>
#include <dlfcn.h>
#endif // ! PLATFORM_WINDOWS

#ifdef CORE_PTHREADS
#include "pthreads_core.h"
#endif // PTHREADS



#include <stdarg.h> // GCC wanted this

#include "lists.h"
#include "file.h"
//#include "image.h" // Moved to below function declares ...
#include "interface.h"
#include "math_general.h"
#include "math_vector.h"
#include "math_matrix.h"
#include "pak.h"
#include "stringlib.h"
#include "base64.h"
#include "zip.h"
// #include "download.h" // Moved to below function declares
#include "enumbits.h"
#include "timelib.h"
#include "music.h"
#include "links.h"
#include "memchain.h"

///////////////////////////////////////////////////////////////////////////////
//  CORE: Basic function setup
///////////////////////////////////////////////////////////////////////////////

typedef void (*error_fn_t) (const char *error, ...); // __attribute__((__format__(__printf__,1,2), __noreturn__));
typedef int (*print_fn_t) (const char *fmt, ...); // __fp_attribute__((__format__(__printf__,1,2)));

typedef FILE * (*fopenread_fn_t) (const char *, const char *);
typedef FILE * (*fopenwrite_fn_t) (const char *, const char *);
typedef int (*fclose_fn_t) (FILE*);

typedef void * (*malloc_fn_t) (size_t);
typedef void * (*calloc_fn_t) (size_t, size_t);
typedef void * (*realloc_fn_t)(void *, size_t);
typedef char * (*strdup_fn_t) (const char*);
typedef void (*free_fn_t)(void *);


typedef struct
{
	error_fn_t		ferror_fn;
	print_fn_t		fwarning_fn;
	print_fn_t		fprint_fn;
	print_fn_t		fdprint_fn;

	malloc_fn_t		fmalloc_fn;
	calloc_fn_t		fcalloc_fn;
	realloc_fn_t	frealloc_fn;
	strdup_fn_t		fstrdup_fn;

	free_fn_t		ffree_fn;

	fopenread_fn_t	ffopenread_fn;
	fopenwrite_fn_t	ffopenwrite_fn;
	fclose_fn_t		ffclose_fn;
} fn_set_t;


// Initializer, application passes function set.  appname is important
// and affects appdata folder names and window titles.

#include "image.h" // Moved to below function declares ...
#include "download.h" // Moved to below function declares ...

void Core_Init (const char *appname, fn_set_t *fnset, sys_handle_t handle );


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: MESSAGEBOX
///////////////////////////////////////////////////////////////////////////////

int System_MessageBox (const char *title, const char *fmt, ...);
int System_Alert (const char *fmt, ...);

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: PROCESSES
///////////////////////////////////////////////////////////////////////////////

sys_handle_t System_Process_Create (const char *path_to_file, const char *args, const char *working_directory_url);
int System_Process_Still_Running (sys_handle_t pid);
int System_Process_Terminate_Console_App (sys_handle_t pid);
int System_Process_Close (sys_handle_t pid);

#ifdef PLATFORM_WINDOWS
#define System_GL_GetProcAddress wglGetProcAddress
#else // not PLATFORM_WINDOWS ...
void * System_GetProcAddress (const char *pfunction_name);
#define System_GL_GetProcAddress System_GetProcAddress
#endif // !PLATFORM_WINDOWS

// #define FSOUND_GETFUNC(f, g) (qFSOUND_##f = (void *)GetProcAddress(fmod_handle, "_FSOUND_" #f #g))
// #define FSOUND_GETFUNC(f, g) (qFSOUND_##f = (void *)dlsym(fmod_handle, "FSOUND_" #f))




#ifdef CORE_LOCAL

///////////////////////////////////////////////////////////////////////////////
//  CORE: Private Local Shared
///////////////////////////////////////////////////////////////////////////////

char gCore_Appname[MAX_OSPATH];
sys_handle_t gCore_hInst; // Why is this a pointer?  Undid that.
sys_handle_t gCore_Window; // DO NOT LIKE

extern fopenread_fn_t	core_fopen_read;
extern fopenwrite_fn_t	core_fopen_write;
extern fclose_fn_t		core_fclose;

extern malloc_fn_t		core_malloc;
extern calloc_fn_t		core_calloc;
extern realloc_fn_t		core_realloc;
extern strdup_fn_t		core_strdup;
extern free_fn_t		_core_free;

extern error_fn_t		Core_Error; // Terminate
extern print_fn_t		Core_Warning; // Special notification
extern print_fn_t		Core_Printf; // Informational notification
extern print_fn_t		Core_DPrintf; // Secondary notification

void* core_free (const void* ptr); // Returns null

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: TIME
///////////////////////////////////////////////////////////////////////////////

void System_Sleep (unsigned long milliseconds);
double System_Time (void);
double System_Time_Now_Precise (void); // no set metric

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: URL QUERY
///////////////////////////////////////////////////////////////////////////////

const char * System_URL_Binary (void);
const char * System_URL_Binary_Folder (void);
const char * System_URL_Caches (void);

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE DIALOGS PROMPT FOR A FILE OR FOLDER
///////////////////////////////////////////////////////////////////////////////

const char * System_Dialog_Open_Directory (const char *title, const char *starting_folder_url);
const char * System_Dialog_Open_Type (const char *title, const char * starting_folder_url, const char *extensions_comma_delimited);

// starting_file_url is default save name, add a "/" merely to suggest starting folder
const char * System_Dialog_Save_Type (const char *title, const char * starting_file_url, const char *extensions_comma_delimited);


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE MANAGER INTERACTION
///////////////////////////////////////////////////////////////////////////////

cbool System_Folder_Open (const char *path_url);
cbool System_Folder_Open_Highlight (const char *path_to_file);
cbool System_Folder_Open_Highlight_Binary (void);

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE AND DIRECTORY MANAGEMENT
///////////////////////////////////////////////////////////////////////////////

void System_chdir (const char *path_url); // change dir
const char *System_getcwd (void); // current working directory
void System_mkdir (const char *path_url); // make dir
int System_rmdir (const char *path_url); // remove dir
cbool System_File_Exists (const char *path_to_file); // file existence
cbool System_File_Is_Folder (const char *path_to_file);
size_t System_File_Length (const char *path_to_file);
double System_File_Time (const char *path_to_file);
cbool System_File_URL_Is_Relative (const char *path_to_file);

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: CLIPBOARD OPERATIONS
///////////////////////////////////////////////////////////////////////////////

#define SYS_CLIPBOARD_SIZE_256 256
const char *System_Clipboard_Get_Text_Line (void);
const char *System_Clipboard_Get_Text_Alloc (void);
cbool System_Clipboard_Set_Text (const char *text_to_clipboard);
cbool System_Clipboard_Set_Image_RGBA (const unsigned *rgba, int width, int height);
unsigned *System_Clipboard_Get_Image_RGBA_Alloc (int *outwidth, int *outheight);

#endif // CORE_LOCAL

#endif // ! __CORE_H__



