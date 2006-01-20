.EXPORT_ALL_VARIABLES:

include config.mk

BINARY_PATH = ..

.PHONY: all clean depend distclean
all:
	${MAKE} -C src -f ${MAKEFILE} all

depend:
	${MAKE} -C src -f ${MAKEFILE} depend

clean:
	${MAKE} -C src -f ${MAKEFILE} clean

distclean:
	${MAKE} -C src -f ${MAKEFILE} distclean
	${RM} config.mk
