/* FakeNES GT - A portable, Open Source NES and Famicom emulator.
 * Copyright © 2011-2012 Digital Carat Group
 * 
 * This is free software. See 'License.txt' for additional copyright and
 * licensing information. You must read and accept the license prior to any
 * modification or use of this software.
 */
#include <utf8.h>	// For UTF8-CPP functions.
#include <cctype>	// For toupper().
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>
#include "Common/Global.h"
#include "Common/Debug.h"
#include "Common/Types.h"
#include "Unicode.h"

using namespace std;

// --------------------------------------------------------------------------------
// LEGACY INTERFACE
// --------------------------------------------------------------------------------

/* This allocates a new, empty UTF_STRING, which must then be destroyed when
 * you are done with it via delete_utf_string().
 */
UTF_STRING* create_utf_string(const UNICODE_FORMAT format) {
	/* Allocate and initialize the memory to contain the UTF_STRING class.
	 * The memory for data is allocated separately as needed. */
	UTF_STRING* output = new UTF_STRING;
	if( !output ) {
		Warning("Out of memory.");
		return NULLPTR;
	}

	memset((void*)output, 0, sizeof(UTF_STRING));

	/* Just initializing the memory of the UTF_STIRNG isn't quite
	 * sufficient; we also have to call clear_utf_string() to ensure that
	 * it contains a valid empty string. */
	clear_utf_string(output);

	// Lastly, we set the format and return the string.
	output->format = format;
	return output;
}

/* This creates a new UTF_STRING from a block of character data, which may be
 * in any supported Unicode format, and the size parameter defines the length
 * of the string in UTF_DATA segments (bytes), not characters.
 * 
 * Note that if the requested format is different from the source format,
 * some loss (from unrepresentable characters) may occur.
 * 
 * Returns the new string upon sucess, otherwise a null value.
 */
UTF_STRING* create_utf_string_from_data(const UNICODE_FORMAT format, const UTF_DATA* source, const UNICODE_FORMAT source_format, const SIZE size) {
	Safeguard( source );

	if( size == 0 )
		return create_utf_string(format);

	const ucstring slave = ucstring_from_data(source, source_format, size);
	return ucstring_to_utf_string(slave, format);
}

/* These functions easily convert from C strings. Make sure that any strings
 * passed to these functions are properly null terminated.
 */
UTF_STRING* create_utf_string_from_c_string(const UNICODE_FORMAT format, const char* source) {
	Safeguard( source );

	const size_type length = strlen(source);
	if( length == 0 )
		return create_utf_string(format);

	return create_utf_string_from_c_string_length(format, source, length);
}

UTF_STRING* create_utf_string_from_c_string_length(const UNICODE_FORMAT format, const char* source, const SIZE length) {
	Safeguard( source );

	if( length == 0 )
		return create_utf_string(format);

	const ucstring slave = ucstring_from_c_string(source, length);
	return ucstring_to_utf_string(slave, format);
}

/* This de-allocates the memory used by a UTF_STRING created by any of the
 * create_utf_string*() functions. After calling delete_utf_string(), the
 * pointer should be set to a null value for safety.
 */
void delete_utf_string(UTF_STRING* target) {
	if( target ) {
		clear_utf_string(target);
		delete target;
	}
}

/* These functions just return internal information contained within the
 * UTF_STRING class. This is the *proper* way to access such information, as
 * fields from UTF_STRING should never be referenced directly in case they are
 * changed in a future version.
 * 
 * get_utf_string_data():
 * 	Returns a pointer to the internal data buffer.
 * get_utf_string_data_const():
 * 	Likewise, but with const-correctness.
 * get_utf_string_linear_data():
 * 	Returns a pointer to the internal data buffer, in linear format.
 * 	This only works with UNICODE_FORMAT_LINEAR.
 * get_utf_string_linear_data_const():
 * 	Likewise, but with const-correctness.
 * get_utf_string_size():
 * 	Gets the size of the data, in UTF_DATA segments.
 * get_utf_string_length():
 * 	Gets the number of actual characters in the string.
 */
UTF_DATA* get_utf_string_data(UTF_STRING* source) {
	Safeguard( source );
	return source->data;
}

const UTF_DATA* get_utf_string_data_const(const UTF_STRING* source) {
	Safeguard( source );
	return source->data;
}

UCCHAR* get_utf_string_linear_data(UTF_STRING* source) {
	Safeguard( source );

	if( source->format != UNICODE_FORMAT_LINEAR )
		return NULLPTR;
	
	return source->linear_data;
}

