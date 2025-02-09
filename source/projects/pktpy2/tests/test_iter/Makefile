PKTPY2 = ../..

C_DEFS = -DPK_ENABLE_OS=1 -Wno-shorten-64-to-32 

.PHONY: all build clean

all: build

build:
	@clang -std=c11 -o test_iter test_iter.c $(PKTPY2)/pocketpy.c -I$(PKTPY2) $(C_DEFS)

clean:
	@rm -f ./test_iter
