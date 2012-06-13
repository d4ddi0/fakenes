/* FakeNES - A portable, Open Source NES and Famicom emulator.
 * Copyright © 2011-2012 Digital Carat Group
 * 
 * This is free software. See 'License.txt' for additional copyright and
 * licensing information. You must read and accept the license prior to any
 * modification or use of this software.
 */
#ifndef TOOLKIT__UNICODE_H__INCLUDED
#define TOOLKIT__UNICODE_H__INCLUDED
#include "Common/Global.h"
#include "Common/Types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* This is the implementation of the Unicode routines, which allow text to be
 * stored both in ASCII and multiple Unicode UTF encoded formats for the
 * purposes of representing multilingual text in a portable way. It primarily
 * uses the UTF8-CPP library as a workhorse, along with alot of wrapper and
 * glue code to make it as robust and extensible as possible.
 * 
 * The Unicode routines are split into two parts:
 * 
 * The legacy interface (UTF_STRING), which works with both C and C++. This
 * creates strings encoded directly in Unicode UTF formats. Such strings can
 * have their data exported directly to library functions and the operating
 * system (if the encoding is supported by either), but are very slow to work
 * with, as changing even a single character requires modifying the entire
 * encoding of the string.
 * 
 * The modern interface (ucstring), which uses the STL basic_string template
 * from <string>. This simply represents the data in an internal 32-bit
 * "chunky" format similar to UTF-32, making it very efficient to work with as
 * no real encoding is used. However, the downside is that such strings must be
 * encoded to a standard format prior to display.
 * 
 * Converting betwen the two interfaces is fairly simple as well. The legacy
 * interface actually does this and uses the modern interface to manage
 * everything internally (albeit making it even slower). For the best
 * performance with UTF_STRINGs, use UNICODE_FORMAT_FASTEST as the format and
 * only convert to another format prior to display, similar to how ucstrings
 * are meant to be used (although this increases memory usage).
 * 
 * These routines are designed for core multilanguage support and thus
 * currently lack a full set of Unicode-compatible features:
 * - Endianness and Byte Order Mark (BOM) is not supported. This is not usually
 *   an issue since most UTF input should match the endianess of the machine
 *   the software is currently running on.
 * - Text direction (left-right) is not supported directly, although it could
 *   be handled by a renderer if checks were added.
 * - Code points that either compose less than a single character or more than
 *   one character are not supported for string length checking. Currently each
 *   code point (UCCHAR) must represent an individual character.
 * 
 * Some of these limitations may be marginalized in future versions.
 */
/* Supported Unicode formats. */
enum UNICODE_FORMAT {
	UNICODE_FORMAT_ASCII	= 0,	/* Used by DOS, C and STL strings. */
	UNICODE_FORMAT_UTF8,		/* Used by Linux, compatible with ASCII. */
	UNICODE_FORMAT_UTF16,		/* Used by Windows. */
	UNICODE_FORMAT_UTF32,

	/* There is a special format UNICODE_FORMAT_LINEAR which guarantees
	 * that the stored character data is always in a linear format; in
	 * other words, you can access it directly as an array of UCCHARs by
	 * calling get_utf_string_linear_data().  Some functions for
	 * are optimized to work with linear data. */
	UNICODE_FORMAT_LINEAR	= UNICODE_FORMAT_UTF32,
	
	/* UNICODE_FORMAT_SMALLEST represents the most light-weight format,
	 * FASTEST represents the most efficient (performance-wise) format, and
	 * SAFEST represents the most compatible format. */
	UNICODE_FORMAT_SMALLEST	= UNICODE_FORMAT_UTF8,
	UNICODE_FORMAT_FASTEST	= UNICODE_FORMAT_LINEAR,
	UNICODE_FORMAT_SAFEST	= UNICODE_FORMAT_ASCII,

	/* This represents the native format of the platform. In other words,
	 * this is what it expects for filenames, console-printed text, etc. */
#if defined(SYSTEM_LINUX) || defined(SYSTEM_MACOSX)
	UNICODE_FORMAT_NATIVE	= UNICODE_FORMAT_UTF8
#elif defined(SYSTEM_WINDOWS)
	/* Windows 2000 and above only. */
	/* TODO: Add a test in the build system for Win9x and WinNT. */
	UNICODE_FORMAT_NATIVE	= UNICODE_FORMAT_UTF16
#else
	UNICODE_FORMAT_NATIVE	= UNICODE_FORMAT_ASCII
#endif
};
/* A single Unicode character. */
typedef UINT32	UCCHAR;
/* A stream of Unicode data. Each character may be made up of multiple segments
 * each represented by UTF_DATA (an unsigned byte). */
