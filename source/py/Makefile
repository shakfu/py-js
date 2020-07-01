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


# ifdef MYFLAG
# CFLAGS += -DMYFLAG
# endif

.PHONY: all

all: clean test build
	@echo "All DONE!"

# DEPLOYING
# -----------------------------------------------------------------------
.PHONY: default pyjs homebrew-pkg homebrew-ext python-org-pkg python-org-ex pkg pkg-support dist


default: clean build pkg
	$(call section,"build -> deploy to local package")

homebrew-pkg: clean build-homebrew-pkg pkg pkg-support

homebrew-ext: build-homebrew-ext pkg

python-org-pkg: build-python-pkg pkg pkg-support

python-org-ext: build-python-ext pkg 


dist:
	$(call section,"preparing for distribution")
	@echo "do it here with git"

# bump:
# 	$(call section,"bumping patch version and pushing")
# 	@bumpversion patch
# 	@git push

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
	$(call section,"build homebrew sys default")
	@for target in ${TARGETS}; do \
		xcodebuild -project targets/homebrew-sys/py-js.xcodeproj -target $$target ; \
	done

build-homebrew-pkg: prep-homebrew-pkg build-homebrew  

build-homebrew-ext: prep-homebrew-ext build-homebrew

build-homebrew: build-osx-homebrew

build-osx-homebrew: build-extension
	$(call section,"build osx homebrew")
	@for target in ${TARGETS}; do \
		xcodebuild -project targets/osx/py-js.xcodeproj -xcconfig targets/osx/homebrew.xcconfig -target $$target ; \
	done


build-python-pkg: prep-python-pkg build-python

build-python-ext: prep-python-ext build-python

build-python: build-extension
	$(call section,"compile python source with xcode")
	@xcodebuild OTHER_CFLAGS=${CFLAGS} -project targets/python-org/py.xcodeproj


build-extension:
	$(call section,"generate c code from cython extension")
	@cython -3 ${EXTENSION}



# PRE-BUILD PREP
# -----------------------------------------------------------------------
.PHONY: prep-python-pkg prep-python-ext prep-homebrew-pkg prep-homebrew-ext

prep-python-pkg:
	$(call section,"prepare python from source for package")
	@bash scripts/python-org.sh pkg

prep-python-ext:
	$(call section,"prepare python from source for external")
	@bash scripts/python-org.sh ext

prep-homebrew-pkg:
	$(call section,"build homebrew python for package")
	@bash scripts/homebrew.sh pkg
	
prep-homebrew-ext:
	$(call section,"build homebrew python for external")
	@bash scripts/homebrew.sh ext

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

clean-build: clean-build-osx \
			 clean-build-python-org \
			 clean-build-python-fwk

clean-externals:
	$(call section,"cleaning externals")
	@for target in ${TARGETS}; do \
		rm -rf ${ROOTDIR}/externals/$$target.mxo  ; \
	done

clean-build-osx:
	$(call section,"cleaning build artifacts from osx target")
	@find **/osx | grep -E "(build)" | xargs rm -rf

clean-build-python-org:
	$(call section,"cleaning build artifacts from python-org target")
	@find **/python-org | grep -E "(build)" | xargs rm -rf

clean-build-python-fwk:
	$(call section,"cleaning build artifacts from python-fwk target")
	@find **/python-fwk | grep -E "(build)" | xargs rm -rf

clean-support:
	$(call section,"cleaning support directory")
	@rm -rf ${ROOTDIR}/support/*

clean-pkg:
	$(call section,"cleaning py pkg")
	@rm -rf ${PACKAGE}

