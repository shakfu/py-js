# ============================================================================
# VARIABLES & CONSTANTS

# architecture of current system
# currently only macOS x86_64 and arm64 supported
ARCH = $(shell uname -m)

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
# BUILDDIR := $(HOME)/.build_pyjs
BUILDDIR := $(PYDIR)/targets/build
CFLOW := $(PYDIR)/resources/cflow

# project variables
NAME = py
PROJECT = $(NAME).xcodeproj
TARGETS = py pyjs
EXTENSION = $(PYDIR)/api.pyx
PKG_NAME = py-js

DEPENDENCIES = "cmake zmq czmq"

# system python override: eg. `make shared-ext PYTHON_VERSION=3.11.9`
# PYTHON_VERSION = 3.8.17

MAX_APP := "/Applications/Studio/Max.app"
MAX_VERSION := 8
MAX_DIR := "Max\ $(MAX_VERSION)"
PACKAGES := $(HOME)/Documents/$(MAX_DIR)/Packages
PYJS_PACKAGE := $(HOME)/Documents/$(MAX_DIR)/Packages/$(PKG_NAME)
PKG_DIRS = docs examples extensions externals help init \
           javascript jsextensions media misc patchers

#NUMPY_EXISTS := $(shell sh -c '$(PYTHON) $(SCRIPTS)/check_numpy.py --exists')
INCLUDE_NUMPY := 0 # change this to 1 if you want to enable numpy in api.pyx
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
$(call section,"builder $1 $2 $3 $4 $5 $6")
@cd '$(PYDIR)' && $(PYTHON) -m builder $1 $2 $3 $4 $5 $6
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
		shared-pkg shared-ext shared-tiny-ext \
		static-pkg static-ext static-tiny-ext

# -----------------------------------------------------------------------
# python3 external argets


default: local-sys api

local-sys: clean-local-sys api
	$(call call-builder,"pyjs" "local_sys")

homebrew-pkg: clean-homebrew-pkg
	$(call call-builder,"pyjs" "homebrew_pkg")
	@make sign

homebrew-ext: clean-homebrew-ext
	$(call call-builder,"pyjs" "homebrew_ext")
	@make sign

shared-pkg: clean-shared-pkg
	$(call call-builder,"pyjs" "shared_pkg" "--install" "--build" "-p" "$(PYTHON_VERSION)")

shared-ext: clean-shared-ext
	$(call call-builder,"pyjs" "shared_ext" "--install" "--build" "-p" "$(PYTHON_VERSION)")

shared-tiny-ext: clean-externals
	$(call call-builder,"pyjs" "shared_tiny_ext" "--install" "--build" "--release" "-p" "$(PYTHON_VERSION)")

static-pkg: clean-static-pkg
	$(call call-builder,"pyjs" "static_pkg" "--install" "--build" "-p" "$(PYTHON_VERSION)")

static-ext: clean-static-ext
	$(call call-builder,"pyjs" "static_ext" "--install" "--build" "-p" "$(PYTHON_VERSION)")

static-tiny-ext: clean-externals
	$(call call-builder,"pyjs" "static_tiny_ext" "--install" "--build" "--release" "-p" "$(PYTHON_VERSION)")

framework-pkg: clean-framework-pkg
	$(call call-builder,"pyjs" "framework_pkg" "--install" "--build" "-p" "$(PYTHON_VERSION)")

framework-ext: clean-framework-ext
	$(call call-builder,"pyjs" "framework_ext" "--install" "--build" "-p" "$(PYTHON_VERSION)")

relocatable-pkg: clean-framework-pkg
	$(call call-builder,"pyjs" "relocatable_pkg" "--install" "--build")

relocatable-pkg-nopip: clean-framework-pkg
	$(call call-builder,"pyjs" "relocatable_pkg" "--install" "--build" "--without-pip")

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

.PHONY: python-shared python-shared-pkg python-shared-ext python-shared-tiny \
		python-static  python-static-tiny \
		python-framework python-framework-ext python-framework-pkg \
		python-beeware python-cmake

