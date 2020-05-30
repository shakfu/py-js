
CC = gcc
CFLAGS = -Wall -g -std=c99
INCLUDES=`python3.7-config --cflags`
LDFLAGS=`python3.7-config --ldflags`


READLINE_INCLUDES=-I/usr/local/Cellar/readline/8.0.4/include \
				  -I/usr/local/Cellar/readline/8.0.4/include/readline
READLINE_LDFLAGS=-L/usr/local/Cellar/readline/8.0.4/lib -lreadline

TARGETS=test_interactive test_py


.PHONY: clean

all: 


# BUILDING
# -----------------------------------------------------------------------
.PHONY: build test_py test_interactive

build: test_py test_interactive
	$(call section,"build tests")

test_py:
	$(call section,"build test_py")
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(LDFLAGS) $@.c


test_interactive:
	$(call section,"build test_interactive")
	$(CC) $(CFLAGS) $(INCLUDES) $(READLINE_INCLUDES) -o $@ $(LDFLAGS) $(READLINE_LDFLAGS) $@.c



# TESTING
# -----------------------------------------------------------------------
.PHONY: test

test:
	$(call section,"testing planned")
	echo "remember to test!"


# CHECK
# -----------------------------------------------------------------------
.PHONY: memcheck

memcheck: test_py
	$(call section,"checking for memory leaks")
	valgrind --tool=memcheck \
			 --dsymutil=yes \
			 --track-origins=yes \
			 --show-leak-kinds=all \
			 --trace-children=yes \
			 --suppressions=$HOME/.valgrind-python.supp \
	 		 --leak-check=full $<
			 # --gen-suppressions=yes \


# STYLING
# -----------------------------------------------------------------------
.PHONY: style clang-format

style: clang-format

clang-format:
	$(call section,"clang-format")
	@clang-format -i -style=../.clang-format *.c


# CLEANING
# -----------------------------------------------------------------------
.PHONY: clean

clean: clean-build
	$(call section,"cleaning DONE")


clean-build:
	$(call section,"cleaning build artifacts")
	@rm -rf *.dSYM
	@for f in $(TARGETS) ; do \
		rm -rf $$f ; \
	done


