# ============================================================================
# VARIABLES & CONSTANTS


# python executable to use
# PYTHON = /usr/bin/python3
PYTHON = python3

# paths
ROOTDIR := $(shell pwd)
SRCDIR := $(ROOTDIR)/source
PYDIR := $(SRCDIR)/py
BUILDDIR := $(HOME)/.build_pyjs

# project variables
NAME = py
PROJECT = $(NAME).xcodeproj
TARGETS = py pyjs
EXTENSION = $(PYDIR)/api.pyx

MAX_VERSION := 8
MAX_DIR := "Max\ $(MAX_VERSION)"
PACKAGES := $(HOME)/Documents/$(MAX_DIR)/Packages
PYJS_PACKAGE := $(HOME)/Documents/$(MAX_DIR)/Packages/py-js
PKG_DIRS = docs examples externals help init \
           javascript jsextensions media patchers

# constants
COLOR_BOLD_CYAN = "\033[1;36m"
COLOR_RESET = "\033[m"

# ifdef MYFLAG
# CFLAGS += -DMYFLAG
# endif

# ============================================================================
# FUNCTIONS

# $(call section,string)
section = @echo ${COLOR_BOLD_CYAN}">>> ${1}"${COLOR_RESET}

# $(call pybuild,name)
define call-builder
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
define xclean-target
$(call section,"cleaning build artifacts from $1 target")
@rm -rf '$(PYDIR)'/targets/$1/build
endef


# $(call xclean,name)
define xcleanlib
$(call section,"cleaning build product from python build $1")
@rm -rf '$(PYDIR)'/targets/build/lib/$1
@rm -rf '${BUILDDIR}'/lib/$1
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
		static-pkg static-ext \
		vanilla-ext vanilla-pkg \
		python-shared python-shared-pkg python-shared-ext \
		python-static \
		python-framework python-framework-ext python-framework-pkg \
		python-vanilla python-vanilla-ext python-vanilla-pkg


# -----------------------------------------------------------------------
# python external argets


default: local-sys

local-sys: clean-local-sys
	$(call call-builder,"pyjs" "local_sys")

homebrew-pkg: clean-homebrew-pkg
	$(call call-builder,"pyjs" "homebrew_pkg")

homebrew-ext: clean-homebrew-ext
	$(call call-builder,"pyjs" "homebrew_ext")

shared-pkg: clean-shared-pkg
	$(call call-builder,"pyjs" "shared_pkg" "--install" "--build")

shared-ext: clean-shared-ext
	$(call call-builder,"pyjs" "shared_ext" "--install" "--build")

static-ext: clean-static-ext
	$(call call-builder,"pyjs" "static_ext" "--install" "--build")

static-pkg: clean-static-pkg
	$(call call-builder,"pyjs" "static_pkg" "--install" "--build")

framework-pkg: clean-framework-pkg
	$(call call-builder,"pyjs" "framework_pkg" "--install" "--build")

framework-ext: clean-framework-ext
	$(call call-builder,"pyjs" "framework_ext" "--install" "--build")

relocatable-pkg: clean-framework-pkg
	$(call call-builder,"pyjs" "relocatable_pkg")

vanilla-ext: clean
	$(call call-builder,"pyjs" "vanilla_ext" "--install" "--build")

vanilla-pkg: clean
	$(call call-builder,"pyjs" "vanilla_pkg" "--install" "--build")

pymx:
	@bash source/projects/pymx/build_pymx.sh

# -----------------------------------------------------------------------
# python targets

python-shared: clean-python-shared
	$(call call-builder,"python" "shared" "--install")

python-shared-ext: clean-python-shared-ext
	$(call call-builder,"python" "shared_ext" "--install")

python-shared-pkg: clean-python-shared-pkg
	$(call call-builder,"python" "shared_pkg" "--install")

python-static: clean-python-static
	$(call call-builder,"python" "static" "--install")

python-framework: clean-python-framework
	$(call call-builder,"python" "framework" "--install")

python-framework-ext: clean-python-framework-ext
	$(call call-builder,"python" "framework_ext" "--install")

python-framework-pkg: clean-python-framework-pkg
	$(call call-builder,"python" "framework_pkg" "--install")

python-relocatable: clean-python-framework-pkg
	$(call call-builder,"python" "relocatable_pkg")

python-vanilla: clean-python-framework-ext clean-python-framework-pkg
	$(call call-builder,"python" "vanilla" "--install")

python-vanilla-ext: clean-python-vanilla-ext
	$(call call-builder,"python" "vanilla_ext" "--install")

