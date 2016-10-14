TARGET = builtin.o
CC=gcc
CFLAGS = -g -DDEBUG -Wall -ansi -pedantic -Wswitch-enum

$(TARGET):builtin.c
	$(CC) -c $(CFLAGS) -I../../include $*.c

BUILTIN_SCRIPT = \
  ../../require/diksam/lang.dkh\
  ../../require/diksam/lang.dkm\
  ../../require/diksam/math.dkh

builtin.c:builtin_conv head.txt tail.txt $(BUILTIN_SCRIPT)
	./builtin_conv

CONV_OBJS =\
  ../../memory/memory.o\
  ../../debug/debug.o

builtin_conv:builtin_conv.c ../../include/DVM.h ../diksamc.h
	$(CC) $(CFLAGS) -o $@ -I../../include builtin_conv.c $(CONV_OBJS)
############################################################
builtin.o: builtin.c ../diksamc.h ../../include/MEM.h ../../include/DKC.h \
  ../../include/DVM_code.h ../../include/DVM.h ../../include/DVM_code.h \
  ../../include/share.h ../../include/DKC.h
builtin_conv.o: builtin_conv.c ../../include/DVM.h ../../include/DBG.h
