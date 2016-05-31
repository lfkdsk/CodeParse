ex_calc: y.tab.o lex.yy.o link_list.o
	$ cc -o ex_calc link_list.c y.tab.c lex.yy.c -ly -ll

link_list.o: link_list.c link_list.h

lex.yy.o: lex.yy.c y.tab.h

lex.yy.o y.tab.o: link_list.h

y.tab.c y.tab.h: ex_calc.y
			$ yacc -d ex_calc.y

lex.yy.c: ex_calc.l
	$ lex ex_calc.l