python-vanilla-pkg: clean-python-vanilla-pkg
	$(call call-builder,"python" "vanilla_pkg" "--install")

# -----------------------------------------------------------------------
# dependencies

.PHONY: bz2 ssl xz

bz2:
	$(call call-builder, "dep" "bz2")

ssl:
	$(call call-builder, "dep" "ssl")

xz:
	$(call call-builder, "dep" "xz")

# -----------------------------------------------------------------------
# utilities

.PHONY: help setup update-submodules link max-check

help:
	$(call call-builder,"help")

setup: update-submodules link
	$(call section,"setup complete")

update-submodules:
	$(call section,"updating git submodules")
	@git submodule init && git submodule update

link:
	$(call section,"symlink to $(PYJS_PACKAGE)")
	@if ! [ -L "$(PYJS_PACKAGE)" ]; then \
		ln -s "$(ROOTDIR)" "$(PYJS_PACKAGE)" ; \
		echo "... symlink created" ; \
	else \
		echo "... symlink already exists" ; \
	fi

max-check:
	$(call section,"display contents of pyjs package")
	@echo "$(PYJS_PACKAGE)"
	@ls "$(PYJS_PACKAGE)"



# DEPLOYING
# -----------------------------------------------------------------------
.PHONY: sign dist

# sign:
# 	$(call call-builder,"utils" "sign" "--dev-id" $(DEV_ID))

dist:
	$(call section,"preparing for distribution")
	@echo "do it here with git"



# BUILDING
# -----------------------------------------------------------------------
.PHONY: build-homebrew-pkg build-homebrew-ext \
		build-framework-pkg build-framework-ext \
		build-shared-pkg build-shared-ext \
		build-static-pkg build-static-ext \
		build-vanilla-ext build-vanilla-pkg


build-shared-pkg: clean-shared-pkg
	$(call call-builder,"pyjs" "shared_pkg" "--build")

build-shared-ext: clean-shared-ext
	$(call call-builder,"pyjs" "shared_ext" "--build")

build-static-ext: clean-static-ext
	$(call call-builder,"pyjs" "static_ext" "--build")

build-static-pkg: clean-static-pkg
	$(call call-builder,"pyjs" "static_pkg" "--build")

build-framework-pkg: clean-framework-pkg
	$(call call-builder,"pyjs" "framework_pkg" "--build")

build-framework-ext: clean-framework-ext
	$(call call-builder,"pyjs" "framework_ext" "--build")

build-vanilla-pkg:
	$(call call-builder,"pyjs" "vanilla_pkg" "--build")

build-vanilla-ext:
	$(call call-builder,"pyjs" "vanilla_ext" "--build")

# re-compile only
# -----------------------------------------------------------------------
.PHONY: compile-extension \

compile-extension:
	$(call section,"generate c code from cython extension")
	@cython -3 ${EXTENSION}


# Testing
# -----------------------------------------------------------------------
.PHONY: test \
		test-homebrew-pkg test-homebrew-ext \
		test-framework-pkg test-framework-ext \
		test-shared-pkg test-shared-ext \
		test-static-pkg test-static-ext \
		test-vanilla-ext test-vanilla-pkg

test: clean
	$(call section,"running all tests")
	$(call call-builder,"test")

test-default: clean
	$(call section,"running default test")
	$(call call-builder,"test" "default")


test-homebrew-ext: clean
	$(call section,"running homebrew-ext test")
	$(call call-builder,"test" "homebrew_ext")

test-homebrew-pkg: clean
	$(call section,"running homebrew-pkg test")
	$(call call-builder,"test" "homebrew_pkg")

test-shared-ext: clean
	$(call section,"running shared-ext test")
	$(call call-builder,"test" "shared_ext")

test-shared-pkg: clean
	$(call section,"running shared-pkg test")
	$(call call-builder,"test" "shared_pkg")

test-framework-ext: clean
	$(call section,"running framework-ext test")
	$(call call-builder,"test" "framework_ext")

test-framework-pkg: clean
	$(call section,"running framework-pkg test")
	$(call call-builder,"test" "framework_pkg")

test-static-ext: clean
	$(call section,"running static-ext test")
	$(call call-builder,"test" "static_ext")

test-static-pkg: clean
	$(call section,"running static-pkg test")
	$(call call-builder,"test" "static_pkg")

test-vanilla-ext: clean
	$(call section,"running vanilla-ext test")
	$(call call-builder,"test" "vanilla_ext")

test-vanilla-pkg: clean
	$(call section,"running vanilla-pkg test")
	$(call call-builder,"test" "vanialla_pkg")


