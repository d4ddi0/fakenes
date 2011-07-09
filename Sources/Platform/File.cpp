/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Common/Global.h"
#include "Common/Debug.h"
#include "Common/Types.h"
#include "File.h"

namespace {

// Determine the native byte order.
#ifdef LSB_FIRST
   // Use little-endian byte ordering.
   const FILE_ORDER NativeOrder = FILE_ORDER_INTEL;
#else
   // Use big-endian byte ordering.
   const FILE_ORDER NativeOrder = FILE_ORDER_MOTOROLA;
#endif

// This is how much the data buffer grows at a time during writing.
const FILE_SIZE ChunkSize = 4096;

} // namespace anonymous

// --------------------------------------------------------------------------------

FILE_CONTEXT* open_file(const UDATA* filename, const FILE_MODE mode, const FILE_ORDER order)
{
   Safeguard(filename);

   FILE_CONTEXT* file = malloc(sizeof(FILE_CONTEXT));
   if(!file) {
      Warning("Out of memory.");
      return NULL;
   }

   memset(file, 0, sizeof(FILE_CONTEXT));

   file->type = FILE_TYPE_PHYSICAL;
   file->mode = mode;
   file->order = order;

   const char* access;
   if(mode == FILE_MODE_READ) {
      access = "r";
   } else if(mode == FILE_MODE_WRITE) {
      access = "w":
   } else {
      GenericWarning();
      return NULL;
   }

   file->handle = fopen(filename, access);
   if(!file->handle) {
      free(file);
      return NULL;
   }

   return file;
}

FILE_CONTEXT* open_memory_file(const FILE_MODE mode, const FILE_ORDER order)
{
   FILE_CONTEXT* file = malloc(sizeof(FILE_CONTEXT));
   if(!file) {
      Warning("Out of memory.");
      return NULL;
   }

   memset(file, 0, sizeof(FILE_CONTEXT));

   file->type = FILE_TYPE_VIRTUAL;
   file->mode = mode;
   file->order = order;

   file->buffer.data = malloc(ChunkSize);
   if(!file->buffer.data) {
      Warning("Out of memory.");
      free(file);

      return NULL;
   }

   file->buffer.limit = ChunkSize;

   return file;
}

void* get_file_buffer(FILE_CONTEXT* file, FILE_SIZE* size)
{
   Safeguard(file);

   if(file->type == FILE_TYPE_PHYSICAL) {
      Warning("This is not a memory file.");
      return NULL;
   }

   if(size)
      *size = file->buffer.size;

   return file->buffer.data;
}

void set_file_buffer(FILE_CONTEXT* file, void* data, const FILE_SIZE size)
{
   Safeguard(file);
   Safeguard(data);
   Safeguard(size > 0);

   if(file->type == FILE_TYPE_PHYSICAL) {
      Warning("This is not a memory file.");
      return;
   }

   if(file->mode == FILE_MODE_WRITE) {
      Warning("This action is not valid in write mode.");
      return;
   }

   file->buffer.data = data;
   file->buffer.size = size;
   file->buffer.limit = size;
   file->buffer.position = 0;
}

// --------------------------------------------------------------------------------

static forceinline uint16 SwapWord(const uint16 data, FILE_ORDER order)
{
   // We only need to swap when a mismatch is detected.
   if((order == FILE_ORDER_NATIVE) || (order == NativeOrder))
      return data;

   const uint8 lower = data & 0x00FF;
   const uint8 upper = (data & 0xFF00) >> 8;
   return (lower << 8) | upper;
}

static forceinline uint32 SwapLong(const uint32 data, FILE_ORDER order)
{
   if((order == FILE_ORDER_NATIVE) || (order == NativeOrder))
      return data;

   const uint16 lower = data & 0x0000FFFF;
   const uint16 upper = (data & 0xFFFF0000) >> 16;
   return (SwapWord(upper) << 16) | SwapWord(lower);
}

static FILE_SIZE File_Read(FILE_CONTEXT* file, void* data, const FILE_SIZE size)
{
   Safeguard(file);
   Safeguard(data);

   // Verify that the file is opened in read mode.
   if(file->mode != FILE_MODE_READ) {
      GenericWarning();
      return 0;
   }

   if(size == 0) {
      // Nothing to do.
      return 0;
   }

   if(file->type == FILE_TYPE_PHYSICAL) {
      if(!file->handle) {
         GenericWarning();
         return 0;
      }

      return fread(data, size, 1, file->handle);
   }

   FILE_BUFFER& buffer = file->buffer;
   if(!buffer.data) {
      GenericWarning();
      return 0;
   }

   const FILE_SIZE start = buffer.offset;
   const FILE_SIZE end = Clamp<FILE_SIZE>(buffer.offset + size, 0, buffer.size - 1);
   const FILE_SIZE count = end - start;

   memcpy(data, buffer.data + start, count);
   return count;
}

