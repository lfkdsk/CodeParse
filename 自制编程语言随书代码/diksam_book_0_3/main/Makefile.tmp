TARGET = diksam
OBJS = \
  ../compiler/compiler.o\
  ../dvm/dvm.o\
  ../share/share.o\
  ../memory/mem.o\
  ../debug/dbg.o\
  ./main.o

CFLAGS = -c -g -Wall -ansi -Wswitch-enum -DDEBUG

INCLUDES = \
  -I/usr/local/include\
  -I../include

$(TARGET):$(OBJS)
	cd ../compiler; $(MAKE)
	cd ../dvm; $(MAKE);
	cd ../share; $(MAKE);
	cd ../memory; $(MAKE);
	cd ../debug; $(MAKE);
	$(CC) $(OBJS) -o $@ -lm

clean:
	rm -f *.o *~
.c.o:
	$(CC) $(CFLAGS) $*.c $(INCLUDES)
../memory/mem.o:
	cd ../memory; $(MAKE);
../debug/dbg.o:
	cd ../debug; $(MAKE);
../share/share.o:
	cd ../share; $(MAKE);
../compiler/compiler.o:
	cd ../compiler; $(MAKE);
../dvm/dvm.o:
	cd ../dvm; $(MAKE);
############################################################
main.o: main.c ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
  ../include/DVM.h ../include/MEM.h
