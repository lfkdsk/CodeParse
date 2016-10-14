TARGET = diksam
CC=gcc
OBJS = \
  lex.yy.o\
  y.tab.o\
  main.o\
  interface.o\
  create.o\
  fix_tree.o\
  generate.o\
  string.o\
  wchar.o\
  util.o\
  error.o\
  error_message.o\
  ../dvm/dvm.o\
  ../share/share.o\
  ../memory/mem.o\
  ../debug/dbg.o

CFLAGS = -c -g -Wall -Wswitch-enum -ansi -pedantic -DDEBUG -DSHIFT_JIS_SOURCE

INCLUDES = \
  -I/usr/local/include\
  -I../include

$(TARGET):$(OBJS)
	cd ../dvm; $(MAKE);
	cd ../share; $(MAKE);
	cd ../memory; $(MAKE);
	cd ../debug; $(MAKE);
	$(CC) $(OBJS) -o $@ -lm -lmsvcp60

clean:
	rm -f *.o lex.yy.c y.tab.c y.tab.h *~
y.tab.h : diksam.y
	bison --yacc -dv diksam.y
y.tab.c : diksam.y
	bison --yacc -dv diksam.y
lex.yy.c : diksam.l diksam.y y.tab.h
	flex diksam.l
y.tab.o: y.tab.c diksamc.h ../include/MEM.h
	$(CC) -c -g $*.c $(INCLUDES)
lex.yy.o: lex.yy.c diksamc.h ../include/MEM.h
	$(CC) -c -g $*.c $(INCLUDES)
.c.o:
	$(CC) $(CFLAGS) $*.c $(INCLUDES)
../memory/mem.o:
	cd ../memory; $(MAKE);
../debug/dbg.o:
	cd ../debug; $(MAKE);
../share/share.o:
	cd ../share; $(MAKE);
../dvm/dvm.o:
	cd ../dvm; $(MAKE);
############################################################
create.o: create.c ../include/MEM.h ../include/DBG.h diksamc.h \
 ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
 ../include/share.h
error.o: error.c ../include/MEM.h ../include/DBG.h diksamc.h \
 ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
 ../include/share.h
error_message.o: error_message.c diksamc.h ../include/MEM.h \
 ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
 ../include/share.h
fix_tree.o: fix_tree.c ../include/MEM.h ../include/DBG.h diksamc.h \
 ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
 ../include/share.h
generate.o: generate.c ../include/MEM.h ../include/DBG.h diksamc.h \
 ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
 ../include/share.h
interface.o: interface.c ../include/MEM.h ../include/DBG.h diksamc.h \
 ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
 ../include/share.h
main.o: main.c ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
 ../include/MEM.h
string.o: string.c ../include/MEM.h diksamc.h ../include/DKC.h \
 ../include/DVM_code.h ../include/DVM.h ../include/share.h
util.o: util.c ../include/MEM.h ../include/DBG.h diksamc.h \
 ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
 ../include/share.h
wchar.o: wchar.c ../include/DBG.h diksamc.h ../include/MEM.h \
 ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
 ../include/share.h
