/*
Copyright (C) 2011-2014 Baker

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
// core_linux.c -- Linux platform interface


#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <errno.h>




#define CORE_LOCAL

#include "environment.h"
#include "core.h"



///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: URL QUERY
///////////////////////////////////////////////////////////////////////////////

// Baker: Function is unused currently
const char * System_URL_Binary (void)
{
	static char binary_url[MAX_OSPATH];
	pid_t pid = getpid();
	int ret;
	
	char linkname[256];
	c_snprintf (linkname, "/proc/%i/exe", pid);

	ret = readlink (linkname, binary_url, sizeof(binary_url)-1);
	
    // In case of an error, leave the handling up to the caller
    if (ret == -1 || ret >= sizeof(binary_url) )
	{
		Core_Error ("Couldn't determine executable directory");
    	return NULL;
	}

    binary_url[ret] = 0;

	System_Alert ("%s\n", buffer);

	return (const char*)binary_url;
}


const char * System_URL_Binary_Folder (void)
{
	static char binary_folder_url[MAX_OSPATH];
	const char* binary_url = System_URL_Binary ();

	c_strlcpy (binary_folder_url, binary_url);
	File_URL_Edit_Reduce_To_Parent_Path (binary_folder_url);

	return binary_folder_url;
}

/* Quakespasm
	size_t		n;
	const char	*home_dir = NULL;
	struct passwd	*pwent;

	pwent = getpwuid( getuid() );
	if (pwent == NULL)
		perror("getpwuid");
	else
		home_dir = pwent->pw_dir;
	if (home_dir == NULL)
		home_dir = getenv("HOME");
	if (home_dir == NULL)
		Sys_Error ("Couldn't determine userspace directory");

/* what would be a maximum path for a file in the user's directory...
 * $HOME/SYS_USERDIR/game_dir/dirname1/dirname2/dirname3/filename.ext
 * still fits in the MAX_OSPATH == 256 definition, but just in case :
 */
	n = strlen(home_dir) + strlen(SYS_USERDIR) + 50;
	if (n >= dstsize)
		Sys_Error ("Insufficient array size for userspace directory");

	q_snprintf (dst, dstsize, "%s/%s", home_dir, SYS_USERDIR);
}
*/


static const char * sSystem_URL_AppData (void)
{
Error
	static char appdatapath[MAX_OSPATH];
	char *_appdata = getenv("APPDATA");
 * $HOME/SYS_USERDIR/game_dir/dirname1/dirname2/dirname3/filename.ext
	c_strlcpy (appdatapath, _appdata);
	File_URL_Edit_SlashesForward_Like_Unix (appdatapath);

	return appdatapath;
}


static const char *sSystem_URL_Caches_By_AppName (const char* appname)
    {
        static char cachesfolder[MAX_OSPATH];
        c_strlcpy (cachesfolder, "./_caches"); // I hope this works ok.
        return cachesfolder;
    }


const char * System_URL_Caches (void)
{
	return sSystem_URL_Caches_By_AppName (gCore_Appname);
}

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE DIALOGS
///////////////////////////////////////////////////////////////////////////////

//static NSString *lastOpenDirectory;

const char * System_Dialog_Open_Directory (const char *title, const char *starting_folder_url)
{
	static char directorySelected[MAX_OSPATH];

	directorySelected[0] = 0;

	return directorySelected;
}


