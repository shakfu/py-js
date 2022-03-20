# ============================================================================
# VARIABLES & CONSTANTS

# constants
COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESET="\033[m"


# paths
ROOTDIR:=$(shell pwd)
SRCDIR:=$(ROOTDIR)/source
PYDIR:=$(SRCDIR)/py

# project variables
NAME=py
PROJECT=${NAME}.xcodeproj
TARGETS=py pyjs
EXTENSION=$(PYDIR)/api.pyx

MAX_VERSION := "8"
MAX_DIR := Max\ $(MAX_VERSION)
PACKAGE := $(HOME)/Documents/$(MAX_DIR)/Packages
PKG_DIRS=docs examples externals help init \
         javascript jsextensions media patchers





# ifdef MYFLAG
# CFLAGS += -DMYFLAG
# endif

# ============================================================================
# FUNCTIONS

# $(call section,string)
section = @echo ${COLOR_BOLD_CYAN}">>> ${1}"${COLOR_RESET}

# $(call xbuild,name)
define xbuild-targets
$(call section,"build $1")
@for target in ${TARGETS}; do \
		xcodebuild -project targets/$1/py-js.xcodeproj -target $$target ; \
	done
endef

# $(call xbuild,name,flags)
define xbuild-targets-flags
$(call section,"build $1 with flags: $2")
@for target in ${TARGETS}; do \
		xcodebuild -project targets/$1/py-js.xcodeproj -target $$target GCC_PREPROCESSOR_DEFINITIONS='$$GCC_PREPROCESSOR_DEFINITIONS $2 ' ; \
	done
endef


# $call xclean,name)
define xclean-build
$(call section,"cleaning build artifacts from $1 target")
@rm -rf $(PYDIR)/targets/$1/build 
endef

# ============================================================================
# TARGETS

.PHONY: all

all: default

# High-Level
# -----------------------------------------------------------------------
.PHONY: default local-sys \
		homebrew-pkg homebrew-ext \
		framework-pkg framework-ext \
		shared-pkg shared-ext \
		static-pkg static-ext

default: local-sys

local-sys: clean-local-sys build-local-sys

max:
	@echo $(PACKAGE)
	@ls $(PACKAGE)


homebrew-pkg: clean-homebrew-pkg build-homebrew-pkg

homebrew-ext: clean-homebrew-ext build-homebrew-ext

framework-pkg: clean-framework-pkg build-framework-pkg

framework-ext: clean-framework-ext build-framework-ext

shared-pkg: clean-shared-pkg build-shared-pkg

shared-ext: clean-shared-ext build-shared-ext

static-pkg: clean-static-pkg build-static-pkg

static-ext: clean-static-ext build-static-ext

# DEPLOYING
# -----------------------------------------------------------------------
.PHONY: pkg pkg-support dist


dist:
	$(call section,"preparing for distribution")
	@echo "do it here with git"




# Building
# -----------------------------------------------------------------------
.PHONY: build build-extension \
		build-local-sys \
		build-homebrew-pkg build-homebrew-ext \
		build-framework-pkg build-framework-ext \
		build-shared-pkg build-shared-ext \
		build-static-pkg build-static-ext

build: build-local-sys
	$(call section,"build project")

# build-bin-beeware-ext: build-extension
# 	$(call xbuild-targets-flags,"bin-beeware-ext","PY_STATIC_EXT")

build-local-sys: build-extension
	@echo $(ROOTDIR)
	@python3 source/py/targets/local-sys/build.py

build-homebrew-pkg: prep-homebrew-pkg
	$(call xbuild-targets,"homebrew-pkg")

build-homebrew-ext: prep-homebrew-ext
	$(call xbuild-targets,"homebrew-ext")

build-framework-pkg: prep-framework-pkg
	$(call xbuild-targets,"framework-pkg")

build-framework-ext: prep-framework-ext
	$(call xbuild-targets,"framework-ext")

build-shared-pkg: prep-shared-pkg
	$(call xbuild-targets,"shared-pkg")

build-shared-ext: prep-shared-ext
	$(call xbuild-targets,"shared-ext")

build-static-pkg: prep-static-pkg
	$(call xbuild-targets,"static-pkg")

build-static-ext: prep-static-ext
	$(call xbuild-targets,"static-ext")

build-extension:
	$(call section,"generate c code from cython extension")
	@cython -3 ${EXTENSION}

# re-compile only (without prep)
# -----------------------------------------------------------------------
.PHONY: compile compile-extension \
		compile-local-sys \
		compile-homebrew-pkg compile-homebrew-ext \
		compile-framework-pkg compile-framework-ext \
		compile-shared-pkg compile-shared-ext \
		compile-static-pkg compile-static-ext

compile: compile-local-sys
	$(call section,"compile project")

compile-local-sys: compile-extension
	$(call xbuild-targets,"local-sys")

