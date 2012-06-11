/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011-2012 Digital Carat Group

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include <cctype>
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

/* This allocates a new, empty string, which must then be destroyed
   when you are done with it via delete_utf_string().
*/
UTF_STRING* create_utf_string(const UNICODE_FORMAT format)
{
   UTF_STRING* utf_string = new UTF_STRING;
   if(!utf_string) {
      Warning("Out of memory.");
      return NULLPTR;
   }

   memset((void*)utf_string, 0, sizeof(UTF_STRING));
   clear_utf_string(utf_string);

   utf_string->format = format;
   return utf_string;
}

/* This creates a new string from a block of character data. The data
   may be in any supported Unicode format, and a check will first be
   made to ensure that it is valid. The size parameter defines the
   length of the string in bytes, not characters.

   Note: You can also use UNICODE_FORMAT_ASCII combined with a cast to
   convert from standard C strings to UTF. However, you shouldn't
   include the null termination character in the length.

   Returns the new string upon sucess, otherwise a null value.
*/
UTF_STRING* create_utf_string_from_data(const UNICODE_FORMAT format,
   const UTF_DATA* data, const UNICODE_FORMAT data_format, const SIZE size)
{
   Safeguard(data);

   if(size == 0)
      return create_utf_string(format);

   const ustring slave = ustring_from_data(data, data_format, size);
   return ustring_to_utf_string(slave, format);
}

/* These functions easily convert from C-style strings. */
UTF_STRING* create_utf_string_from_c_string(const UNICODE_FORMAT format, const char* input)
{
   Safeguard(input);

   const ustring slave = ustring_from_c_string(input);
   return ustring_to_utf_string(slave, format);
}

UTF_STRING* create_utf_string_from_c_string_size(const UNICODE_FORMAT format, const char* input, const SIZE size)
{
   Safeguard(input);

   const ustring slave = ustring_from_c_string(input, size);
   return ustring_to_utf_string(slave, format);
}

/* This de-allocates the memory used by a string created by any of the
   create_utf_string*() functions. The pointer must still be set to a
   null value manually for safety.
*/
void delete_utf_string(UTF_STRING* utf_string)
{
   if(utf_string) {
      clear_utf_string(utf_string);
      delete utf_string;
   }
}

/* These functions just return internal information contained in the
   UTF_STRING structure. This is the *proper* way to access such
   information, as fields from UTF_STRING should never be referenced
   directly in case they are changed in the future.

   get_utf_string_data():
      Returns a pointer to the internal data buffer.
   get_utf_string_const_data():
      Likewise, but with const-correctness.
   get_utf_string_size():
      Gets the size of the data, in bytes.
   get_utf_string_length():
      Gets the number of characters in the string.
*/
UTF_DATA* get_utf_string_data(UTF_STRING* utf_string)
{
   Safeguard(utf_string);
   return utf_string->data;
}

const UTF_DATA* get_utf_string_const_data(const UTF_STRING* utf_string)
{
   Safeguard(utf_string);
   return utf_string->data;
}

SIZE get_utf_string_size(const UTF_STRING* string)
{
   Safeguard(utf_string);
   return utf_string->size;
}

SIZE get_utf_string_length(const UTF_STRING* string)
{
   Safeguard(utf_string);
   return utf_string->length;
}

/* This clears a string, and de-allocates any memory that it maybe using
   internally to store character data.
*/
UTF_STRING* clear_utf_string(UTF_STRING* utf_string)
{
   Safeguard(utf_string);

   if(utf_string->data)
      delete[] utf_string->data;

   utf_string->data = NULLPTR;
   utf_string->size = 0;
   utf_string->length = 0;
}

/* This converts a string from one encoding to another. If some
   characters cannot be properly represented in the new format, they
   will be lost. Traditionally, this results in them being replaced
   with an ASCII question mark (?) character code 0x3F.

   If an error occurs, the string is left unmodified and a null value
   is returned instead of the converted string.
*/
UTF_STRING* convert_utf_string(const UTF_STRING* utf_string, const UNICODE_FORMAT format)
{
   Safeguard(utf_string);

   const ustring slave = ustring_from_utf_string(*utf_string);
   return ustring_to_utf_string(slave, *utf_string, format);
}

