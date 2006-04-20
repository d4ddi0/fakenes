#define foobarbaz /*
# A shell script within a source file. Ugly, but it works...

TRGT=${0/.c/.exe}
for i in $@ ; do
   if [ "$i" = "--make-compiled" ] ; then
      echo "Compiling '$TRGT', please wait..."
      gcc -W -Wall -O2 -o $TRGT $0
      exit $?
   fi
done

gcc -W -Wall -Werror -o /tmp/$TRGT "$0" && { "/tmp/$TRGT" $@ ; RET=$? ; rm -f "/tmp/$TRGT" ; exit $RET ; }
exit $?
*/

#include <stdio.h>

int main (void)
{
   unsigned test;

   /* Byte order. */
   test = 1;
   if ((*(unsigned char *)&test) == 1)
      printf ("#define LSB_FIRST\n");

   /* Data type sizes. */
   printf ("#define SIZEOF_SHORT_INT %d\n", sizeof (short int));
   printf ("#define SIZEOF_INT_INT   %d\n", sizeof (int));
   printf ("#define SIZEOF_LONG_INT  %d\n", sizeof (long int));

   return (0);
}
