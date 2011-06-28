#include <allegro.h>

/* Check if the current version is compatible with Allegro 4.2.0 */
#if (MAKE_VERSION(ALLEGRO_VERSION, ALLEGRO_SUB_VERSION, ALLEGRO_WIP_VERSION) < MAKE_VERSION(4, 2, 0))
#	error The installed version of Allegro is too old. At least version 4.2.0 is required.
#endif

int main(void) {
	allegro_init();

	return 0;
}
END_OF_MAIN()