compile-homebrew-pkg:
	$(call xbuild-targets,"homebrew-pkg")

compile-homebrew-ext:
	$(call xbuild-targets,"homebrew-ext")

compile-framework-pkg:
	$(call xbuild-targets,"framework-pkg")

compile-framework-ext:
	$(call xbuild-targets,"framework-ext")

compile-shared-pkg:
	$(call xbuild-targets,"shared-pkg")

compile-shared-ext:
	$(call xbuild-targets,"shared-ext")

compile-static-pkg:
	$(call xbuild-targets,"static-pkg")

compile-static-ext:
	$(call xbuild-targets,"static-ext")

compile-extension:
	$(call section,"generate c code from cython extension")
	@cython -3 ${EXTENSION}


# Pre-build prep
# -----------------------------------------------------------------------
.PHONY: prep-homebrew-pkg prep-homebrew-ext \
		prep-framework-pkg prep-framework-ext \
		prep-shared-pkg prep-shared-ext \
		prep-static-pkg prep-static-ext
		

prep-homebrew-pkg:
	$(call section,"build homebrew python from binary for package")
	@bash scripts/bin-homebrew.sh pkg
	
prep-homebrew-ext:
	$(call section,"build homebrew python from binary for external")
	@bash scripts/bin-homebrew.sh ext

prep-framework-pkg:
	$(call section,"build framework python from source for package")
	@bash scripts/src-framework.sh pkg
	
prep-framework-ext:
	$(call section,"build framework python from source for external")
	@bash scripts/src-framework.sh ext

prep-shared-pkg:
	$(call section,"prepare shared python from source for package")
	@bash scripts/src-shared.sh pkg

prep-shared-ext:
	$(call section,"prepare shared python from source for external")
	@bash scripts/src-shared.sh ext

prep-static-pkg:
	$(call section,"build static python from source for package")
	@bash scripts/src-static.sh pkg

prep-static-ext:
	$(call section,"build static python from source for external")
	@bash scripts/src-static.sh ext

# Testing
# -----------------------------------------------------------------------
.PHONY: test

test:
	$(call section,"testing planned")
	echo "remember to test!"

# Styling
# -----------------------------------------------------------------------
.PHONY: style clang-format duplo

style: clang-format

clang-format:
	$(call section,"clang-format")
	@clang-format -i -style=file py.c
	@clang-format -i -style=file py.h
	@clang-format -i -style=file pyjs.c

lizard:
	$(call section,"lizard complexity analysis")
	@lizard -o report.html py.c

duplo:
	$(call section,"checking code duplication")
	#@find . -type f \( -iname "*.c" -o -iname "*.h" \) > files.lst
	@find . -type f \( -iname "py.c" \) > files.lst
	@duplo files.lst duplicates.txt
	@rm files.lst


# Cleaning
# -----------------------------------------------------------------------
.PHONY: reset clean clean-build clean-pkg clean-support clean-externals \
		clean-targets-build clean-local-sys \
		clean-homebrew-pkg clean-homebrew-ext \
		clean-framework-pkg clean-framework-ext \
		clean-shared-pkg clean-shared-ext \
		clean-static-pkg clean-static-ext

reset: clean clean-targets-build

clean: clean-externals clean-build clean-support clean-pkg

clean-build: clean-local-sys  \
			 clean-homebrew-pkg clean-homebrew-ext  \
			 clean-framework-pkg clean-framework-ext \
			 clean-shared-pkg clean-shared-ext \
			 clean-static-pkg clean-static-ext

clean-targets-build:
	$(call section,"cleaning targets/build directory")
	@rm -rf ${PYDIR}/targets/build

clean-externals:
	$(call section,"cleaning externals")
	@for target in ${TARGETS}; do \
		rm -rf ${ROOTDIR}/externals/$$target.mxo  ; \
	done

clean-local-sys: clean-externals
	$(call xclean-build,"local-sys")

clean-homebrew-pkg:
	$(call xclean-build,"homebrew-pkg")

clean-homebrew-ext:
	$(call xclean-build,"homebrew-ext")

clean-framework-pkg:
	$(call xclean-build,"framework-pkg")

clean-framework-ext:
	$(call xclean-build,"framework-ext")

clean-shared-pkg:
	$(call xclean-build,"shared-pkg")

clean-shared-ext:
	$(call xclean-build,"shared-ext")

clean-static-pkg:
	$(call xclean-build,"static-pkg")

clean-static-ext:
	$(call xclean-build,"static-ext")

clean-support:
	$(call section,"cleaning support directory")
	@rm -rf ${ROOTDIR}/support/*

clean-pkg:
	$(call section,"cleaning py pkg")
	@rm -rf ${PACKAGE}

clean-xcode: clean-build
	$(call section,"cleaning xcode detritus")
	@find . | grep -E "(project.xcworkspace|xcuserdata)" | xargs rm -rf
