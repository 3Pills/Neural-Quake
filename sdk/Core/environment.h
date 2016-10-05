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
// environment.h -- platform environment


#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

/*
** Special basic datatypes SHALL be prefixed with 'c'.
** Special basic functions/function-wrapping macros SHALL be prefixed with 'c_'
**
** datatype examples: cbool, crect_t, etc.
** function examples: c_strlcpy, c_snprint2, c_rint, c_min, c_max, etc.
*/

///////////////////////////////////////////////////////////////////////////////
// Platform identification: Create our own define for Mac OS X, etc.
///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__
	#include "TargetConditionals.h"
	#define PLATFORM_FAMILY_APPLE
	#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
		// iOS device
		// iOS Simulator
		#pragma message ("IPHONE DETECTED")
		# define PLATFORM_IOS
		# define PLATFORM_NAME "Apple iOS"
		# define PLATFORM_SHORTNAME "iOS"
	#elif TARGET_OS_MAC
		# define PLATFORM_OSX
		# define PLATFORM_NAME "Mac OS X"
		# define PLATFORM_SHORTNAME "Mac"
	#else
		// Unsupported platform
		#pragma message ("UNKNOWN APPLE PLATFORM")
	#endif
#endif // __APPLE__

#ifdef _WIN32
 	# define PLATFORM_WINDOWS
	# define PLATFORM_NAME "Windows"
	# define PLATFORM_SHORTNAME "Windows"
#endif // _WIN32

#if defined (__linux__) || defined (__linux)
    # define PLATFORM_LINUX
	# define PLATFORM_NAME "Linux"
	# define PLATFORM_SHORTNAME "Linux"
#endif

#ifndef PLATFORM_NAME
    #error Unknown platform
#endif

#define CORE_NEEDS_STRRSTR
#define CORE_NEEDS_VSNPRINTF_SNPRINTF

///////////////////////////////////////////////////////////////////////////////
//  GCC compile attributes, not needed for CLANG and not supported in MSVC6
///////////////////////////////////////////////////////////////////////////////

#if !defined(__GNUC__)
	#define	__attribute__(x)
#endif	/* __GNUC__ */

/* argument format attributes for function
 * pointers are supported for gcc >= 3.1
 */
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 0))
	#define	__fp_attribute__	__attribute__
#else
	#define	__fp_attribute__(x)
#endif

#if (defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)) && defined (__GNUC__)
// Linux does not have BSD strlcpy which is not standard C
	#define CORE_NEEDS_STRLCPY_STRLCAT // Really?
	#define CORE_NEEDS_STRCASESTR
#endif

#ifdef __GNUC__
	#define CORE_NEEDS_STRNDUP // Mingw is pissing me off ..
	#define CORE_NEEDS_STRNLEN // Mingw is pissing me off ..
#endif // __GNUC__

///////////////////////////////////////////////////////////////////////////////
//  Microsoft Visual Studio
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

	
	#if defined(_WIN64)
		#define ssize_t	SSIZE_T
	#else
		typedef int	ssize_t;
	#endif // _WIN64

//	#define CORE_NEEDS_VSNPRINTF_SNPRINTF   // All need
//	#define CORE_NEEDS_STRRSTR				// All need
	#define CORE_NEEDS_STRLCPY_STRLCAT
	#define CORE_NEEDS_STRCASESTR
	#define CORE_NEEDS_STRPTIME
    #define CORE_NEEDS_STRNDUP

	#if _MSC_VER <= 1200
		#define __VISUAL_STUDIO_6__
		#define CORE_NEEDS_STRNLEN
	#endif // Visual Studio 6

	#if _MSC_VER < 1400

		#define INVALID_FILE_ATTRIBUTES ((DWORD)-1) // GetFileAttributes MSVC6
	#endif // _MSC_VER < 1400

	#if !defined(__cplusplus)
		#define inline __inline
	#endif	// not cpp

	// Visual Studio has different names for these functions

	#ifdef __VISUAL_STUDIO_6__
		#define strcasecmp stricmp
		#define strncasecmp strnicmp
	#else
		#define strcasecmp _stricmp
		#define strncasecmp _strnicmp
	#endif // ! __VISUAL_STUDIO_6__

	#ifndef __VISUAL_STUDIO_6__
		#define _CRT_SECURE_NO_WARNINGS // Get rid of error messages about secure functions
		#define POINTER_64 __ptr64 // VS2008+ include order involving DirectX SDK can cause this not to get defined
	#endif
	#pragma warning(disable :4244)     // MIPS
	#pragma warning(disable:4244) // 'argument'	 : conversion from 'type1' to 'type2', possible loss of data
	#pragma warning(disable:4305) // 'identifier': truncation from 'type1' to 'type2', in our case, truncation from 'double' to 'float' */
	#pragma warning(disable:4996) // VS2008+ do not like fopen, but fopen_s is not standard C so unusable here