typedef UINT8	UTF_DATA; 

typedef struct _UTF_STRING {
	UNICODE_FORMAT	format;		/* Unicode format of the string. */
	UTF_DATA*	data;		/* Segmented data buffer. */
	UCCHAR* 	linear_data;	/* UNICODE_FORMAT_LINEAR only. */
	SIZE		size, length;	/* Size of the string in bytes, and
					   length of the string in characters. */
} UTF_STRING;

extern UTF_STRING*	create_utf_string(const UNICODE_FORMAT format);
extern UTF_STRING*	create_utf_string_from_data(const UNICODE_FORMAT format, const UTF_DATA* source, const UNICODE_FORMAT source_format, const SIZE size);
extern UTF_STRING*	create_utf_string_from_c_string(const UNICODE_FORMAT format, const char* source);
extern UTF_STRING*	create_utf_string_from_c_string_length(const UNICODE_FORMAT format, const char* source, const SIZE length);
extern void		delete_utf_string(UTF_STRING* target);
extern UNICODE_FORMAT	get_utf_string_format(const UTF_STRING* source);
extern UTF_DATA*	get_utf_string_data(UTF_STRING* source);
extern const UTF_DATA*	get_utf_string_data_const(const UTF_STRING* source);
extern UCCHAR*		get_utf_string_linear_data(UTF_STRING* source);
extern const UCCHAR*	get_utf_string_linear_data_const(const UTF_STRING* source);
extern SIZE		get_utf_string_size(const UTF_STRING* source);
extern SIZE		get_utf_string_length(const UTF_STRING* source);
extern UTF_STRING*	clear_utf_string(UTF_STRING* target);
extern UTF_STRING*	convert_utf_string(UTF_STRING* target, const UNICODE_FORMAT format);
extern UTF_STRING*	convert_utf_string_const(const UTF_STRING* source, const UNICODE_FORMAT format);
extern UCCHAR		utf_getc(const UTF_STRING* source, const SIZE position);
extern UCCHAR		utf_putc(UTF_STRING* target, const SIZE position, const UCCHAR code);
extern SIZE		utf_strcpy(UTF_STRING* to, const UTF_STRING* from);
extern SIZE		utf_strcat(UTF_STRING* to, const UTF_STRING* from);
extern SIZE		utf_strcmp(const UTF_STRING* first, const UTF_STRING* second);
extern SIZE		utf_stricmp(const UTF_STRING* first, const UTF_STRING* second);
extern UTF_STRING*	utf_strdup(const UTF_STRING* source);
extern SIZE		utf_strlen(const UTF_STRING* source);

#ifdef __cplusplus
} // extern "C"

#include <string>

typedef UCCHAR		ucchar;
typedef UTF_DATA	utf_data;
typedef UTF_STRING	utf_string;

typedef std::basic_string<ucchar> ucstring;

extern ucstring		ucstring_from_data(const utf_data* source, const UNICODE_FORMAT format);
extern ucstring		ucstring_from_string(const std::string& source);
extern ucstring		ucstring_from_c_string(const char* source);
extern ucstring		ucstring_from_c_string(const char* source, const size_type length);
extern ucstring		ucstring_from_utf_string(const utf_string& source);
extern utf_data*	ucstring_to_data(const ucstring& source, utf_data* target, const UNICODE_FORMAT format, size_type& size);
extern std::string	ucstring_to_string(const ucstring& source);
extern char*		ucstring_to_c_string(const ucstring& source, char* target, size_type& size);
extern utf_string*	ucstring_to_utf_string(const ucstring& source, const UNICODE_FORMAT format);
extern utf_string*	ucstring_to_utf_string(const ucstring& source, utf_string& target);
extern utf_string*	ucstring_to_utf_string(const ucstring& source, utf_string& target, const UNICODE_FORMAT format);

#endif /* __cplusplus */
#endif /* !TOOLKIT__UNICODE_H__INCLUDED */
