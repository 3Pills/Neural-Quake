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
// core.c -- core functions


#include "core.h"


///////////////////////////////////////////////////////////////////////////////
//  CORE: Basic function setup
///////////////////////////////////////////////////////////////////////////////

const char * const empty_string = "";


fopenread_fn_t	core_fopen_read;
fopenwrite_fn_t core_fopen_write;
fclose_fn_t		core_fclose;

malloc_fn_t		core_malloc;
calloc_fn_t		core_calloc;
realloc_fn_t	core_realloc;
strdup_fn_t		core_strdup;
free_fn_t		_core_free; // Because real core_free returns NULL

error_fn_t		Core_Error;
print_fn_t		Core_Warning;
print_fn_t		Core_DPrintf;
print_fn_t		Core_Printf;

char gCore_Appname[MAX_OSPATH];
sys_handle_t gCore_hInst;
sys_handle_t gCore_Window; // Focus window really.  I DO NOT LIKE THIS.  Works ok for single instance of Window.

#ifdef PLATFORM_WINDOWS
#include "core_windows.h" // GetModuleHandle
#endif // PLATFORM_WINDOWS

static fn_set_t default_function_set = { perrorf, printf, printf, printf, malloc, calloc, realloc, strdup, free, fopen, fopen, fclose } ;
void Core_Init (const char *appname, fn_set_t *fnset, sys_handle_t pmainwindow )
{
	if (!fnset) fnset = &default_function_set;
	c_strlcpy (gCore_Appname, appname);
	gCore_Window = pmainwindow;

#ifdef PLATFORM_WINDOWS
	gCore_hInst = GetModuleHandle(NULL);
#endif

	Core_Error	= fnset->ferror_fn;
	Core_Warning= fnset->fwarning_fn;
	Core_Printf	= fnset->fprint_fn;
	Core_DPrintf= fnset->fdprint_fn;

	core_malloc	= fnset->fmalloc_fn;
	core_calloc	= fnset->fcalloc_fn;
	core_realloc= fnset->frealloc_fn;
	core_strdup	= fnset->fstrdup_fn;
	_core_free	= fnset->ffree_fn;

	core_fopen_read	= fnset->ffopenread_fn;
	core_fopen_write = fnset->ffopenwrite_fn;
	core_fclose	= fnset->ffclose_fn;

#pragma message ("gCache folder -- Determine cache folder and get a session id.  Lock the session file in there fcntl.flock(fd, fcntl.LOCK_EX) or ")
//  Core_Printf ("Cache folder")
	/*
HANDLE WINAPI CreateFile(
  _In_      LPCTSTR lpFileName,
  _In_      DWORD dwDesiredAccess,
  _In_      DWORD dwShareMode,  <-------------------------- set to 0 and we have exclusive
  _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  _In_      DWORD dwCreationDisposition,
  _In_      DWORD dwFlagsAndAttributes,
  _In_opt_  HANDLE hTemplateFile
);*/
	Core_Printf ("Core initialized\n");
}

void *core_free (const void* ptr)
{
	_core_free ((void*)ptr);

	return NULL;
}

#pragma message ("Update the cache marker")
//void Core_Heartbeat (



