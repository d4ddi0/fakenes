/* FakeNES - A free, portable, Open Source NES emulator.

   log.c: Implementation of the logging functions.

   Copyright (c) 2001-2006, FakeNES Team.
   This is free software.  See 'LICENSE' for details.
   You must read and accept the license prior to use. */

#include <allegro.h>
#include <stdio.h>
#include <time.h>
#include "common.h"
#include "debug.h"
#include "log.h"
#include "types.h"

#define MAX_LOG_SIZE 65536

static FILE *log_file = NULL;

void log_open (const char *filename)
{
   time_t start;

   RT_ASSERT(filename);

   if (file_size (filename) >= MAX_LOG_SIZE)
   {
      /* Truncate. */
      log_file = fopen (filename, "w");
   }
   else
   {
      /* Append. */
      log_file = fopen (filename, "a");
   }

   if (!log_file)
      WARN("Couldn't open log file");

   time (&start);

   log_printf ("\n--- %s", asctime (localtime (&start)));
}

void log_close (void)
{
   if (log_file)
      fclose (log_file);
}

void log_printf (const UCHAR *message, ...)
{
   va_list format;
   USTRING buffer;

   RT_ASSERT(message);

   if (!log_file)
      return;

   va_start (format, message);
   uvszprintf (buffer, sizeof (buffer), message, format);
   va_end (format);

   fputs (buffer, log_file);
}
