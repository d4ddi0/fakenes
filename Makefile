# Makefile to build cbuild

CFLAGS=-O2 -W -Wall

ifneq ($(MINGDIR)$(DJDIR),)
	CC = gcc
	EXEEXT = .exe
else
	EXEEXT =
endif

CBUILD=cbuild$(EXEEXT)

all: $(CBUILD) msg

.PHONY: msg

$(CBUILD): cbuild.c
	$(CC) $(CFLAGS) -o $@ $<

msg: $(CBUILD)
	@echo done, now type \'cbuild\' to build FakeNES
	@echo for a list of all build options, type \'cbuild --help\'