const UCCHAR* get_utf_string_linear_data_const(const UTF_STRING* source) {
	Safeguard( source );

	if( source->format != UNICODE_FORMAT_LINEAR )
		return NULLPTR;

	return source->linear_data;
}

SIZE get_utf_string_size(const UTF_STRING* source) {
	Safeguard( source );
	return source->size;
}

SIZE get_utf_string_length(const UTF_STRING* source) {
	Safeguard( source );
	return source->length;
}

/* This clears a UTF_STRING, and de-allocates any memory that it may be using
 * internally to store character data.
 * 
 * Empty strings are format-specific, so the safest way to ensure that a
 * UTF_STRING is empty is to call clear_utf_string(). Check the return value of
 * utf_strlen() when testing for emptiness (length=0).
 */
UTF_STRING* clear_utf_string(UTF_STRING* target) {
	Safeguard( target );

	if( target->data )
		delete[] target->data;

	target->data = NULLPTR;
	target->linear_data = NULLPTR;
	target->size = 0;
	target->length = 0;
}

/* This converts a UTF_STRING from one encoding to another. If some characters
 * cannot be properly represented in the new format, they may be lost.
 * 
 * If an error occurs, the string is left unmodified and a null value is
 * returned instead of the converted string.
 */
UTF_STRING* convert_utf_string(UTF_STRING* target, const UNICODE_FORMAT format) {
	Safeguard( target );

	const ucstring slave = ucstring_from_utf_string(*target);
	return ucstring_to_utf_string(slave, *target, format);
}

/* This creates a new UTF_STRING instead of performing an in-place conversion,
 * allowing it be used with read-only source strings. The returned UTF_STRING
 * must be deleted when you are done with it.
 */
UTF_STRING* convert_utf_string_const(const UTF_STRING* source, const UNICODE_FORMAT format) {
	Safeguard( source );

	const ucstring slave = ucstring_from_utf_string(*source);
	return ucstring_to_utf_string(slave, format);
}

/* This gets a single Unicode character from a UTF_STRING. Note that the
 * position is to be specified in characters, not bytes.
 * 
 * Returns the character code, or zero if the requested character is past the
 * end of the string.
 */
UCCHAR utf_getc(const UTF_STRING* source, const SIZE position) {
	Safeguard( source );

	if( position >= source->length )
		return 0;

	if( source->format == UNICODE_FORMAT_LINEAR ) {
		return source->linear_data[position];
	}
	else {
		const ucstring slave = ucstring_from_utf_string(*source);
		if( position >= slave.length() )
			return 0;

		return slave[position];
	}
}

/* This sets a Unicode character in a UTF_STRING. It only works if the string
 * is already large enough to hold the character; the string will not
 * automatically grow to accomodate the new character.
 * 
 * Note that if you write a character code that is out of range of the current
 * Unicode format of the string, the behavior is undefined, so this function is
 * only 100% safe to use with UNICODE_FORMAT_LINEAR which is guaranteed to be
 * able to store a full UCCHAR in all cases.
 * 
 * Returns the character code written to the string. If an error occurs, the
 * string is left unmodified and zero is returned.
 */
UCCHAR utf_putc(UTF_STRING* target, const SIZE position, const UCCHAR code) {
	Safeguard( target );

	if( position >= target->length )
		return 0;

	if( target->format == UNICODE_FORMAT_LINEAR ) {
		target->linear_data[position] = code;
	}
	else {
		ucstring slave = ucstring_from_utf_string(*target);
		if( position >= slave.length() )
			return 0;

		slave[position] = code;
		if( !ucstring_to_utf_string(slave, *target) )
			return 0;
	}

	return code;
}

/* This copies characters from one UTF_STRING to another. The format of the
 * destination is not changed, so some characters from the source may be lost
 * if they cannot be properly represented.
 * 
 * Returns the number of characters copied. If an error occurs, the destination
 * is left unmodified and zero is returned.
 */
SIZE utf_strcpy(UTF_STRING* to, const UTF_STRING* from) {
	Safeguard( to );
	Safeguard( from );

	const ucstring slave = ucstring_from_utf_string(*from);
	if( !ucstring_to_utf_string(slave, *to) )
		return 0;

	return from->length;
}

/* This appends characters from one UTF_STRING to another. It has the same
 * encoding limitations as utf_strcpy().
 * 
 * Returns the number of characters appended. If an error occurs, the
 * destination is left unmodified and zero is returned.
 */
