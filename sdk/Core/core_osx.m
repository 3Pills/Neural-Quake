/*
Copyright (C) 2001-2012 Axel 'awe' Wefers (Fruitz Of Dojo)
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
// core_osx.m -- Mac platform interface


/*
#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <mach/mach_time.h>

#import <dlfcn.h>
#import <unistd.h>
#import <signal.h>
#import <stdlib.h>
#import <limits.h>
#import <sys/time.h>
#import <sys/types.h>
#import <unistd.h>
#import <fcntl.h>
#import <stdarg.h>
#import <stdio.h>
#import <sys/ipc.h>
#import <sys/shm.h>
#import <sys/stat.h>
#import <string.h>
#import <ctype.h>
#import <sys/wait.h>
#import <sys/mman.h>
#import <sys/param.h>
#import <errno.h>
*/





#if defined (__OBJC__)

#import <Cocoa/Cocoa.h>
#import <ApplicationServices/ApplicationServices.h>

#endif

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
    
    NSString* _basePath = [[[NSBundle mainBundle] bundlePath] stringByStandardizingPath];
    const char *basePath = [_basePath cStringUsingEncoding:NSASCIIStringEncoding];
    c_strlcpy (binary_url, basePath);
    return (const char*)&binary_url[0];
}


const char * System_URL_Binary_Folder (void)
{
	static char binary_folder_url[MAX_OSPATH];
	const char* binary_url = System_URL_Binary ();

	c_strlcpy (binary_folder_url, binary_url);
	File_URL_Edit_Reduce_To_Parent_Path (binary_folder_url);

	return binary_folder_url;
}


static const char *sSystem_URL_Caches_By_AppName (const char* appname)
    {
        static char cachesfolder[MAX_OSPATH];
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
        NSString* mydir = [paths objectAtIndex:0];
        NSString *myAppID = [[NSBundle mainBundle] bundleIdentifier];
        NSString *cachesDirectory = [NSString stringWithFormat:@"%@/%@", mydir, myAppID];
        c_strlcpy (cachesfolder, [cachesDirectory cStringUsingEncoding:NSASCIIStringEncoding] );
#ifdef PLATFORM_OSX
    
        if (![[NSFileManager defaultManager] fileExistsAtPath:cachesDirectory])
        {
            
            NSError* error;
            if ([[NSFileManager defaultManager] createDirectoryAtPath:cachesDirectory withIntermediateDirectories:NO attributes:nil error:&error])
                ;// success
            else
            {
                NSLog(@"[%@] ERROR: attempting to write create directory", cachesDirectory);
//              NSAssert( FALSE, @"Failed to create directory maybe out of disk space?");
            }
        }
            
//          NSLog (@"Caches copy would be %@", path);
//          NSFileManager* fileManager = [NSFileManager defaultManager];
//          if(![fileManager fileExistsAtPath: path])
//          {
//              // No preferences file so copy from the bundle
//              NSString* bundle = [[NSBundle mainBundle] pathForResource:@"test" ofType:@"tga"];
//              [fileManager copyItemAtPath: bundle toPath: path error: &error];
//          }

#endif      
        
        return cachesfolder;    
    }


const char * System_URL_Caches (void)
{
	return sSystem_URL_Caches_By_AppName (gCore_Appname);
}

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE DIALOGS
///////////////////////////////////////////////////////////////////////////////

static NSString *lastOpenDirectory;

const char * System_Dialog_Open_Directory (const char *title, const char *starting_folder_url)
{
	static char directorySelected[MAX_OSPATH];
	
	directorySelected[0] = 0;
	
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
	return directorySelected;
}


