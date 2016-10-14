TARGET = dvm.o
CC=gcc
CFLAGS = -c -g -Wall -Wswitch-enum -ansi -pedantic -DDEBUG
OBJS = execute.o \
  heap.o \
  native.o \
  nativeif.o \
  wchar.o \
  util.o \
  error.o \
  error_message.o

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -I../include $*.c
error.o: error.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
 ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
 ../include/share.h
error_message.o: error_message.c dvm_pri.h ../include/DVM_code.h \
 ../include/DVM.h ../include/DVM_dev.h ../include/share.h
execute.o: execute.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
 ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
 ../include/share.h
heap.o: heap.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
 ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
 ../include/share.h
native.o: native.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
 ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
 ../include/share.h
nativeif.o: nativeif.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
 ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
 ../include/share.h
util.o: util.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
 ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
 ../include/share.h
wchar.o: wchar.c ../include/DBG.h ../include/MEM.h dvm_pri.h \
 ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
 ../include/share.h
