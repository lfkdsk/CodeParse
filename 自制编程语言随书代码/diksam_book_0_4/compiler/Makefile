TARGET = compiler.o
CC=gcc
ENCODING_FLAG = -DSHIFT_JIS_SOURCE
CFLAGS = -c -g -Wall -ansi -pedantic -Wswitch-enum -DDEBUG -DDKM_WINDOWS $(ENCODING_FLAG)
OBJS = \
  lex.yy.o\
  y.tab.o\
  interface.o\
  create.o\
  fix_tree.o\
  generate.o\
  string.o\
  wchar.o\
  util.o\
  error.o\
  error_message.o

INCLUDES = \
  -I../include

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)

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
############################################################
create.o: create.c ../include/MEM.h ../include/DBG.h diksamc.h \
  ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
  ../include/DVM_code.h ../include/share.h ../include/DKC.h \
  ../include/DVM_dev.h
diksam.tab.o: diksam.tab.c diksamc.h ../include/MEM.h ../include/DKC.h \
  ../include/DVM_code.h ../include/DVM.h ../include/DVM_code.h \
  ../include/share.h ../include/DKC.h
error.o: error.c ../include/MEM.h ../include/DBG.h diksamc.h \
  ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
  ../include/DVM_code.h ../include/share.h ../include/DKC.h
error_message.o: error_message.c diksamc.h ../include/MEM.h \
  ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
  ../include/DVM_code.h ../include/share.h ../include/DKC.h
fix_tree.o: fix_tree.c ../include/MEM.h ../include/DBG.h diksamc.h \
  ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
  ../include/DVM_code.h ../include/share.h ../include/DKC.h
generate.o: generate.c ../include/MEM.h ../include/DBG.h diksamc.h \
  ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
  ../include/DVM_code.h ../include/share.h ../include/DKC.h
interface.o: interface.c ../include/MEM.h ../include/DBG.h diksamc.h \
  ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
  ../include/DVM_code.h ../include/share.h ../include/DKC.h
string.o: string.c ../include/MEM.h diksamc.h ../include/DKC.h \
  ../include/DVM_code.h ../include/DVM.h ../include/DVM_code.h \
  ../include/share.h ../include/DKC.h
util.o: util.c ../include/MEM.h ../include/DBG.h diksamc.h \
  ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
  ../include/DVM_code.h ../include/share.h ../include/DKC.h
wchar.o: wchar.c ../include/DBG.h diksamc.h ../include/MEM.h \
  ../include/DKC.h ../include/DVM_code.h ../include/DVM.h \
  ../include/DVM_code.h ../include/share.h ../include/DKC.h
