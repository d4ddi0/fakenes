#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#ifndef NULL
#define NULL 0
#endif

#define DEFAULT_FILE "Configure.h"

int main(int argc, char* argv[]) {
	const char* filename = NULL;
	FILE *file = NULL;
	unsigned test;
	size_t sizeof_short, sizeof_int, sizeof_long, sizeof_longlong;

	if(argc >= 2)
		filename = argv[1];
	else
		filename = DEFAULT_FILE;

	/* printf("Output file is \"%s\".\n", filename); */

	file = fopen(filename, "w");
	if(!file) {
		fprintf(stderr, "Failed to open \"%s\" for writing.\n", filename);
		printf("Exiting.\n");
		return 1;
	}

	/* Byte order. */
	test = 1;
	if((*(unsigned char*)&test) == 1) {
		printf("Using little-endian byte order.\n");
		fprintf(file, "#define LSB_FIRST\n");
	}
	else {
		printf("Using big-endian byte order.\n");
	}

	/* Data type sizes. */
	sizeof_short = sizeof(short);
	sizeof_int = sizeof(int);
	sizeof_long = sizeof(long);
#ifdef LLONG_MAX
	sizeof_longlong = sizeof(long long);
#else
	sizeof_longlong = 0;
#endif

	fprintf(file, "#define SIZEOF_SHORT %d\n", (int)sizeof_short);
	fprintf(file, "#define SIZEOF_INT %d\n", (int)sizeof_int);
	fprintf(file, "#define SIZEOF_LONG %d\n", (int)sizeof_long);
	fprintf(file, "#define SIZEOF_LONG_LONG %d\n", (int)sizeof_longlong);

#if __STDC_VERSION__ >= 199901L
	printf("Using C99 data types.\n");
	fprintf(file, "#define C99_TYPES\n");
#endif

	fclose(file);
	return 0;
}