const char * System_Dialog_Open_Type (const char *title, const char * starting_folder_url, const char *extensions_comma_delimited)
{
	static char fileSelected[MAX_OSPATH];
//
//	fileSelected[0] = 0;
//
//	// Determine permitted extensions
//	NSArray *allowedExtensions = nil;
//
//	if (extensions_comma_delimited)
//	{
//		NSString *extensionsList = [NSString stringWithUTF8String:extensions_comma_delimited];
//		allowedExtensions = [extensionsList componentsSeparatedByString:@","];
//	}
//
//	// Determine starting directory
//	NSString *startingDirectory = nil;
//
//	if (starting_folder_url)
//		startingDirectory = [NSString stringWithUTF8String:starting_folder_url];
//	else if ([lastOpenDirectory length] == 0)
//		startingDirectory = [NSString stringWithUTF8String:Folder_Binary_Folder_URL()];
//	else startingDirectory = [lastOpenDirectory copy];
//
//	NSURL *startingDirectoryURL = [NSURL URLWithString:[startingDirectory
//														stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
//
//	// Determine title string
//	NSString *titleString = @"Select Folder";
//
//	if (title)
//		titleString = [NSString stringWithUTF8String:title];
//
//	NSAutoreleasePool*  pool    = [[NSAutoreleasePool alloc] init];
//	NSOpenPanel*        panel   = [[[NSOpenPanel alloc] init] autorelease];
//
//
//	[panel setDirectoryURL:startingDirectoryURL];
//	[panel setAllowsMultipleSelection: NO];
//	[panel setCanChooseFiles: YES];
//	[panel setCanChooseDirectories: NO];
//	[panel setTitle: titleString];
//
//	// Conditionally limit file extension
//	if (extensions_comma_delimited)
//		[panel setAllowedFileTypes:allowedExtensions];
//
//	if ([panel runModal])
//	{
//		// Ok button was selected
//		NSString *_directorySelected = [[panel directoryURL] path]; // Directory result
//		c_strlcpy (fileSelected, [_directorySelected cStringUsingEncoding: NSASCIIStringEncoding]);
//
//		// Get the URL and convert it to an NSString without URL encoding (i.e. no %20 as a space, etc.)
//		NSURL    *_selectedFileURL   = [[panel URLs]objectAtIndex:0];
//		NSString *_selectedFile      = [NSString stringWithFormat:@"%@", _selectedFileURL];
//		NSString *_selectedFile2     = [_selectedFile stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
//		NSString *selectedFile       = [_selectedFile2 lastPathComponent];
//
//		c_strlcat (fileSelected, "/");
//		c_strlcat (fileSelected, [selectedFile cStringUsingEncoding: NSASCIIStringEncoding]);
//
//		lastOpenDirectory = [_directorySelected copy];
//	}
//
//	[pool release];
	return fileSelected;
}


const char * System_Dialog_Save_Type (const char *title, const char * starting_folder_url, const char *extensions_comma_delimited)
{
	return NULL;
}


#if 0
const char * File_Dialog_Open (const char *title, const char * starting_folder_url)
{
	// Determine starting directory
	NSString *startingDirectory;
	if (starting_folder_url)
		startingDirectory = [NSString stringWithUTF8String:starting_folder_url];
	else if [startingDirectory length] == 0
		startingDirectory = [NSString stringWithUTF8String:Folder_Binary_Folder_URL()];
	else startingDirectory = lastOpenDirectory;


}
#endif


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE MANAGER INTERACTION
///////////////////////////////////////////////////////////////////////////////


cbool System_Folder_Open (const char *path_url)
{
	execl ("/usr/bin/xdg-open", "xdg-open", path_url, (char *)0);

    return true;
}


cbool System_Folder_Open_Highlight (const char *path_to_file)
{
	char folder_url[MAX_OSPATH];

	c_strlcpy (binary_folder_url, path_to_file);
	File_URL_Edit_Reduce_To_Parent_Path (folder_url);
	return System_Folder_Open (folder_url); // Doesn't highlight at this time



    //http://stackoverflow.com/questions/7652928/launch-osx-finder-window-with-specific-files-selected
    return true;
}


