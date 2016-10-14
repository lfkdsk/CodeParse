bison --yacc -dv mycalc.y
flex mycalc.l
gcc -o mycalc y.tab.c lex.yy.c
