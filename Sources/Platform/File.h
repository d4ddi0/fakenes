/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#ifndef PLATFORM__FILE_H__INCLUDED
#define PLATFORM__FILE_H__INCLUDED
#include <stdio.h>
#include "Common/Global.h"
#include "Common/Types.h"
#include "Toolkit/Unicode.h"
#ifdef __cplusplus
extern "C" {
#endif

/* For physical files, the file routines simply act as a layer on top of
   the C library file I/O routines, and the 'buffer' field of the file
   context is never used for anything.

   For reading memory files, the data buffer must first be set to an
   existing block of memory via a call to set_file_buffer(). 

   For writing memory files, the data buffer is automatically allocated
   and managed accordingly. As data is added to the buffer, it will grow
   by the chunk size to accomodate the new data. You can retrieve the
   data buffer via a call to get_file_buffer(). */
enum FILE_TYPE {
   FILE_TYPE_PHYSICAL = 0,	/* Physical file. */
   FILE_TYPE_VIRTUAL		/* Memory file. */
};

/* Note that no duplex mode is supported yet. */
enum FILE_MODE {
   FILE_MODE_READ = 0,		/* Open for reading. */
   FILE_MODE_WRITE		/* Open for writing. */
};

enum FILE_ORDER {
   FILE_ORDER_NATIVE = 0,	/* Auto-detect native byte order. */
   FILE_ORDER_INTEL,		/* Always little-endian. */
   FILE_ORDER_MOTOROLA		/* Always big-endian. */
};

/* Data type used for sizes, positions, etc. This is used instead of
   the standard size data type (size_t) because we still want to
   support e.g 64-bit file sizes on 32-bit platforms. */
typedef UINT64 FILE_SIZE;
/* Data type used for relative offsets. */
typedef INT64 FILE_OFFSET;

typedef struct _FILE_BUFFER {
   FILE_SIZE size, limit;
   FILE_SIZE position;
   UINT8* data;

} FILE_BUFFER;

typedef struct _FILE_CONTEXT {
   FILE_TYPE type;	/* File type. See above.*/
   FILE_MODE mode;	/* File access mode. See above. */
   FILE_ORDER order;	/* File byte order. See above. */
   FILE_BUFFER buffer;	/* File data buffer and related parameters. */

   /* General functions. These can be overriden by replacing the file
      pointers, allowing custom routines to be used for a given file
      without changing any code that accesses it. */
   FILE_SIZE (*read)(struct _FILE_CONTEXT* file, void* data, const FILE_SIZE size);
   FILE_SIZE (*write)(struct _FILE_CONTEXT* file, const void* data, const FILE_SIZE size);
   void (*seek_from)(struct _FILE_CONTEXT* file, const FILE_OFFSET offset);
   void (*seek_to)(struct _FILE_CONTEXT* file, const FILE_SIZE position);
   void (*flush)(struct _FILE_CONTEXT* file);
   void (*close)(struct _FILE_CONTEXT* file);

   /* Data access functions. Most of these are subject to endianness.
      Note that get_real() and put_real() are reduced to single precision,
      and booleans require an entire byte of storage space. Again, these
      can be overriden on a per-file basis.*/
   UINT8 (*read_byte)(struct _FILE_CONTEXT* file);
   UINT16 (*read_word)(struct _FILE_CONTEXT* file);
   UINT32 (*read_long)(struct _FILE_CONTEXT* file);
   BOOL (*read_boolean)(struct _FILE_CONTEXT* file);
   REAL (*read_real)(struct _FILE_CONTEXT* file);
   void (*write_byte)(struct _FILE_CONTEXT* file, const UINT8 data);
   void (*write_word)(struct _FILE_CONTEXT* file, const UINT16 data);
   void (*write_long)(struct _FILE_CONTEXT* file, const UINT32 data);
   void (*write_boolean)(struct _FILE_CONTEXT* file, const BOOL data);
   void (*write_real)(struct _FILE_CONTEXT* file, const REAL data);

   /* Underlying file handle in the C library. This is only used for
      physical files, not memory files. */
   FILE* handle;

} FILE_CONTEXT;

extern FILE_CONTEXT* open_file(const UTF_STRING* filename, const FILE_MODE mode, const FILE_ORDER order);
extern FILE_CONTEXT* open_memory_file(const FILE_MODE mode, const FILE_ORDER order);
extern void* get_file_buffer(FILE_CONTEXT* file, FILE_SIZE* size);
extern void set_file_buffer(FILE_CONTEXT* file, void* buffer, const FILE_SIZE size);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* PLATFORM__FILE_H__INCLUDED */
