TARGET = dbg.o
CC=gcc
CFLAGS = -c -g -Wall -DDBG_NO_DEBUG
OBJS = debug.o
INCLUDES = -I..

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $*.c
debug.o: debug.c ../MEM.h debug.h ../DBG.h
