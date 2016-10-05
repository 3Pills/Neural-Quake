/*
 Copyright (C) 2009-2014 Baker
 *
 *Permission to use, copy, modify, and distribute this software for any
 *purpose with or without fee is hereby granted, provided that the above
 *copyright notice and this permission notice appear in all copies.
 *
 *THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
// stringlib.h -- extended string functionality


#ifndef __STRING_LIB_H__
#define __STRING_LIB_H__

#include "environment.h"

#include <stdio.h> // fopen, etc.
#include <stdlib.h> // malloc, etc.
#include <stdarg.h> // va_list, etc.

void perrorf (const char *fmt, ...);

#define CORE_STRINGS_VA_ROTATING_BUFFER_BUFSIZ_1024 1024
#define CORE_STRINGS_VA_ROTATING_BUFFERS_COUNT_32 32

char *va (const char *format, ...) __attribute__((__format__(__printf__,1,2)));


#define c_snprintf(_var,_fmt,_s1) c_snprintfc (_var, sizeof(_var), _fmt, _s1)
#define c_snprintf2(_var,_fmt,_s1,_s2) c_snprintfc (_var, sizeof(_var), _fmt, _s1,_s2)
#define c_snprintf3(_var,_fmt,_s1,_s2,_s3) c_snprintfc (_var, sizeof(_var), _fmt, _s1,_s2,_s3)
#define c_snprintf4(_var,_fmt,_s1,_s2,_s3,_s4) c_snprintfc (_var, sizeof(_var), _fmt, _s1,_s2,_s3,_s4)
#define c_snprintf5(_var,_fmt,_s1,_s2,_s3,_s4,_s5) c_snprintfc (_var, sizeof(_var), _fmt, _s1,_s2,_s3,_s4,_s5)
#define c_snprintf5(_var,_fmt,_s1,_s2,_s3,_s4,_s5) c_snprintfc (_var, sizeof(_var), _fmt, _s1,_s2,_s3,_s4,_s5)
#define c_snprintf6(_var,_fmt,_s1,_s2,_s3,_s4,_s5,_s6) c_snprintfc (_var, sizeof(_var), _fmt, _s1,_s2,_s3,_s4,_s5,_s6)

#ifndef _DEBUG
#define String_Does_Contain(_s, _find) (strstr(_s, _find) != NULL)

#define String_Does_Match(_s1, _s2) !strcmp(_s1, _s2)
#define String_Does_Match_Caseless(_s1, _s2) !strcasecmp(_s1, _s2)
#define String_Does_Match_Nullproof(_s1, _s2) ( _s1 == _s2 ? 1 : ((_s1 == NULL || _s2 == NULL) ? 0 : !strcmp (_s1, _s2)))

#define String_Does_Not_Match(_s1, _s2) !!strcmp(_s1, _s2)
#define String_Does_Not_Match_Caseless(_s1, _s2) !!strcasecmp(_s1, _s2)

#define String_Does_Not_Start_With(_s, _prefix) !!strncmp(_s, _prefix, strlen(_prefix))
#define String_Does_Not_Start_With_Caseless(_s, _prefix) !!strncasecmp(_s, _prefix, strlen(_prefix))

#define String_Does_Start_With(_s, _prefix) !strncmp(_s, _prefix, strlen(_prefix))
#define String_Does_Start_With_Caseless(_s, _prefix) !strncasecmp(_s, _prefix, strlen(_prefix))

#endif // _DEBUG


#if 0
#ifdef _DEBUG
	#define COMPILE_TIME_ASSERT_SIZE(pred) switch(0){case 0:case pred:;}
#else
	#define COMPILE_TIME_ASSERT_SIZE(pred)
#endif

#define c_strlcpy(_dest, _source) COMPILE_TIME_ASSERT_SIZE((sizeof(_dest) != sizeof(void *))) \
		strlcpy (_dest, _source, sizeof(_dest))
#endif

#define c_strlcpy(_dest, _source) strlcpy (_dest, _source, sizeof(_dest))
#define c_strlcat(_dest, _source) strlcat (_dest, _source, sizeof(_dest))

void String_Command_Argv_To_String (char *cmdline, int numargc, char **argvz, size_t siz);
// Note, the tokenizer counts on the string being available as long as the tokenizer is in use
void String_Command_String_To_Argv (char *cmdline, int *numargc, char **argvz, int maxargs);

#define ARG_BUCKETS_COUNT_64 64
struct arg_buckets_64_s
{
	char	cmdline[SYSTEM_STRING_SIZE_1024];
	char	*argvs[ARG_BUCKETS_COUNT_64];
	int		argcount;
};

void String_To_Arg_Buckets (struct arg_buckets_64_s *argbucket, const char *s);


int String_Count_Char (const char *s, int ch_findchar);
int String_Count_Newlines (const char *s);
int String_Count_String (const char *s, const char *s_find);

#ifdef _DEBUG // Not for release version, we use a macro

int String_Does_Contain (const char *s, const char *s_find);

#endif // _DEBUG ... release version uses macro

int String_Does_Contain_Spaces (const char *s);
int String_Does_List_Delimited_Contain (const char *s, const char *s_find, int ch_delimiter);
int String_Does_End_With_Caseless (const char *s, const char *s_suffix);
int String_Does_End_With (const char *s, const char *s_suffix);
int String_Does_Have_Excel_Numeric_Representation (const char *s);
int String_Does_Have_Lowercase (const char *s);
int String_Does_Have_Quotes(const char *s); // Name not ideal, but consistent
int String_Does_Have_Uppercase (const char *s);



#ifdef _DEBUG // Not for release version, we use a macro

int String_Does_Match (const char *s1, const char *s2);
int String_Does_Match_Caseless (const char *s1, const char *s2);
int String_Does_Match_Nullproof (const char *s1, const char *s2);
int String_Does_Not_Match (const char *s1, const char *s2);
int String_Does_Not_Match_Caseless (const char *s1, const char *s2);
int String_Does_Not_Start_With (const char *s, const char *s_prefix);
int String_Does_Not_Start_With_Caseless (const char *s, const char *s_prefix);

int String_Does_Start_With (const char *s, const char *s_prefix);
int String_Does_Start_With_Caseless (const char *s, const char *s_prefix);

#endif // _DEBUG ... release version uses macro

char *String_Edit_Delete_At (char *s_edit, size_t s_size, size_t offset, size_t num);
int String_Edit_Insert_At (char* s_edit, size_t s_size, const char* s_insert, size_t offset);
char *String_Edit_LTrim_Whitespace_Excluding_Spaces (char *s_edit);
char *String_Edit_LTrim_Whitespace_Including_Spaces (char *s_edit);
char *String_Edit_RTrim_Whitespace_Excluding_Spaces (char *s_edit);
char *String_Edit_RTrim_Whitespace_Including_Spaces (char *s_edit);
char *String_Edit_Range_Delete (char *s_edit, size_t s_size, const char *s_start, const char *s_end);
char *String_Edit_Remove_End (char *s_edit);
char *String_Edit_RemoveTrailingSpaces (char *s_edit);
char *String_Edit_Replace (char *s_edit, size_t s_size, const char *s_find, const char *s_replace);
char *String_Edit_Replace_Char (char *s_edit, int ch_find, int ch_replace, int *outcount);
char *String_Edit_Repeat (char *s_edit, size_t s_size, const char *s_repeat, int count);
char *String_Edit_To_Lower_Case (char *s_edit);
char *String_Edit_To_Proper_Case (char *s_edit);
char *String_Edit_To_Upper_Case (char *s_edit);
char *String_Edit_Trim (char *s_edit); // memmoves
char *String_Edit_UnQuote (char *s_edit); // memmoves
char *String_Edit_Whitespace_To_Space (char *s_edit);
char *String_Find (const char *s, const char *s_find);
char *String_Find_Caseless (const char *s, const char *s_find);
char *String_Find_Skip_Past (const char *s, const char *s_find);
char *String_Find_Reverse (const char *s, const char *s_find);
char *String_Find_Char (const char *s, int ch_findchar);
char *String_Find_Char_Nth_Instance (const char *s, int ch_findchar, int n_instance);
char *String_Find_Char_Reverse (const char *s, int ch_findchar);
char *String_Find_End (const char *s);
char *String_Length_Copy (char *s_dest, size_t siz, const char *src_start, size_t length);
char *String_Range_Copy (char *s_dest, size_t siz, const char *src_start, const char *src_end);
int String_Range_Count_Char (const char *s_start, const char *s_end, int ch_findchar);

char *String_Range_Find_Char (const char *s_start, const char *s_end, int ch_findchar);
char *String_Range_Find_Char_Reverse (const char *s_start, const char *s_end, int ch_findchar);
char *String_Skip_Char (const char *s, int ch_findchar);
char *String_Skip_WhiteSpace_Excluding_Space (const char *s);
char *String_Skip_WhiteSpace_Including_Space (const char *s);
char *String_Skip_NonWhiteSpace (const char *s);

char *String_Write_NiceFloatString (char *s_dest, size_t siz, float floatvalue);

int String_To_Array_Index (const char *s, const char *s_array[]);
int String_To_Array_Index_Caseless (const char *s, const char *s_array[]);
int String_List_Match (const char *s, const char *s_find, int ch_delim);

#ifdef CORE_NEEDS_STRLCPY_STRLCAT
size_t strlcpy(char *dst, const char *src, size_t siz);
size_t strlcat(char *dst, const char *src, size_t siz);
#endif // CORE_NEEDS_STRLCPY_STRLCAT

#ifdef CORE_NEEDS_STRRSTR
// The strstr() function finds the last occurrence of the substring
char *strrstr(const char *s1, const char *s2);
#endif // CORE_NEEDS_STRRSTR

#ifdef CORE_NEEDS_STRCASESTR
//The strcasestr() function is similar to strstr(), but ignores the case of both strings.
char * strcasestr(const char *s, const char *find);
#endif // CORE_NEEDS_STRCASESTR


#ifdef CORE_NEEDS_STRNLEN
size_t strnlen (const char *string, size_t maxlen);
#endif // CORE_NEEDS_STRNLEN

#ifdef CORE_NEEDS_STRNDUP
char * strndup (const char *s, size_t n);
#endif // CORE_NEEDS_STRNDUP


#ifdef CORE_NEEDS_STRPTIME
// Example: strptime("6 Dec 2001 12:33:45", "%d %b %Y %H:%M:%S", &tm)
// time_t t:  month is 0 to 11, year begins at 1900 (add 1900 for actual year), day is day
// t = mktime(&tm); must be used to get weekday and day of year
char *strptime(const char *buf, const char *fmt, struct tm *tm);
#endif // CORE_NEEDS_STRPTIME


#ifdef CORE_NEEDS_VSNPRINTF_SNPRINTF
int c_vsnprintf (char * s, size_t n, const char * format, va_list arg);
int c_snprintfc (char * s, size_t n, const char * format, ...);
#else
#define c_vsnsprintf vsnprintf
#define c_snprintfc snprintf
#endif // CORE_NEEDS_VSNPRINTF_SNPRINTF


int StringAlloc_Cat (const char ** dst, const char *src);
int StringAlloc_Catf (const char ** dst, const char *fmt, ...);

// wildcard compare (i.e. if ( wildcmp ("bl?h.*", "blah.jpg") )
int wildcmp (const char *wild, const char *string);

// multi-wildcard compare (wildcard compare (i.e. if ( wildmultcmp ("bl?h.*;*.png", "blah.jpg") )
int wildmultcmp (const char *wild, const char *string);

#define VA_EXPAND(_text, _len, _fmt) \
	char		_text[_len]; \
	va_list		argptr; \
	va_start	(argptr, _fmt); \
	c_vsnprintf (_text, sizeof(_text), _fmt, argptr); \
	va_end		(argptr);


// environment.h
//
//#define MAX_CMD_256 256
//#define MAX_ARGS_80	80

typedef struct
{
	char		original[MAX_CMD_256];
	char		chopped[MAX_CMD_256];
	void 		*source;	// user defined behavior
	int			count;
	const char	*args[MAX_ARGS_80]; // pointers to chopped
	
} lparse_t;

lparse_t *Line_Parse_Alloc (const char *s, cbool allow_empty_args); // allow_empty_args can allow a blank final parameter
void *Line_Parse_Free (lparse_t *ptr);


typedef struct split_s
{
	int count;
	char **vars;
	int *isnum;
} split_t;


split_t *Split_Alloc (const char *s, const char *sdelim);
char *Split_To_String_Alloc (split_t *split, const char *sdelim);
split_t *Split_Free (split_t *split);

split_t *Split_EXA6_Alloc (const char *s); // STRICT.  Splits on spaces that are not inside a quote.
// E1: Type:TextBox Text:"Mary had a little lamb" Locked:True
// 1) Type:TextBox
// 2) Text:"Mary had a little lamb"
// 3) Locked:True
//
// E2:  Type:ListBox List:{frogs,dogs,"dig dogger"} Sorted:True
// 1) Type:ListBox
// 2) List:{frogs,dogs,"dig dogger"}
// 3) Sorted:True

// Spreads = Simple Spreadsheet

typedef struct spreads_s
{
	int rows;
	int cols;
	int count;
	char **vars;  // Think of vars[rows][cols]
	int *isnum;
	float *nums; // Value representation
} spreads_t;

spreads_t *Spreads_Free (spreads_t *spreads);
char *Spreads_To_String_Alloc (spreads_t *spreads, const char *sdelim, const char *slinedelim);
spreads_t *Spreads_Alloc (const char *s, const char *sdelim, const char *slinedelim);


char *String_C_Escape_Alloc (const char *s);
char *String_C_UnEscape_Alloc (const char *s, int *anyerror);

// http://stackoverflow.com/questions/1135841/c-multiline-string-literal
/* Example of newlines escaped into a string literal
const char *text2 =
  "Here, on the other hand, I've gone crazy \
  and really let the literal span several lines, \
  without bothering with quoting each line's \
  content. This works, but you can't indent.";
*/

void *c_memdup (const void *src, size_t len);
// Will add a trailing empty line after a newline.
#endif	// ! __STRING_LIB_H__

