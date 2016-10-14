TARGET = mem.o
CC=gcc
CFLAGS = -c -g -DDEBUG -Wall
OBJS = memory.o storage.o

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)
testp : $(OBJS) main.o
	$(CC) -o $@ $(OBJS) main.o
.c.o:
	$(CC) $(CFLAGS) -I../include $*.c
main.o: main.c ../include/MEM.h
memory.o: memory.c memory.h ../include/MEM.h
storage.o: storage.c memory.h ../include/MEM.h
