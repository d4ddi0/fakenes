CC = gcc

# Attempt to auto-detect DJGPP or MinGW.
ifneq ($(MINGDIR)$(DJDIR),)
	RM = del
	SUFFIX = .exe
else
	RM = rm
	SUFFIX =
endif

CBUILD_PATH = Build
CBUILD = $(CBUILD_PATH)/CBuild$(SUFFIX)

# Default to an optimized build. Remove this for a generic build.
CBUILD_OPTIONS = --optimize
ifdef DEBUG
	CBUILD_OPTIONS = --debug
endif
ifdef PROFILE
	CBUILD_OPTIONS = --profile
endif

ifdef VERBOSE
	CBUILD_OPTIONS += --verbose
endif

# Note that these flags are only used to compile CBuild.
CFLAGS = -pedantic -O2 -g -W -Wall

INSTALL_PATH = /usr/bin

.PHONY: all bootstrap clean distclean install uninstall

all: bootstrap $(CBUILD)
	@$(CBUILD) $(CBUILD_OPTIONS)

bootstrap:
	@echo Starting the build process.
	@echo If you encounter any problems, please submit a bug report.
ifneq ($(MINGDIR)$(DJDIR),)
	@echo (Type 'make VERBOSE=1' to debug build errors.)
	@echo **********************************************************
else
#	Unix shells are a bit more annoying than the Windows one...
	@echo "(Type 'make VERBOSE=1' to debug build errors.)"
	@echo "**********************************************************"
endif

clean: $(CBUILD)
	@$(CBUILD) clean $(CBUILD_OPTIONS)

distclean: $(CBUILD)
#	Clean up all builds and sanitize the source tree.
	@$(CBUILD) clean
	@$(CBUILD) clean --debug
	@$(CBUILD) clean --profile
	@$(CBUILD) clean --optimize
	@echo Removing build system files.
	@$(RM) $(CBUILD)

install:
	@echo "Installing program files to $(INSTALL_PATH)."
	@echo "(Type 'make install INSTALL_PATH=path' if you want to use another path.)"
	@$(CBUILD) install --prefix=$(INSTALL_PATH) $(CBUILD_OPTIONS) 

uninstall:
	@echo "Uninstalling program files from $(INSTALL_PATH)."
	@echo "(Type 'make uninstall INSTALL_PATH=path' if you want to use another path.)"
	@$(CBUILD) uninstall --prefix=$(INSTALL_PATH) $(CBUILD_OPTIONS)

$(CBUILD): $(CBUILD_PATH)/CBuild.c
	@echo Preparing the build system, please wait.
	@$(CC) -o $@ $< $(CFLAGS)
