# constants
COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESET="\033[m"

# helper functions
section = @echo ${COLOR_BOLD_CYAN}">>> ${1}"${COLOR_RESET}

# project variables
NAME=py
EXTERNAL=${NAME}.c
HEADER=${NAME}.h
PROJECT=${NAME}.xcodeproj
EXTENSION=api.pyx
MAXHELP=${NAME}.maxhelp
PACKAGE=${HOME}"/Documents/Max 8/Packages/"${NAME}
PKG_DIRS=docs examples externals help init \
         javascript jsextensions media patchers

PYDIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
SRCDIR:=$(shell dirname $(abspath ${PYDIR}))
ROOTDIR:=$(shell dirname $(abspath ${SRCDIR}))

.PHONY: all

all: clean test build
	@echo "All DONE!"

# DEPLOYING
# -----------------------------------------------------------------------
.PHONY: default homebrew-pkg homebrew-ext python-org-pkg python-org-ex pkg pkg-support dist


default: clean build pkg
	$(call section,"build -> deploy to local package")

homebrew-pkg: clean build-homebrew-pkg pkg pkg-support

homebrew-ext: build-homebrew-ext pkg

python-org-pkg: build-python-org-pkg pkg pkg-support

python-org-ext: build-python-org-ext pkg 


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


show:
	@echo "PYDIR: ${PYDIR}"
	@echo "SRCDIR: ${SRCDIR}"
	@echo "ROOTDIR: ${ROOTDIR}"
	@echo "syncing $(ROOTDIR) with $(PACKAGE)..." 


# BUILDING
# -----------------------------------------------------------------------
.PHONY: build build-default build-homebrew-pkg build-python-org-pkg build-extension

build: build-default
	$(call section,"build project")

build-default: build-extension 
	$(call section,"build default external with xcode")
	@xcodebuild -project targets/default/py.xcodeproj


build-homebrew-pkg: build-extension prepare-homebrew-pkg compile-homebrew  

build-homebrew-ext: build-extension prepare-homebrew-ext compile-homebrew

compile-homebrew:
	$(call section,"compile homebrew")
	@xcodebuild -project targets/homebrew/py.xcodeproj

prepare-homebrew-pkg:
	$(call section,"build homebrew python for package")
	@bash scripts/homebrew.sh pkg
	
prepare-homebrew-ext:
	$(call section,"build homebrew python for external")
	@bash scripts/homebrew.sh ext

compile-python-org-pkg: build-extension compile-python-org pkg pkg-support

build-python-org-pkg: build-extension prepare-python-org-pkg compile-python-org

build-python-org-ext: build-extension prepare-python-org-ext compile-python-org

compile-python-org:
	$(call section,"compile python source with xcode")
	@xcodebuild -project targets/python-org/py.xcodeproj

prepare-python-org-pkg:
	$(call section,"prepare python from source for package")
	@bash scripts/python-org.sh pkg

prepare-python-org-ext:
	$(call section,"prepare python from source for external")
	@bash scripts/python-org.sh ext


build-extension:
	$(call section,"generate c code from cython extension")
	@cython -3 ${EXTENSION}


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
	@clang-format -i -style=file ${EXTERNAL}
	@clang-format -i -style=file ${HEADER}

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

clean-build: clean-build-default \
			 clean-build-homebrew \
			 clean-build-python-org

clean-externals:
	$(call section,"cleaning externals")
	@rm -rf ${ROOTDIR}/externals/${NAME}.mxo

clean-build-default:
	$(call section,"cleaning build artifacts from default target")
	@find **/default | grep -E "(build)" | xargs rm -rf
	
clean-build-homebrew:
	$(call section,"cleaning build artifacts from homebrew target")
	@find **/homebrew | grep -E "(build)" | xargs rm -rf

clean-build-python-org:
	$(call section,"cleaning build artifacts from python-org target")
	@find **/python-org | grep -E "(build)" | xargs rm -rf

clean-support:
	$(call section,"cleaning support directory")
	@rm -rf ${ROOTDIR}/support/*

