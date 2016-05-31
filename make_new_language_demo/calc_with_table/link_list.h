#ifndef SYMBOL
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
struct symtable {
  /* data */
    char *name;
    double value;
    double (*funcptr)();
    struct symtable *next;
};

struct symtable* addNode(char *symbol);
void printLink();
void addFunction(char *name, double func());
#endif