static FILE_SIZE File_Write(FILE_CONTEXT* file, const void* data, const FILE_SIZE size)
{
   Safeguard(file);
   Safeguard(data);

   if(file->mode != FILE_MODE_WRITE) {
      GenericWarning();
      return 0;
   }

   if(size == 0) {
      // Nothing to do.
      return 0;
   }
   
   if(file->type == FILE_TYPE_PHYSICAL) {
      if(!file->handle) {
         GenericWarning();
         return 0;
      }

      return fwrite(data, size, 1, file->handle);
   }

   FILE_BUFFER& buffer = file->buffer;
   if(!buffer.data) {
      GenericWarning();
      return 0;
   }

   const FILE_SIZE end = buffer.offset + size;
   if(end > buffer.limit) {
      FILE_SIZE resize = 0;
      while(resize < end)
         resize += ChunkSize;

      buffer.data = realloc(buffer.data, resize);
      if(!buffer.data)
         GenericWarning();

      buffer.limit = resize;
   }

   memcpy(buffer.data + buffer.offset, data, size);
   buffer.offset += size;

   // If we passed the end of the file, we have to adjust the size.
   if(buffer.offset > buffer.size)
      buffer.size = buffer.offset;

   return size;
}

static void File_SeekFrom(FILE_CONTEXT* file, const FILE_OFFSET offset)
{
   Safeguard(file);
}

static void File_SeekTo(FILE_CONTEXT* file, const FILE_SIZE position)
{
   Safeguard(file);
}

static void File_Flush(FILE_CONTEXT* file)
{
   Safeguard(file);

   if(file->type != FILE_TYPE_PHYSICAL)
      return;

   if(!file->handle) {
      WARN_GENERIC();
      return;
   }

   fflush(file->handle);
}

static void File_Close(FILE_CONTEXT* file)
{
   Safeguard(file);

   if(file->type == FILE_TYPE_PHYSICAL) {
      fclose(file->handle);
   }

   free(file);
}

static UINT8 File_GetByte(FILE_CONTEXT* file)
{
   Safeguard(file);

   UINT8 data;
   file->read(file, &data, sizeof(data));

   return data;
}

static UINT16 File_GetWord(FILE_CONTEXT* file)
{
   Safeguard(file);

   UINT16 data;
   file->read(file, &data, sizeof(data));

   return SwapWord(data, file->order);
}

static UINT32 File_GetLong(FILE_CONTEXT* file)
{
   Safeguard(file);

   UINT32 data;
   file->read(file, &data, sizeof(data));

   return SwapLong(data, file->order);
}

static BOOL File_GetBoolean(FILE_CONTEXT* file)
{
   Safeguard(file);

   const uint8 data = file->get_byte(file);
   return TRUE_OR_FALSE(data);
}

static REAL File_GetReal(FILE_CONTEXT* file)
{
   const uint32 data = file->get_long(file);
   const float* data_ptr = (float*)&data;
   return *data_ptr;
}

static void File_PutByte(FILE_CONTEXT* file, const UINT8 data)
{
   Safeguard(file);

   file->write(file, &data, sizeof(data));
}

static void File_PutWord(FILE_CONTEXT* file, const UINT16 data)
{
   Safeguard(file);

   const uint16 swapped = SwapWord(data, file->order);
   file->write(file, &swapped, sizeof(swapped));
}

static void File_PutLong(FILE_CONTEXT* file, const UINT32 data)
{
   Safeguard(file);

   const uint32 swapped = SwapLong(data, file->order);
   file->write(file, &swapped, sizeof(swapped));
}

static void File_PutBoolean(FILE_CONTEXT* file, const BOOL data)
{
   Safeguard(file);

   const uint8 value = ZERO_OR_ONE(data);
   file->put_byte(file, value);
}

static void File_PutReal(FILE_CONTEXT* file, const REAL data)
{
   Safeguard(file);

   const uint32* data_ptr = (uint32*)&data;
   file->put_long(file, *data_ptr);
}