/* This gets a Unicode character from a string. Note that the position
   is to be specified in characters, not bytes. 

   Returns the character code, or zero if the requested character is
   past the end of the string. */
UCCHAR utf_getc(const UTF_STRING* utf_string, const SIZE position)
{
   Safeguard(utf_string);

   if(position >= utf_string->length)
      return 0;

   const ustring slave = ustring_from_utf_string(*utf_string);
   if(position >= slave.length())
      return 0;

   return slave[position];
}

/* This puts a Unicode character in a string. It only works if the
   string is already large enough to hold the character; the string
   will not automatically grow to accomodate the character.

   Returns the character code written to the string. If an error occurs,
   the string is left unmodified and zero is returned.
*/
UCCHAR utf_putc(UTF_STRING* utf_string, const SIZE position, const UCCHAR code)
{
   Safeguard(utf_string);

   if(position >= utf_string->length)
      return 0;

   ustring slave = ustring_from_utf_string(*utf_string);
   if(position >= slave.length())
      return 0;

   slave[position] = code;
   if(!ustring_to_utf_string(slave, *utf_string))
      return 0;

   return code;
}

/* This copies characters from one string to another. The format of the
   destination is not changed, so some characters from the source may be
   lost if they cannot be properly represented.

   Returns the number of characters copied. If an error occurs, the
   destination is left unmodified and zero is returned.
*/
SIZE utf_strcpy(UTF_STRING* to, const UTF_STRING* from)
{
   Safeguard(to);
   Safeguard(from);

   const ustring slave = ustring_from_utf_string(*from);
   if(!ustring_to_utf_string(slave, *to))
      return 0;

   return from->length;
}

/* This appends characters from one string to another. It has the same
   encoding limitations as utf_strcpy().

   Returns the number of characters appended. If an error occurs, the
   destination is left unmodified and zero is returned.
*/
SIZE utf_strcat(UTF_STRING* to, const UTF_STRING* from)
{
   Safeguard(to);
   Safeguard(from);

   const ustring slave = ustring_from_utf_string(*to) +
                         ustring_from_utf_string(*from);

   if(!ustring_to_utf_string(slave, *to))
      return 0;

   return from->length;
}

/* This compares two strings for equivalency. This is done using
   character codes with no actual lexical context.

   If both strings are the same, zero is returned. Otherwise, either a
   negative or a positive number is returned similar to strcmp().
*/
SIZE utf_strcmp(const UTF_STRING* first, const UTF_STRING* second)
{
   Safeguard(first);
   Safeguard(second);

   const ustring slave = ustring_from_utf_string(*first);
   return slave.compare(ustring_from_utf_string(*second));
}

/* This performs a case-insensetive compare. This only works for the
   English alphabet however, as it relies on the C library's toupper()
   function, which is typically limited to ASCII.
*/
SIZE utf_stricmp(const UTF_STRING* first, const UTF_STRING* second)
{
   Safeguard(first);
   Safeguard(second);

   ustring s1 = ustring_from_utf_string(*first);
   ustring s2 = ustring_from_utf_string(*second);

   size_type length = s1.length();
   for(size_type i = 0; i < length; i++) {
      ucchar& c = s1[i];
      c = toupper(c);
   }

   length = s2.length();
   for(size_type i = 0; i < length; i++) {
      ucchar& c = s2[i];
      c = toupper(c);
   }

   return s1.compare(s2);
}

/* This creates a copy of a string. The new copy must be deleted when
   you are done with it, just like the original. As this creates an
   entirely new copy of the same format, it is not vulnerable to the
   encoding limtations of utf_strcpy().

   Returns the duplicated string, or a null value on error.
*/
UTF_STRING* utf_strdup(const UTF_STRING* utf_string)
{
   Safeguard(utf_string);

   UTF_STRING* duplicate = create_utf_string(utf_string->format);
   if(!duplicate)
      return NULLPTR;

   if(utf_strcpy(duplicate, utf_string) == 0) {
      delete_utf_string(duplicate);
      return NULLPTR;
   }

   return duplicate;
}