//	#pragma warning(disable:4761) // Baker: vc6 integral size mismatch in argument; conversion supplied
//	#pragma warning(disable:4267) // conversion from 'size_t' to 'type', possible loss of data (/Wp64 warning)

#endif // _MSC_VER

#ifdef __clang__
	#pragma clang diagnostic ignored "-Wconversion"
	#pragma clang diagnostic ignored "-Wformat-nonliteral"
	#pragma clang diagnostic ignored "-Wmissing-prototypes"
	#pragma clang diagnostic ignored "-Wambiguous-macro" // M_PI
#endif // __clang__

///////////////////////////////////////////////////////////////////////////////
//  Data types and defines
///////////////////////////////////////////////////////////////////////////////

#if __STDC_VERSION__ >= 201112L
	/* C11 support */
#endif

#undef true
#undef false


typedef enum
{
	false = 0,
	true = 1,
} cbool;

typedef unsigned char byte;

#ifndef NULL
#if defined(__cplusplus)
#define	NULL		0
#else
#define	NULL		((void *)0)
#endif
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define	CLAMP(_minval, x, _maxval)		\
	((x) < (_minval) ? (_minval) :		\
	 (x) > (_maxval) ? (_maxval) : (x))

// Returns if true if would clamp
#define WOULDCLAMP(_minval, x, _maxval)		\
	((x) < (_minval) ? true :				\
	 (x) > (_maxval) ? true : false)



#define SQUARED(x)	((x)*(x))

#undef min
#undef max

#define	c_min(a, b)	(((a) < (b)) ? (a) : (b))
#define	c_max(a, b)	(((a) > (b)) ? (a) : (b))
#define c_rint(x)	((x) > 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))
#define c_sgn(x)	((x > 0) - (x < 0))

#define INBOUNDS(_low, _test, _high) (_low <= _test && _test <= _high )

typedef void  (*voidfunc_t)			(void);
typedef cbool (*progress_fn_t )		(void *id, int oldamt, int newamt); // Allows boundary checks easier
typedef cbool (*boolfunc_t )		(void);

typedef struct crect_s
{
	int				left, right, bottom, top;
	int				center_x, center_y;
	int				width, height;
} crect_t;

extern const char * const empty_string;

///////////////////////////////////////////////////////////////////////////////
//  Constants
///////////////////////////////////////////////////////////////////////////////

#ifdef PLATFORM_WINDOWS
	#define SYSTEM_BINARY_EXTENSION ".exe"
#else // Non-Windows ..
	#define SYSTEM_BINARY_EXTENSION ""
#endif // !PLATFORM_WINDOWS

#define SYSTEM_STRING_SIZE_1024 1024

#ifdef PLATFORM_WINDOWS
	#define MAX_OSPATH 256   // Technically 260 +/-
#else// !PLATFORM_WINDOWS ...
	#define MAX_OSPATH 1024 // OS X can have some long pathnames
#endif // !PLATFORM_WINDOWS

///////////////////////////////////////////////////////////////////////////////
//  Command Support
///////////////////////////////////////////////////////////////////////////////


#define MAX_CMD_256 256
#define MAX_ARGS_80	80

typedef int cmdret_t; // 0 = successful, non-zero = error


///////////////////////////////////////////////////////////////////////////////
//  Macros
///////////////////////////////////////////////////////////////////////////////

// example:  int frogs[32]; ARRAY_SIZE(frogs);
// output:   static int _numfrogs = 32
#define ARRAY_SIZE(_array)			       const int _num ## _array = sizeof(_array) / sizeof(_array[0])
#define ARRAY_SIZE_STATIC(_array)	static const int _num ## _array = sizeof(_array) / sizeof(_array[0])

#define OFFSET_ZERO_0 0

#define TRUISM(_x) ((_x) || 1) // Result of express is always true

///////////////////////////////////////////////////////////////////////////////
//  Objects
///////////////////////////////////////////////////////////////////////////////

// Future?  Parent, Children, Next, Prev?


//typedef (void *(*MShutdown) (void *));
typedef void  (*MShutdown)			(void *);

#define __OBJ_REQUIRED__			\
	const char * _cname;			\
	struct cobj_s * _parent;		\
	struct cobj_s * _child;			\
	MShutdown * Shutdown;

	//void *(*Shutdown) (void *);

// We put this at the end of object initialization that way if it failed to initialize we null it
// We can also auto-register it
#define OBJ_REQUIRED_HOOKUP(_x)		\
	if (_x)							\
	{								\
		_x->_cname		= _tag;		\
		_x->Shutdown	= (MShutdown *)Shutdown; \
	}

///////////////////////////////////////////////////////////////////////////////
//  Compile Time Assert
///////////////////////////////////////////////////////////////////////////////

#define	COMPILE_TIME_ASSERT(name, x)	\
	typedef int dummy_ ## name[(x) * 2 - 1]



//typedef unsigned int in_addr_t;	/* uint32_t */ // Kill!


#endif // ! __ENVIRONMENT_H__




