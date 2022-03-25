# ============================================================================
# VARIABLES & CONSTANTS

# constants
COLOR_BOLD_CYAN = "\033[1;36m"
COLOR_RESET = "\033[m"

# exec
PYTHON = python3

# paths
ROOTDIR := $(shell pwd)
SRCDIR := $(ROOTDIR)/source
PYDIR := $(SRCDIR)/py

# project variables
NAME = py
PROJECT = ${NAME}.xcodeproj
TARGETS = py pyjs
EXTENSION = $(PYDIR)/api.pyx

MAX_VERSION := "8"
MAX_DIR := "Max $(MAX_VERSION)"
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
$(call section,"builder $1 $2 $3 $4")
@cd '$(PYDIR)' && $(PYTHON) -m builder $1 $2 $3 $4
endef

# $(call xbuild,name)
define xbuild-targets
$(call section,"build $1")
@for target in ${TARGETS}; do \
		xcodebuild -project targets/'$1'/py-js.xcodeproj -target $$target ; \
	done
endef

# $(call xbuild,name,flags)
define xbuild-targets-flags
$(call section,"build $1 with flags: $2")
@for target in ${TARGETS}; do \
		xcodebuild -project targets/'$1'/py-js.xcodeproj -target $$target GCC_PREPROCESSOR_DEFINITIONS='$$GCC_PREPROCESSOR_DEFINITIONS $2 ' ; \
	done
endef


# $(call xclean,name)
define xclean-build
$(call section,"cleaning build artifacts from $1 target")
@rm -rf '$(PYDIR)'/targets/'$1'/build 
endef

# $(call xcleanlib,name)
define xclean-build
$(call section,"cleaning build lib from $1 target")
@rm -rf '$(PYDIR)'/targets/build/lib/'$1' 
endef


# ============================================================================
# TARGETS

.PHONY: all

all: default

# High-Level
# -----------------------------------------------------------------------
.PHONY: default help local-sys \
		homebrew-pkg homebrew-ext \
		framework-pkg framework-ext \
		shared-pkg shared-ext \
		static-pkg static-ext \
		relocatable-pkg \
		python-shared python-shared-pkg python-shared-ext \
		python-static python-static-full \
		python-framework python-framework-ext python-framework-pkg \
		python-relocatable


# -----------------------------------------------------------------------
# python external argets

help:
	@python3 source/py/scripts/help.py

default: local-sys

local-sys: clean-local-sys
	$(call pybuild-targets, "pyjs" "local_sys")

homebrew-pkg: clean-homebrew-pkg
	$(call pybuild-targets, "pyjs" "homebrew_pkg")

homebrew-ext: clean-homebrew-ext
	$(call pybuild-targets,"pyjs" "homebrew_ext")

shared-pkg: clean-shared-pkg
	$(call pybuild-targets, "pyjs" "shared_pkg" "--install" "--build")

shared-ext: clean-shared-ext
	$(call pybuild-targets, "pyjs" "shared_ext" "--install" "--build")

static-ext: clean-static-ext
	$(call pybuild-targets, "pyjs" "static_ext" "--install" "--build")

static-pkg: clean-static-pkg
	$(call pybuild-targets, "pyjs" "static_pkg" "--install" "--build")

framework-pkg: clean-framework-pkg
	$(call pybuild-targets, "pyjs" "framework_pkg" "--install" "--build")

framework-ext: clean-framework-ext
	$(call pybuild-targets, "pyjs" "framework_ext" "--install" "--build")

relocatable-pkg: clean-framework-pkg
	$(call pybuild-targets, "pyjs" "relocatable_pkg")

pymx:
	@bash source/projects/pymx/build_pymx.sh

# -----------------------------------------------------------------------
# python targets

python-shared: clean-python-shared
	$(call pybuild-targets, "python" "shared" "--install")

python-shared-ext: clean-python-shared-ext
	$(call pybuild-targets, "python" "shared-ext" "--install")

python-shared-pkg: clean-python-shared-pkg
	$(call pybuild-targets, "python" "shared-pkg" "--install")

python-static: clean-python-static
	$(call pybuild-targets, "python" "static" "--install")

python-static-full: clean-python-static-full
	$(call pybuild-targets, "python" "static-full" "--install")

python-framework: clean-python-framework
	$(call pybuild-targets, "python" "framework" "--install")

python-framework-ext: clean-python-framework-ext
	$(call pybuild-targets, "python" "framework_ext" "--install")

python-framework-pkg: clean-python-framework-pkg
	$(call pybuild-targets, "python" "framework_pkg" "--install")

python-relocatable: clean-python-framework-pkg
	$(call pybuild-targets, "python" "relocatable")


# -----------------------------------------------------------------------
# utilities

max-check:
	@echo $(PACKAGE)
	@ls $(PACKAGE)


# DEPLOYING
# -----------------------------------------------------------------------
.PHONY: pkg pkg-support dist


dist:
	$(call section,"preparing for distribution")
	@echo "do it here with git"



# BUILDING
# -----------------------------------------------------------------------
.PHONY: build-homebrew-pkg build-homebrew-ext \
		build-framework-pkg build-framework-ext \
		build-shared-pkg build-shared-ext \
		build-static-pkg build-static-ext


build-shared-pkg: clean-shared-pkg
	$(call pybuild-targets, "pyjs" "shared_pkg" "--build")

build-shared-ext: clean-shared-ext
	$(call pybuild-targets, "pyjs" "shared_ext" "--build")

build-static-ext: clean-static-ext
	$(call pybuild-targets, "pyjs" "static_ext" "--build")

build-static-pkg: clean-static-pkg
	$(call pybuild-targets, "pyjs" "static_pkg" "--build")

build-framework-pkg: clean-framework-pkg
	$(call pybuild-targets, "pyjs" "framework_pkg" "--build")

build-framework-ext: clean-framework-ext
	$(call pybuild-targets, "pyjs" "framework_ext" "--build")


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

clean-build-lib: clean-python-shared \
				 clean-python-shared-ext \
				 clean-python-shared-pkg \
				 clean-python-static \
				 clean-python-static-full \
				 clean-python-framework


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

clean-xcode: clean-build
	$(call section,"cleaning xcode detritus")
	@find . | grep -E "(project.xcworkspace|xcuserdata)" | xargs rm -rf

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

# -----------------------------------------------------------------------
# python clean targets


clean-python-shared:
	$(call xcleanlib, "python-shared")

clean-python-shared-ext:
	$(call xcleanlib, "python-shared")

clean-python-shared-pkg: clean-externals clean-support
	$(call xcleanlib, "python-shared")

clean-python-static:
	$(call xcleanlib, "python-static")

clean-python-static-full:
	$(call xcleanlib, "python-static")

clean-python-framework:
	$(call xcleanlib, "python-framework")

clean-python-framework-ext:
	$(call xcleanlib, "python-framework-ext")

clean-python-framework-pkg: clean-externals clean-support
	$(call xcleanlib, "python-framework-pkg")


