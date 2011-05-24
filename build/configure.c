#include <stdio.h>
#include <limits.h>

int main (void)
{
   unsigned test;

   /* Byte order. */
   test = 1;
   if ((*(unsigned char *)&test) == 1)
      printf ("#define LSB_FIRST\n");

   /* Data type sizes. */
   printf ("#define SIZEOF_SHORT	%d\n", sizeof (short));
   printf ("#define SIZEOF_INT		%d\n", sizeof (int));
   printf ("#define SIZEOF_LONG		%d\n", sizeof (long));
#ifdef LLONG_MAX
   printf ("#define SIZEOF_LONG_LONG	%d\n", sizeof (long long));
#else
   printf ("#define SIZEOF_LONG_LONG	0\n");
#endif

   return (0);
}
