CC = gcc

# Attempt to auto-detect DOS (DJGPP) or Windows. Note that MINGDIR isn't too
# reliable as MinGW users don't always set it, and there are other GCC-derived
# compilers on Windows besides MinGW, so we check WINDIR as well.
POSIX = true
ifneq ($(WINDIR)$(MINGDIR)$(DJDIR),)
	ifneq ($(OSTYPE),cygwin)	
		POSIX = false
	endif
endif

ifeq ($(POSIX),true)
	RM = rm
	SEPARATOR = /
	SUFFIX =
else
	RM = del
	SEPARATOR = \\
	SUFFIX = .exe
endif

CBUILD_PATH = Build$(SEPARATOR)
CBUILD = $(CBUILD_PATH)CBuild$(SUFFIX)

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
ifeq ($(POSIX),true)
#	Disable parsing on Unix shells.
	@echo "(Type 'make VERBOSE=1' to debug build errors.)"
	@echo "**********************************************************"
else
	@echo (Type 'make VERBOSE=1' to debug build errors.)
	@echo **********************************************************
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

$(CBUILD): $(CBUILD_PATH)CBuild.c
	@echo Preparing the build system, please wait.
	@$(CC) -o $@ $< $(CFLAGS)