SIZE utf_strcat(UTF_STRING* to, const UTF_STRING* from) {
	Safeguard( to );
	Safeguard( from );

	const ucstring slave = ucstring_from_utf_string(*to) + ucstring_from_utf_string(*from);
	if( !ucstring_to_utf_string(slave, *to) )
		return 0;

	return from->length;
}

/* This compares two UTF_STRINGs for value equivalency. This is done using
 * character codes with no actual lexical context.
 * 
 * If both strings are the same, zero is returned. Otherwise, either a negative
 * or a positive number is returned similar to strcmp().
 */
SIZE utf_strcmp(const UTF_STRING* first, const UTF_STRING* second) {
	Safeguard( first );
	Safeguard( second );

	const ucstring slave = ucstring_from_utf_string(*first);
	return slave.compare(ucstring_from_utf_string(*second));
}

/* This performs a case-insensetive compare on two UTF_STRINGs. This only
 * works for the English alphabet however, as it relies on the C library's
 * toupper() function, which is typically limited to A-Z.
 * 
 * A localized version of toupper() is provided in C++ via STL, unfortunately
 * the language does not define any locals other than the default C locale,
 * making it useless without some kind of platform- or language-specific code.
 */
SIZE utf_stricmp(const UTF_STRING* first, const UTF_STRING* second) {
	Safeguard( first );
	Safeguard( second );

	ucstring s1 = ucstring_from_utf_string(*first);
	size_type length = s1.length();
	for( size_type i = 0; i < length; i++ ) {
		ucchar& c = s1[i];
		c = toupper(c);
	}

	utring s2 = ucstring_from_utf_string(*second);
	length = s2.length();
	for( size_type i = 0; i < length; i++ ) {
		ucchar& c = s2[i];
		c = toupper(c);
	}

	return s1.compare(s2);
}

/* This creates a copy of a UTF_STRING. The new copy must be deleted when you
 * are done with it, just like the original.
 * 
 * Returns the duplicated string, or a null value on error.
 */
UTF_STRING* utf_strdup(const UTF_STRING* source) {
	Safeguard( source );

	UTF_STRING* duplicate = create_utf_string(source->format);
	if( !duplicate )
		return NULLPTR;

	if( utf_strcpy(duplicate, source) == 0 ) {
		delete_utf_string(duplicate);
		return NULLPTR;
	}

	return duplicate;
}

/* This returns the length of a UTF_STRING, in characters. This is functionally
 * identical to get_utf_string_length(), except that it also checks for a valid
 * data buffer and data size, making it generally safer.
 */
SIZE utf_strlen(const UTF_STRING* source) {
	Safeguard( source );

	if( !source->data ||
	    ( source->size == 0 ) || ( source->length == 0 ) )
		return 0;
	
	return source->length;
}

// --------------------------------------------------------------------------------
// MODERN INTERFACE
// --------------------------------------------------------------------------------

/* This expands a UTF_DATA buffer to fill a ucstring. The width of each data
 * segment is specified as a template argument data type.
 * 
 * This is invoked after using the UTF8-CPP library to convert from the
 * dynamically encoded UTF formats to raw character codes of a fixed width. For
 * example, data that is already in ASCII format can easily be expanded to fill
 * a ucstring by adding 3 bytes of padding to each character, transforming it
 * from an 8-bit 'char' value to a 32-bit UCCHAR value.
 * 
 * As the STL template basic_string which ucstring is based upon utilizes
 * operator overloading, we can do this quite simply using concatentation of
 * individual characters combined with type-casting.
 */
template<typename TYPE>
void Expand(const utf_data* input, ucstring& output, const size_type size) {
	Safeguard( input );

	// Start with an empty output string.
	output.clear();
	if( size == 0 )
		return;

	const TYPE* buffer = (const TYPE*)input;
	const size_type length = size / sizeof(TYPE);
	for( size_type i = 0; i < length; i++ )
		output += (ucchar)buffer[i];
}

/* This converts a UTF_DATA buffer to fill an arbitrary buffer.
 * 
 * This is similar to the above template function, except instead of always
 * expanding to the internal format of UCCHAR, it can convert to any format for
 * which there is a data type available. THE CONVERSION MAY BE LOSSY!
 * 
 * The output buffer is represented as a STL vector, sized as needed.
 */
