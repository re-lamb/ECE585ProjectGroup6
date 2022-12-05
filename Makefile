CC = gcc
DEFS = defs.h
SRCS = llcmain.c cacheops.c

INCLUDE_PATHS =
LIBRARY_PATHS =

CFLAGS = -Wall
LDFLAGS = 

OBJ_NAME = llc

all: $(DEFS) $(SRCS)
	$(CC) $(SRCS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(CFLAGS) $(LDFLAGS) -o $(OBJ_NAME)

test: GenTest.c	
	$(CC) GenTest.c $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(CFLAGS) $(LDFLAGS) -o gentest

clean:
	rm -f $(OBJ_NAME) $(OBJ_NAME).exe gentest gentest.exe
