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
CBUILD_TARGET = --optimize
CBUILD_OPTIONS =

ifdef DEBUG
	CBUILD_TARGET = --debug
endif
ifdef PROFILE
	CBUILD_TARGET = --profile
endif

ifdef VERBOSE
	CBUILD_OPTIONS += --verbose
endif

# Which system to build for, this is passed to gcc as '-march'.
ifdef SYSTEM
	CBUILD_OPTIONS += --system=$(SYSTEM)
endif

# Note that these flags are only used to compile CBuild.
CFLAGS = -std=c99 -pedantic -Os -g -Wall -Wextra

# Default install path on Unices (e.g Linux and Mac).
INSTALL_PATH = /usr/bin

.PHONY: all bootstrap clean distclean install uninstall

all: bootstrap $(CBUILD)
	@$(CBUILD) $(CBUILD_TARGET) $(CBUILD_OPTIONS)

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
	@$(CBUILD) clean $(CBUILD_TARGET) $(CBUILD_OPTIONS)

distclean: $(CBUILD)
#	Clean up all builds and sanitize the source tree.
	@$(CBUILD) clean $(CBUILD_OPTIONS)
	@$(CBUILD) clean --debug $(CBUILD_OPTIONS)
	@$(CBUILD) clean --optimize $(CBUILD_OPTIONS)
	@$(CBUILD) clean --profile $(CBUILD_OPTIONS)
	@echo Removing build system files.
	@$(RM) $(CBUILD)

install:
	@echo "Installing program files to $(INSTALL_PATH)."
	@echo "(Type 'make install INSTALL_PATH=path' if you want to use another path.)"
	@$(CBUILD) install --prefix=$(INSTALL_PATH) $(CBUILD_TARGET) $(CBUILD_OPTIONS) 

uninstall:
	@echo "Uninstalling program files from $(INSTALL_PATH)."
	@echo "(Type 'make uninstall INSTALL_PATH=path' if you want to use another path.)"
	@$(CBUILD) uninstall --prefix=$(INSTALL_PATH) $(CBUILD_TARGET) $(CBUILD_OPTIONS)

$(CBUILD): $(CBUILD_PATH)CBuild.c
	@echo Preparing the build system, please wait.
	@$(CC) -o $@ $< $(CFLAGS)
