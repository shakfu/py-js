PROJECT = "demo"
XCODEPROJ = $(PROJECT).xcodeproj
HELPFILE = $(PROJECT).maxhelp
TARGETS = 

.PHONY: all generate build clean py pyjs

all: build

generate: clean
	@xcodegen

py: generate
	xcodebuild -project $(XCODEPROJ) -target py

build: generate
	@for t in $(TARGETS) ; do \
		xcodebuild -project $(XCODEPROJ) -target $$t ; \
	done
	
clean:
	@rm -rf $(XCODEPROJ) build
