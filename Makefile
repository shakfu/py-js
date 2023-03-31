# ============================================================================
# VARIABLES & CONSTANTS


# python executable to use
# PYTHON = /usr/bin/python3
PYTHON = python3

# paths
ROOTDIR := $(shell pwd)
SRCDIR := $(ROOTDIR)/source
PROJECTS := $(SRCDIR)/projects
PYDIR := $(PROJECTS)/py
BUILDER := $(PYDIR)/builder
SCRIPTS := $(PYDIR)/scripts
BUILDDIR := $(HOME)/.build_pyjs
CFLOW := $(PYDIR)/resources/cflow

# project variables
NAME = py
PROJECT = $(NAME).xcodeproj
TARGETS = py pyjs
EXTENSION = $(PYDIR)/api.pyx
PKG_NAME = py-js

MAX_APP := "/Applications/Studio/Max.app"
MAX_VERSION := 8
MAX_DIR := "Max\ $(MAX_VERSION)"
PACKAGES := $(HOME)/Documents/$(MAX_DIR)/Packages
PYJS_PACKAGE := $(HOME)/Documents/$(MAX_DIR)/Packages/py-js
PKG_DIRS = docs examples extensions externals help init \
           javascript jsextensions media misc patchers

#NUMPY_EXISTS := $(shell sh -c '$(PYTHON) $(SCRIPTS)/check_numpy.py --exists')
NUMPY_EXISTS := 0 # change this to 1 if you want to enable numpy in api.pyx
				  # this requires numpy to be available to the python
				  # interpreter.	


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
@for target in $(TARGETS); do \
		xcodebuild -project targets/'$1'/py-js.xcodeproj -target $$target ; \
	done
endef

# $(call xbuild,name,flags)
define xbuild-targets-flags
$(call section,"build $1 with flags: $2")
@for target in $(TARGETS); do \
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
		python-shared python-shared-pkg python-shared-ext \
		python-static static-tiny-ext tiny \
		python-framework python-framework-ext python-framework-pkg


# -----------------------------------------------------------------------
# python3 external argets


default: local-sys api

local-sys: clean-local-sys api
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
	$(call call-builder,"pyjs" "relocatable_pkg" "--install" "--build")

relocatable-pkg-nopip: clean-framework-pkg
	$(call call-builder,"pyjs" "relocatable_pkg" "--install" "--build" "--without-pip")

static-tiny-ext: clean-externals
	$(call call-builder,"pyjs" "static_tiny_ext" "--install" "--build" "--release")

tiny: static-tiny-ext

beeware-ext: clean-externals
	$(call call-builder,"pyjs" "beeware_ext" "--install" "--build" "--release")


# -----------------------------------------------------------------------
# release external targets

.PHONY: release-framework-pkg release-framework-ext \
		release-shared-pkg release-shared-ext \
		release-static-ext release-static-pkg \
		release-relocatable-pkg

release-shared-pkg: clean-shared-pkg
	$(call call-builder,"pyjs" "shared_pkg" "--install" "--build" "--release")

release-shared-ext: clean-shared-ext
	$(call call-builder,"pyjs" "shared_ext" "--install" "--build" "--release")

release-static-ext: clean-static-ext
	$(call call-builder,"pyjs" "static_ext" "--install" "--build" "--release")

release-static-pkg: clean-static-pkg
	$(call call-builder,"pyjs" "static_pkg" "--install" "--build" "--release")

release-framework-pkg: clean-framework-pkg
	$(call call-builder,"pyjs" "framework_pkg" "--install" "--build" "--release")

release-framework-ext: clean-framework-ext
	$(call call-builder,"pyjs" "framework_ext" "--install" "--build" "--release")

release-relocatable-pkg: clean-framework-pkg
	$(call call-builder,"pyjs" "relocatable_pkg" "--install" "--build" "--release")


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
	$(call call-builder,"python" "relocatable" "--install")

# python-relocatable: clean-python-framework-pkg
# 	$(call call-builder,"python" "relocatable_pkg")

python-static-tiny:
	$(call call-builder,"python" "static_tiny" "--install")

python-beeware:
	$(call call-builder,"python" "beeware" "--install")


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

sign:
	$(call section,"sign externals")
	$(call call-builder,"sign")

sign-dry:
	$(call section,"sign externals")
	$(call call-builder,"sign" "--dry-run")

package:
	$(call section,"make package")
	$(call call-builder,"package" "$(PKG_NAME)")

dmg:
	$(call section,"make package as dmg")
	$(call call-builder,"dmg" "$(PKG_NAME)")

sign-dmg:
	$(call section,"sign dmg")
	$(call call-builder,"sign" "dmg")

make fix-framework:
	$(call section,"fix framework in support")
	$(call call-builder,"fix" "framework")


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
		build-static-pkg build-static-ext


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