python-shared: clean-python-shared
	$(call call-builder,"python" "shared" "--install")

python-shared-ext: clean-python-shared-ext
	$(call call-builder,"python" "shared_ext" "--install" "-p" "$(PYTHON_VERSION)")

python-shared-pkg: clean-python-shared-pkg
	$(call call-builder,"python" "shared_pkg" "--install" "-p" "$(PYTHON_VERSION)")

python-static: clean-python-static
	$(call call-builder,"python" "static" "--install" "-p" "$(PYTHON_VERSION)")

python-framework: clean-python-framework
	$(call call-builder,"python" "framework" "--install" "-p" "$(PYTHON_VERSION)")

python-framework-ext: clean-python-framework-ext
	$(call call-builder,"python" "framework_ext" "--install" "-p" "$(PYTHON_VERSION)")

python-framework-pkg: clean-python-framework-pkg
	$(call call-builder,"python" "framework_pkg" "--install" "-p" "$(PYTHON_VERSION)")

python-relocatable: clean-python-framework-pkg
	$(call call-builder,"python" "relocatable" "--install")

# python-relocatable: clean-python-framework-pkg
# 	$(call call-builder,"python" "relocatable_pkg")

python-static-tiny:
	$(call call-builder,"python" "static_tiny" "--install" "-p" "$(PYTHON_VERSION)")

python-shared-tiny:
	$(call call-builder,"python" "shared_tiny" "--install" "-p" "$(PYTHON_VERSION)")

python-beeware:
	$(call call-builder,"python" "beeware" "--install")

python-cmake: clean-python-cmake
	$(call call-builder,"python" "cmake" "--install" "-p" "3.9.17")




# -----------------------------------------------------------------------
# build dependencies

.PHONY: check_deps deps bz2 ssl xz

check_deps:
	@brew --prefix > /dev/null || (echo "Homebrew not found"; exit 1)
	@for dep in $(DEPENDENCIES); do \
		brew --prefix $$dep > /dev/null || \
			(echo "$$dep not available, install via homebrew: 'brew install $$dep'"; \
				exit 1); \
	done

deps: ssl bz2 xz

bz2:
	$(call call-builder, "dep" "bz2")

ssl:
	$(call call-builder, "dep" "ssl")

xz:
	$(call call-builder, "dep" "xz")


# BUILDING
# -----------------------------------------------------------------------
.PHONY: build-homebrew-pkg build-homebrew-ext \
		build-framework-pkg build-framework-ext \
		build-shared-pkg build-shared-ext build-shared-tiny-ext \
		build-static-pkg build-static-ext build-static-tiny-ext


build-shared-pkg: clean-shared-pkg
	$(call call-builder,"pyjs" "shared_pkg" "--build")

build-shared-ext: clean-shared-ext
	$(call call-builder,"pyjs" "shared_ext" "--build" "-p" "$(PYTHON_VERSION)")

build-static-ext: clean-static-ext
	$(call call-builder,"pyjs" "static_ext" "--build" "-p" "$(PYTHON_VERSION)")

build-static-pkg: clean-static-pkg
	$(call call-builder,"pyjs" "static_pkg" "--build" "-p" "$(PYTHON_VERSION)")

build-framework-pkg: clean-framework-pkg
	$(call call-builder,"pyjs" "framework_pkg" "--build" "-p" "$(PYTHON_VERSION)")

build-framework-ext: clean-framework-ext
	$(call call-builder,"pyjs" "framework_ext" "--build" "-p" "$(PYTHON_VERSION)")

build-relocatable-pkg: clean-externals
	$(call call-builder,"pyjs" "relocatable_pkg" "--build")

build-static-tiny-ext: clean-static-ext
	$(call call-builder,"pyjs" "static_tiny_ext" "--build" "--release" "-p" "$(PYTHON_VERSION)")

build-shared-tiny-ext: clean-shared-ext
	$(call call-builder,"pyjs" "shared_tiny_ext" "--build" "--release" "-p" "$(PYTHON_VERSION)")

build-beeware-ext: clean-static-ext
	$(call call-builder,"pyjs" "beeware_ext" "--build" "--release")