template<typename INPUT_TYPE, typename OUTPUT_TYPE>
void Convert(const utf_data* input, vector<OUTPUT_TYPE>& output, const size_type size, const unsigned minimum, const unsigned maximum) {
	output.clear();
	if( size == 0 )
		return;

	const INPUT_TYPE* buffer = (const INPUT_TYPE*)input;
	const size_type length = size / sizeof(INPUT_TYPE);
	for( size_type i = 0; i < length; i++ ) {
		const unsigned c = Clamp<unsigned>((unsigned)input[i], minimum, maximum);
		output.push_back((OUTPUT_TYPE)c);
	}
}

/* Like the above, but performs a lossless copy with no conversion.
 */
template<typename TYPE>
void Copy(const utf_data* input, vector<TYPE>& output, const size_type size) {
	if( size == 0 ) {
		output.clear();
		return;
	}

	// Vectors are allocated in 'data units', rather than bytes.
	const size_type length = size / sizeof(TYPE);
	output.resize(length);
	/* When copying, reverse-compute length to avoid a potential buffer
	 * overflow in cases where the input buffer does not contain an exact
	 * integer quantity of data units. */
	memcpy((void*)input, ArrayCast(output, void*), length * sizeof(TYPE));
}

/* This creates a ucstring from a block of data. The data may be in
   any supported Unicode format.
*/
ucstring ucstring_from_data(const utf_data* source, const UNICODE_FORMAT format, const size_type size) {
	Safeguard( source );

	ucstring output = ucstring();
	if( size == 0 )
		return output;

	switch( format ) {
		case UNICODE_FORMAT_ASCII:
			// Replace any invalid code sequences.
			vector<char> patched;
			Convert<uint8, char>(source, patched, size, 0x0, 0x7F);

			// Convert to internal format.
			Expand<char>(ArrayCast(patched, const utf_data*), output, size);
			patched.clear();
			break;
      
		case UNICODE_FORMAT_UTF8: {
			// Copy the read-only data into a read-write buffer.
			vector<uint8> buffer;
			Copy<uint8>(source, buffer, size);

			// Replace any invalid code sequences.
			vector<uint8> patched;
			utf8::replace_invalid(buffer.begin(), buffer.end(), back_inserter(patched));
			buffer.clear();

			// Convert to internal format.
			utf8::utf8to32(patched.begin(), patched.end(), back_inserter(output));
			patched.clear();
			break;
		}

		case UNICODE_FORMAT_UTF16: {
			// Copy the read-only data into a read-write buffer.
			vector<uint16> buffer;
			Copy<uint16>(source, buffer, size);

			/* Convert from UTF-16 to UTF-8 since the UTF8-CPP
			 * library can only repair broken code sequences in the
			 * UTF-8 encoding. */
			vector<uint8> converted;
			utf8::utf16to8(buffer.begin(), buffer.end(), back_inserter(converted));
			buffer.clear();

			// Replace any invalid code sequences.
			vector<uint8> patched;
			utf8::replace_invalid(converted.begin(), converted.end(), back_inserter(patched));
			converted.clear();

			// Convert to internal format.
			utf8::utf8to32(patched.begin(), patched.end(), back_inserter(output));
			patched.clear();
			break;
		}

		case UNICODE_FORMAT_UTF32:
			// Convert to internal format.
			Expand<uint32>(source, output, size);
			break;

		default:
			GenericWarning();
			break;
	}

	return output;
}

/* This creates a ucstring from an STL string.
 */
ucstring ucstring_from_string(const std::string& source) {
	// Check for an empty input string.
	if( source.length() == 0 )
		return ucstring();

	return ucstring_from_c_string(source.c_str());
}

/* This creates a ucstring from a C string, automatically detecting the length
 * with a call to strlen(), which is potentially unsafe. Make sure that any
 * strings passed to this function are properly null terminated.
 */
ucstring ucstring_from_c_string(const char* source) {
	Safeguard( source );

	const size_type length = strlen(source);
	if( length == 0 )
		return ucstring();

	return ucstring_from_c_string(source, length);
}

/* This version allows you to explicitly specify the length of the input string
 * (in characters), and thus can safely handle unterminated strings.
*/
ucstring ucstring_from_c_string(const char* source, const size_type length) {
	Safeguard( source );

	if( length == 0 )
		return ucstring();

	return ucstring_from_data((const utf_data*)source, UNICODE_FORMAT_ASCII, length);
}