build-relocatable-pkg: clean-externals
	$(call call-builder,"pyjs" "relocatable_pkg" "--build")

build-static-tiny-ext: clean-static-ext
	$(call call-builder,"pyjs" "static_tiny_ext" "--build" "--release")

build-beeware-ext: clean-static-ext
	$(call call-builder,"pyjs" "beeware_ext" "--build" "--release")



# re-compile only
# -----------------------------------------------------------------------
.PHONY: compile-extension api

compile-extension:
	$(call section,"generate c code from cython extension")
	@cython -3 -E INCLUDE_NUMPY=$(NUMPY_EXISTS) ${EXTENSION}

api: compile-extension

# Testing
# -----------------------------------------------------------------------
.PHONY: test maxtest \
		test-homebrew-pkg test-homebrew-ext \
		test-framework-pkg test-framework-ext \
		test-shared-pkg test-shared-ext \
		test-static-pkg test-static-ext

test: clean
	$(call section,"running all tests")
	$(call call-builder,"test")

test-default: clean-products
	$(call section,"running default test")
	$(call call-builder,"test" "default")


test-homebrew-ext: clean-products
	$(call section,"running homebrew-ext test")
	$(call call-builder,"test" "homebrew_ext")

test-homebrew-pkg: clean-products
	$(call section,"running homebrew-pkg test")
	$(call call-builder,"test" "homebrew_pkg")

test-shared-ext: clean-products
	$(call section,"running shared-ext test")
	$(call call-builder,"test" "shared_ext")

test-shared-pkg: clean-products
	$(call section,"running shared-pkg test")
	$(call call-builder,"test" "shared_pkg")

test-framework-ext: clean-products
	$(call section,"running framework-ext test")
	$(call call-builder,"test" "framework_ext")

test-framework-pkg: clean-products
	$(call section,"running framework-pkg test")
	$(call call-builder,"test" "framework_pkg")

test-static-ext: clean-products
	$(call section,"running static-ext test")
	$(call call-builder,"test" "static_ext")

test-static-pkg: clean-products
	$(call section,"running static-pkg test")
	$(call call-builder,"test" "static_pkg")

check:
	$(call section,"checking test results")
	$(call call-builder,"logs" "check_current")

check_logs:
	$(call section,"checking all test results")
	$(call call-builder,"logs" "check_all")

maxtests:
	$(call section,"running all .maxtests results")
	@cd $(SCRIPTS)/ruby; ruby test.rb $(MAX_APP)

numpy:
	echo $(NUMPY_INCL)


# Styling
# -----------------------------------------------------------------------
.PHONY: docs style clang-format duplo cflow

docs:
	@doxygen

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
	@find . -type f \( -iname "py.c" \) > files.lst
	@duplo files.lst duplicates.txt
	@rm -f files.lst

cflow:
	@cflow2dot -x $(CFLOW)/ignore.txt -i source/py/py.c -f pdf -o $(CFLOW)/py_cflow
	@rm -f $(CFLOW)/*.dot

	@cflow2dot -x $(CFLOW)/ignore.txt -i source/projects/mamba/py.h -f pdf -o $(CFLOW)/mamba_cflow
	@rm -f $(CFLOW)/*.dot


projects:
	@bash $(SCRIPTS)/build_projects.sh

# fat:
# 	@echo "NOT FUNCTIONAL UNLESS DEPS ARE ALSO FAT"
# 	@bash $(SCRIPTS)/build_universal.sh


# Cleaning
# -----------------------------------------------------------------------
.PHONY: reset clean clean-targets clean-support clean-externals \
		clean-build clean-local-sys \
		clean-homebrew-pkg clean-homebrew-ext \
		clean-framework-pkg clean-framework-ext \
		clean-shared-pkg clean-shared-ext \
		clean-static-pkg clean-static-ext \
		clean-relocatable-pkg

clean: clean-externals clean-support clean-targets clean-build clean-docs

clean-targets:  clean-local-sys  \
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

clean-products: clean-support clean-externals clean-build

clean-build:
	$(call section,"cleaning build directory")
	@rm -rf '${ROOTDIR}'/build
	@rm -rf '${PYDIR}'/targets/build/src/*
	@rm -rf '${BUILDDIR}'/src/*
	@rm -rf '${BUILDDIR}'/lib/python-shared
	@rm -rf '${BUILDDIR}'/lib/python-static
	@rm -rf '${BUILDDIR}'/lib/Python.framework

clean-externals:
	$(call section,"cleaning externals")
	@rm -rf '$(ROOTDIR)'/externals/*.mxo

clean-docs:
	$(call section,"cleaning docs directory")
	@rm -rf $(PYDIR)/docs

clean-support:
	$(call section,"cleaning support directory")
	@rm -rf $(ROOTDIR)/support/*

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



