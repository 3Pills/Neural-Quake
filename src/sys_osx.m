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
// sys.c -- system

#include "core.h"
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


#import "quakedef.h"
#import "macquake.h"





sysplat_t sysplat;


///////////////////////////////////////////////////////////////////////////////
//  CLOCK: Baker
///////////////////////////////////////////////////////////////////////////////

// Double self-initializes now
double System_DoubleTime (void)
{
    static uint64_t start_time   = 0;
    static double   scale       = 0.0;
    const uint64_t  time        = mach_absolute_time();

    if (start_time == 0)
    {
        mach_timebase_info_data_t   info = { 0 };

        if (mach_timebase_info (&info) != 0)
        {
            System_Error ("Failed to read timebase!");
        }

        scale       = 1e-9 * ((double) info.numer) / ((double) info.denom);
        start_time   = time;
    }

    return (double) (time - start_time) * scale;
}

///////////////////////////////////////////////////////////////////////////////
//  FILE IO: Baker
///////////////////////////////////////////////////////////////////////////////



const char *System_FixFileNameCase (const char *path_to_file)
{
    BOOL                isDirectory = NO;
    NSFileManager*      fileManager = [NSFileManager defaultManager];
    NSString*           path        = [fileManager stringWithFileSystemRepresentation: path_to_file length: strlen (path_to_file)];
    BOOL                pathExists  = [fileManager fileExistsAtPath: path isDirectory: &isDirectory];

    if ((pathExists == NO) || (isDirectory == YES))
    {
        NSString*   outName  = nil;
        NSArray*    outArray = nil;

        if ([path completePathIntoString: &outName caseSensitive: NO matchesIntoArray: &outArray filterTypes: nil] > 0)
        {
            path_to_file = [outName fileSystemRepresentation];
        }
    }

    return path_to_file;
}


int System_FileOpenRead (const char *path_to_file, int *pHandle)
{
    struct stat fileStat = { 0 };

    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    *pHandle = open (System_FixFileNameCase (path_to_file), O_RDONLY, 0666);

    [pool release];

    if (*pHandle == -1)
    {
        return -1;
    }

    if (fstat (*pHandle, &fileStat) == -1)
    {
        System_Error ("Can\'t open file \"%s\", reason: \"%s\".", path_to_file, strerror (errno));
    }

    return (int) fileStat.st_size;
}



int System_FileOpenWrite (const char *path_to_file)
{
    int handle = -1;

    umask (0);

    handle = open (path_to_file, O_RDWR | O_CREAT | O_TRUNC, 0666);

    if (handle == -1)
    {
        System_Error ("Can\'t open file \"%s\" for writing, reason: \"%s\".", path_to_file, strerror (errno));
    }

    return handle;
}

void System_FileClose (int handle)
{
    close (handle);
}


void System_FileSeek (int handle, int position)
{
    lseek (handle, position, SEEK_SET);
}


int System_FileRead (int handle, void* dest, int count)
{
    return (int) read (handle, dest, count);
}

int System_FileWrite (int handle, const void* pdata, int numbytes)
{
    return (int) write (handle, pdata, numbytes);
}




///////////////////////////////////////////////////////////////////////////////
//  SYSTEM IO
///////////////////////////////////////////////////////////////////////////////


#if id386

/*
================
System_MakeCodeWriteable
================
*/
void System_MakeCodeWriteable (unsigned long startaddr, unsigned long len)
{
    const int           pageSize    = getpagesize();
    const unsigned long address     = (startaddr & ~(pageSize - 1)) - pageSize;

#ifdef SERVER_ONLY
    fprintf (stderr, "writable code %lx(%lx)-%lx, length=%lx\n", startaddr, address, startaddr + len, len);
#endif // SERVER_ONLY

    if (mprotect ((char*) address, len + startaddr - address + pageSize, 7) < 0)
    {
        System_Error ("Memory protection change failed!");
    }
}
#endif // id386






///////////////////////////////////////////////////////////////////////////////
//  SYSTEM ERROR: Baker
///////////////////////////////////////////////////////////////////////////////


void System_Error (const char *fmt, ...)
{
    char        text[SYSTEM_STRING_SIZE_1024];
	va_list     argptr;

    fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);

    va_start (argptr, fmt);
    c_vsnprintf (text, sizeof(text), fmt, argptr);
    va_end (argptr);

#ifdef SERVER_ONLY

    fprintf (stderr, "Error: %s\n", text);