cbool System_Folder_Open_Highlight_Binary (void)
{
	const char* binary_url = System_URL_Binary ();

	return System_Folder_Open_Highlight (binary_url);
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE AND DIRECTORY MANAGEMENT
///////////////////////////////////////////////////////////////////////////////

void System_chdir (const char *path_url)
{
    chdir (path_url);
}

//Get current working directory.
const char * System_getcwd (void)
{
    static char workingdir[MAX_OSPATH];

	// Improve this
    if (!getcwd (workingdir, sizeof(workingdir) - 1))
		return NULL;

    return workingdir;
}


void System_mkdir (const char *path_url)
{
    if (mkdir (path_url, 0777) == -1)
    {
        if (errno != EEXIST)
        {
            Core_Error ("\"mkdir %s\" failed, reason: \"%s\".", path_url, strerror (errno));
        }
    }
}


int System_rmdir (const char *path_url)
{
	return rmdir (path_url);
}


cbool System_File_Exists (const char *path_to_file)
{
	struct stat st_buf = {0};
	int status = stat (path_to_file, &st_buf );

	if (status != 0)
		return false;

	return true;
}


cbool System_File_Is_Folder (const char* path_to_file)
{
    struct stat st_buf = {0};
	int status = stat (path_to_file, &st_buf);

	if (status != 0)
		return false;
		
    if (S_ISREG (st_buf.st_mode))
		return false; // Is regular file ...
    
    if (S_ISDIR (st_buf.st_mode))
        return true; // directory

    return false;
}


// File length
size_t System_File_Length (const char *path_to_file)
{
	struct stat st_buf = {0};
	int status = stat (path_to_file, &st_buf );
	if (status != 0)
		return 0;

	return st_buf.st_size;
}


// Returns the seconds since midnight 1970
double System_File_Time (const char *path_to_file)
{
	struct stat st_buf = {0};

	int status = stat (path_to_file, &st_buf );
	if (status != 0)
		return 0;

	return (double)st_buf.st_mtime;
}

cbool System_File_URL_Is_Relative (const char *path_to_file)
{
	return path_to_file[0]!='/'; // Right?  That's it isn't it.
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: CLIPBOARD OPERATIONS
///////////////////////////////////////////////////////////////////////////////

const char *System_Clipboard_Get_Text_Alloc (void)
{
// Using SDL2 functions.  
// Cheesy?  Yes.  But the XWindow functions are a little choatic.
// And I don't feel like dealing with that right now.
	const char *test = SDL_GetClipboardText();  //(!SDL_HasClipboardText())
	const char *out  = test ? core_strdup (test) : NULL;
	
	return out;
}




// copies given text to clipboard
cbool System_Clipboard_Set_Text (const char *text_to_clipboard)
{
    if (text_to_clipboard == NULL)
    {
//        Message_Warning("System_Clipboard_Set_Text: Null string");
        return false;
    }

	// SDL_SetClipboardText returns 0 on success
	return !SDL_SetClipboardText(text_to_clipboard);
}


cbool System_Clipboard_Set_Image_RGBA (const unsigned *rgba_data, int width, int height)
{
	return false;
}



unsigned *System_Clipboard_Get_Image_RGBA_Alloc (int *outwidth, int *outheight)
{

        return NULL;
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: MESSAGEBOX
///////////////////////////////////////////////////////////////////////////////


#include <SDL2/SDL.h> // Cheating
void System_MessageBox (const char *_title, const char *fmt, ...)
{
	const char *title = _title ? _title : "Alert";
	VA_EXPAND (text, SYSTEM_STRING_SIZE_1024, fmt)

    SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_INFORMATION, title, text, NULL);
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: PROCESSES
///////////////////////////////////////////////////////////////////////////////

// Changes working directory
sys_handle_t System_Process_Create (const char *path_to_file, const char *args, const char* working_directory_url)
{
	// Sanity check
	if (!File_Exists (path_to_file))
		return NULL;

	if (working_directory_url && !File_Exists (working_directory_url))
		return NULL;

	// Change the working dir
	const char *_workdir = working_directory_url ? working_directory_url : Folder_Binary_Folder_URL ();
	System_chdir(_workdir);

	/*
	 TASK SETUP
	*/



	return (sys_handle_t)0 /*task*/;

}

// Works reliably!
int System_Process_Still_Running (sys_handle_t pid)
{
//	NSTask *task = (NSTask*)pid;
//	if ([task isRunning])
//		return 1;

	return 0;
}



int System_Process_Terminate_Console_App (sys_handle_t pid)
{
#pragma message ("Make this")
	return 0;
}


int System_Process_Close (sys_handle_t pid)
{
	#pragma message ("Make this")
/*

	NSTask *task = (NSTask*)pid;

	int status = [aTask terminationStatus];
		if (status == ATASK_SUCCESS_VALUE)
			NSLog(@"Task succeeded.");
		else
			NSLog(@"Task failed.");
	}	return 0;  // No longer running; completed
}
*/
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: TIME
///////////////////////////////////////////////////////////////////////////////

// Returns seconds since 1970
double System_Time (void)
{
	time_t t1 = time (NULL);
	return (double) t1;
}

void System_Sleep (unsigned long milliseconds)
{
    // Usleep works in microseconds, not milliseconds
    usleep (milliseconds * 1000);
}


void * System_GetProcAddress (const char *pfunction_name)
{
//    void* psymbol = dlsym (RTLD_DEFAULT, pfunction_name);
//
//    if (psymbol == NULL)
//    {
//        Core_Error ("Failed to import a required function!");
//    }

    return NULL; //psymbol;
}

//
// End of File
//