/* This returns the length of a string, in characters. */
SIZE utf_strlen(const UTF_STRING* utf_string)
{
   Safeguard(utf_string);
   return utf_string->length;
}

// --------------------------------------------------------------------------------
// MODERN INTERFACE
// --------------------------------------------------------------------------------

/* This expands a UTF_DATA buffer to fill a ustring. The width of each data
 * segment is specified as a template argument (as a data type).
 * 
 * This is invoked after using the UTF8-CPP library to convert from the
 * dynamically encoded UTF formats to raw character codes of a fixed width. For
 * example, data that is already in ASCII format can easily be expanded to fill
 * a ustring by adding 3 bytes of padding to each character, transforming it
 * from an 8-bit 'char' value to a 32-bit UCCHAR value.
 * 
 * As the STL template basic_string which ustring is based upon utilizes
 * operator overloading, we can do this quite simply using concatentation of
 * individual characters combined with type-casting.
 */
template<typename TYPE>
void Expand(const utf_data* input, ustring& output, const size_type size)
{
   Safeguard(input);

   if(size == 0)
      return;

   const TYPE* buffer = (const TYPE*)input;
   const size_type length = size / sizeof(TYPE);

   for(size_type i = 0; i < length; i++)
      output += (ucchar)buffer[i];
}

/* This remaps a UTF_DATA buffer to fill an arbitrary buffer.
 * 
 * This is similar to the above template function, except instead of always
 * expanding to the internal format of UCCHAR, it can expand to any format for
 * which there is a data type available. THE CONVERSION MAY BE LOSSY!
 * 
 * The output buffer is represented as a STL vector of the data type.
 */
template<typename TYPE>
void Convert(const utf_data* input, vector<TYPE>& output, const size_type size)
{
   if(size == 0)
      return;

   const TYPE* buffer = (const TYPE*)input;
   const size_type length = size / sizeof(TYPE);

   for(size_type i = 0; i < length; i++)
      output.push_back(buffer[i]);
}

/* Like the above, but performs a lossless copy with no conversion. If any
 * conversion is to take place, an error will be generated.
 */
template<typename TYPE>
void Copy(const utf_data* input, vector<TYPE>& output, const size_type size)
{
   if(size == 0)
      return;

   /* Make sure the data types match (the length is the same). */
   if(size != (size / sizeof(TYPE)))
      GenericWarning();
   
   /* When the source and destination buffers are of the same data type,
    * invoking Convert() is equivalent to a lossless copy. */
   Convert<TYPE>(input, output, size);
}