/* This creates a ucstring from a UTF_STRING. This is mainly used by the
   legacy interface as a workhorse.
*/
ucstring ucstring_from_utf_string(const utf_string& source) {
	// Check for an empty input string.
	if( utf_strlen(source) == 0 )
		return ucstring();

	return ucstring_from_data(source.data, source.format, source.size);
}

/* This converts a ucstring to a block of data using the specified format. As
 * the size of the encoded data can't be known ahead of time, this function
 * automatically allocates the neccessary memory.
 * 
 * Returns the block of data upon success, which must later be delete[]'d, and
 * updates the 'size' parameter to reflect how much memory was allocated to
 * fully store the encoded data.
*/
utf_data* ucstring_to_data(const ucstring& source, const UNICODE_FORMAT format, size_type& size) {
	const size_type length = source.length();

	vector<utf_data> output;

	switch( format ) {
		case UNICODE_FORMAT_ASCII:
			Convert<ucchar, uint8>(source, copied, size);
			break;

		case UNICODE_FORMAT_UTF8:
			utf8::utf32to8(source.begin(), source.end(), back_inserter(output));
			break;
      
		case UNICODE_FORMAT_UTF16: {
			vector<uint16> temp;
			vector<uint32> temp2;
			utf8::utf32to8(source.begin(), source.end(), back_inserter(temp));
			utf8::utf8to16(temp.begin(), temp.end(), back_inserter(temp2));

			for(size_type i = 0; i < length; i++) {
				const ucchar& c = temp2[i];
				const utf_data d0 = (c >> 8) & 0xFF;
				const utf_data d1 = c & 0xFF;
				output.push_back(d0);
				output.push_back(d1);
			}

			break;
		}

		case UNICODE_FORMAT_UTF32: {
			for(size_type i = 0; i < length; i++) {
				const ucchar& c = source[i];
				const utf_data d0 = (c >> 24) & 0xFF;
				const utf_data d1 = (c >> 16) & 0xFF;
				const utf_data d2 = (c >> 8) & 0xFF;
				const utf_data d3 = c & 0xFF;
				output.push_back(d0);
				output.push_back(d1);
				output.push_back(d2);
				output.push_back(d3);
			}

			break;
		}

		default:
			GenericWarning();
			break;
	}
	}

   const size_type new_size = output.size();
   const utf_data* copy = new utf_data[new_size];
   memcpy((void*)copy, (void*)&output[0], new_size);
   output.clear();

   size = new_size;
   return copy;
}

/* This version stores the encoded data in a buffer instead of a block
   of memory that has to be deleted. The amount of bytes written to
   the buffer can be limited, although if not enough space is
   available the encoding may break and become invalid.
*/
utf_data* ucstring_to_data(const ucstring& source, utf_data* target, const UNICODE_FORMAT format, size_type& size)
{
   Safeguard( target );

   size_type temp_size = 0;
   const utf_data* temp = ucstring_to_data(input, format, temp_size);
   const size_type count = MIN2(size, temp_size);
   memcpy((void*)output, (void*)temp, count);
   delete[] temp;

   size = count;
   return output;
}

/* This converts a ucstring to an STL string, using the ASCII character
   encoding. For a different encoding, use ucstring_to_data().
*/
std::string ucstring_to_string(const ucstring& source)
{
   size_type size;
   utf_data* data = ucstring_to_data(source, UNICODE_FORMAT_ASCII, size);
   string output = string();
   for(size_type i = 0; i < size; i++) {
      const char c = data[i];
      output += c;
   }

   delete[] data;
   return output;
}

/* This converts a ucstring to a C string, using the ASCII character
   encoding. For a different encoding, use ucstring_to_data(). The
   resulting C-style string will always be null terminated, even if
   the output buffer is not large enough.

   Returns the actual number of characters written to the output
   buffer, including thet null termination character.
*/
char* ucstring_to_c_string(const ucstring& source, char* target, size_type& size)
{
   Safeguard( target );

   if( size == 0 )
      return target;

   const std::string converted = ucstring_to_string(source);
   const size_type count = MIN2(converted.length(), size - 1);
   for(size_type i = 0; i < count; i++)
      target[i] = source[i];

   target[i] = 0;

   size = count + 1;
   return target;
}

utf_string* ucstring_to_utf_string(const ucstring& source, const UNICODE_FORMAT format)
{
}

utf_string* ucstring_to_utf_string(const ucstring& source, utf_string& target)
{
}

utf_string* ucstring_to_utf_string(const ucstring& source, utf_string& target, const UNICODE_FORMAT format)
{
}
