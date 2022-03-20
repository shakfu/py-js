# ============================================================================
# VARIABLES & CONSTANTS

# constants
COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESET="\033[m"

# exec
PYTHON=/usr/local/bin/python3

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

# $(call pybuild,name)
define pybuild-targets
$(call section,"builder $1 $2")
@cd $(PYDIR) && $(PYTHON) -m builder $1 $2 $3 $4
endef

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

homebrew-pkg: clean-homebrew-pkg build-homebrew-pkg

homebrew-ext: clean-homebrew-ext build-homebrew-ext

shared-pkg: clean-shared-pkg build-shared-pkg

shared-ext: clean-shared-ext build-shared-ext

static-ext: clean-static-ext build-static-ext

# static-pkg: clean-static-pkg build-static-pkg

framework-pkg: clean-framework-pkg build-framework-pkg

framework-ext: clean-framework-ext build-framework-ext

pymx:
	@bash source/projects/pymx/build_pymx.sh


max-check:
	@echo $(PACKAGE)
	@ls $(PACKAGE)

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
	$(call pybuild-targets, "pyjs" "local_sys")

build-homebrew-pkg:
	$(call pybuild-targets, "pyjs" "homebrew_pkg")

build-homebrew-ext:
	$(call pybuild-targets,"pyjs" "homebrew_ext")

build-framework-pkg:
	$(call pybuild-targets, "pyjs" "framework_pkg" "--install" "--build")

build-framework-ext:
	$(call pybuild-targets, "pyjs" "framework_ext" "--install" "--build")

build-shared-pkg:
	$(call pybuild-targets, "pyjs" "shared_pkg" "--install" "--build")

build-shared-ext:
	$(call pybuild-targets, "pyjs" "shared_ext" "--install" "--build")

build-static-ext:
	$(call pybuild-targets, "pyjs" "static_ext" "--install" "--build")

build-static-pkg:
	$(call pybuild-targets, "pyjs" "static_pkg" "--install" "--build")

build-extension:
	$(call section,"generate c code from cython extension")
	@cython -3 ${EXTENSION}


# re-compile only
# -----------------------------------------------------------------------
.PHONY: compile-extension \

compile-extension:
	$(call section,"generate c code from cython extension")
	@cython -3 ${EXTENSION}


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
	@clang-format -i -style=file $(PYDIR)/py.c
	@clang-format -i -style=file $(PYDIR)/py.h
	@clang-format -i -style=file $(PYDIR)/pyjs.c

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
.PHONY: reset clean clean-build clean-support clean-externals \
		clean-targets-build clean-local-sys \
		clean-homebrew-pkg clean-homebrew-ext \
		clean-framework-pkg clean-framework-ext \
		clean-shared-pkg clean-shared-ext \
		clean-static-pkg clean-static-ext

reset: clean clean-targets-build

clean: clean-externals clean-build clean-support

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

clean-support:
	$(call section,"cleaning support directory")
	@rm -rf ${ROOTDIR}/support/*

clean-local-sys: clean-externals
	$(call xclean-build,"local-sys")

clean-homebrew-pkg: clean-externals clean-support
	$(call xclean-build,"homebrew-pkg")

clean-homebrew-ext: clean-externals
	$(call xclean-build,"homebrew-ext")

clean-framework-pkg: clean-externals clean-support
	$(call xclean-build,"framework-pkg")

clean-framework-ext: clean-externals
	$(call xclean-build,"framework-ext")

clean-shared-pkg: clean-externals clean-support
	$(call xclean-build,"shared-pkg")

clean-shared-ext: clean-externals
	$(call xclean-build,"shared-ext")

clean-static-pkg: clean-externals clean-support
	$(call xclean-build,"static-pkg")

clean-static-ext: clean-externals
	$(call xclean-build,"static-ext")

clean-xcode: clean-build
	$(call section,"cleaning xcode detritus")
	@find . | grep -E "(project.xcworkspace|xcuserdata)" | xargs rm -rf