/* This creates a ustring from a block of data. The data may be in
   any supported Unicode format.
*/
ustring ustring_from_data(const utf_data* data, const UNICODE_FORMAT format, const size_type size)
{
   Safeguard(data);

   ustring output = ustring();
   if(size == 0)
      return output;

   switch(format) {
      case UNICODE_FORMAT_ASCII: {
         // Convert to internal format.
         Expand<uint8>(data, output, size);
         break;
      }

      case UNICODE_FORMAT_UTF8: {
         // Copy the read-only data into a read-write buffer.
         vector<uint8> buffer;
         Copy<uint8>(data, buffer, size);

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
         // Copy the read-only data into a vector.
         vector<uint16> buffer;
         Copy<uint16>(data, copied, size);

         /* Convert from UTF-16 to UTF-8 since the UTF8-CPP library can only
	  * repair broken sequences in the UTF-8 encoding. */
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

      case UNICODE_FORMAT_UTF32: {
         // Convert to internal format.
         Copy<uint32>(data, converted, size);
         break;
      }

      default: {
         GenericWarning();
         break;
      }
   }

   return output;
}

/* This creates a ustring from an STL string. Note that the format is
   assumed to be ASCII. If you want another format, you will need to
   call ustring_from_data() manually. */
*/
ustring ustring_from_string(const std::string& input)
{
   // Check for an empty input string.
   if(input.length() == 0)
      return ustring();

   return ustring_from_c_string(input.c_str());
}

/* This creates a ustring from a C string, automatically detecting the
   length with a call to strlen(), which is potentially unsafe. The
   format is assumed to be ASCII. To use another format, call
   ustring_from_data() manually. */
*/
ustring ustring_from_c_string(const char* input)
{
   Safeguard(input);

   const size_type legnth = strlen(input);
   if(length == 0)
      return ustring();

   return ustring_from_c_string(input, length);
}

/* This version allows you to specify the length of the input string
   manually, which can be safer. As this function is designed to accept
   C-style strings, rather than raw data, it assumes that 'size'
   includes a null termination character.
*/
ustring ustring_from_c_string(const char* input, const size_type size)
{
   Safeguard(input);

   const size_type length = size - 1;
   if(length == 0)
      return ustring();

   return ustring_from_data((const utf_data*)input, UNICODE_FORMAT_ASCII, length);
}

/* This creates a ustring from a UTF_STRING. This is mainly used by the
   legacy interface as a workhorse.
*/
ustring ustring_from_utf_string(const utf_string& input)
{
   // Check for an empty input string.
   if(!input.data || (input.size == 0) || (input.length == 0))
      return ustring();

   return ustring_from_data(input.data, input.format, input.size);
}

/* This converts a ustring to a block of data using the specified format.
   As the length of the encoded data can't be known ahead of time, this
   function automatically allocates the neccessary memory.

   Returns the block of data upon success, which must later be deleted,
   and updates the 'size' parameter to reflect how much memory was
   allocated to store the encoded data.
*/
utf_data* ustring_to_data(const ustring& input, const UNICODE_FORMAT format, size_type& size)
{
   const size_type length = input.length();
   vector<utf_data> output;

   switch(format) {
      case UNICODE_FORMAT_ASCII: {
         for(size_type i = 0; i < length; i++) {
            const ucchar& c = input[i];

            const utf_data data = (c <= 0x7F) ? c : '?';
            output.push_back(data);
         }

         break;
      }

      case UNICODE_FORMAT_UTF8: {
         utf8::utf32to8(input.begin(), input.end(), back_inserter(output));
         break;
      }

      case UNICODE_FORMAT_UTF16: {
         vector<uint16> temp;
         vector<uint32> temp2;
         utf8::utf32to8(input.begin(), input.end(), back_inserter(temp));
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
            const ucchar& c = input[i];

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

      default: {
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
utf_data* ustring_to_data(const ustring& input, utf_data* output, const UNICODE_FORMAT format, size_type& size)
{
   Safeguard(output);
   Safeguard(size > 0);

   size_type temp_size = 0;
   const utf_data* temp = ustring_to_data(input, format, temp_size);
   const size_type count = MIN2(size, temp_size);
   memcpy((void*)output, (void*)temp, count);
   delete[] temp;

   size = count;
   return output;
}

/* This converts a ustring to an STL string, using the ASCII character
   encoding. For a different encoding, use ustring_to_data().
*/
std::string ustring_to_string(const ustring& input)
{
   size_type size;
   utf_data* data = ustring_to_data(input, UNICODE_FORMAT_ASCII, size);
   string output = string();
   for(size_type i = 0; i < size; i++) {
      const char c = data[i];
      output += c;
   }

   delete[] data;
   return output;
}

/* This converts a ustring to a C string, using the ASCII character
   encoding. For a different encoding, use ustring_to_data(). The
   resulting C-style string will always be null terminated, even if
   the output buffer is not large enough.

   Returns the actual number of characters written to the output
   buffer, including thet null termination character.
*/
char* ustring_to_c_string(const ustring& input, char* output, size_type& size)
{
   Safeguard(output);

   if( size == 0 )
      return output;

   const std::string converted = ustring_to_string(input);
   const size_type count = MIN2(converted.length(), size - 1);
   for(size_type i = 0; i < count; i++)
      output[i] = input[i];

   output[i] = 0;

   size = count + 1;
   return output;
}

utf_string* ustring_to_utf_string(const ustring& input, const UNICODE_FORMAT format)
{
}

utf_string* ustring_to_utf_string(const ustring& input, utf_string& output)
{
}

utf_string* ustring_to_utf_string(const ustring& input, utf_string& output, const UNICODE_FORMAT format)
{
}
