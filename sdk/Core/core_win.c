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
// core_win.c -- Windows platform interface

#define CORE_LOCAL

#include "core.h"
#include <windows.h> // GetCurrentDirectory, etc.
#include <direct.h> // _chdir, etc.
#include <sys/types.h>
#include <sys/stat.h> // _stat, etc.
#include <time.h>

#include "core.h"

#include <Shlwapi.h>  // PathIsRelative function

#pragma comment (lib, "shell32.lib")  // ShellExecute
#pragma comment (lib, "Shlwapi.lib")  // PathIsRelative function
#pragma comment (lib, "gdi32.lib")	  // CreateBitmap (clipboard image ops)


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: URL QUERY
///////////////////////////////////////////////////////////////////////////////

// Baker: Function is unused currently
const char * System_URL_Binary (void)
{
	static char binary_url[MAX_OSPATH];
	int ret;

	ret = GetModuleFileNameA (NULL, binary_url, sizeof(binary_url) - 1 );
	if (ret == 0)
	{
		Core_Error ("Couldn't determine executable directory");
		return NULL;
	}
	//MSDN: Windows XP:  The string is truncated to nSize characters and is not null-terminated.
	binary_url[ret] = 0;  

	File_URL_Edit_SlashesForward_Like_Unix (binary_url);

	return (const char*)binary_url;
}


const char * System_URL_Binary_Folder (void)
{
	static char binary_folder_url[MAX_OSPATH];
	const char *binary_url = System_URL_Binary ();

	c_strlcpy (binary_folder_url, binary_url);
	File_URL_Edit_Reduce_To_Parent_Path (binary_folder_url);

	return binary_folder_url;
}


static const char * sSystem_URL_AppData (void)
{
	static char appdatapath[MAX_OSPATH];
	char *_appdata = getenv("APPDATA");

	c_strlcpy (appdatapath, _appdata);
	File_URL_Edit_SlashesForward_Like_Unix (appdatapath);

	return appdatapath;
}


static const char * sSystem_URL_Caches_By_AppName (const char *appname)
{
#define CACHES_DIR_SUBPATH_OF_APPDATA_WINDOWS "caches"
	static char cachesfolder [MAX_OSPATH];

	c_snprintf3 (cachesfolder, "%s/%s/%s", sSystem_URL_AppData (), appname, CACHES_DIR_SUBPATH_OF_APPDATA_WINDOWS);
	return cachesfolder;
}


const char * System_URL_Caches (void)
{
	return sSystem_URL_Caches_By_AppName (gCore_Appname);
}

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE DIALOGS
///////////////////////////////////////////////////////////////////////////////

static void WIN_Fill_Extensions_Request (char *s, const char *extension_comma_list, size_t len)
{
	const char *allfiles0 = "All Files (*.*)";
	const char *allfiles1 = "*.*";

	clist_t * extensions = List_String_Split (extension_comma_list, ',');

	clist_t *cur;
	int count;

	for (cur = extensions, count = 0; cur; cur = cur->next)
	{
		const char *_description0 = (cur->name[0] && cur->name[1] ) ? &cur->name[1] : "";
		const char *_description1 = va ("%s", _description0);
		const char *description;

		String_Edit_To_Upper_Case ((char *)_description1); // Evile ...

		description = va("%s files (*%s)", _description1, cur->name);

		strcpy (&s[count], description); count += strlen(description); // Add, increment count
		s[count] = 0; count ++; // Add, increment count
		strcpy (&s[count], "*"); count ++; // Add, increment count
		strcpy (&s[count], cur->name); count += strlen(cur->name); // Add, increment count
		s[count] = 0; count ++; // Add, increment count
	}
	strcpy (&s[count], allfiles0); count += strlen(allfiles0); // Add, increment count
	s[count] = 0; count ++; // Add, increment count
	strcpy (&s[count], allfiles1); count += strlen(allfiles1); // Add, increment count
	s[count] = 0; count ++; // Add, increment count

	// Double NULL termination
	s[count] = 0;
	List_Free (&extensions);
}

static char m_last_open_directory[MAX_OSPATH];

