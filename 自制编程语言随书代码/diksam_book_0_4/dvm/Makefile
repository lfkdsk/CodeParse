TARGET = dvm.o
CC=gcc
CFLAGS = -c -g -DDEBUG -DDKM_WINDOWS -Wall -ansi -pedantic -Wswitch-enum
OBJS = execute.o \
  load.o \
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
  ../include/share.h ../include/DVM_code.h
error_message.o: error_message.c dvm_pri.h ../include/DVM_code.h \
  ../include/DVM.h ../include/DVM_dev.h ../include/share.h \
  ../include/DVM_code.h
execute.o: execute.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
  ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
  ../include/share.h ../include/DVM_code.h
heap.o: heap.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
  ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
  ../include/share.h ../include/DVM_code.h
load.o: load.c ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
  ../include/MEM.h ../include/DBG.h dvm_pri.h ../include/DVM_code.h \
  ../include/DVM_dev.h ../include/share.h
native.o: native.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
  ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
  ../include/share.h ../include/DVM_code.h
nativeif.o: nativeif.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
  ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
  ../include/share.h ../include/DVM_code.h
util.o: util.c ../include/MEM.h ../include/DBG.h dvm_pri.h \
  ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
  ../include/share.h ../include/DVM_code.h
wchar.o: wchar.c ../include/DBG.h ../include/MEM.h dvm_pri.h \
  ../include/DVM_code.h ../include/DVM.h ../include/DVM_dev.h \
  ../include/share.h ../include/DVM_code.h
