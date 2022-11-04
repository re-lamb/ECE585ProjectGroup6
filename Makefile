CC = gcc
OBJS = llcmain.c

#INCLUDE_PATHS = -IC:\
#LIBRARY_PATHS = -LC:\
INCLUDE_PATHS =
LIBRARY_PATHS =

CFLAGS = -Wall
LDFLAGS = 

OBJ_NAME = llc

all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(CFLAGS) $(LDFLAGS) -o $(OBJ_NAME)
	
clean:
	rm -f $(OBJ_NAME) $(OBJ_NAME).exe