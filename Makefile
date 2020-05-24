# constants
COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESET="\033[m"

# helper functions
section = @echo ${COLOR_BOLD_CYAN}">>> ${1}"${COLOR_RESET}


# project variables
NAME=py
EXTERNAL=${NAME}.c
PROJECT=${NAME}.xcodeproj
EXTENSION=api.pyx
MAXHELP=${NAME}.maxhelp


.PHONY: all

all: clean test style build
	@echo "All DONE!"



# DEPLOYING
# -----------------------------------------------------------------------
.PHONY: dist

dist:
	$(call section,"preparing for distribution")
	@echo "do it here with git"

# bump:
# 	$(call section,"bumping patch version and pushing")
# 	@bumpversion patch
# 	@git push


# BUILDING
# -----------------------------------------------------------------------
.PHONY: build

build: build-extension build-external build-help
	$(call section,"build project")

build-help:
	$(call section,"moving ${MAXHELP} to package help folder")
	@cp -rf py.maxhelp ../../../help

build-external:
	$(call section,"build with xcode")
	@xcodebuild -project py.xcodeproj

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
.PHONY: style clang-format

style: clang-format

clang-format:
	$(call section,"clang-format")
	@clang-format -i -style=file ${EXTERNAL}


# CLEANING
# -----------------------------------------------------------------------
.PHONY: clean

clean: clean-build
	$(call section,"cleaning DONE")


clean-build:
	$(call section,"cleaning build artifacts")
	@rm -rf build
	@rm -rf ../../../externals/py.mxo
	@rm -rf ../../../help/py.maxhelp


