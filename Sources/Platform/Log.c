/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include <allegro.h>
#include <stdio.h>
#include <time.h>
#include "common.h"
#include "debug.h"
#include "gui.h"
#include "log.h"
#include "types.h"

#define MAX_LOG_SIZE 65536

static FILE *log_file = NULL;

static USTRING log_text;

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

   USTRING_CLEAR(log_text);

   time (&start);

   log_printf ("\n--- %s", asctime (localtime (&start)));
}

void log_close (void)
{
   if (log_file)
      fclose (log_file);
}

void log_printf (const UTF_STRING *message, ...)
{
   va_list format;
   USTRING buffer;

   RT_ASSERT(message);

   if (!log_file)
      return;

   USTRING_CLEAR(buffer);

   va_start (format, message);
   uvszprintf (buffer, (sizeof (buffer) - 1), message, format);
   va_end (format);

   ustrncat (buffer, "\n", (sizeof (buffer) - 1));
   fputs (buffer, log_file);

   fflush (log_file);

   ustrzncat (log_text, sizeof (log_text), buffer, sizeof (buffer));

   gui_log_message(buffer);
}

UTF_STRING *get_log_text (void)
{
   return (log_text);
}

static USTRING console_text;

void console_clear (void)
{
   USTRING_CLEAR(console_text);
}

void console_printf (const UTF_STRING *message, ...)
{
   va_list format;
   USTRING buffer;

   RT_ASSERT(message);

   va_start (format, message);
   uvszprintf (buffer, sizeof (buffer), message, format);
   va_end (format);

   ustrzncat (console_text, sizeof (console_text), buffer, sizeof (buffer));
}

UTF_STRING *get_console_text (void)
{
   return (console_text);
}
