TARGET = crowbar
CC=gcc
OBJS = \
  lex.yy.o\
  y.tab.o\
  main.o\
  interface.o\
  create.o\
  execute.o\
  eval.o\
  string.o\
  heap.o\
  util.o\
  native.o\
  wchar.o\
  error.o\
  error_message.o\
  ./memory/mem.o\
  ./debug/dbg.o
CFLAGS = -c -g -Wall -Wswitch-enum -ansi -pedantic -DDEBUG -DSHIFT_JIS_SOURCE
INCLUDES = \

$(TARGET):$(OBJS)
	cd ./memory; $(MAKE);
	cd ./debug; $(MAKE);
	$(CC) $(OBJS) -o $@ -lm -lmsvcp60
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
./memory/mem.o:
	cd ./memory; $(MAKE);
./debug/dbg.o:
	cd ./debug; $(MAKE);
############################################################
create.o: create.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
error.o: error.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
error_message.o: error_message.c crowbar.h MEM.h CRB.h CRB_dev.h
eval.o: eval.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
execute.o: execute.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
heap.o: heap.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
interface.o: interface.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
main.o: main.c CRB.h MEM.h
native.o: native.c MEM.h DBG.h CRB_dev.h CRB.h crowbar.h
string.o: string.c MEM.h crowbar.h CRB.h CRB_dev.h
util.o: util.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
wchar.o: wchar.c DBG.h crowbar.h MEM.h CRB.h CRB_dev.h
