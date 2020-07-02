# constants
COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESET="\033[m"

# helper functions
section = @echo ${COLOR_BOLD_CYAN}">>> ${1}"${COLOR_RESET}

# project variables
NAME=py
PROJECT=${NAME}.xcodeproj
TARGETS=py pyjs
EXTENSION=api.pyx

PACKAGE=${HOME}"/Documents/Max 8/Packages/"${NAME}
PKG_DIRS=docs examples externals help init \
         javascript jsextensions media patchers

PYDIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
SRCDIR:=$(shell dirname $(abspath ${PYDIR}))
ROOTDIR:=$(shell dirname $(abspath ${SRCDIR}))



# $(call xbuild,name)
define xbuild-targets
$(call section,"build $1")
@for target in ${TARGETS}; do \
		xcodebuild -project targets/$1/py-js.xcodeproj -target $$target ; \
	done
endef

# $call xclean-build,name)
define xclean-build
$(call section,"cleaning build artifacts from $1 target")
@find **/$1 | grep -E "(build)" | xargs rm -rf
endef

# ifdef MYFLAG
# CFLAGS += -DMYFLAG
# endif

.PHONY: all

all: clean test build
	@echo "All DONE!"

# HIGH-LEVEL
# -----------------------------------------------------------------------
.PHONY: default homebrew-sys homebrew-pkg homebrew-ext shared-pkg shared-ext

default: homebrew-sys

homebrew-sys: clean build pkg
	$(call section,"build -> deploy to local package")

homebrew-pkg: clean build-homebrew-pkg pkg pkg-support

homebrew-ext: build-homebrew-ext pkg

shared-pkg: build-shared-pkg pkg pkg-support

shared-ext: build-shared-ext pkg 



# DEPLOYING
# -----------------------------------------------------------------------
.PHONY: pkg pkg-support dist


dist:
	$(call section,"preparing for distribution")
	@echo "do it here with git"

pkg:
	$(call section,"deploying to local package")
	@mkdir -p ${PACKAGE}
	@for target in ${PKG_DIRS}; do \
    	rsync -a --delete $(ROOTDIR)/$${target} ${PACKAGE}; \
	done

pkg-support:
	$(call section,"deploying support/ to local package")
	@rsync -a --delete $(ROOTDIR)/support ${PACKAGE}


# BUILDING
# -----------------------------------------------------------------------
.PHONY: build build-homebrew-pkg build-python-pkg build-extension

build: build-homebrew-sys
	$(call section,"build project")

build-homebrew-sys: build-extension
	$(call xbuild-targets,"homebrew-sys")

build-homebrew-pkg: prep-homebrew-pkg
	$(call xbuild-targets,"homebrew-pkg")

build-homebrew-ext: prep-homebrew-ext
	$(call xbuild-targets,"homebrew-ext")

build-shared-pkg: prep-shared-pkg
	$(call xbuild-targets,"shared-pkg")

build-shared-ext: prep-shared-ext
	$(call xbuild-targets,"shared-ext")

build-framework-pkg: prep-framework-pkg
	$(call xbuild-targets,"framework-pkg")

build-framework-ext: prep-framework-ext
	$(call xbuild-targets,"framework-ext")

build-extension:
	$(call section,"generate c code from cython extension")
	@cython -3 ${EXTENSION}



# PRE-BUILD PREP
# -----------------------------------------------------------------------
.PHONY: prep-shared-pkg prep-shared-ext prep-homebrew-pkg prep-homebrew-ext \
		prep-framework-pkg prep-framework-ext

prep-homebrew-pkg:
	$(call section,"build homebrew python for package")
	@bash scripts/homebrew.sh pkg
	
prep-homebrew-ext:
	$(call section,"build homebrew python for external")
	@bash scripts/homebrew.sh ext

prep-shared-pkg:
	$(call section,"prepare shared python from source for package")
	@bash scripts/shared.sh pkg

prep-shared-ext:
	$(call section,"prepare shared python from source for external")
	@bash scripts/shared.sh ext

prep-framework-pkg:
	$(call section,"build framework python for package")
	@bash scripts/framework.sh pkg
	
prep-framework-ext:
	$(call section,"build framework python for external")
	@bash scripts/framework.sh ext

# TESTING
# -----------------------------------------------------------------------
.PHONY: test

test:
	$(call section,"testing planned")
	echo "remember to test!"


# STYLING
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


# CLEANING
# -----------------------------------------------------------------------
.PHONY: clean clean-build clean-support clean-externals

clean: clean-externals clean-build clean-support 
	$(call section,"cleaning DONE")

clean-build: clean-build-homebrew-sys \
			 clean-build-homebrew-pkg \
			 clean-build-homebrew-ext \
			 clean-build-shared-pkg \
			 clean-build-shared-ext \
			 clean-build-framework-pkg \
			 clean-build-framework-ext

clean-build-all:
	$call xclean-build,"targets")


clean-externals:
	$(call section,"cleaning externals")
	@for target in ${TARGETS}; do \
		rm -rf ${ROOTDIR}/externals/$$target.mxo  ; \
	done

clean-build-homebrew-sys:
	$call xclean-build,"homebrew-sys")

clean-build-homebrew-pkg:
	$call xclean-build,"homebrew-pkg")

clean-build-homebrew-ext:
	$call xclean-build,"homebrew-ext")

clean-build-shared-pkg:
	$call xclean-build,"shared-pkg")

clean-build-shared-pkg:
	$call xclean-build,"shared-ext")

clean-build-framework-pkg:
	$call xclean-build,"framework-pkg")

clean-build-framework-ext:
	$call xclean-build,"framework-ext")

clean-support:
	$(call section,"cleaning support directory")
	@rm -rf ${ROOTDIR}/support/*

clean-pkg:
	$(call section,"cleaning py pkg")
	@rm -rf ${PACKAGE}