const char * System_Dialog_Open_Type (const char *title, const char * starting_folder_url, const char *extensions_comma_delimited)
{
	static char fileSelected[MAX_OSPATH];
	
	fileSelected[0] = 0;
	
	// Determine permitted extensions
	NSArray *allowedExtensions = nil;
	
	if (extensions_comma_delimited)
	{
		NSString *extensionsList = [NSString stringWithUTF8String:extensions_comma_delimited];
		allowedExtensions = [extensionsList componentsSeparatedByString:@","];
	}
	
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
	[panel setCanChooseFiles: YES];
	[panel setCanChooseDirectories: NO];
	[panel setTitle: titleString];
	
	// Conditionally limit file extension
	if (extensions_comma_delimited)
		[panel setAllowedFileTypes:allowedExtensions];

	if ([panel runModal])
	{
		// Ok button was selected
		NSString *_directorySelected = [[panel directoryURL] path]; // Directory result
		c_strlcpy (fileSelected, [_directorySelected cStringUsingEncoding: NSASCIIStringEncoding]);
		
		// Get the URL and convert it to an NSString without URL encoding (i.e. no %20 as a space, etc.)
		NSURL    *_selectedFileURL   = [[panel URLs]objectAtIndex:0];
		NSString *_selectedFile      = [NSString stringWithFormat:@"%@", _selectedFileURL];
		NSString *_selectedFile2     = [_selectedFile stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
		NSString *selectedFile       = [_selectedFile2 lastPathComponent];
		
		c_strlcat (fileSelected, "/");
		c_strlcat (fileSelected, [selectedFile cStringUsingEncoding: NSASCIIStringEncoding]);
		
		lastOpenDirectory = [_directorySelected copy];
	}
	
	[pool release];
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
    NSString*   filePath = [NSString stringWithUTF8String:path_url];
    NSURL *     fileURL = [NSURL fileURLWithPath:filePath isDirectory:YES];
    
    if (File_Is_Folder (path_url) == false)
    {
        // Strip
        fileURL = [fileURL URLByDeletingLastPathComponent];
    }
    
    [[NSWorkspace sharedWorkspace] openURL:fileURL];        

    return true;
}


cbool System_Folder_Open_Highlight (const char *path_to_file)
{
    NSString*   filePath = [NSString stringWithUTF8String:path_to_file];
    NSURL *     fileURL = [NSURL fileURLWithPath:filePath isDirectory:NO];
    
    NSArray *fileURLs = [NSArray arrayWithObjects:fileURL, nil];
    [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:fileURLs];       
    
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
	const char	*out = NULL;

    NSPasteboard*   pasteboard  = [NSPasteboard generalPasteboard];
    NSArray*        types       = [pasteboard types];


    
    if (![types containsObject: NSStringPboardType])
        return NULL;
    
    NSString* clipboardString = [pasteboard stringForType: NSStringPboardType];
        
    if ( !clipboardString || ![clipboardString length])
        return NULL;

	out = core_strdup ([clipboardString cStringUsingEncoding: NSASCIIStringEncoding]);
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
                        
#ifdef PLATFORM_OSX // Clipboard
    //Get general pasteboard
    NSPasteboard*   myPasteboard    = [NSPasteboard generalPasteboard];
    NSString*       stringToWrite   = [NSString stringWithUTF8String:text_to_clipboard];

    // NSPasteboardTypeString instead of NSStringPboardType as NSStringPboardType is deprecated in 10.6
    [myPasteboard clearContents];
    [myPasteboard declareTypes:[NSArray arrayWithObjects:NSStringPboardType, nil] owner:nil];
    [myPasteboard setString:stringToWrite forType:NSStringPboardType];  
#endif // PLATFORM_OSX // Clipboard
    return true;
}


cbool System_Clipboard_Set_Image_RGBA (const unsigned *rgba_data, int width, int height)
{
#ifdef PLATFORM_OSX // Clipboard
    // TO PASTEBOARD:
    NSBitmapImageRep* image_rep = [[NSBitmapImageRep alloc]
                                   initWithBitmapDataPlanes:NULL
                                   pixelsWide:width
                                   pixelsHigh:height
                                   bitsPerSample:8
                                   samplesPerPixel:4
                                   hasAlpha:YES
                                   isPlanar:NO
                                   colorSpaceName:NSCalibratedRGBColorSpace
                                   bitmapFormat:0
                                   bytesPerRow:width*4
                                   bitsPerPixel:32];
    
    memcpy([image_rep bitmapData], rgba_data, width * height * 4);
    
    
    NSPasteboard*   myPasteboard    = [NSPasteboard generalPasteboard];
    //  [myPasteboard clearContents];
    [myPasteboard declareTypes:[NSArray arrayWithObjects:NSTIFFPboardType, nil] owner:nil];
    [myPasteboard setData:[image_rep TIFFRepresentation] forType:NSTIFFPboardType];
    return true;
#endif // PLATFORM_OSX // Clipboard
}



unsigned *System_Clipboard_Get_Image_RGBA_Alloc (int *outwidth, int *outheight)
{
        byte* imageData = NULL;
#ifdef PLATFORM_OSX // Clipboard
        NSPasteboard    *myPasteboard       = [NSPasteboard generalPasteboard];
        
        if ( [NSBitmapImageRep canInitWithPasteboard:myPasteboard] == NO)
        {
            return NULL; // Can't get
        }
        
        NSString *type = [myPasteboard availableTypeFromArray:[NSArray arrayWithObjects:NSTIFFPboardType /*,NSPICTPboardType*/ ,nil]];
        
        // Might be a small hole in this: What if datatype isn't TIFF but PICT or something?
        NSData* data            = [myPasteboard dataForType:type];

        // Load and format the data
        #ifdef PLATFORM_OSX
        CGImageRef  imgRepData  = [[NSBitmapImageRep imageRepWithData: data] CGImage];
        #endif  

        if (imgRepData == nil) 
        { 
            Core_Warning ("System_Clipboard_Get_Image_RGBA_Alloc: \"%s\" exists but couldn't load", "Fix me");
            return NULL; 
        }

        // Allocate image data and fill it in
        {   
            int imageWidth  = (int)CGImageGetWidth(imgRepData);
            int imageHeight = (int)CGImageGetHeight(imgRepData);
            size_t numBytes = imageWidth * imageHeight * 4 /* RGBA_BYTES_PER_PIXEL_IS_4 */;
            imageData       = core_calloc (1, numBytes);
            
            // Create a bitmap context with specified characteristics
            CGContextRef imageContext = CGBitmapContextCreate (imageData, imageWidth, imageHeight, 8, 
                                                                imageWidth * 4 /* RGBA_BYTES_PER_PIXEL_IS_4 */ ,
                                                                 CGImageGetColorSpace(imgRepData),
                                                                 kCGImageAlphaPremultipliedLast);
        
            // Fill in the texture context
            CGContextDrawImage          (imageContext, CGRectMake(0.0, 0.0, (float)imageWidth, (float)imageHeight), imgRepData);
            CGContextRelease            (imageContext);
        
            *outwidth = imageWidth, *outheight = imageHeight;
        }
        
        // No cleanup needed here due to ARC!
#endif // PLATFORM_OSX // Clipboard
        return (unsigned *)imageData;
}


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: MESSAGEBOX
///////////////////////////////////////////////////////////////////////////////


int System_MessageBox (const char *_title, const char *fmt, ...)
{	
	const char *title = _title ? _title : "Alert";
	VA_EXPAND (text, SYSTEM_STRING_SIZE_1024, fmt)

    // Objective C hates NSString NULLs with stringWithUTF8String    
    NSString* myMessage =[NSString stringWithUTF8String:text];
    NSString* myTitle =[NSString stringWithUTF8String:title];
    
#ifdef PLATFORM_OSX // NSAlert
    NSAlert *alert = [[NSAlert alloc] init]; ///*autorelease*/];
    [alert setMessageText:myTitle];
    [alert setInformativeText:myMessage];
    [alert runModal];   
#endif // PLATFORM_OSX  .. NSAlert, iOS would be an alertview

#ifdef PLATFORM_IOS // NSAlert
    NSString* myTitle =[NSString stringWithUTF8String:title];
    UIAlertView *alert = [[UIAlertView alloc]
                          initWithTitle:myTitle message:myMessage delegate:nil
                          cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
    [alert show];
    NSRunLoop *rl = [NSRunLoop currentRunLoop];
    NSDate *d;
    while ([alert isVisible])
    {
        d = [[NSDate alloc] init];
        [rl runUntilDate:d];
    }
#endif // PLATFORM_IOS  .. NSAlert, iOS would be an alertview
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

	NSTask *task = [[NSTask alloc] init];
	
	// Construct the path to file
	
	NSString *pathToFile = [NSString stringWithUTF8String:path_to_file];
	
	[task setLaunchPath:pathToFile];
    
	// Tokenize the command line
	int  argscount = 0;
	char *argvs[64];
	char cmdline[SYSTEM_STRING_SIZE_1024];
	c_strlcpy(cmdline, args);
	String_Command_String_To_Argv(cmdline, &argscount, argvs, 64);
	
	NSMutableArray *mfinalArgs = [[NSMutableArray alloc] init];
	for (int i = 0; i < argscount; i ++)
	{
		NSString *newstr = [NSString stringWithUTF8String:argvs[i]];
		[mfinalArgs addObject:newstr];
	}

	NSArray *finalArgs = [mfinalArgs copy];
	[task setArguments:finalArgs];
    
	// Launch it
    [task launch];
    
	return (sys_handle_t)task;

}

// Works reliably!
int System_Process_Still_Running (sys_handle_t pid)
{	
	NSTask *task = (NSTask*)pid;
	if ([task isRunning])
		return 1;
	
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
    void* psymbol = dlsym (RTLD_DEFAULT, pfunction_name);
    
    if (psymbol == NULL)
    {
        Core_Error ("Failed to import a required function!");
    }
    
    return psymbol;
}

//
// End of File
//

