# 'make'        build executable file
# 'make clean'  removes all .o and executable files
# 'make check'	checks for memory leaks

# define the executable file 
MAIN = prog

INCLUDES = -I<add here>
LFLAGS = -L<add here>
LIBS = -lpq

# directories
SRCDIR = src
OBJDIR = obj

# define the C compiler and flags to use
CC = gcc
CFLAGS = -Wall -g -std=c99


# The following part of the makefile is generic:

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: clean

all: $(MAIN)
	@echo $(MAIN) has been compiled

$(MAIN): $(OBJECTS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJECTS) $(LFLAGS) $(LIBS)
	strip $(MAIN)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@echo "Compiled "$<" successfully!"

clean:
	$(RM) $(OBJDIR)/*.o *~ $(MAIN)

check:
	valgrind --leak-check=yes ./$(MAIN)