# re-compile only
# -----------------------------------------------------------------------
.PHONY: compile-extension api

compile-extension:
	$(call section,"generate c code from cython extension")
	@cython -3 -E INCLUDE_NUMPY=$(ENABLE_NUMPY) ${EXTENSION}

api: compile-extension


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

.PHONY: strip sign collect dmg sign-dmg notarize-dmg staple-dmg fix-framework

strip:
	$(call section,"strip then sign externals")
	@python3 source/scripts/strip.py
	$(call section,"re-sign externals")
	$(call call-builder,"package" "sign")

sign:
	$(call section,"sign externals and binary dependencies")
	$(call call-builder,"package" "sign")

dist:
	$(call section,"create package distribution folder")
	$(call call-builder,"package" "dist")

dmg:
	$(call section,"package dist as dmg")
	$(call call-builder,"package" "dmg")

sign-dmg:
	$(call section,"sign dmg")
	$(call call-builder,"package" "sign_dmg")

notarize-dmg:
	$(call section,"notarize dmg")
	$(call call-builder,"package" "notarize_dmg")

staple-dmg:
	$(call section,"staple dmg")
	$(call call-builder,"package" "staple_dmg")

collect-dmg:
	$(call section,"collect dmg")
	$(call call-builder,"package" "collect_dmg")

fix-framework:
	$(call section,"fix framework in support")
	$(call call-builder,"fix" "framework")


# Building with logging
# -----------------------------------------------------------------------
.PHONY: log-shared-ext log-framework-ext log-tiny

log-shared-ext:
	@echo "building 'shared-ext' capturing output to logs/shared-ext.log"
	@mkdir -p logs
	@time make shared-ext > logs/shared-ext.log 2>&1

log-framework-ext:
	@echo "building 'framework-ext' capturing output to logs/framework-ext.log"
	@mkdir -p logs
	@time make framework-ext > logs/framework-ext.log 2>&1

log-tiny:
	@echo "building 'static-tiny-ext' capturing output to logs/framework-ext.log"
	@mkdir -p logs
	@time make tiny > logs/tiny.log 2>&1



# Testing
# -----------------------------------------------------------------------
.PHONY: test \
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

numpy:
	echo $(NUMPY_INCL)


# Styling
# -----------------------------------------------------------------------
.PHONY: docs style clang-format duplo cflow projects dev 

docs:
	@doxygen

style: clang-format

clang-format:
	$(call section,"clang-format")
	@clang-format -i -style=file $(PYDIR)/py.c
	@clang-format -i -style=file $(PYDIR)/pyjs.c

lizard:
	$(call section,"lizard complexity analysis")
	@lizard -o report.html $(PYDIR)/py.c

duplo:
	$(call section,"checking code duplication")
	@find . -type f \( -iname "$(PYDIR)/py.c" \) > files.lst
	@duplo files.lst duplicates.txt
	@rm -f files.lst

cflow:
	@cflow2dot -x $(CFLOW)/ignore.txt -i source/py/py.c -f pdf -o $(CFLOW)/py_cflow
	@rm -f $(CFLOW)/*.dot

	@cflow2dot -x $(CFLOW)/ignore.txt -i source/projects/mamba/py.h -f pdf -o $(CFLOW)/mamba_cflow
	@rm -f $(CFLOW)/*.dot


projects:
	@bash $(SCRIPTS)/build_projects.sh

dev:
	@bash $(SCRIPTS)/build_dev.sh


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
				 clean-python-framework \
				 clean-python-cmake

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

reset-build:
	$(call section,"cleaning build directory")
	@rm -rf '${ROOTDIR}'/build
	@rm -rf '${BUILDDIR}'

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

clean-python-cmake:
	$(call xcleanlib,"python-cmake")


demo: README.md FAQ.md CHANGELOG.md
	@echo "target: '$@'"
	@echo "first-prereq: '$<'"
	@echo "all-prereqs: '$^'"
	@echo "newer-prereqs:'$?'"

version:
	@echo $(PYTHON_VERSION)