#else

    Host_Shutdown();
    [[NSApp delegate] setQuakeRunning: NO];

    NSString* msg = [NSString stringWithCString: text encoding: NSASCIIStringEncoding];

    NSRunCriticalAlertPanel (@"An error has occured:", msg, nil, nil, nil);
    NSLog (@"An error has occured: %@\n", msg);

#endif // SERVER_ONLY


    exit (1);
}



///////////////////////////////////////////////////////////////////////////////
//  SYSTEM EVENTS
///////////////////////////////////////////////////////////////////////////////

//
//
//
//
//
//
//
void System_SendKeyEvents (void)
{
    // will only be called if in modal loop
    NSAutoreleasePool*  pool    = [[NSAutoreleasePool alloc] init];
    NSEvent*            event   = [NSApp nextEventMatchingMask: NSAnyEventMask
                                                     untilDate: [NSDate distantPast]
                                                        inMode: NSDefaultRunLoopMode
                                                       dequeue: YES];

    [NSApp sendEvent: event];
    [pool release];

    Input_Local_SendKeyEvents();

}




///////////////////////////////////////////////////////////////////////////////
//  SYSTEM MAIN LOOP
///////////////////////////////////////////////////////////////////////////////


void System_Quit (void)
{
#ifndef SERVER_ONLY
    // shutdown host:
    Host_Shutdown ();
    [[NSApp delegate] setQuakeRunning: NO];
    fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
    fflush (stdout);
#endif // ! SERVER_ONLY

    exit (0);
}

void System_Init (void)
{
    signal (SIGFPE, SIG_IGN); // Baker: Ignore floating point exceptions.  WinQuake.
}

cbool stdin_ready;
int main (int argc, const char **pArgv)
{
#ifdef SERVER_ONLY
	char		cmdline[SYSTEM_STRING_SIZE_1024];
	uintptr_t 	fakemainwindow = 0;

	String_Command_Argv_To_String (cmdline, argc - 1, &pArgv[1], sizeof(cmdline));

	// Ease life
	if (!String_Find_Caseless(cmdline, "-dedicated"))
			c_strlcat (cmdline, "-dedicated 4");
#ifdef _DEBUG
	c_strlcat (cmdline, " -basedir /Users/iOS/Desktop/Quake -window");
#endif
	
	stdin_ready = true; // Console
	
	Main_Central (cmdline, &fakemainwindow, true );
		
    /* return success of application */
    return 0; // Baker: unreachable
#else

    NSAutoreleasePool * pool        = [[NSAutoreleasePool alloc] init];
    NSUserDefaults *    defaults    = [NSUserDefaults standardUserDefaults];

    [defaults registerDefaults: [NSDictionary dictionaryWithObject: @"YES" forKey: @"AppleDockIconEnabled"]];
    [pool release];

    return NSApplicationMain (argc, pArgv);

#endif // SERVER_ONLY
}



///////////////////////////////////////////////////////////////////////////////
//  CONSOLE:  OS X project doesn't have the equivalent file of conproc.h/conproc.c
///////////////////////////////////////////////////////////////////////////////


const char *Dedicated_ConsoleInput (void)
{
    char *pText = NULL;

#ifdef SERVER_ONLY

    if (stdin_ready != 0)
    {
        static char     text[256];
        const int		length = read (0, text, FD_SIZE_OF_ARRAY (text));

        stdin_ready = 0;

        if (length > 0)
        {
//			Con_Printf ("%i\n",(int)length);
            text[length - 1]    = '\0';
            pText               = &(text[0]);
        }
    }

	stdin_ready = 1;
#endif // SERVER_ONLY

    return pText;
}

void Dedicated_Local_Print (const char *text)
{
#ifdef SERVER_ONLY

    for (unsigned char *i = (unsigned char*) &(text[0]); *i != '\0'; ++i)
    {
        *i &= 0x7f;

        if ((*i > 128 || *i < 32) && (*i != 10) && (*i != 13) && (*i != 9))
        {
            fprintf (stderr, "[%02x]", *i);
        }
        else
        {
            putc (*i, stderr);
        }
    }

    fflush (stderr);
#else // ! SERVER_ONLY ...
    FD_UNUSED (fmt);
#endif // ! SERVER_ONLY
}

void System_SleepUntilInput (int time)
{
	// Nothing?
}


#import <Cocoa/Cocoa.h>

@interface QApplication : NSApplication

@end

#import "QController.h"


@implementation QApplication
@end



