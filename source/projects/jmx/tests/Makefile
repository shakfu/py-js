CC = clang
EXECUTABLES = $(patsubst %.c, %, $(wildcard *.c))
PYTHON_VERSION = $(shell python3 --version | sed s/Python[[:space:]]//) # 3.9.5
PY_MAJOR = $(shell echo $(PYTHON_VERSION) | cut -f1 -d'.')
PY_MINOR = $(shell echo $(PYTHON_VERSION) | cut -f2 -d'.')
BREW_PREFIX = $(shell brew --prefix)
PYTHON_VER = $(PY_MAJOR).$(PY_MINOR) # 3.9
CFLAGS = $(shell python3-config --cflags) -I$(BREW_PREFIX)/include -L$(BREW_PREFIX)/lib
LDFLAGS = $(shell python3-config --ldflags) -lpython$(PYTHON_VER) -lczmq -lzmq

.PHONY: clean analyze

all: $(EXECUTABLES)
	@echo compile zeromq python tests..

%: %.c
	@$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) 

analyze:
	@infer run -- make

clean:
	@rm -f $(EXECUTABLES)
	@rm -rf *.dSYM
	@rm -rf infer-out