const char * System_Dialog_Open_Directory (const char *title, const char *starting_folder_url)
{
	static char directorySelected[MAX_OSPATH];

	directorySelected[0] = 0;
#if 0

	// Determine starting directory
	NSString *startingDirectory = nil;

	if (starting_folder_url)
		startingDirectory = [NSString stringWithUTF8String:starting_folder_url];
	else if ([lastOpenDirectory length] == 0)
		startingDirectory = [NSString stringWithUTF8String:Folder_Binary_Folder_URL()];
	else startingDirectory = [lastOpenDirectory copy];

	NSURL *startingDirectoryURL = [NSURL URLWithString:[startingDirectory
														stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];

	// Determine title string
	NSString *titleString = @"Select Folder";

	if (title)
		titleString = [NSString stringWithUTF8String:title];

	NSAutoreleasePool*  pool    = [[NSAutoreleasePool alloc] init];
	NSOpenPanel*        panel   = [[[NSOpenPanel alloc] init] autorelease];


	[panel setDirectoryURL:startingDirectoryURL];
	[panel setAllowsMultipleSelection: NO];
	[panel setCanChooseFiles: NO];
	[panel setCanChooseDirectories: YES];
	[panel setTitle: titleString];

	if ([panel runModal])
	{
		// Ok button was selected
		NSString *_directorySelected = [[panel directoryURL] path]; // Directory result
		c_strlcpy (directorySelected, [_directorySelected cStringUsingEncoding: NSASCIIStringEncoding]);
		lastOpenDirectory = [_directorySelected copy];
	}

	[pool release];
#endif
	return directorySelected;
}

#pragma comment (lib, "comdlg32.lib")

const char * System_Dialog_Open_Type (const char *title, const char * starting_folder_url, const char *extensions_comma_delimited)
{
	static char file_selected[MAX_OSPATH];

	char win_filter[SYSTEM_STRING_SIZE_1024] = {0};// FilterSpec ="Object Files(.obj)\0*.obj\0Text Files(.txt)\0.txt\0All Files(.)\0*.*\0";
	char startingfolder[MAX_OSPATH];

    OPENFILENAME ofn = {0};

	// Fill in the extensions list
	WIN_Fill_Extensions_Request (win_filter, extensions_comma_delimited, sizeof(win_filter));

	// Determine starting folder
	if (starting_folder_url)
		c_strlcpy (startingfolder, starting_folder_url);
	else if (strlen (m_last_open_directory))
		c_strlcpy (startingfolder, m_last_open_directory);
	else c_strlcpy (startingfolder, Folder_Binary_Folder_URL ());

	File_URL_Edit_SlashesBack_Like_Windows (startingfolder);

	// Erase file_selected
	memset (file_selected, 0, sizeof(file_selected));

	// Fill in ofn struct

    ofn.lStructSize			= sizeof(OPENFILENAME);
    ofn.hwndOwner			= GetFocus(); // Useful, avoids passing the HWND
	#pragma message ("varning: Used GetFocus.  HWMD with keyboard focus .. might not be us")
    ofn.lpstrFilter			= win_filter;
    ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 0;
    ofn.lpstrFile			= file_selected;
    ofn.nMaxFile			= sizeof(file_selected);
    ofn.lpstrInitialDir		= startingfolder;
    ofn.lpstrFileTitle		= startingfolder; // Returned bare file without title according to what I see in API docs
    ofn.nMaxFileTitle		= sizeof(startingfolder);
    ofn.lpstrTitle			= title;
    ofn.lpstrDefExt			= NULL;  // This might apply to save files

    ofn.Flags				= OFN_FILEMUSTEXIST; // flags like OFN_HIDEREADONLY, etc.

    if (!GetOpenFileName ((LPOPENFILENAME)&ofn))
        return NULL;

    File_URL_Edit_SlashesForward_Like_Unix (file_selected);

	// Store last directory
	c_strlcpy (m_last_open_directory, file_selected);
	File_URL_Edit_Reduce_To_Parent_Path (m_last_open_directory);

	// Set last directory
	 return file_selected;
}



const char * System_Dialog_Save_Type (const char *title, const char * starting_folder_url, const char *extensions_comma_delimited)
{

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE MANAGER INTERACTION
///////////////////////////////////////////////////////////////////////////////


cbool System_Folder_Open (const char *path_url)
{
	char windows_style_url[MAX_OSPATH];

	// Copy it
	c_strlcpy (windows_style_url, path_url);

	// Windows format the slashes
	File_URL_Edit_SlashesBack_Like_Windows (windows_style_url);

	return (ShellExecute(0, "Open", "explorer.exe", windows_style_url, NULL, SW_NORMAL) != 0);
}


cbool System_Folder_Open_Highlight (const char *path_to_file)
{
	char windows_style_url[MAX_OSPATH];
	char command_line [MAX_OSPATH];

	if (!File_Exists (path_to_file))
	{
		Core_DPrintf ("File \"%s\" does not exist to show\n", path_to_file);
		Core_Printf ("File does not exist to show\n");
		return false;
	}

	// Copy it
	c_strlcpy (windows_style_url, path_to_file);

	// Windows format the slashes
	File_URL_Edit_SlashesBack_Like_Windows (windows_style_url);

	// Construct the command line
	c_snprintf (command_line, "/select,%s", windows_style_url);

	// Zero is failure, non-zero is success
	Core_DPrintf ("Folder highlight: explorer.exe with \"%s\"\n", command_line);

	return (ShellExecute(0, "Open", "explorer.exe", command_line, NULL, SW_NORMAL) != 0);
}


cbool System_Folder_Open_Highlight_Binary (void)
{
	const char *binary_url = System_URL_Binary ();

	return System_Folder_Open_Highlight (binary_url);
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE AND DIRECTORY MANAGEMENT
///////////////////////////////////////////////////////////////////////////////

void System_chdir (const char *path_url)
{
	_chdir (path_url);
}

//Get current working directory.
const char *System_getcwd (void)
{
	static char workingdir[MAX_OSPATH];

	if (!GetCurrentDirectory (sizeof(workingdir), workingdir))
		Core_Error ("Couldn't determine current directory");

	if (workingdir[strlen(workingdir)-1] == '/')
		workingdir[strlen(workingdir)-1] = 0;

	return workingdir;
}


void System_mkdir (const char *path_url)
{
	_mkdir (path_url);
}


int System_rmdir (const char *path_url)
{
	return _rmdir (path_url);
}


cbool System_File_Exists (const char *path_to_file)
{
	struct _stat st_buf = {0};
	int status = _stat (path_to_file, &st_buf);

	if (status != 0)
		return false;

	return true;
}


cbool System_File_Is_Folder (const char *path_to_file)
{
// Requires at least Windows XP
//	#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
	DWORD attributes = GetFileAttributes (path_to_file);
	if (attributes & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	if (INVALID_FILE_ATTRIBUTES)
		return false;

	return false;
}


// File length
size_t System_File_Length (const char *path_to_file)
{
	struct _stat st_buf = {0};
	int status = _stat (path_to_file, &st_buf );
	if (status != 0)
		return 0;

	return st_buf.st_size;
}


// Returns the seconds since midnight 1970
double System_File_Time (const char *path_to_file)
{
	struct _stat st_buf = {0};

	int status = _stat (path_to_file, &st_buf );
	if (status != 0)
		return 0;

	return (double)st_buf.st_mtime;
}


cbool System_File_URL_Is_Relative (const char *path_to_file)
{
	return PathIsRelative (path_to_file);
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: CLIPBOARD OPERATIONS
///////////////////////////////////////////////////////////////////////////////

const char *System_Clipboard_Get_Text_Alloc (void)
{
	const char	*out = NULL;

	if (OpenClipboard(NULL))
	{
		HANDLE th = GetClipboardData(CF_TEXT);
		const char	*clipboard_text;

		if (th && (clipboard_text = GlobalLock(th))  )
		{
			out = core_strdup (clipboard_text);
			GlobalUnlock (th);
		}

		CloseClipboard ();
	}

	return out;
}


// copies given text to clipboard
cbool System_Clipboard_Set_Text (const char *text_to_clipboard)
{
	char *clipboard_text;
	HGLOBAL hglbCopy;
	size_t len = strlen(text_to_clipboard) + 1;

	if (!OpenClipboard(NULL))
		return false;

	if (!EmptyClipboard())
	{
		CloseClipboard();
		return false;
	}

	if (!(hglbCopy = GlobalAlloc(GMEM_DDESHARE, len + 1)))
	{
		CloseClipboard();
		return false;
	}

	if (!(clipboard_text = GlobalLock(hglbCopy)))
	{
		CloseClipboard();
		return false;
	}

	strlcpy ((char *) clipboard_text, text_to_clipboard, len);
	GlobalUnlock(hglbCopy);
	SetClipboardData(CF_TEXT, hglbCopy);

	CloseClipboard();
	return true;
}

static void sSystem_Clipboard_Set_Image_BGRA (const unsigned *bgra, int width, int height)
{
	HBITMAP hBitmap= CreateBitmap (width, height, 1, 32 /* bits per pixel is 32 */, bgra);

	OpenClipboard (NULL);

	if (EmptyClipboard())
	{
		if ((SetClipboardData (CF_BITMAP, hBitmap)) == NULL)
			Core_Error ("SetClipboardData failed");
	}

	CloseClipboard ();
}


static void sSystem_Clipboard_Set_Image_RGBA_Maybe_Flip (const unsigned *rgba, int width, int height, cbool flip)
{
	int		pelscount = width * height;
	int		buffersize = pelscount * RGBA_4;
	byte    *bgra_data = core_malloc (buffersize); // Clipboard From RGBA work
//	int		i;
//	byte	temp;

	memcpy (bgra_data, rgba, buffersize);

	// If flip ....
	if (flip)
		Image_Flip_Buffer (bgra_data, width, height, RGBA_4);

	// RGBA to BGRA so clipboard will take it
#if 1
	Image_Flip_RedGreen (bgra_data, width * height * RGBA_4);
#else
	for (i = 0 ; i < buffersize ; i += RGBA_4)
	{
		temp = bgra_data[i];

		bgra_data[i] = bgra_data[i + 2];
		bgra_data[i + 2] = temp;
	}
#endif

	sSystem_Clipboard_Set_Image_BGRA ((unsigned *)bgra_data, width, height);
	core_free (bgra_data);
}

cbool System_Clipboard_Set_Image_RGBA (const unsigned *rgba, int width, int height)
{
	sSystem_Clipboard_Set_Image_RGBA_Maybe_Flip (rgba, width, height, false);
	return true;
}

unsigned *System_Clipboard_Get_Image_RGBA_Alloc (int *outwidth, int *outheight)
{
// Baker: Was extremely helpful info ... https://sites.google.com/site/michaelleesimons/clipboard
	byte *ptr = NULL;

	if (OpenClipboard(NULL))
	{
		HBITMAP hBitmap = GetClipboardData (CF_BITMAP);
		BITMAP csBitmap;
		if (hBitmap && GetObject(hBitmap, sizeof(csBitmap), &csBitmap) && csBitmap.bmBitsPixel == 32)
		{
			// allocate buffer
			int i, bufsize = csBitmap.bmWidth * csBitmap.bmHeight * (csBitmap.bmBitsPixel / 8);

			csBitmap.bmBits = ptr = core_malloc (bufsize); // "bmbits buffer"
			GetBitmapBits((HBITMAP)hBitmap, bufsize, csBitmap.bmBits );

			// Convert BGRA --> RGBA, set alpha full since clipboard loses it somehow
			for (i = 0; i < bufsize; i += 4)
			{
				byte temp = ptr[i + 0];
				ptr[i + 0] = ptr[i + 2];
				ptr[i + 2] = temp;
				ptr[i + 3] = 255; // Full alpha
			}
			*outwidth = csBitmap.bmWidth;
			*outheight = csBitmap.bmHeight;
		}
		CloseClipboard ();
	}
	return (unsigned *)ptr;
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: MESSAGEBOX
///////////////////////////////////////////////////////////////////////////////


int System_MessageBox (const char *_title, const char *fmt, ...)
{
	const char *title = _title ? _title : "Alert";
	VA_EXPAND (text, SYSTEM_STRING_SIZE_1024, fmt)

	MessageBox (NULL, text, title , MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
	return 0;
}




///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: PROCESSES
///////////////////////////////////////////////////////////////////////////////

// arguments won't be converted to Windows format!
sys_handle_t System_Process_Create (const char *path_to_file, const char *args, const char *working_directory_url)
{
	char		windows_path_to_file[MAX_OSPATH]; // c:/quake/whatever.exe
	char		windows_work_dir[MAX_OSPATH];  // c:/quake
	char		windows_binary_path[MAX_OSPATH];  // c:/quake
	char*		windows_working_dir;

	// Sanity check
	if (!File_Exists (path_to_file))
		return NULL;

	if (working_directory_url && !File_Exists (working_directory_url))
		return NULL;

	// Construct windows version
	c_strlcpy (windows_path_to_file, path_to_file); // Need to "Windows-ize" it c:/quake --> c:\quake
	c_strlcpy (windows_binary_path, windows_path_to_file);

	// If working directory specified, use that otherwise use binary's path as working dir
	if (working_directory_url)
	{
		c_strlcpy (windows_work_dir, working_directory_url); // So we can Windows-ize the path (c:/quake --> c:\quake)
		windows_working_dir = windows_work_dir;
	}
	else  // No working directory provided, working directory is the path of the binary
	{
		File_URL_Edit_Reduce_To_Parent_Path (windows_binary_path); // Strip off .exe name ---> c:/quake
		File_URL_Edit_SlashesBack_Like_Windows (windows_binary_path);
		windows_working_dir = windows_binary_path;
	}

	// All good, Windows-ize paths ...
	File_URL_Edit_SlashesBack_Like_Windows (windows_path_to_file);
	File_URL_Edit_SlashesBack_Like_Windows (windows_work_dir);

	{
		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi;
		char 		command_line[1024];
		int ret;

		// Construct command line ...
		c_snprintf2 (command_line, "%s %s", windows_path_to_file, args);

		si.cb = sizeof(si);
		si.wShowWindow = SW_HIDE /*SW_SHOWMINNOACTIVE*/;
		si.dwFlags = STARTF_USESHOWWINDOW;

		ret = CreateProcess (
			NULL, 					//  _In_opt_     LPCTSTR lpApplicationName,
			command_line,			//  _Inout_opt_  LPTSTR lpCommandLine,
			NULL,					//  _In_opt_     LPSECURITY_ATTRIBUTES lpProcessAttributes,
			NULL,					//  _In_opt_     LPSECURITY_ATTRIBUTES lpThreadAttributes,
			FALSE,					//  _In_         BOOL bInheritHandles,
			GetPriorityClass(GetCurrentProcess()),
									//  _In_         DWORD dwCreationFlags,
			NULL,					//  _In_opt_     LPVOID lpEnvironment,
			windows_working_dir,	//  _In_opt_     LPCTSTR lpCurrentDirectory,
			&si, 					//  _In_         LPSTARTUPINFO lpStartupInfo,
			&pi						//  _Out_        LPPROCESS_INFORMATION lpProcessInformation
		);

		if (ret) return pi.hProcess; // Return the handle
	}

	return 0;
}

int System_Process_Still_Running (sys_handle_t pid)
{
	DWORD	ExitCode;

	if (!pid)
	{
		Core_Warning ("WARNING: NULL process handle\n");
		return -1; // Error
	}


	if (!GetExitCodeProcess((HANDLE)pid, &ExitCode)) {
		Core_Warning ("WARNING: GetExitCodeProcess failed\n");
		return -1; // Error
	}

	if (ExitCode == STILL_ACTIVE) // 0x00000103L
		return 1; // Still running

	return 0; // No longer running; completed
}


int System_Process_Terminate_Console_App (sys_handle_t pid)
{
	if (TerminateProcess ((HANDLE)pid, 0))
		return 1;

	Core_Printf ("Shutdown failure\n");
	return 0;
}


int System_Process_Close (sys_handle_t pid)
{
	// Send WM_CLOSE to ask process to quit
	if (CloseHandle ( (HANDLE)pid) )
	{
		return 1;
	}

	Core_Warning ("Shutdown failure");
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
	// Windows sleep function works in milliseconds
	Sleep (milliseconds);
}






double System_Time_Now_Precise (void) // no set metric
{
	static	__int64	startcount;
	static double	pfreq;
	static cbool	first = true;

	__int64		pcount;

	QueryPerformanceCounter ((LARGE_INTEGER *)&pcount);
	if (first)
	{
		__int64	freq;
		QueryPerformanceFrequency ((LARGE_INTEGER *)&freq);
		if (freq <= 0)
			Core_Error ("Hardware timer not available");

		pfreq = (double)freq;
		first = false;
		startcount = pcount;
		return 0.0;
	}

	// TODO: check for wrapping
	return (pcount - startcount) / pfreq;
}