check:
	$(call section,"checking test results")
	$(call call-builder,"logs" "check_current")
# 	@$(PYTHON) source/py/scripts/utils.py --check-version-logs
# 	@echo

check_logs:
	$(call section,"checking all test results")
	$(call call-builder,"logs" "check_all")
# 	@$(PYTHON) source/py/scripts/utils.py --check-all-logs
# 	@echo


# test: clean
# 	$(call section,"running tests")
# 	@source source/py/scripts/funcs.sh && runlog_all

# check:
# 	$(call section,"checking test results")
# 	@source source/py/scripts/funcs.sh && check_all
# 	@echo

# check_logs:
# 	$(call section,"checking all test results")
# 	@source source/py/scripts/funcs.sh && check_all_logs
# 	@echo


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
.PHONY: reset clean clean-targets clean-support clean-externals \
		clean-build clean-local-sys \
		clean-homebrew-pkg clean-homebrew-ext \
		clean-framework-pkg clean-framework-ext \
		clean-shared-pkg clean-shared-ext \
		clean-static-pkg clean-static-ext \
		clean-relocatable-pkg

clean: clean-externals clean-support clean-targets clean-build

clean-targets: clean-local-sys  \
			   clean-homebrew-pkg clean-homebrew-ext  \
			   clean-framework-pkg clean-framework-ext \
			   clean-shared-pkg clean-shared-ext \
			   clean-static-pkg clean-static-ext \
			   clean-relocatable-pkg

clean-build-lib: clean-python-shared \
				 clean-python-shared-ext \
				 clean-python-shared-pkg \
				 clean-python-static \
				 clean-python-framework

reset: clean clean-build
	$(call section,"reset build system")
	@rm -rf '${BUILDDIR}'/logs/*

clean-build:
	$(call section,"cleaning build directory")
	@rm -rf '${PYDIR}'/targets/build/src/*
	@rm -rf '${BUILDDIR}'/src/*
	@rm -rf '${BUILDDIR}'/lib/python-shared
	@rm -rf '${BUILDDIR}'/lib/python-static
	@rm -rf '${BUILDDIR}'/lib/Python.framework

clean-externals:
	$(call section,"cleaning externals")
	@for target in $(TARGETS); do \
		rm -rf '$(ROOTDIR)'/externals/$$target.mxo ;\
		rm -rf '$(BUILDDIR)'/externals/$$target.mxo ;\
	done

clean-support:
	$(call section,"cleaning support directory")
	@rm -rf ${ROOTDIR}/support/*

clean-xcode: clean-build
	$(call section,"cleaning xcode detritus")
	@find . | grep -E "(project.xcworkspace|xcuserdata)" | xargs rm -rf

clean-local-sys: clean-externals
	$(call xclean-target,"local-sys")

clean-homebrew-pkg: clean-externals clean-support
	$(call xclean-target,"homebrew-pkg")

clean-homebrew-ext: clean-externals
	$(call xclean-target,"homebrew-ext")

clean-framework-pkg: clean-externals clean-support
	$(call xclean-target,"framework-pkg")

clean-framework-ext: clean-externals
	$(call xclean-target,"framework-ext")

clean-shared-pkg: clean-externals clean-support
	$(call xclean-target,"shared-pkg")

clean-shared-ext: clean-externals
	$(call xclean-target,"shared-ext")

clean-static-pkg: clean-externals clean-support
	$(call xclean-target,"static-pkg")

clean-static-ext: clean-externals
	$(call xclean-target,"static-ext")

clean-relocatable-pkg: clean-externals clean-support
	$(call xclean-target,"relocatable-pkg")

# -----------------------------------------------------------------------
# python clean targets


clean-python-shared:
	$(call xcleanlib,"python-shared")

clean-python-shared-ext:
	$(call xcleanlib,"python-shared")

clean-python-shared-pkg: clean-externals clean-support
	$(call xcleanlib,"python-shared")

clean-python-static:
	$(call xcleanlib,"python-static")

clean-python-framework:
	$(call xcleanlib,"Python.framework")

clean-python-framework-ext:
	$(call xcleanlib,"Python.framework")

clean-python-framework-pkg: clean-externals clean-support
	$(call xcleanlib,"Python.framework")

clean-python-vanilla:
	$(call xcleanlib,"Python.framework")

clean-python-vanilla-ext:
	$(call xcleanlib,"Python.framework")

clean-python-vanilla-pkg: clean-externals clean-support
	$(call xcleanlib,"Python.framework")


