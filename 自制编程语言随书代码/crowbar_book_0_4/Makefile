TARGET = crowbar
CC=gcc
MINICROWBAR = minicrowbar
OBJS = \
  lex.yy.o\
  y.tab.o\
  main.o\
  create.o\
  execute.o\
  eval.o\
  string.o\
  heap.o\
  util.o\
  native.o\
  nativeif.o\
  wchar.o\
  regexp.o\
  error.o\
  error_message.o\
  ./memory/mem.o\
  ./debug/dbg.o

FINALOBJS = $(OBJS) interface.o builtin.o
MINIOBJS = $(OBJS) miniinterface.o
BUILTINS = \
  builtin.crb

CFLAGS = -c -g -Wall -Wswitch-enum -ansi -pedantic -DDEBUG -DSHIFT_JIS_SOURCE

INCLUDES = \
  -I/usr/local/include

$(TARGET):$(MINICROWBAR) $(FINALOBJS)
	cd ./memory; $(MAKE);
	cd ./debug; $(MAKE);
	$(CC) $(FINALOBJS) -o $@ -lm -lmsvcp60 -lonig

$(MINICROWBAR):$(MINIOBJS)
	$(CC) $(MINIOBJS) -o $@ -lm -lmsvcp60 -lonig

clean:
	rm -f *.o lex.yy.c y.tab.c y.tab.h *~
y.tab.h : crowbar.y
	bison --yacc -dv crowbar.y
y.tab.c : crowbar.y
	bison --yacc -dv crowbar.y
lex.yy.c : crowbar.l crowbar.y y.tab.h
	flex crowbar.l
y.tab.o: y.tab.c crowbar.h MEM.h
	$(CC) -c -g $*.c $(INCLUDES)
lex.yy.o: lex.yy.c crowbar.h MEM.h
	$(CC) -c -g $*.c $(INCLUDES)
.c.o:
	$(CC) $(CFLAGS) $*.c $(INCLUDES)
miniinterface.o: interface.c
	$(CC) $(CFLAGS) -DMINICROWBAR -o $@ interface.c $(INCLUDES)
interface.o:
	$(CC) $(CFLAGS) -o $@ interface.c $(INCLUDES)
builtin.c: ./builtin/builtin.crb
	cd ./builtin; ../$(MINICROWBAR) conv.crb $(BUILTINS)

./memory/mem.o:
	cd ./memory; $(MAKE);
./debug/dbg.o:
	cd ./debug; $(MAKE);
############################################################
builtin.o: builtin.c CRB.h
create.o: create.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
error.o: error.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
error_message.o: error_message.c crowbar.h MEM.h CRB.h CRB_dev.h
eval.o: eval.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
execute.o: execute.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
heap.o: heap.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
interface.o: interface.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
main.o: main.c CRB.h MEM.h
native.o: native.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
nativeif.o: nativeif.c DBG.h crowbar.h MEM.h CRB.h CRB_dev.h
regexp.o: regexp.c DBG.h crowbar.h MEM.h CRB.h CRB_dev.h
string.o: string.c MEM.h crowbar.h CRB.h CRB_dev.h
util.o: util.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
wchar.o: wchar.c DBG.h crowbar.h MEM.h CRB.h CRB_dev.h
